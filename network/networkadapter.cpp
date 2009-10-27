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
#include "rpc.h"

#include <QUrl>

// Includes for protobuf
#include "protocol/waveclient-rpc.pb.h"
#include <sstream>
#include <string>

/**
  * Convert the protocol buffer representation to the Qt-ified representation.
  */
WaveletDelta convert( const protocol::ProtocolWaveletDelta& delta )
{
    WaveletDelta result;
    result.setAuthor( QString::fromStdString(delta.author()));
    result.version().version = delta.hashed_version().version();
    std::string hash = delta.hashed_version().history_hash();
    result.version().hash = QByteArray( hash.data(), hash.length() );

    for( int o = 0; o < delta.operation_size(); ++o )
    {
        const protocol::ProtocolWaveletOperation op = delta.operation(o);
        if ( op.has_add_participant() )
        {
            WaveletDeltaOperation wo;
            wo.setAddParticipant( QString::fromStdString(op.add_participant()));
            result.addOperation(wo);
        }
        if ( op.has_remove_participant() )
        {
            WaveletDeltaOperation wo;
            wo.setRemoveParticipant( QString::fromStdString(op.remove_participant()));
            result.addOperation(wo);
        }
        if ( op.has_mutate_document() )
        {
            DocumentMutation m;
            const protocol::ProtocolWaveletOperation_MutateDocument& mut = op.mutate_document();
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
                else if ( comp.has_delete_characters() )
                {
                    QString chars = QString::fromStdString(comp.delete_characters());
                    m.deleteChars(chars);
                }
                else if ( comp.has_element_end() )
                {
                    m.insertEnd();
                }
                else if ( comp.has_element_start() )
                {
                    QString type = QString::fromStdString(comp.element_start().type());
                    QHash<QString,QString> attribs;
                    for( int a = 0; a < comp.element_start().attribute_size(); ++a )
                        attribs[ QString::fromStdString(comp.element_start().attribute(a).key()) ] = QString::fromStdString(comp.element_start().attribute(a).value() );
                    m.insertStart(type, attribs );
                }
                else if ( comp.has_annotation_boundary() )
                {
                    QList<QString> endKeys;
                    QHash<QString,QString> changes;
                    for( int e = 0; e < comp.annotation_boundary().end_size(); ++e )
                        endKeys.append( QString::fromStdString( comp.annotation_boundary().end(e) ) );
                    for( int a = 0; a < comp.annotation_boundary().change_size(); ++a )
                    {
                        if ( comp.annotation_boundary().change(a).has_new_value() )
                            changes[ QString::fromStdString(comp.annotation_boundary().change(c).key()) ] = QString::fromStdString(comp.annotation_boundary().change(c).new_value() );
                        else
                            changes[ QString::fromStdString(comp.annotation_boundary().change(c).key()) ] = QString::null;
                    }
                    m.annotationBoundary(endKeys, changes);
                }
                else
                {
                    qDebug("Oooops, yet unsupported operation");
                    // TODO
                }
            }
            WaveletDeltaOperation wo;
            wo.setMutation(m);
            wo.setDocumentId( QString::fromStdString( mut.document_id() ) );
            result.addOperation(wo);
        }
    }
    return result;
}

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
        // Extract the wave name
        QString path = url.path();
        int index = path.indexOf('/', 1);
        if ( path.length() < 4 || path[0] != '/' || index == -1 )
        {
            qDebug("URL too short");
            return;
        }
        QString waveid = path.mid(1, index - 1);
        QString waveletid = path.mid(index+1);

        if ( waveid == "/!indexwave/" )
        {
            Wave* wave = environment()->wave(waveid);
            // If the wave does not yet exist, then create it in the inbox
            if ( !wave )
            {
                wave = environment()->createWave(waveid);
                environment()->inbox()->addWave(wave);
            }

            // Apply all updates to the wave digest
            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                WaveletDelta wd = convert( update.applied_delta(i) );
                wave->updateDigest(wd);
            }
        }
        else
        {
            Wavelet* wavelet = environment()->wavelet(waveletid);
            if ( !wavelet )
                return;
            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                wavelet->processor()->handleReceive( convert( update.applied_delta(i) ) );
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
