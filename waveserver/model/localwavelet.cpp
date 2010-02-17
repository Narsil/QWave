#include "localwavelet.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "model/participant.h"
#include "model/wave.h"
#include "model/wavefolk.h"
#include "network/xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"

#include <QDateTime>

LocalWavelet::LocalWavelet(Wave* wave, const QString& waveletDomain, const QString& waveletId)
        : Wavelet( wave, waveletDomain, waveletId )
{
}

int LocalWavelet::apply( const protocol::ProtocolSignedDelta& protobufDelta, QString* errorMessage, int operationsApplied, qint64 applicationTime )
{
    bool ok;
    SignedWaveletDelta delta( &protobufDelta, &ok );
    if ( !ok )
    {
        errorMessage->append("Error decoding signedDelta");
        return -1;
    }
    return apply( delta, errorMessage, operationsApplied, applicationTime );
}

int LocalWavelet::apply( const SignedWaveletDelta& signedDelta, QString* errorMessage, int operationsApplied, qint64 applicationTime )
{
    // Make a copy of the delta because we might have to transform it
    WaveletDelta clientDelta( signedDelta.delta() );

    // Check its applicability
    if ( !checkHashedVersion( clientDelta, errorMessage ) )
        return -1;

    // Transform if required
    bool ok;
    bool transformed = transform( clientDelta, errorMessage, &ok );
    if ( !ok )
        return -1;

    // TODO: Rollback if something went wrong, or report that only a subset of ops succeeded

    for( QList<WaveletDeltaOperation>::const_iterator it = clientDelta.operations().begin(); it != clientDelta.operations().end(); it++ )
    {
        QString docId = (*it).documentId();
        WaveletDocument* doc = m_documents[docId];
        if ( !doc )
        {
            doc = new WaveletDocument();
            m_documents[docId] = doc;
        }

        if ( (*it).hasMutation() )
        {
            bool check = doc->apply( (*it).mutation(), clientDelta.author() );
            if ( !check )
            {
                // TODO: rollback
                errorMessage->append("Failed to apply delta to " + docId);
                return -1;
            }
            // Remember that the digest will need an update
        }
        if ( (*it).hasAddParticipant() )
        {
            QString p = (*it).addParticipant();
            JID jid(p);
            if ( !jid.isValid() )
            {
                errorMessage->append("Invalid JID " + p );
                return -1;
            }
            if ( !m_participants.contains( p ) )
            {
                m_participants.insert( p );
                // Is this a remote user?
                if ( !jid.isLocal() )
                    subscribeRemote( jid );
                else
                {
                    // Add the wavelet to the participant (and  make sure that such a participant exists.
                    // TODO: Error if we know that this participant is not known?
                    Participant::participant( p, true )->addWavelet(this);
                }
            }
        }
        if ( (*it).hasRemoveParticipant() )
        {
            QString p = (*it).removeParticipant();
            JID jid(p);
            if ( !jid.isValid() )
            {
                errorMessage->append("Invalid JID " + p );
                return -1;
            }
            if ( m_participants.contains( p ) )
            {
                m_participants.remove( p );

                // Is it a remote user?
                if ( !jid.isLocal() )
                    unsubscribeRemote( jid );
                else
                {
                    // Remove the wavelet from the participant
                    Participant* pptr = Participant::participant( p, false );
                    if ( pptr )
                        pptr->removeWavelet(this);
                }
            }
        }
    }

    bool restore = ( operationsApplied != -1 );
    if ( operationsApplied == -1 )
        operationsApplied = clientDelta.operations().count();
    else if ( operationsApplied != clientDelta.operations().count() )
    {
        errorMessage->append("Number of operations applied is different than expected.");
        return -1;
    }
    if ( applicationTime == -1 )
        // Time since 1.1.1970 in milliseconds
        applicationTime = (qint64)(QDateTime::currentDateTime().toTime_t()) * 1000;

    // Construct a AppliedWaveletDelta and sign it (if required)
    AppliedWaveletDelta appliedDelta( signedDelta, applicationTime, operationsApplied );
    if ( transformed )
        appliedDelta.setTransformedDelta( clientDelta );

    // Send the delta to all local subscribers
    commit( appliedDelta, restore );

    // Send the delta to all remote subscribers (if XMPP is enabled)
    XmppComponentConnection* comcon = XmppComponentConnection::connection();
    if ( comcon )
    {
        foreach( QString rid, m_remoteSubscribers.keys() )
        {
            XmppVirtualConnection* con = comcon->virtualConnection( rid );
            if ( !con )
                continue;
            con->sendWaveletUpdate( url().toString(), appliedDelta );
        }
    }

    return version();
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

void LocalWavelet::dispatch( const QSharedPointer<IMessage>& message )
{
    PBMessage<messages::LocalSubmitRequest>* submitMsg = dynamic_cast< PBMessage<messages::LocalSubmitRequest>* >( message.data() );
    if ( submitMsg )
    {
        new SubmitRequestActor( this, message.dynamicCast<PBMessage<messages::LocalSubmitRequest> >() );
        return;
    }

    this->Wavelet::dispatch( message );
}

/****************************************************************************
 *
 * WaveletActor
 *
 ***************************************************************************/

LocalWavelet::WaveletActor::WaveletActor( LocalWavelet* wavelet ) : m_wavelet( wavelet ), m_actorId( WaveFolk::actorId( m_wavelet->url() ) )
{
}

void LocalWavelet::WaveletActor::log( const char* error, const char* file, int line )
{
    QString d = m_wavelet->url().toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("INFO in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void LocalWavelet::WaveletActor::log( const QString& error, const char* file, int line )
{
    log( error.toAscii().constData(), file, line );
}

void LocalWavelet::WaveletActor::logErr( const char* error, const char* file, int line )
{
    QString d = m_wavelet->url().toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("ERROR in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void LocalWavelet::WaveletActor::logErr( const QString& error, const char* file, int line )
{
    logErr( error.toAscii().constData(), file, line );
}

/****************************************************************************
 *
 * SubmitRequestActor
 *
 ***************************************************************************/

#define ERROR(msg) { logErr(msg, __FILE__, __LINE__); sendFailedSubmitResponse(msg); TERMINATE(); }
#define LOG(msg) { log(msg, __FILE__, __LINE__); }

void LocalWavelet::SubmitRequestActor::EXECUTE()
{
    qDebug("EXECUTE WaveletSubmitRequestActor");

    BEGIN_EXECUTE;

    // Decode the delta
    {
        bool ok;
        m_signedDelta = SignedWaveletDelta( &m_message->signed_delta(), &ok );
        if ( !ok ) ERROR("Could not decode the signed delta");

        // Make a copy of the delta because we might have to transform it
        WaveletDelta clientDelta( m_signedDelta.delta() );

        QString errorMessage;
        // Check its applicability
        if ( !m_wavelet->checkHashedVersion( clientDelta, &errorMessage ) )
            ERROR( errorMessage );

        // Transform if required
        bool transformed = m_wavelet->transform( clientDelta, &errorMessage, &ok );
        if ( !ok )
            ERROR( errorMessage );

        // TODO: Rollback if something went wrong, or report that only a subset of ops succeeded

        for( QList<WaveletDeltaOperation>::const_iterator it = clientDelta.operations().begin(); it != clientDelta.operations().end(); it++ )
        {
            QString docId = (*it).documentId();
            WaveletDocument* doc = m_wavelet->m_documents[docId];
            if ( !doc )
            {
                doc = new WaveletDocument();
                m_wavelet->m_documents[docId] = doc;
            }

            if ( (*it).hasMutation() )
            {
                bool check = doc->apply( (*it).mutation(), clientDelta.author() );
                if ( !check ) { ERROR( "Failed to apply delta to " + docId); }
                // Remember that the digest will need an update
            }
            if ( (*it).hasAddParticipant() )
            {
                QString p = (*it).addParticipant();
                JID jid(p);
                if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
                if ( !m_wavelet->m_participants.contains( p ) )
                {
                    m_wavelet->m_participants.insert( p );
                    // Is this a remote user?
                    if ( !jid.isLocal() )
                        m_wavelet->subscribeRemote( jid );
                    else
                    {
                        // Add the wavelet to the participant (and  make sure that such a participant exists.
                        // TODO: Error if we know that this participant is not known?
                        Participant::participant( p, true )->addWavelet(m_wavelet);
                    }
                }
            }
            if ( (*it).hasRemoveParticipant() )
            {
                QString p = (*it).removeParticipant();
                JID jid(p);
                if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
                if ( m_wavelet->m_participants.contains( p ) )
                {
                    m_wavelet->m_participants.remove( p );

                    // Is it a remote user?
                    if ( !jid.isLocal() )
                        m_wavelet->unsubscribeRemote( jid );
                    else
                    {
                        // Remove the wavelet from the participant
                        Participant* pptr = Participant::participant( p, false );
                        if ( pptr )
                            pptr->removeWavelet(m_wavelet);
                    }
                }
            }
        }

        int operationsApplied = -1;
        qint64 applicationTime = -1;
        bool restore = ( operationsApplied != -1 );
        if ( operationsApplied == -1 )
            operationsApplied = clientDelta.operations().count();
        else if ( operationsApplied != clientDelta.operations().count() ) { ERROR("Number of operations applied is different than expected."); }
        if ( applicationTime == -1 )
            // Time since 1.1.1970 in milliseconds
            applicationTime = timeStamp();

        // Construct a AppliedWaveletDelta and sign it (if required)
        AppliedWaveletDelta appliedDelta( m_signedDelta, applicationTime, operationsApplied );
        if ( transformed )
            appliedDelta.setTransformedDelta( clientDelta );

        // Send the delta to all local subscribers
        m_wavelet->commit( appliedDelta, restore );

        // Send information back
        if ( !m_message->sender().isNull() )
        {
            LOG("Sending response to caller");
            PBMessage<messages::SubmitResponse>* response = new PBMessage<messages::SubmitResponse>( m_message->Id() );
            response->set_operations_applied( operationsApplied );
            response->set_application_timestamp( applicationTime );
            QByteArray hash = appliedDelta.resultingVersion().hash;
            response->mutable_hashed_version_after_application()->set_history_hash( hash.constData(), hash.length() );
            response->mutable_hashed_version_after_application()->set_version( appliedDelta.resultingVersion().version );
            send( m_message->sender(), response );
        }

        // Send the delta to all remote subscribers (if XMPP is enabled)
        XmppComponentConnection* comcon = XmppComponentConnection::connection();
        if ( comcon )
        {
            foreach( QString rid, m_wavelet->m_remoteSubscribers.keys() )
            {
                XmppVirtualConnection* con = comcon->virtualConnection( rid );
                if ( !con )
                    continue;
                con->sendWaveletUpdate( m_wavelet->url().toString(), appliedDelta );
            }
        }
    }

    LOG("Submit request completed");

    END_EXECUTE;
}

void LocalWavelet::SubmitRequestActor::sendFailedSubmitResponse(const QString& error)
{
    // Send information back
    if ( !m_message->sender().isNull() )
    {
        messages::SubmitResponse response;
        response.set_operations_applied( 0 );
        response.set_application_timestamp( timeStamp() );
        response.set_error_message( error.toStdString() );
        send( m_message->sender(), new PBMessage<messages::SubmitResponse>( response, m_message->Id() ) );
    }
}

qint64 LocalWavelet::SubmitRequestActor::timeStamp()
{
    return (qint64)(QDateTime::currentDateTime().toTime_t()) * 1000;
}
