#include "wavelet.h"
#include "wave.h"
#include "waveletdocument.h"
#include "network/clientconnection.h"
#include "network/xmppcomponent.h"
#include "participant.h"

Wavelet::Wavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId )
    : m_wave(wave), m_domain(waveletDomain), m_id(waveletId), m_version(0)
{
    // The initial hash is the wave URL
    m_hash = url().toString().toAscii();
}

Wavelet::~Wavelet()
{
    foreach( WaveletDocument* doc, m_documents.values() )
        delete doc;
}

WaveUrl Wavelet::url() const
{
    return WaveUrl( m_wave->domain(), m_wave->id(), m_domain, m_id );
}

int Wavelet::receive( const WaveletDelta& delta, QString* errorMessage )
{
    WaveletDelta indexDelta;
    indexDelta.setAuthor( "digest-author" );
    indexDelta.version().version = 0;

//    QSet<QString> participants( m_participants );

    // This is a delta from the future? -> error
    if ( delta.version().version > m_version )
    {
        errorMessage->append("Version number did not match");
        return 0;
    }

    // Make a shallow copy of the client delta
    WaveletDelta clientDelta(delta);

    // The delta needs to be transformed?
    if ( clientDelta.version().version < m_version )
    {
        //
        // Perform OT on the delta
        //

        // Compare the history hash. The hash of version 0 is a special case
        qint64 clientVersion = clientDelta.version().version;
        if ( clientVersion == 0 && url().toString().toAscii() != clientDelta.version().hash )
        {
            errorMessage->append("History hash does not match");
            return 0;
        }
        else if ( clientVersion > 0 && clientDelta.version().hash != m_deltas[clientVersion].version().hash )
        {
            errorMessage->append("History hash does not match");
            return 0;
        }

        // Make a shallow copy of the server-deltas which need to participate in transformations.
        // These copies will be modified during OT
        QList<WaveletDelta> server;
        for( int v = delta.version().version; v < m_deltas.count(); ++v )
            server.append( m_deltas[v] );

        // Loop over all client operations and transform them
        for( int c = 0; c < clientDelta.operations().count(); ++c )
        {
            for( int i = 0; i < server.count(); ++i )
            {
                WaveletDelta& serverDelta = server[i];
                for( int s = 0; s < serverDelta.operations().count(); ++s )
                {
                    bool ok;
                    QPair<WaveletDeltaOperation,WaveletDeltaOperation> pair = WaveletDeltaOperation::xform(serverDelta.operations()[s], clientDelta.operations()[c], &ok);
                    if ( !ok )
                    {
                        qDebug("Wavelet could not be applied");
                        errorMessage->append("Wavelet could not be applied");
                        return 0;
                    }                
                    serverDelta.operations()[s] = pair.first;
                    clientDelta.operations()[c] = pair.second;
                }
            }
        }    
        clientDelta.version().hash = m_hash;
        clientDelta.version().version = m_version;
    }
    // This is an update to the latest version?
    else
    {
        // Compare the history hash
        if ( clientDelta.version().hash != m_hash )
        {
            errorMessage->append("History hash does not match");
            return 0;
        }
    }

    QSet<QString> newParticipants;

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
                return 0;
            }
            // Remember that the digest will need an update
        }
        if ( (*it).hasAddParticipant() )
        {
            QString p = (*it).addParticipant();
            JID jid(p);
            // Is this a valid participant name?
            if ( jid.isValid() )
            {
                if ( !m_participants.contains( p ) )
                {
                    m_participants.insert( p );
                    // Add the wavelet to the participant (and  make sure that such a participant exists.
                    // TODO: Error if we know that this participant is not known?
                    Participant::participant( p, true )->addWavelet(this);
                    // Send the new participant an index wave entry
                    newParticipants.insert( p );

                    // Is this a remote user?
                    if ( !jid.isLocal() )
                        subscribeRemote( jid );
                }
                // The digest needs an update
                WaveletDeltaOperation op;
                op.setAddParticipant( (*it).addParticipant() );
                indexDelta.addOperation(op);
            }
        }
        if ( (*it).hasRemoveParticipant() )
        {
            QString p = (*it).removeParticipant();
            JID jid(p);
            // Is this a valid participant name?
            if ( jid.isValid() )
            {
                if ( m_participants.contains( p ) )
                {
                    m_participants.remove( p );
                    // Remove the wavelet from the participant
                    Participant* pptr = Participant::participant( p, false );
                    if ( pptr )
                        pptr->removeWavelet(this);

                    // Is it a remote user?
                    if ( !jid.isLocal() )
                        unsubscribeRemote( jid );
                }
            }
            // The digest needs an update
            WaveletDeltaOperation op;
            op.setRemoveParticipant( (*it).removeParticipant() );
            indexDelta.addOperation(op);
        }
    }

    // Increase the version number
    m_version++;
    // TODO: Compute the new hash
    m_hash = clientDelta.version().hash;
    // Add the new delta to the list
    m_deltas.append(clientDelta);

    QList<WaveletDelta> deltas;
    deltas.append( clientDelta );

    // Send the delta to all local subscribers
    foreach( QString cid, m_subscribers )
    {
        ClientConnection* c = ClientConnection::connectionById(cid);
        if ( !c )
            m_subscribers.remove(cid);
        else
            c->sendWaveletUpdate( this, deltas, m_version, m_hash );
    }    
    // Send the delta to all remote subscribers
    XmppComponentConnection* comcon = XmppComponentConnection::connection();
    if ( comcon )
    {
        foreach( QString rid, m_remoteSubscribers.keys() )
        {
            XmppVirtualConnection* con = comcon->virtualConnection( rid );
            if ( !con )
                continue;
            con->sendWaveletUpdate( url().toString(), delta );
        }
    }

    // Prepare a digest update
    DocumentMutation m;
    if ( !m_lastDigest.isEmpty() )
        m.deleteChars( m_lastDigest );
    m_lastDigest = digest();
    m.insertChars( m_lastDigest );
    WaveletDeltaOperation op;
    op.setMutation(m);
    indexDelta.addOperation(op);

    // Send the digest delta to all connected participants
    foreach( QString p, m_participants )
    {
        // Digest deltas go only to local users
        JID jid(p);
        if ( !jid.isLocal() )
            continue;
        // A new participant? Send him an initial digest
        if ( newParticipants.contains(p) )
        {
            WaveletDelta digest = initialDigest();
            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
            {
                c->sendIndexUpdate(this, digest);
            }
        }
        // An old participant -> send him a digest update
        else
        {
            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
            {
                c->sendIndexUpdate(this, indexDelta);
            }
        }
    }

    return clientDelta.operations().count();
}

