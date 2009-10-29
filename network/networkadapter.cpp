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
#include "rpc.h"

#include <QUrl>

// Includes for protobuf
#include "protocol/waveclient-rpc.pb.h"
#include <sstream>
#include <string>

/**
  * Convert the Qt-ified representation to the protocol buffer representation.
  */
void convert(protocol::ProtocolWaveletDelta* result, const WaveletDelta& delta )
{
    // Author
    result->set_author(delta.author().toStdString());
    // HashedVersion
    result->mutable_hashed_version()->set_version( delta.version().version );
    result->mutable_hashed_version()->set_history_hash( std::string( delta.version().hash.constData(), delta.version().hash.length() ) );

    for( int i = 0; i < delta.operations().length(); ++i )
    {
        const WaveletDeltaOperation& op = delta.operations()[i];
        protocol::ProtocolWaveletOperation* o = result->add_operation();
        if ( op.hasAddParticipant() )
            o->set_add_participant( op.addParticipant().toStdString() );
        if ( op.hasRemoveParticipant() )
            o->set_remove_participant( op.removeParticipant().toStdString() );
        if ( op.hasMutation() )
        {
            const DocumentMutation* mutation = op.mutation();
            protocol::ProtocolWaveletOperation_MutateDocument* m = o->mutable_mutate_document();
            m->set_document_id( op.documentId().toStdString() );
            protocol::ProtocolDocumentOperation* mo = m->mutable_document_operation();
            for( QList<DocumentMutation::Item>::const_iterator it = mutation->begin(); it != mutation->end(); ++it )
            {
                protocol::ProtocolDocumentOperation_Component* comp = mo->add_component();
                switch( (*it).type )
                {
                    case DocumentMutation::ElementStart:
                        comp->mutable_element_start()->set_type( (*it).text.toStdString() );
                        if ( (*it).map )
                            foreach( QString key, (*it).map->keys() )
                            {
                                protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = comp->mutable_element_start()->add_attribute();
                                pair->set_key( key.toStdString() );
                                pair->set_value( ((*it).map)->value(key).toStdString() );
                            }
                        break;
                    case DocumentMutation::ElementEnd:
                        comp->set_element_end(true);
                        break;
                    case DocumentMutation::Retain:
                        comp->set_retain_item_count((*it).count);
                        break;
                    case DocumentMutation::InsertChars:
                        comp->set_characters( (*it).text.toStdString() );
                        break;
                    case DocumentMutation::DeleteStart:
                        comp->mutable_delete_element_start()->set_type( (*it).text.toStdString() );
                        if ( (*it).map )
                            foreach( QString key, (*it).map->keys() )
                            {
                                protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = comp->mutable_element_start()->add_attribute();
                                pair->set_key( key.toStdString() );
                                pair->set_value( ((*it).map)->value(key).toStdString() );
                            }
                        break;
                    case DocumentMutation::DeleteEnd:
                        comp->set_delete_element_end(true);
                        break;
                    case DocumentMutation::DeleteChars:
                        comp->set_delete_characters( (*it).text.toStdString() );
                        break;
                    case DocumentMutation::AnnotationBoundary:                        
                        if ( (*it).map )
                            foreach( QString key, (*it).map->keys() )
                            {
                                protocol::ProtocolDocumentOperation_Component_KeyValueUpdate* pair = comp->mutable_annotation_boundary()->add_change();
                                pair->set_key( key.toStdString() );
                                pair->set_new_value( ((*it).map)->value(key).toStdString() );
                                // TODO: Set old value ...
                            }
                        if ( (*it).endKeys )
                        {
                            foreach( QString ek, *((*it).endKeys) )
                            {
                                comp->mutable_annotation_boundary()->add_end( ek.toStdString() );
                            }
                        }
                        break;
                    case DocumentMutation::NoItem:
                        // Do nothing by intention
                        break;
                }
            }
        }
    }
}

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

    qDebug("OpenRequest>> %s", req.DebugString().data());

    m_rpc->send("waveserver.ProtocolOpenRequest", str.str().data(), str.str().length());
}

//void NetworkAdapter::sendAddParticipant(Wavelet* wavelet, Participant* participant)
//{
//    QString waveId = wavelet->wave()->id();
//    QString waveletId = wavelet->id();
//    waveserver::ProtocolSubmitRequest req;
//    QUrl url;
//    url.setScheme("wave");
//    url.setHost(m_serverName);
//    url.setPath(waveId + "/" + waveletId);
//    req.set_wavelet_name( url.toString().toStdString() );
//    protocol::ProtocolWaveletDelta* delta = req.mutable_delta();
//    delta->set_author(environment()->localUser()->address().toStdString());
//    delta->mutable_hashed_version()->set_version(0);
//    delta->mutable_hashed_version()->set_history_hash(url.toString().toStdString());
//    protocol::ProtocolWaveletOperation* op = delta->add_operation();
//    op->set_add_participant(participant->address().toStdString());
//
//    std::ostringstream str;
//    req.SerializeToOstream(&str);
//
//    qDebug("SubmitRequest>> %s", req.DebugString().data());
//
//    m_rpc->send("waveserver.ProtocolSubmitRequest", str.str().data(), str.str().length());
//}

void NetworkAdapter::submit(const WaveletDelta& delta, Wavelet* wavelet)
{
    QString waveId = wavelet->wave()->id();
    QString waveletId = wavelet->id();
    waveserver::ProtocolSubmitRequest req;
    QUrl url;
    url.setScheme("wave");
    url.setHost(m_serverName);
    url.setPath(waveId + "/" + waveletId);
    req.set_wavelet_name( url.toString().toStdString() );
    protocol::ProtocolWaveletDelta* d = req.mutable_delta();
    convert( d, delta );

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

        // Digest?
        if ( waveid == "!indexwave" )
        {
            Wave* wave = environment()->wave(waveletid);
            // If the wave does not yet exist, then create it in the inbox
            if ( !wave )
            {
                wave = environment()->createWave(waveletid);
                environment()->inbox()->addWave(wave);
            }

            // Apply all updates to the wave digest
            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                wave->digest()->processor()->handleReceive( convert( update.applied_delta(i) ) );
            }
        }
        // Wavelet?
        else
        {
            Wavelet* wavelet = environment()->wavelet(waveletid);
            if ( !wavelet )
                return;

            for( int i = 0; i < update.applied_delta_size(); ++i )
            {
                WaveletDelta wd = convert( update.applied_delta(i) );
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
