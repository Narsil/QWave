#include "networkadapter.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "app/environment.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "rpc.h"

#include <QUrl>

NetworkAdapter* NetworkAdapter::s1 = 0;
NetworkAdapter* NetworkAdapter::s2 = 0;

NetworkAdapter::NetworkAdapter(QObject* parent)
        : QObject( parent )
{
    if ( !s1 )
        s1 = this;
    else
        s2 = this;

    m_rpc = new RPC(this);
    connect( m_rpc, SIGNAL(online()), SLOT(getOnline()));
    connect( m_rpc, SIGNAL(offline()), SLOT(getOffline()));
    connect( m_rpc, SIGNAL(messageReceived(QString,QByteArray)), SLOT(messageReceived(QString,QByteArray)));

    m_rpc->open("localhost", 9876);
}

#include "protocol/waveclient-rpc.pb.h"
#include <sstream>
#include <string>

void NetworkAdapter::sendOpenWave()
{
    std::ostringstream str;
    waveserver::ProtocolOpenRequest req;
    req.set_participant_id("torben@localhost");
    req.set_wave_id("!indexwave");
    req.add_wavelet_id_prefix("");
    req.SerializeToOstream(&str);

    m_rpc->send("waveserver.ProtocolOpenRequest", str.str().data(), str.str().length());
}

void NetworkAdapter::messageReceived(const QString& methodName, const QByteArray& data)
{
    if ( methodName == "waveserver.ProtocolWaveletUpdate" )
    {
        waveserver::ProtocolWaveletUpdate update;
        update.ParseFromArray(data.constData(), data.length());
        qDebug("msg>> %s", update.DebugString().data());

        QUrl url( QString::fromStdString( update.wavelet_name() ) );
        if ( url.path().left(12) == "/!indexwave/" )
        {
            QString waveid = url.path().mid(12);
            Wave* wave = environment()->wave(waveid);
            if ( !wave )
            {
                wave = environment()->createWave(waveid);
                environment()->inbox()->addWave(wave);
            }
        }
    }
}

void NetworkAdapter::send( const DocumentMutation& mutation, const QString& waveletId, const QString& docId )
{
    if ( this == s1 )
        s2->receive(mutation, waveletId, docId);
    else
        s1->receive(mutation, waveletId, docId);
}

void NetworkAdapter::receive( const DocumentMutation& mutation, const QString& waveletId, const QString& docId )
{
    Wavelet* wavelet = environment()->wavelet(waveletId);
    if (wavelet)
    {
        Blip* blip = wavelet->blip(docId);
        if ( blip )
        {
            blip->receive(mutation);
        }
    }
}

Environment* NetworkAdapter::environment() const
{
    return (Environment*)parent();
}

void NetworkAdapter::getOnline()
{
    sendOpenWave();
}

void NetworkAdapter::getOffline()
{
}