void Wavelet::subscribe( ClientConnection* connection )
{
    m_subscribers.insert( connection->id() );

    // Send the history
    connection->sendWaveletUpdate( this, m_deltas, m_version, m_hash );
}

void Wavelet::unsubscribe( ClientConnection* connection )
{
    m_subscribers.remove( connection->id() );
}

void Wavelet::subscribeRemote( const JID& remoteJid )
{
    if ( !m_remoteSubscribers.contains( remoteJid.domain() ) )
        m_remoteSubscribers[ remoteJid.domain() ] = 1;
    else
        m_remoteSubscribers[ remoteJid.domain() ] = m_remoteSubscribers[ remoteJid.domain() ] + 1;
}

void Wavelet::unsubscribeRemote( const JID& remoteJid )
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

QString Wavelet::firstRootBlipId() const
{
    WaveletDocument* doc = m_documents["conversation"];
    if ( !doc )
        return QString::null;

    for( int i = 0; i < doc->count(); ++i )
    {
        if ( doc->typeAt(i) == StructuredDocument::Start && doc->tagAt(i) == "blip" )
        {
            QString id = doc->attributesAt(i)["id"];
            if ( id.left(2) == "b+" )
                return id;
            return QString::null;
        }
    }

    return QString::null;
}

WaveletDocument* Wavelet::firstRootBlip() const
{
    QString id = firstRootBlipId();
    if ( id.isEmpty() )
        return 0;
    return m_documents[id];
}

QString Wavelet::digest() const
{
    // TODO: This is not a very nice digest ...
    WaveletDocument* doc = firstRootBlip();
    if ( !doc )
        return QString("-");
    return doc->toPlainText();
}

WaveletDelta Wavelet::initialDigest() const
{
    DocumentMutation m;
    m.insertChars( m_lastDigest );
    WaveletDeltaOperation op;
    op.setMutation(m);

    WaveletDelta indexDelta;
    indexDelta.setAuthor( "digest-author" );
    indexDelta.version().version = 0;
    indexDelta.addOperation(op);

    foreach( QString participantId, m_participants )
    {
        WaveletDeltaOperation op2;
        op2.setAddParticipant( participantId );
        indexDelta.addOperation(op2);
    }

    return indexDelta;
}

bool Wavelet::hasParticipant(const QString& jid) const
{
    return m_participants.contains(jid);
}