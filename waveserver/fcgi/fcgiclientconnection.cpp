#include "fcgiclientconnection.h"
#include "network/clientactorfolk.h"
#include "protocol/messages.pb.h"
#include "model/jid.h"
#include "model/waveurl.h"
#include "model/wavefolk.h"
#include "network/clientindexwaveactor.h"

FCGIClientConnection::FCGIClientConnection(const QString& sessionId, const QString& participant, ClientActorFolk* parent)
    : ActorGroup( sessionId, parent ), m_participant( participant ), m_sessionId( sessionId ), m_waveletsOpened( false ), m_clientSequenceNumber(1), m_serverSequenceNumber(0), m_ackedSequenceNumber(0)
{
}

FCGIClientConnection::~FCGIClientConnection()
{
    while( !m_outQueue.isEmpty() )
    {
        PBMessage<webclient::Response>* msg = m_outQueue.dequeue();
        delete msg;
    }
}

void FCGIClientConnection::customEvent( QEvent* event )
{
    PBMessage<webclient::Request>* request = dynamic_cast<PBMessage<webclient::Request>*>( event );
    if ( request )
    {
        handleRequest( request );
        return;
    }
    PBMessage<waveserver::ProtocolWaveletUpdate>* update = dynamic_cast<PBMessage<waveserver::ProtocolWaveletUpdate>*>(event);
    if ( update )
    {
        PBMessage<webclient::Response>* r = new PBMessage<webclient::Response>();
        r->mutable_update()->MergeFrom( *update );
        reply(r);
        return;
    }
    PBMessage<messages::WaveletDigest>* digest = dynamic_cast<PBMessage<messages::WaveletDigest>*>(event);
    if ( digest )
    {
        qDebug("FCGI: GOT DIGEST");
        PBMessage<webclient::Response>* r = new PBMessage<webclient::Response>();
        r->mutable_update()->add_applied_delta()->MergeFrom( digest->digest_delta() );
        r->mutable_update()->set_wavelet_name( digest->wavelet_name() );
        reply(r);
        return;
    }
    PBMessage<messages::WaveletNotify>* notify = dynamic_cast<PBMessage<messages::WaveletNotify>*>(event);
    if ( notify )
    {
        // There is a new wavelet in which the connected user is a participant -> subscribe to it to receive updates and digest
        PBMessage<messages::SubscribeWavelet>* subscribe = new PBMessage<messages::SubscribeWavelet>( notify->sender() );
        subscribe->set_subscribe( true );
        subscribe->set_index( true );
        subscribe->set_actor_id( actorId().toString().toStdString() );
        subscribe->set_content( m_waveletsOpened );
        post( subscribe );
        return;
    }

    this->ActorGroup::customEvent( event );
}

void FCGIClientConnection::handleRequest( const PBMessage<webclient::Request>* request )
{
    m_ackedSequenceNumber = qMax( request->client_ack(), m_ackedSequenceNumber );
    // TODO: If the client sequence number is not what we expected?
    m_clientSequenceNumber = request->client_sequence_number();

    if ( request->has_open() )
    {
        openRequest( &request->open() );
    }

    // Is a message pending? If not then keep the HTTP connection open
    if ( m_outQueue.isEmpty() )
    {
        m_pendingRequest = request->sender();
        return;
    }

    // Dequeue a message and send it via HTTP
    PBMessage<webclient::Response>* out = m_outQueue.dequeue();
    out->setReceiver( request->sender() );
    bool ok = post( out );
    if ( !ok )
    {
        qDebug("Could not send response via FCGI");
        // TODO: Re-enqueue message
    }
}

void FCGIClientConnection::openRequest( const waveserver::ProtocolOpenRequest* msg )
{
    qDebug("msg<< %s", msg->DebugString().data());

    // This is a chance of getting the client participant ID. A bit strange place, but well ...
    QString participantId = QString::fromStdString( msg->participant_id() );
    JID jid( participantId );
    if ( !jid.isValid() )
    {
        qDebug("Malformed JID");
        return;
    }
    if ( !jid.isLocal() )
    {
        qDebug("Not a local user");
        return;
    }

    // No participant or a different one than before? -> not allowed
    if ( m_participant != participantId )
    {
        qDebug("Cannot change user for open connection");
        return;
    }

    QString waveId = QString::fromStdString( msg->wave_id() );
    if ( waveId == "!indexwave" )
    {
        new ClientIndexWaveActor( this );
        return;
    }
    m_waveletsOpened = true;

    for( int i = 0; i < msg->wavelet_id_prefix_size(); ++i )
    {
        QString waveletId = QString::fromStdString( msg->wavelet_id_prefix(i) );

        int index1 = waveId.indexOf('!');
        int index2 = waveletId.indexOf('!');
        if ( index1 == -1 || index2 == -1 )
        {
            qDebug("Malformed wave id or wavelet id");
            return;
        }

        WaveUrl url( waveId.left(index1), waveId.mid(index1+1), waveletId.left(index2), waveletId.mid(index2+1) );
        if ( url.isNull() )
        {
            qDebug("Malformed wave id");
            return;
        }

        // Subscribe to this wavelet
        PBMessage<messages::SubscribeWavelet>* subscribe = new PBMessage<messages::SubscribeWavelet>( WaveFolk::actorId( url ) );
        subscribe->setCreateOnDemand( true );
        subscribe->set_content( true );
        subscribe->set_index( true );
        subscribe->set_subscribe( true );
        subscribe->set_actor_id( actorId().toString().toStdString() );
        bool ok = post( subscribe );
        if ( !ok )
        {
            qDebug("Could not subscribe to wavelet");
            return;
        }
    }
}

void FCGIClientConnection::reply( PBMessage<webclient::Response>* response )
{
    response->set_server_sequence_number( ++m_serverSequenceNumber );

    if ( m_pendingRequest.isNull() )
    {
        qDebug("FCGI: Queueu");
        m_outQueue.enqueue( response );
    }
    else
    {
        qDebug("FCGI: Post to %s", m_pendingRequest.toString().toAscii().constData());
        response->setReceiver( m_pendingRequest );
        m_pendingRequest = ActorId();
        response->set_server_ack( m_clientSequenceNumber );
        bool ok = post( response );
        if ( !ok )
        {
            qDebug("FCGI: Post failed");
            m_outQueue.enqueue( response );
        }
    }
}

void FCGIClientConnection::errorReply( const std::string& msg )
{
    PBMessage<webclient::Response>* response = new PBMessage<webclient::Response>();
    response->set_error_message( msg );
    response->set_server_sequence_number( 0 );
    reply( response );
}
