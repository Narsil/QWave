#include "wavelet.h"
#include "wave.h"
#include "waveletdocument.h"
#include "signedwaveletdelta.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "app/settings.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/jid.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"
#include <QDateTime>
#include <openssl/sha.h>

Wavelet::Wavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId )
    : ActorGroup( waveletDomain + "$" + waveletId, wave ), m_version(0), m_wave(wave), m_domain(waveletDomain), m_id(waveletId)
{
    // The initial hash is the wave URL
    m_hash = url().toString().toAscii();
}

Wavelet::~Wavelet()
{
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
        else if ( clientVersion == m_version )
        {
            if ( m_hash != clientDelta.version().hash )
            {
                errorMessage->append("History hash does not match");
                return false;
            }
        }
        else if ( m_deltas[clientVersion].isNull() )
        {
            errorMessage->append("Applying at invalid version number");
            return false;
        }
        else if ( clientDelta.version().hash != m_deltas[clientVersion].version().hash )
        {
            errorMessage->append("History hash does not match");
            return false;
        }
    }

    return true;
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
        server.append( m_deltas[v] );

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

void Wavelet::broadcast( const WaveletDelta& delta )
{
    foreach( QString p, m_contentSubscribers )
    {
        ActorId actorid( p );
        PBMessage<waveserver::ProtocolWaveletUpdate>* msg = new PBMessage<waveserver::ProtocolWaveletUpdate>( actorid );
        protocol::ProtocolWaveletDelta* pdelta = msg->add_applied_delta();
        Converter::convert( pdelta, delta );
        msg->set_wavelet_name( url().toString().toStdString() );
        msg->mutable_resulting_version()->set_version( m_version );
        msg->mutable_resulting_version()->set_history_hash( m_hash.constData(), m_hash.length() );
        post( msg );        
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
    op.setDocumentId("digest");
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
    op.setDocumentId( "digest" );
    digest.addOperation(op);

    foreach( QString p, m_indexSubscribers )
    {
        ActorId actorid( p );
        PBMessage<messages::WaveletDigest>* digestMsg = new PBMessage<messages::WaveletDigest>( actorid );
        Converter::convert( digestMsg->mutable_digest_delta(), digest );
        digestMsg->set_wavelet_name( url().toString().toStdString() );
        post( digestMsg );
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

void Wavelet::customEvent( QEvent* event )
{
    PBMessage<messages::SubscribeWavelet>* subscribe = dynamic_cast<PBMessage<messages::SubscribeWavelet>*>( event );
    if ( subscribe )
    {
        new SubscribeActor( this, *subscribe );
        return;
    }

    this->ActorGroup::customEvent( event );
}

void Wavelet::unsubscribeAllClients( const QString& user)
{
    foreach( QByteArray ba, m_indexSubscribers )
    {
        ActorId id( QString::fromUtf8(ba) );
        if ( id.groups()[0] == user )
            m_indexSubscribers.remove( ba );
    }
    foreach( QByteArray ba, m_contentSubscribers )
    {
        ActorId id( QString::fromUtf8(ba) );
        if ( id.groups()[0] == user )
            m_contentSubscribers.remove( ba );
    }
}

void Wavelet::notifyAllClients( const QString& participant )
{
    // Tell all clients that the participant is now added to this wave
    PBMessage<messages::WaveletNotify>* notify = new PBMessage<messages::WaveletNotify>( ActorId("client", participant ) );
    notify->set_wavelet_name( url().toString().toStdString() );
    post(notify);
}

void Wavelet::toWaveletUpdate( waveserver::ProtocolWaveletUpdate* update )
{
    update->set_wavelet_name( url().toString().toStdString() );
    update->mutable_resulting_version()->set_version( m_version );
    update->mutable_resulting_version()->set_history_hash( m_hash.constData(), m_hash.length() );
    if ( m_version == 0 )
        return;
    protocol::ProtocolWaveletDelta* delta = update->add_applied_delta();
    delta->mutable_hashed_version()->set_version(0);
    delta->mutable_hashed_version()->set_history_hash( update->wavelet_name() );
    delta->set_author( m_creator.toStdString() );
    int v = 0;
    foreach( QString p, m_participants )
    {
        protocol::ProtocolWaveletOperation* op = delta->add_operation();        
        op->set_add_participant( p.toStdString() );
    }
    foreach( WaveletDocument* doc, m_documents.values() )
    {
        // Paranoia
        if ( doc->authors().count() == 0 )
            continue;
        delta = update->add_applied_delta();
        delta->mutable_hashed_version()->set_version(++v);
        delta->mutable_hashed_version()->set_history_hash( update->wavelet_name() );
        delta->set_author( doc->authors()[0].toStdString() );
        protocol::ProtocolWaveletOperation* op = delta->add_operation();
        protocol::ProtocolWaveletOperation::MutateDocument* mutate = op->mutable_mutate_document();
        mutate->set_document_id( doc->name().toStdString() );
        doc->toDocumentOperation( mutate->mutable_document_operation() );
    }
}

#define ERROR(msg) { errorMessage->append( msg ); return AppliedWaveletDelta(); }

AppliedWaveletDelta Wavelet::process( const protocol::ProtocolSignedDelta* signed_delta, const protocol::ProtocolAppliedWaveletDelta* applied_delta, QSet<QString>* addedUsers, QSet<QString>* removedUsers, QString* errorMessage )
{
    bool ok;
    // Decode the delta, check it, and transform it (if required)
    SignedWaveletDelta signedDelta( signed_delta, &ok );
    if ( !ok ) ERROR("Could not decode the signed delta");

    // Make a copy of the delta because we might have to transform it
    WaveletDelta delta( signedDelta.delta() );

    // Check its applicability
    if ( !checkHashedVersion( delta, errorMessage ) )
        ERROR( errorMessage );

    // Transform if required
    bool transformed = transform( delta, errorMessage, &ok );
    if ( !ok )
        ERROR( errorMessage );

    // Apply all operations contained in the delta
    for( QList<WaveletDeltaOperation>::const_iterator it = delta.operations().begin(); it != delta.operations().end(); it++ )
    {
        if ( (*it).hasAddParticipant() && addedUsers )
        {
            JID jid( (*it).addParticipant() );
            if ( jid.isLocal() )
                addedUsers->insert( jid.toString() );
        }
        if ( (*it).hasRemoveParticipant() && removedUsers )
        {
            JID jid( (*it).removeParticipant() );
            if ( jid.isLocal() )
                removedUsers->insert( (*it).removeParticipant() );
        }
    }

    // Construct an AppliedWaveletDelta
    if ( applied_delta )
    {
        AppliedWaveletDelta appliedDelta( applied_delta, &ok );
        if ( !ok ) ERROR("Could not decode applied wavelet delta");
        if ( transformed )
            appliedDelta.setTransformedDelta( delta );
        return appliedDelta;
    }

    // Construct an AppliedWaveletDelta
    int operationsApplied = delta.operations().count();
    qint64 applicationTime = (qint64)(QDateTime::currentDateTime().toTime_t()) * 1000;
    AppliedWaveletDelta appliedDelta( signedDelta, applicationTime, operationsApplied );
    if ( transformed )
        appliedDelta.setTransformedDelta( delta );
    return appliedDelta;
}

#undef ERROR
#define ERROR(msg) { errorMessage->append(msg); return false; }

bool Wavelet::apply( const AppliedWaveletDelta& appliedDelta, QString* errorMessage )
{
    if ( m_version == 0 )
        m_creator = appliedDelta.signedDelta().delta().author();

    const WaveletDelta* delta = &appliedDelta.signedDelta().delta();
    if ( !appliedDelta.transformedDelta().isNull() )
        delta = &appliedDelta.transformedDelta();

    // Apply all operations contained in the delta
    for( QList<WaveletDeltaOperation>::const_iterator it = delta->operations().begin(); it != delta->operations().end(); it++ )
    {
        QString docId = (*it).documentId();
        WaveletDocument* doc = m_documents[docId];
        if ( !doc )
        {
            doc = new WaveletDocument(this, docId);
            m_documents[docId] = doc;
        }

        if ( (*it).hasMutation() )
        {
            bool check = doc->apply( (*it).mutation(), appliedDelta.signedDelta().delta().author() );
            if ( !check ) { ERROR( "Failed to apply delta to " + docId); }
        }
        if ( (*it).hasAddParticipant() )
        {
            QString p = (*it).addParticipant();
            JID jid(p);
            if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
            if ( !m_participants.contains( p ) )
            {
                m_participants.insert( p );
                onAddParticipant(jid);
            }
        }
        if ( (*it).hasRemoveParticipant() )
        {
            QString p = (*it).removeParticipant();
            JID jid(p);
            if ( !jid.isValid() ) { ERROR("Invalid JID " + p ); }
            if ( m_participants.contains( p ) )
            {
                m_participants.remove( p );
                onRemoveParticipant(jid);
            }
        }
    }

    int oldVersion = m_version;

    // Build the bye stream over which the hash is computed
    QByteArray ba2 = appliedDelta.toBinary();
    ba2.prepend( m_hash );
    // Compute the hash
    QByteArray hashBuffer( 32, 0 );
    SHA256( (const unsigned char*)ba2.constData(), ba2.length(), (unsigned char*)hashBuffer.data() );
    // Copy over the first 20 bytes
    m_hash.resize(20);
    for( int i = 0; i < 20; ++i )
        m_hash.data()[i] = hashBuffer.data()[i];
    // Update the hashed version
    m_version += appliedDelta.operationsApplied();

    // For the intermediate versions (if any) there is no information.
    for( int v = oldVersion + 1; v < m_version; ++v )
        m_deltas.append( WaveletDelta() );
    // Add the new delta to the list
    m_deltas.append(*delta);

    broadcast( *delta );
    broadcastDigest( *delta );

    return true;
}


/****************************************************************************
 *
 * WaveletActor
 *
 ***************************************************************************/

qint64 Wavelet::WaveletActor::s_id = 0;

Wavelet::WaveletActor::WaveletActor( Wavelet* wavelet ) : Actor( QString::number( s_id++), wavelet ), m_wavelet(wavelet)
{
}

void Wavelet::WaveletActor::log( const char* error, const char* file, int line )
{
    QString d = m_wavelet->url().toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("INFO in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void Wavelet::WaveletActor::log( const QString& error, const char* file, int line )
{
    log( error.toAscii().constData(), file, line );
}

void Wavelet::WaveletActor::logErr( const char* error, const char* file, int line )
{
    QString d = m_wavelet->url().toString();
    QString t = QDateTime::currentDateTime().toString();
    qDebug("ERROR in %s:%i talking to %s at %s: %s", file, line, d.toAscii().constData(), t.toAscii().constData(), error );
}

void Wavelet::WaveletActor::logErr( const QString& error, const char* file, int line )
{
    logErr( error.toAscii().constData(), file, line );
}

/****************************************************************************
 *
 * InitActor
 *
 ***************************************************************************/

// TODO: Better error handler. If InitActor fails, the wavelet remains locked because the critical section is disabled
#undef ERROR
#define ERROR(msg) { logErr(msg, __FILE__, __LINE__); TERMINATE(); }
#define LOG(msg) { log(msg, __FILE__, __LINE__); }

Wavelet::InitActor::InitActor( Wavelet* wavelet )
    : WaveletActor( wavelet )
{
}

void Wavelet::InitActor::execute()
{
    qDebug("EXECUTE LocalWavelet::InitActor");

    BEGIN_EXECUTE;

    // Send a query to the database
    {
        m_msgId = nextId();
        PBMessage<messages::QueryWaveletUpdates>* query = new PBMessage<messages::QueryWaveletUpdates>( ActorId("store", m_wavelet->url().toString() ), m_msgId );
        query->setCreateOnDemand( true );
        query->set_wavelet_name( m_wavelet->url().toString().toStdString() );
        query->set_start_version( 0 );
        query->set_end_version( 0xFFFFFFFF ); // TODO: Use max qint64 here
        bool ok = post( query );
        if ( !ok ) { ERROR("Internal server error. Could not talk to database."); }
    }

    // Wait for a response from the database
    yield( RecvPB<messages::QueryWaveletUpdatesResponse>(m_msgId) | Timeout(10000) );
    if ( REASON(RecvPB<messages::QueryWaveletUpdatesResponse>) )
    {
        if ( !REASON->ok() ) { ERROR("Data base reported an error:" + QString::fromStdString( REASON->error() )); }
        for( int i = 0; i < REASON->applied_delta_size(); ++i )
        {
            protocol::ProtocolAppliedWaveletDelta protobuf;
            if ( !protobuf.ParseFromArray( REASON->applied_delta(i).data(), REASON->applied_delta(i).length() ) ) { ERROR("Database delivered corrupted data"); }
            QString err;
            AppliedWaveletDelta delta = m_wavelet->process( &protobuf.signed_original_delta(), &protobuf, 0, 0, &err );
            if ( delta.isNull() ) ERROR("Database delivered corrupted data: " + err);
            if ( !m_wavelet->apply( delta, &err ) ) ERROR("Could not apply delta: " + err);
        }
    }
    else { ERROR("Timeout waiting for database"); }

    m_wavelet->criticalSection()->enable();

    END_EXECUTE;
}

/****************************************************************************
 *
 * SubscribeActor
 *
 ***************************************************************************/

Wavelet::SubscribeActor::SubscribeActor( Wavelet* wavelet, const PBMessage<messages::SubscribeWavelet>& message)
        : WaveletActor( wavelet ), m_message( message )
{
}

void Wavelet::SubscribeActor::execute()
{
    qDebug("EXECUTE Wavelet::SubscribeActor");

    BEGIN_EXECUTE;

    if ( !m_wavelet->criticalSection()->tryEnter(this) )
        yield( RecvCriticalSection( m_wavelet->criticalSection() ) );

    qDebug("Subscribing to %s", m_wavelet->url().toString().toAscii().constData() );
    QByteArray id( m_message.actor_id().data(), m_message.actor_id().length() );
    ActorId actorid( QString::fromUtf8(id) );
    if ( m_message.subscribe() )
    {
        if ( m_message.content() )
        {
            if ( !m_wavelet->m_contentSubscribers.contains(id) )
            {
                m_wavelet->m_contentSubscribers.insert( id );
                if ( m_wavelet->m_version > 0 )
                {
                    qDebug("Sending content to %s", actorid.toString().toAscii().constData() );
                    PBMessage<waveserver::ProtocolWaveletUpdate>* update = new PBMessage<waveserver::ProtocolWaveletUpdate>( actorid );
                    m_wavelet->toWaveletUpdate( update );
                    post( update );
                }
            }
        }
        if ( m_message.index() )
        {
            if ( !m_wavelet->m_indexSubscribers.contains(id) )
            {
                m_wavelet->m_indexSubscribers.insert( id );
                if ( m_wavelet->m_version > 0 )
                {
                    qDebug("Sending digest to %s", actorid.toString().toAscii().constData() );
                    // Send an initial digest
                    WaveletDelta delta = m_wavelet->initialDigest();
                    PBMessage<messages::WaveletDigest>* digest = new PBMessage<messages::WaveletDigest>( actorid );
                    Converter::convert( digest->mutable_digest_delta(), delta );
                    digest->set_wavelet_name( m_wavelet->url().toString().toStdString() );
                    post( digest );
                }
            }
        }
    }
    else
    {
        if ( m_message.content() )
        {
            m_wavelet->m_contentSubscribers.remove( id );
        }
        if ( m_message.index() )
        {
            m_wavelet->m_indexSubscribers.remove( id );
        }
    }

    // Allow other actors to modify the wavelet
    m_wavelet->criticalSection()->leave(this);

    END_EXECUTE
}

