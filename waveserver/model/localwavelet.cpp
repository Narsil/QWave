#include "localwavelet.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "model/wave.h"
#include "model/wavefolk.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"
#include "app/settings.h"
#include <QDateTime>

LocalWavelet::LocalWavelet(Wave* wave, const QString& waveletDomain, const QString& waveletId)
        : Wavelet( wave, waveletDomain, waveletId )
{
    criticalSection()->disable();
    new InitActor(this);
}

void LocalWavelet::onAddParticipant( const JID& jid )
{
    // Is this a remote user?
    if ( !jid.isLocal() )
        subscribeRemote( jid );
    else
        notifyAllClients( jid.toString() );
}

void LocalWavelet::onRemoveParticipant( const JID& jid )
{
    // Is it a remote user?
    if ( !jid.isLocal() )
        unsubscribeRemote( jid );
    else
        unsubscribeAllClients( jid.toString() );
}

void LocalWavelet::subscribeRemote( const JID& remoteJid )
{
    if ( !m_remoteSubscribers.contains( remoteJid.domain() ) )
        m_remoteSubscribers[ remoteJid.domain() ] = 1;
    else
        m_remoteSubscribers[ remoteJid.domain() ] = m_remoteSubscribers[ remoteJid.domain() ] + 1;
}

void LocalWavelet::unsubscribeRemote( const JID& remoteJid )
{
    if ( m_remoteSubscribers.contains( remoteJid.domain() ) )
    {
        int count = m_remoteSubscribers[ remoteJid.domain() ];
        if ( count == 1 )
            m_remoteSubscribers.remove( remoteJid.domain() );
        else
            m_remoteSubscribers[ remoteJid.domain() ] = m_remoteSubscribers[ remoteJid.domain() ] - 1;
    }
}

bool LocalWavelet::isRemote() const
{
    return false;
}

bool LocalWavelet::isLocal() const
{
    return true;
}

void LocalWavelet::customEvent( QEvent* event )
{
    PBMessage<messages::LocalSubmitRequest>* submitMsg = dynamic_cast< PBMessage<messages::LocalSubmitRequest>* >( event );
    if ( submitMsg )
    {
        new SubmitRequestActor( this, submitMsg );
        return;
    }

    this->Wavelet::customEvent( event );
}

/****************************************************************************
 *
 * SubmitRequestActor
 *
 ***************************************************************************/

#undef ERROR
#define ERROR(msg) { logErr(msg, __FILE__, __LINE__); sendFailedSubmitResponse(msg); wavelet()->criticalSection()->leave(this); TERMINATE(); }
#undef LOG
#define LOG(msg) { log(msg, __FILE__, __LINE__); }

LocalWavelet::SubmitRequestActor::SubmitRequestActor( LocalWavelet* wavelet, PBMessage<messages::LocalSubmitRequest>* message )
    : WaveletActor( wavelet ), m_message( *message )
{
}

void LocalWavelet::SubmitRequestActor::execute()
{
    qDebug("EXECUTE LocalWavelet::SubmitRequestActor");

    BEGIN_EXECUTE;

    if ( !wavelet()->criticalSection()->tryEnter(this) )
        yield( RecvCriticalSection( wavelet()->criticalSection() ) );

    {
        QString err;
        m_appliedDelta = wavelet()->process( &m_message.signed_delta(), 0, &m_addLocalUser, &m_removeLocalUser, &err );
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

    // Send information back to the submitting actor
    if ( !m_message.sender().isNull() )
    {
        LOG("Sending response to caller");
        PBMessage<messages::SubmitResponse>* response = new PBMessage<messages::SubmitResponse>( m_message.sender(), m_message.id() );
        response->set_operations_applied( m_appliedDelta.operationsApplied() );
        response->set_application_timestamp( m_appliedDelta.applicationTime() );
        response->mutable_hashed_version_after_application()->set_history_hash( m_appliedDelta.resultingVersion().hash.constData(), m_appliedDelta.resultingVersion().hash.length() );
        response->mutable_hashed_version_after_application()->set_version( m_appliedDelta.resultingVersion().version );
        bool ok = post( response );
        if ( !ok ) { LOG("Cout not send response to caller."); }
    }

    // Send the delta to all remote subscribers (if XMPP is enabled)
    {
        if ( Settings::settings()->federationEnabled() )
        {
            foreach( QString rid, wavelet()->m_remoteSubscribers.keys() )
            {
                PBMessage<messages::WaveletUpdate>* update = new PBMessage<messages::WaveletUpdate>( ActorId("federation", rid ) );
                update->set_wavelet_name( wavelet()->url().toString().toStdString() );
                update->set_applied_delta( m_binary.data(), m_binary.length() );
                update->setCreateOnDemand( true );
                bool ok = post( update );
                if ( !ok ) { LOG("Could not send update to " + rid ); }
            }
        }
    }

    LOG("Submit request completed");

    END_EXECUTE;
}

void LocalWavelet::SubmitRequestActor::sendFailedSubmitResponse(const QString& error)
{
    // Send information back
    if ( !m_message.sender().isNull() )
    {
        PBMessage<messages::SubmitResponse>* response = new PBMessage<messages::SubmitResponse>( m_message.sender(), m_message.id() );
        response->set_operations_applied( 0 );
        response->set_application_timestamp( timeStamp() );
        response->set_error_message( error.toStdString() );
        post( response );
    }
}

qint64 LocalWavelet::SubmitRequestActor::timeStamp()
{
    return (qint64)(QDateTime::currentDateTime().toTime_t()) * 1000;
}
