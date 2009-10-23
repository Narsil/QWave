#include "networkadapter.h"
#include "model/documentmutation.h"
#include "app/environment.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "rpc.h"

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
    m_rpc->open("localhost", 9876);
}

#include "protocol/waveclient-rpc.pb.h"
#include <sstream>

void NetworkAdapter::sendOpenWave()
{
    std::ostringstream str;
    waveserver::ProtocolOpenRequest req;
    req.set_participant_id("deps@localhost");
    req.set_wave_id("!indexwave");
    req.add_wavelet_id_prefix("");
    req.SerializeToOstream(&str);

    m_rpc->send("waveserver.ProtocolOpenRequest", str.str().data(), str.str().length());
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
