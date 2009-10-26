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

// Includes for protobuf
#include "protocol/waveclient-rpc.pb.h"
#include <sstream>
#include <string>

NetworkAdapter* NetworkAdapter::s1 = 0;
NetworkAdapter* NetworkAdapter::s2 = 0;

NetworkAdapter::NetworkAdapter(QObject* parent)
        : QObject( parent ), m_isOnline(false)
{
    if ( !s1 )
        s1 = this;
    else
        s2 = this;

    m_rpc = 0;
}

void NetworkAdapter::setServer( const QString& serverName, quint32 serverPort )
{
    if ( m_rpc )
        delete m_rpc;

    m_serverName = serverName;
    m_serverPort = serverPort;
    m_rpc = new RPC(this);
    connect( m_rpc, SIGNAL(online()), SLOT(getOnline()));
    connect( m_rpc, SIGNAL(offline()), SLOT(getOffline()));
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

    qDebug("msg>> %s", req.DebugString().data());

    m_rpc->send("waveserver.ProtocolOpenRequest", str.str().data(), str.str().length());
}

void NetworkAdapter::sendAddParticipant(Wavelet* wavelet, Participant* participant)
{
    QString waveId = wavelet->wave()->id();
    QString waveletId = wavelet->id();
    waveserver::ProtocolSubmitRequest req;
    QUrl url;
    url.setScheme("wave");
    url.setHost(m_serverName);
    url.setPath(waveId + "/" + waveletId);
    req.set_wavelet_name( url.toString().toStdString() );
    protocol::ProtocolWaveletDelta* delta = req.mutable_delta();
    delta->set_author(environment()->localUser()->address().toStdString());
    delta->mutable_hashed_version()->set_version(0);
    delta->mutable_hashed_version()->set_history_hash(url.toString().toStdString());
    protocol::ProtocolWaveletOperation* op = delta->add_operation();
    op->set_add_participant(participant->address().toStdString());

    std::ostringstream str;
    req.SerializeToOstream(&str);

    qDebug("msg>> %s", req.DebugString().data());

    m_rpc->send("waveserver.ProtocolSubmitRequest", str.str().data(), str.str().length());
}

void NetworkAdapter::messageReceived(const QString& methodName, const QByteArray& data)
{
    if ( methodName == "waveserver.ProtocolWaveletUpdate" )
    {
        waveserver::ProtocolWaveletUpdate update;
        update.ParseFromArray(data.constData(), data.length());
        qDebug("msg<< %s", update.DebugString().data());

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
    if ( this == s1 && s2 )
        s2->receive(mutation, waveletId, docId);
    else if ( this == s2 && s1 )
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
    m_isOnline = true;
    sendOpenWave("!indexwave", "");
}

void NetworkAdapter::getOffline()
{
    m_isOnline = false;
}
