#include "localwavelet.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "model/participant.h"
#include "model/wave.h"
#include "model/wavefolk.h"
#include "network/xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"

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


///****************************************************************************
// *
// * InitActor
// *
// ***************************************************************************/
//
//// TODO: Better error handler. If InitActor fails, the wavelet remains locked because the critical section is disabled
//#define ERROR(msg) { logErr(msg, __FILE__, __LINE__); TERMINATE(); }
//#define LOG(msg) { log(msg, __FILE__, __LINE__); }
//
//LocalWavelet::InitActor::InitActor( LocalWavelet* wavelet )
//    : WaveletActor( wavelet )
//{
//}
//
//void LocalWavelet::InitActor::execute()
//{
//    qDebug("EXECUTE LocalWavelet::InitActor");
//
//    BEGIN_EXECUTE;
//
//    // Send a query to the database
//    {
//        m_msgId = nextId();
//        PBMessage<messages::QueryWaveletUpdates>* query = new PBMessage<messages::QueryWaveletUpdates>( ActorId("store", wavelet()->url().toString() ), m_msgId );
//        query->setCreateOnDemand( true );
//        query->set_wavelet_name( wavelet()->url().toString().toStdString() );
//        query->set_start_version( 0 );
//        query->set_end_version( 0xFFFFFFFF ); // TODO: Use max qint64 here
//        bool ok = post( query );
//        if ( !ok ) { ERROR("Internal server error. Could not talk to database."); }
//    }
//
//    // Wait for a response from the database
//    yield( RecvPB<messages::QueryWaveletUpdatesResponse>(m_msgId) | Timeout(10000) );
//    if ( REASON(RecvPB<messages::QueryWaveletUpdatesResponse>) )
//    {
//        if ( !REASON->ok() ) { ERROR("Data base reported an error:" + QString::fromStdString( REASON->error() )); }
//        for( int i = 0; i < REASON->applied_delta_size(); ++i )
//        {
//            protocol::ProtocolAppliedWaveletDelta protobuf;
//            if ( !protobuf.ParseFromArray( REASON->applied_delta(i).data(), REASON->applied_delta(i).length() ) ) { ERROR("Database delivered corrupted data"); }
//            QString err;
//            AppliedWaveletDelta delta = wavelet()->process( &protobuf.signed_original_delta(), &protobuf, 0, 0, &err );
//            if ( delta.isNull() ) ERROR("Database delivered corrupted data: " + err);
//            if ( !wavelet()->apply( delta, &err ) ) ERROR("Could not apply delta: " + err);
//        }
//    }
//    else { ERROR("Timeout waiting for database"); }
//
//    wavelet()->criticalSection()->enable();
//
//    END_EXECUTE;
//}

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

//    // Decode the delta, check it, and transform it (if required)
//    {
//        bool ok;
//        m_signedDelta = SignedWaveletDelta( &m_message.signed_delta(), &ok );
//        if ( !ok ) ERROR("Could not decode the signed delta");
//
//        // Make a copy of the delta because we might have to transform it
//        m_delta = WaveletDelta( m_signedDelta.delta() );
//
//        QString errorMessage;
//        // Check its applicability
//        if ( !wavelet()->checkHashedVersion( m_delta, &errorMessage ) )
//            ERROR( errorMessage );
//
//        // Transform if required
//        m_transformed = wavelet()->transform( m_delta, &errorMessage, &ok );
//        if ( !ok )
//            ERROR( errorMessage );
//    }
//
//    // Apply all operations contained in the delta
//    {
//        // TODO: Rollback if something went wrong, or report that only a subset of ops succeeded
//
//        for( QList<WaveletDeltaOperation>::const_iterator it = m_delta.operations().begin(); it != m_delta.operations().end(); it++ )
//        {
//            QString docId = (*it).documentId();
//            WaveletDocument* doc = wavelet()->m_documents[docId];
//            if ( !doc )
//            {
//                doc = new WaveletDocument(wavelet(), docId);
//                wavelet()->m_documents[docId] = doc;
//            }
//
//            if ( (*it).hasMutation() )
//            {
//                bool check = doc->apply( (*it).mutation(), m_delta.author() );
//                if ( !check ) { ERROR( "Failed to apply delta to " + docId); }
//                // Remember that the digest sendFailedSubmitResponse(msg);will need an update
//            }
//            if ( (*it).hasAddParticipant() )
//            {
//                QString p = (*it).addParticipant();
//                JID jid(p);
//                if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
//                if ( !wavelet()->m_participants.contains( p ) )
//                {
//                    wavelet()->m_participants.insert( p );
//                    // Is this a remote user?
//                    if ( !jid.isLocal() )
//                        wavelet()->subscribeRemote( jid );
//                    else
//                    {
//                        m_addLocalUser.insert( p );
//                        wavelet()->notifyAllClients( p );
//                    }
//                }
//            }
//            if ( (*it).hasRemoveParticipant() )
//            {
//                QString p = (*it).removeParticipant();
//                JID jid(p);
//                if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
//                if ( wavelet()->m_participants.contains( p ) )
//                {
//                    wavelet()->m_participants.remove( p );
//
//                    // Is it a remote user?
//                    if ( !jid.isLocal() )
//                        wavelet()->unsubscribeRemote( jid );
//                    else
//                    {
//                        m_removeLocalUser.insert( p );
//                        wavelet()->unsubscribeAllClients( p );
//                    }
//                }
//            }
//        }
//    }
//
//    // Compute the resulting version and hash
//    {
//        m_operationsApplied = m_delta.operations().count();
//        m_applicationTime = timeStamp();
//
//        // Construct a AppliedWaveletDelta
//        AppliedWaveletDelta appliedDelta( m_signedDelta, m_applicationTime, m_operationsApplied );
//        if ( m_transformed )
//            appliedDelta.setTransformedDelta( m_delta );
//
//        m_resultingHash = appliedDelta.resultingVersion().hash;
//        m_resultingVersion = appliedDelta.resultingVersion().version;
//
//        // Serialize
//        m_binary = appliedDelta.toBinary();
//
//        // Send the delta to all local subscribers
//        wavelet()->commit( appliedDelta, false );
//        qDebug("Local commit done");
//    }

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
        XmppComponentConnection* comcon = XmppComponentConnection::connection();
        if ( comcon )
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
