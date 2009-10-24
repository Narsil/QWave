#include "networkadapter.h"
#include "model/documentmutation.h"
#include "model/wavelist.h"
#include "model/wave.h"
#include "app/environment.h"
#include "model/wavelet.h"
#include "model/blip.h"
#include "model/participant.h"
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

            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                const protocol::ProtocolWaveletDelta& delta = update.applied_delta(i);
                for( int o = 0; o < delta.operation_size(); ++o )
                {
                    const protocol::ProtocolWaveletOperation op = delta.operation(o);
                    if ( op.has_add_participant() )
                    {
                        // TODO: Do not create the same guy twice
                        Participant* p = new Participant(QString::fromStdString(op.add_participant()));
                        wave->wavelet()->addParticipant(p);
                    }
                    if ( op.has_remove_participant() )
                    {
                        Participant* p = wave->wavelet()->participant(QString::fromStdString(op.remove_participant()));
                        if ( p )
                            wave->wavelet()->removeParticipant(p);
                    }
                    if ( op.has_mutate_document() )
                    {
                        DocumentMutation m;
                        const protocol::ProtocolWaveletOperation_MutateDocument& mut = op.mutate_document();
                        if ( mut.has_document_id() && mut.document_id() == "digest" )
                        {
                            const protocol::ProtocolDocumentOperation& dop = mut.document_operation();
                            for( int c = 0; c < dop.component_size(); ++c )
                            {
                                const protocol::ProtocolDocumentOperation_Component& comp = dop.component(c);
                                if ( comp.has_characters() )
                                {
                                    QString chars = QString::fromStdString(comp.characters());
                                    m.insertChars(chars);
                                }
                                else if ( comp.has_retain_item_count() )
                                {
                                    int retain = comp.retain_item_count();
                                    m.retain(retain);
                                }
                                else
                                {
                                    // TODO
                                }
                            }
                        }
                        wave->mutateDigest(m);
                    }
                }
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
