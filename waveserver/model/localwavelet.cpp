#include "localwavelet.h"
#include "model/waveletdocument.h"
#include "model/jid.h"
#include "model/participant.h"
#include "network/xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"
#include <QDateTime>

LocalWavelet::LocalWavelet(Wave* wave, const QString& waveletDomain, const QString& waveletId)
        : Wavelet( wave, waveletDomain, waveletId )
{
}

int LocalWavelet::apply( const protocol::ProtocolSignedDelta& protobufDelta, QString* errorMessage )
{
    bool ok;
    SignedWaveletDelta delta( &protobufDelta, &ok );
    if ( !ok )
    {
        errorMessage->append("Error decoding signedDelta");
        return -1;
    }
    return apply( delta, errorMessage );
}

int LocalWavelet::apply( const SignedWaveletDelta& signedDelta, QString* errorMessage )
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

//    // The delta needs to be transformed?
//    if ( clientDelta.version().version < m_version )
//    {
//        transformed = true;
//        //
//        // Perform OT on the delta
//        //
//
//        // Make a shallow copy of the server-deltas which need to participate in transformations.
//        // These copies will be modified during OT
//        QList<WaveletDelta> server;
//        for( int v = clientDelta.version().version; v < m_deltas.count(); ++v )
//            server.append( m_deltas[v].signedDelta().delta() );
//
//        // Loop over all client operations and transform them
//        for( int c = 0; c < clientDelta.operations().count(); ++c )
//        {
//            for( int i = 0; i < server.count(); ++i )
//            {
//                WaveletDelta& serverDelta = server[i];
//                for( int s = 0; s < serverDelta.operations().count(); ++s )
//                {
//                    bool ok;
//                    QPair<WaveletDeltaOperation,WaveletDeltaOperation> pair = WaveletDeltaOperation::xform(serverDelta.operations()[s], clientDelta.operations()[c], &ok);
//                    if ( !ok )
//                    {
//                        qDebug("Wavelet could not be applied");
//                        errorMessage->append("Wavelet could not be applied");
//                        return -1;
//                    }
//                    serverDelta.operations()[s] = pair.first;
//                    clientDelta.operations()[c] = pair.second;
//                }
//            }
//        }
//        clientDelta.version().hash = m_hash;
//        clientDelta.version().version = m_version;
//    }

//    // Track which participants are added by the delta
//    QSet<QString> newParticipants;
//
//    // Prepare a delta for the digest
//    WaveletDelta indexDelta;
//    indexDelta.setAuthor( "digest-author" );
//    indexDelta.version().version = 0;

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

    int operationsApplied = clientDelta.operations().count();
    // Time since 1.1.1970 in milliseconds
    qint64 applicationTime = (qint64)(QDateTime::currentDateTime().toTime_t()) * 1000;

    // Construct a AppliedWaveletDelta and sign it (if required)
    AppliedWaveletDelta appliedDelta( signedDelta, applicationTime, operationsApplied );
    if ( transformed )
        appliedDelta.setTransformedDelta( clientDelta );

    // Send the delta to all local subscribers
    commit( appliedDelta );

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

//    // Prepare a digest update
//    DocumentMutation m;
//    if ( !m_lastDigest.isEmpty() )
//        m.deleteChars( m_lastDigest );
//    m_lastDigest = digest();
//    m.insertChars( m_lastDigest );
//    WaveletDeltaOperation op;
//    op.setMutation(m);
//    indexDelta.addOperation(op);

//    broadcastDigest( digest );
//    // Send the digest delta to all connected participants
//    foreach( QString p, m_participants )
//    {
//        // Digest deltas go only to local users
//        JID jid(p);
//        if ( !jid.isLocal() )
//            continue;
//        // A new participant? Send him an initial digest
//        if ( newParticipants.contains(p) )
//        {
//            WaveletDelta digest = initialDigest();
//            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
//            {
//                c->sendIndexUpdate(this, digest);
//            }
//        }
//        // An old participant -> send him a digest update
//        else
//        {
//            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
//            {
//                c->sendIndexUpdate(this, indexDelta);
//            }
//        }
//    }

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
