#include "networkadapter.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "model/wave.h"
#include "app/environment.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/participant.h"
#include "model/waveletdelta.h"
#include "model/otprocessor.h"
#include "model/wavedigest.h"
#include "model/waveurl.h"
#include "rpc.h"
#include "converter.h"

// Includes for protobuf
#include "waveclient-rpc.pb.h"
#include <sstream>
#include <string>

NetworkAdapter::NetworkAdapter(QObject* parent)
        : QNetworkAccessManager( parent ), m_rpc(0), m_isOnline(false), m_hasConnectionError(false)
{
}

void NetworkAdapter::setServer( const QString& serverName, quint32 serverPort )
{
    if ( m_rpc )
        delete m_rpc;

    emit connectionStatus( tr("Connecting ...") );

    m_serverName = serverName;
    m_serverPort = serverPort;
    m_rpc = new RPC(this);
    connect( m_rpc, SIGNAL(online()), SLOT(getOnline()));
    connect( m_rpc, SIGNAL(offline()), SLOT(getOffline()));
    connect( m_rpc, SIGNAL(socketError()), SLOT(networkError()));
    connect( m_rpc, SIGNAL(messageReceived(QString,QByteArray)), SLOT(messageReceived(QString,QByteArray)));

    m_rpc->open(serverName, serverPort);
}

bool NetworkAdapter::openWavelet(Wavelet* wavelet)
{
    QString waveId = wavelet->wave()->domain() + "!" + wavelet->wave()->id();
    QString waveletId = wavelet->domain() + "!" + wavelet->id();
    QString id = waveId + "/" + waveletId;
    if ( m_openWaves.contains(id) )
        return true;

    if ( m_rpc && m_isOnline )
    {
        sendOpenWave(waveId, waveletId);
        m_openWaves.append(id);
        return true;
    }
    return false;
}

void NetworkAdapter::sendOpenWave(const QString& waveId, const QString& waveletId)
{
    std::ostringstream str;
    waveserver::ProtocolOpenRequest req;
    req.set_participant_id( environment()->localUser()->address().toStdString());
    req.set_wave_id(waveId.toStdString());
    req.add_wavelet_id_prefix(waveletId.toStdString());
    req.SerializeToOstream(&str);

    qDebug("OpenRequest>> %s", req.DebugString().data());

    m_rpc->send("waveserver.ProtocolOpenRequest", str.str().data(), str.str().length());
}

void NetworkAdapter::submit(const WaveletDelta& delta, Wavelet* wavelet)
{
    QString waveId = wavelet->wave()->id();
    QString waveletId = wavelet->id();
    waveserver::ProtocolSubmitRequest req;
    req.set_wavelet_name( wavelet->url().toString().toStdString() );
    protocol::ProtocolWaveletDelta* d = req.mutable_delta();
    Converter::convert( d, delta );

    std::ostringstream str;
    req.SerializeToOstream(&str);

    qDebug("SubmitRequest>> %s", req.DebugString().data());

    m_rpc->send("waveserver.ProtocolSubmitRequest", str.str().data(), str.str().length());
}

void NetworkAdapter::messageReceived(const QString& methodName, const QByteArray& data)
{
    if ( methodName == "waveserver.ProtocolWaveletUpdate" )
    {
        waveserver::ProtocolWaveletUpdate update;
        update.ParseFromArray(data.constData(), data.length());
        qDebug("msg<< %s", update.DebugString().data());

        WaveUrl url( QString::fromStdString( update.wavelet_name() ) );
//        // Extract the wave name
//        QString path = url.path();
//        int index = path.indexOf('/', 1);
//        if ( path.length() < 4 || path[0] != '/' || index == -1 )
//        {
//            qDebug("URL too short");
//            return;
//        }
//        QString waveid = path.mid(1, index - 1);
//        QString waveletid = path.mid(index+1);

        // Digest?
        if ( url.waveId() == "!indexwave" )
        {
            Wave* wave = environment()->wave(url.waveDomain(), url.waveletId());
            // If the wave does not yet exist, then create it in the inbox
            if ( !wave )
            {
                wave = environment()->createWave(url.waveDomain(), url.waveletId());
                environment()->inbox()->addWave(wave);
            }

            // Apply all updates to the wave digest
            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                WaveletDelta wd = Converter::convert( update.applied_delta(i) );
                wave->digest()->processor()->handleReceive( wd );
            }
        }
        // Wavelet?
        else
        {
            Wavelet* wavelet = environment()->wavelet(url.toString());
            if ( !wavelet )
                return;

            for( int i = 0; i < update.applied_delta_size(); ++i )
            {                
                WaveletDelta wd = Converter::convert( update.applied_delta(i) );
                wavelet->processor()->handleReceive( wd );
            }

            QByteArray resultingHash( update.resulting_version().history_hash().data(), update.resulting_version().history_hash().length() );
            int resultingVersion = update.resulting_version().version();
            wavelet->processor()->setResultingHash( resultingVersion, resultingHash);
        }
    }
    else if ( methodName == "waveserver.ProtocolSubmitResponse" )
    {
        waveserver::ProtocolSubmitResponse response;
        response.ParseFromArray(data.constData(), data.length());
        qDebug("msg<< %s", response.DebugString().data());
    }
}

Environment* NetworkAdapter::environment() const
{
    return (Environment*)parent();
}

void NetworkAdapter::getOnline()
{
    emit connectionStatus( tr("Online") );

    m_isOnline = true;
    m_hasConnectionError = false;
    sendOpenWave("!indexwave", "");
}

void NetworkAdapter::getOffline()
{
    emit connectionStatus( tr("Offline") );

    m_isOnline = false;
}

void NetworkAdapter::networkError()
{
    emit connectionStatus( tr("Connection to server is broken") );
    m_hasConnectionError = true;
}
