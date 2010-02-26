#include "clientconnection.h"
#include "clientactorfolk.h"
#include "clientsubmitrequestactor.h"
#include "clientindexwaveactor.h"
#include "network/rpc.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "network/converter.h"
#include "model/waveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/waveurl.h"
#include "model/localwavelet.h"
#include "model/remotewavelet.h"
#include "app/settings.h"
#include "model/wavefolk.h"
#include "model/jid.h"

#include <QTcpSocket>
#include <QByteArray>
#include <QUuid>
// TODO: Why do we still use QUrl here?
#include <QUrl>

#include <sstream>
#include <string>

ClientConnection::ClientConnection(QTcpSocket* socket, ClientActorFolk* parent)
        : ActorGroup( QUuid::createUuid().toString(), parent), m_digestVersion(0), m_waveletsOpened( false )
{
    m_rpc = new RPC(socket, this);
    bool check = connect( m_rpc, SIGNAL(offline()), SLOT(getOffline()));
    Q_ASSERT(check);
    check = connect( m_rpc, SIGNAL(socketError()), SLOT(networkError()));
    Q_ASSERT(check);
    check = connect( m_rpc, SIGNAL(messageReceived(QString,QByteArray)), SLOT(messageReceived(QString,QByteArray)));
    Q_ASSERT(check);
}

ClientConnection::~ClientConnection()
{
    qDebug("DELETE ClientConnection");
    // TODO: unsubscribe all wavelets
}

void ClientConnection::messageReceived(const QString& methodName, const QByteArray& data)
{
    if ( methodName == "waveserver.ProtocolOpenRequest" )
    {
        waveserver::ProtocolOpenRequest open;
        if ( !open.ParseFromArray(data.constData(), data.length()) )
        {
            qDebug("Malformed request");
            getOffline();
            return;
        }

        qDebug("msg<< %s", open.DebugString().data());

        // This is a chance of getting the client participant ID. A bit strange place, but well ...
        QString participantId = QString::fromStdString( open.participant_id() );
        JID jid( participantId );
        if ( !jid.isValid() )
        {
            qDebug("Malformed JID");
            getOffline();
            return;
        }
        if ( !jid.isLocal() )
        {
            qDebug("Not a local user");
            getOffline();
            return;
        }

        // No participant or a different one than before? -> not allowed
        if ( !m_participant.isNull() && m_participant != participantId )
        {
            qDebug("Cannot change user for open connection");
            getOffline();
            return;
        }

        // This is the first time to encounter the JID of our client?
        if ( m_participant.isNull() )
        {
            m_participant = participantId;
        }

        QString waveId = QString::fromStdString( open.wave_id() );
        if ( waveId == "!indexwave" )
        {
            new ClientIndexWaveActor( this );
            return;
        }
        m_waveletsOpened = true;

        for( int i = 0; i < open.wavelet_id_prefix_size(); ++i )
        {
            QString waveletId = QString::fromStdString( open.wavelet_id_prefix(i) );

            int index1 = waveId.indexOf('!');
            int index2 = waveletId.indexOf('!');
            if ( index1 == -1 || index2 == -1 )
            {
                qDebug("Malformed wave id or wavelet id");
                getOffline();
                return;
            }

            WaveUrl url( waveId.left(index1), waveId.mid(index1+1), waveletId.left(index2), waveletId.mid(index2+1) );
            if ( url.isNull() )
            {
                qDebug("Malformed wave id");
                getOffline();
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
                getOffline();
                return;
            }
        }
    }
    else if ( methodName == "waveserver.ProtocolSubmitRequest" )
    {
        new ClientSubmitRequestActor( this, data );
    }
}

void ClientConnection::sendFailedSubmitResponse( const QString& errorMessage )
{
    waveserver::ProtocolSubmitResponse response;
    response.set_error_message( errorMessage.toStdString() );

    qDebug("SubmitResponse>> %s", response.DebugString().data());

    QByteArray buffer( response.ByteSize(), 0 );
    response.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolSubmitResponse", buffer.constData(), buffer.length());
}

void ClientConnection::sendSubmitResponse( const waveserver::ProtocolSubmitResponse& response )
{
    qDebug("SubmitResponse>> %s", response.DebugString().data());

    QByteArray buffer( response.ByteSize(), 0 );
    response.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolSubmitResponse", buffer.constData(), buffer.length());
}

void ClientConnection::sendWaveletUpdate( const waveserver::ProtocolWaveletUpdate& update )
{
    qDebug("WaveletUpdate>> %s", update.DebugString().data());

    QByteArray buffer( update.ByteSize(), 0 );
    update.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolWaveletUpdate", buffer.constData(), buffer.length());
}

void ClientConnection::sendIndexUpdate(const QString& waveletName, protocol::ProtocolWaveletDelta* delta)
{
    WaveUrl wurl( waveletName );
    Q_ASSERT( !wurl.isNull() );

    // Set the digest version and hash
    delta->mutable_hashed_version()->set_history_hash( m_digestHash.constData(), m_digestHash.length() );
    delta->mutable_hashed_version()->set_version( m_digestVersion );

    // TODO: Building the indexwave ID is very strange with respect to the domain used
    QUrl url;
    url.setScheme("wave");
    url.setHost( wurl.waveletDomain() );
    url.setPath( "/!indexwave/" + wurl.waveId() );

    waveserver::ProtocolWaveletUpdate update;
    update.set_wavelet_name( url.toString().toStdString() );
    update.mutable_resulting_version()->set_version( ++m_digestVersion );
    update.mutable_resulting_version()->set_history_hash( m_digestHash.constData(), m_digestHash.length() );
    update.add_applied_delta()->MergeFrom( *delta );

    qDebug("Index WaveletUpdate>> %s", update.DebugString().data());

    QByteArray buffer( update.ByteSize(), 0 );
    update.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolWaveletUpdate",buffer.constData(), buffer.length());
}

void ClientConnection::getOffline()
{
    this->deleteLater();
}

void ClientConnection::networkError()
{    
    this->deleteLater();
}

QString ClientConnection::domain() const
{
    return Settings::settings()->domain();
}

void ClientConnection::customEvent( QEvent* event )
{
    PBMessage<waveserver::ProtocolWaveletUpdate>* update = dynamic_cast<PBMessage<waveserver::ProtocolWaveletUpdate>*>(event);
    if ( update )
    {
        sendWaveletUpdate( *update );
        return;
    }
    PBMessage<messages::WaveletDigest>* digest = dynamic_cast<PBMessage<messages::WaveletDigest>*>(event);
    if ( digest )
    {
        sendIndexUpdate( QString::fromStdString( digest->wavelet_name() ), digest->mutable_digest_delta() );
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
}
