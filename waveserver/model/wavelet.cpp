#include "wavelet.h"
#include "wave.h"
#include "waveletdocument.h"
#include "signedwaveletdelta.h"
#include "network/clientconnection.h"
#include "network/xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"
#include "network/converter.h"
#include "participant.h"
#include "protocol/common.pb.h"
#include "app/settings.h"
#include "persistence/commitlog.h"

Wavelet::Wavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId )
    : m_version(0), m_wave(wave), m_domain(waveletDomain), m_id(waveletId)
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

bool Wavelet::checkHashedVersion( const protocol::ProtocolWaveletDelta& protobufDelta, QString* errorMessage )
{
    WaveletDelta clientDelta = Converter::convert( protobufDelta );
    return checkHashedVersion( clientDelta, errorMessage );
}

bool Wavelet::checkHashedVersion( const WaveletDelta& clientDelta, QString* errorMessage )
{
    // Compare the history hash. The hash of version 0 is a special case
    qint64 clientVersion = clientDelta.version().version;
    if ( clientVersion == 0 && url().toString().toAscii() != clientDelta.version().hash )
    {
        errorMessage->append("History hash does not match");
        return false;
    }
    else if ( clientVersion > 0 )
    {
        if ( clientVersion > m_version )
        {
            errorMessage->append("Applying at invalid version number, i.e. version number is from the futur");
            return false;
        }
        if ( m_deltas[clientVersion - 1].isNull() )
        {
            errorMessage->append("Applying at invalid version number");
            return false;
        }
        else if ( clientDelta.version().hash != m_deltas[clientVersion - 1].resultingVersion().hash )
        {
            errorMessage->append("History hash does not match");
            return false;
        }
    }

    return true;
}

void Wavelet::subscribe( ClientConnection* connection )
{
    m_subscribers.insert( connection->id() );

    // Send the history
    connection->sendWaveletUpdate( this, m_deltas );
}

void Wavelet::unsubscribe( ClientConnection* connection )
{
    m_subscribers.remove( connection->id() );
}

bool Wavelet::transform( WaveletDelta& clientDelta, QString* errorMessage, bool* ok )
{
    // The delta needs to be transformed?
    Q_ASSERT( clientDelta.version().version <= m_version );

    if ( ok )
        *ok = true;
    if ( clientDelta.version().version == m_version )
        return false;

    //
    // Perform OT on the delta
    //

    // Make a shallow copy of the server-deltas which need to participate in transformations.
    // These copies will be modified during OT
    QList<WaveletDelta> server;
    for( int v = clientDelta.version().version; v < m_deltas.count(); ++v )
        server.append( m_deltas[v].signedDelta().delta() );

    // Loop over all client operations and transform them
    for( int c = 0; c < clientDelta.operations().count(); ++c )
    {
        for( int i = 0; i < server.count(); ++i )
        {
            WaveletDelta& serverDelta = server[i];
            for( int s = 0; s < serverDelta.operations().count(); ++s )
            {
                bool ok2;
                QPair<WaveletDeltaOperation,WaveletDeltaOperation> pair = WaveletDeltaOperation::xform(serverDelta.operations()[s], clientDelta.operations()[c], &ok2);
                if ( !ok2 )
                {
                    qDebug("Wavelet could not be applied");
                    errorMessage->append("Wavelet could not be applied");
                    if ( ok )
                        *ok = false;
                    return true;
                }
                serverDelta.operations()[s] = pair.first;
                clientDelta.operations()[c] = pair.second;
            }
        }
    }
    clientDelta.version().hash = m_hash;
    clientDelta.version().version = m_version;

    return true;
}

void Wavelet::broadcast( const AppliedWaveletDelta& delta )
{
    QList<AppliedWaveletDelta> deltas;
    deltas.append( delta );
    foreach( QString cid, m_subscribers )
    {
        ClientConnection* c = ClientConnection::connectionById(cid);
        if ( !c )
            m_subscribers.remove(cid);
        else
            c->sendWaveletUpdate( this, deltas );
    }
}

void Wavelet::commit( const AppliedWaveletDelta& appliedDelta )
{
    int oldVersion = m_version;
    // Update the hashed version
    m_version = appliedDelta.resultingVersion().version;
    m_hash = appliedDelta.resultingVersion().hash;

    // For the intermediate versions (if any) there is no information.
    for( int v = oldVersion + 1; v < m_version; ++v )
        m_deltas.append( AppliedWaveletDelta() );
    // Add the new delta to the list
    m_deltas.append(appliedDelta);

    // Write it to the commit log
    CommitLog::commitLog()->write(this, appliedDelta);

    broadcast( appliedDelta );
    broadcastDigest( appliedDelta.signedDelta().delta() );
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

QString Wavelet::digestText() const
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

void Wavelet::broadcastDigest(const WaveletDelta& delta )
{
    // Track which participants are added by the delta
    QSet<QString> newParticipants;

    // Prepare a delta for the digest
    WaveletDelta digest;
    digest.setAuthor( "digest-author" );
    // TODO
    digest.version().version = 0;

    // Find out which participants have been added or removed
    for( QList<WaveletDeltaOperation>::const_iterator it = delta.operations().begin(); it != delta.operations().end(); it++ )
    {
        QString docId = (*it).documentId();
        WaveletDocument* doc = m_documents[docId];
        if ( !doc || docId == "conversation" || docId.left(2) == "a+" )
            continue;

        // The digest needs an update ?
        if ( (*it).hasAddParticipant() )
        {
            QString p = (*it).addParticipant();
            WaveletDeltaOperation op;
            op.setAddParticipant( p );
            digest.addOperation(op);
        }
        if ( (*it).hasRemoveParticipant() )
        {
            QString p = (*it).removeParticipant();        
            WaveletDeltaOperation op;
            op.setRemoveParticipant( p );
            digest.addOperation(op);
        }
    }

    // Prepare a digest update
    DocumentMutation m;
    if ( !m_lastDigest.isEmpty() )
        m.deleteChars( m_lastDigest );
    m_lastDigest = digestText();
    m.insertChars( m_lastDigest );
    WaveletDeltaOperation op;
    op.setMutation(m);
    digest.addOperation(op);

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
            WaveletDelta initdigest = initialDigest();
            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
            {
                c->sendIndexUpdate(this, initdigest);
            }
        }
        // An old participant -> send him a digest update
        else
        {
            foreach( ClientConnection* c, ClientConnection::connectionsByParticipant(p) )
            {
                c->sendIndexUpdate(this, digest);
            }
        }
    }
}

bool Wavelet::hasParticipant(const QString& jid) const
{
    return m_participants.contains(jid);
}

bool Wavelet::isRemote() const
{
    return ( m_domain != Settings::settings()->domain() );
}
