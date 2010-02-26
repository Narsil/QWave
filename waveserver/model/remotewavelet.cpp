#include "remotewavelet.h"
#include "model/wave.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"

RemoteWavelet::RemoteWavelet(Wave* wave, const QString& waveletDomain, const QString& waveletId)
        : Wavelet( wave, waveletDomain, waveletId )
{
    criticalSection()->disable();
    new InitActor(this);
}

void RemoteWavelet::onAddParticipant( const JID& jid )
{
    if ( jid.isLocal() )
        notifyAllClients( jid.toString() );
}

void RemoteWavelet::onRemoveParticipant( const JID& jid )
{
    if ( jid.isLocal() )
        unsubscribeAllClients( jid.toString() );
}

bool RemoteWavelet::isRemote() const
{
    return true;
}

bool RemoteWavelet::isLocal() const
{
    return false;
}

void RemoteWavelet::customEvent( QEvent* event )
{
    PBMessage<messages::RemoteWaveletUpdate>* update = dynamic_cast< PBMessage<messages::RemoteWaveletUpdate>* >( event );
    if ( update )
    {
        new WaveletUpdateActor( this, update );
        return;
    }

    this->Wavelet::customEvent( event );
}

/****************************************************************************
 *
 * WaveletUpdateActor
 *
 ***************************************************************************/

#undef ERROR
#define ERROR(msg) { logErr(msg, __FILE__, __LINE__); wavelet()->criticalSection()->leave(this); TERMINATE(); }
#undef LOG
#define LOG(msg) { log(msg, __FILE__, __LINE__); }

RemoteWavelet::WaveletUpdateActor::WaveletUpdateActor( RemoteWavelet* wavelet, PBMessage<messages::RemoteWaveletUpdate>* message )
    : WaveletActor( wavelet ), m_message( *message )
{
}

void RemoteWavelet::WaveletUpdateActor::execute()
{
    qDebug("EXECUTE RemoteWavelet::WaveletUpdateActor");

    BEGIN_EXECUTE;

    if ( !wavelet()->criticalSection()->tryEnter(this) )
        yield( RecvCriticalSection( wavelet()->criticalSection() ) );

    {
        QString err;
        m_appliedDelta = wavelet()->process( &m_message.applied_delta().signed_original_delta(), &m_message.applied_delta(), &m_addLocalUser, &m_removeLocalUser, &err );
        if ( m_appliedDelta.isNull() ) { ERROR("Could not process delta: " + err); }
        m_binary = m_appliedDelta.toBinary();
    }

    // Commit data to storage
    // Send data to the database
    {
        m_msgId = nextId();
        PBMessage<messages::PersistWaveletUpdate>* persist = new PBMessage<messages::PersistWaveletUpdate>( ActorId("store", wavelet()->url().toString() ), m_msgId );
        persist->setCreateOnDemand( true );
        persist->set_wavelet_name( wavelet()->url().toString().toStdString() );
        persist->set_applied_at_version( wavelet()->version() );
        persist->set_applies_to_version( m_appliedDelta.signedDelta().delta().version().version );
        persist->set_operations_applied( m_appliedDelta.operationsApplied() );
        persist->set_applied_delta( m_binary.data(), m_binary.length() );
        foreach( QString p, m_addLocalUser )
        {
            persist->add_add_user( p.toStdString() );
        }
        foreach( QString p, m_removeLocalUser )
        {
            persist->add_remove_user( p.toStdString() );
        }
        bool ok = post( persist );
        if ( !ok ) { ERROR("Internal server error. Could not talk to database."); }
    }

    // Wait for a response from the database
    yield( RecvPB<messages::PersistAck>(m_msgId) | Timeout(10000) );
    if ( REASON(RecvPB<messages::PersistAck>) )
    {
        if ( !REASON->ok() ) { ERROR("Data base did not store the value"); }
    }
    else { ERROR("Timeout waiting for database"); }

    // Apply the delta locally in RAM
    {
        QString err;
        if ( !wavelet()->apply( m_appliedDelta, &err ) ) ERROR("Could not apply delta: " + err );
    }

    // Allow other actors to modify the wavelet
    wavelet()->criticalSection()->leave(this);

    LOG("Submit request completed");

    END_EXECUTE;
}
