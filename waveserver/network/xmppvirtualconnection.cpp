#include "network/xmppvirtualconnection.h"
#include "network/xmppdiscoactor.h"
#include "network/xmppwaveletupdateactor.h"
#include "network/xmppdiscoresponseactor.h"
#include "network/xmppsignerresponseactor.h"
#include "network/xmpphistoryresponseactor.h"
#include "network/xmppsubmitresponseactor.h"
#include "network/xmpppostsignerresponseactor.h"
#include "network/xmppwaveletupdateresponseactor.h"
#include "network/xmppsubmitrequestactor.h"
#include "network/xmppcomponentconnection.h"
#include "app/settings.h"
#include "model/appliedwaveletdelta.h"
#include "model/signedwaveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/certificatestore.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/waveurl.h"
#include "network/servercertificate.h"
#include "actor/pbmessage.h"

#include <QByteArray>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QCryptographicHash>
#include <QtGlobal>
#include <QDateTime>

XmppVirtualConnection::XmppVirtualConnection( XmppComponentConnection* connection, const QString& domain, bool resolve )
        : ActorGroup( domain, connection ), m_component( connection ), m_domain( domain ), m_postedSigner(false), m_ready(false)
{
    if ( !resolve )
        setReady();
    else
        new XmppDiscoActor(this);
}

XmppVirtualConnection::~XmppVirtualConnection()
{
}

void XmppVirtualConnection::setDomain( const QString& domain )
{
    m_component->renameVirtualConnection( this, m_domain, domain );
    m_domain = domain;
}

bool XmppVirtualConnection::send( const QString& stanza )
{
    return m_component->send( stanza );
}

void XmppVirtualConnection::setReady()
{
    qDebug("Starting service");
    m_ready = true;
    emit ready();
}

void XmppVirtualConnection::dispatch( const QSharedPointer<IMessage>& message )
{
    if ( m_ready )
    {
        XmppStanza* stanza = dynamic_cast<XmppStanza*>( message.data() );
        if ( stanza )
        {
            switch( stanza->kind() )
            {
                case XmppStanza::WaveletUpdate:
                    new XmppWaveletUpdateResponseActor( this, message.dynamicCast<XmppStanza>() );
                    return;
                case XmppStanza::HistoryRequest:
                    new XmppHistoryResponseActor( this, message.dynamicCast<XmppStanza>() );
                    return;
                case XmppStanza::SignerRequest:
                    new XmppSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    return;
                case XmppStanza::PostSigner:
                    new XmppPostSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    return;
                case XmppStanza::SubmitRequest:
                    new XmppSubmitResponseActor( this, message.dynamicCast<XmppStanza>() );
                    return;
                case XmppStanza::DiscoInfo:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoInfo );
                    return;
                case XmppStanza::DiscoItems:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoItems );
                    return;
                default:
                    qDebug("Dispatch %i", stanza->kind() );
                    break;
            }
        }
        PBMessage<waveserver::ProtocolSubmitRequest>* submitMsg = dynamic_cast< PBMessage<waveserver::ProtocolSubmitRequest>* >( message.data() );
        if ( submitMsg )
        {
            new XmppSubmitRequestActor( this, message.dynamicCast<PBMessage<waveserver::ProtocolSubmitRequest> >() );
        }
    }

    this->ActorGroup::dispatch( message );
}

void XmppVirtualConnection::sendWaveletUpdate(const QString& waveletName, const AppliedWaveletDelta& waveletDelta)
{
    new XmppWaveletUpdateActor( this, waveletName, waveletDelta );
}

void XmppVirtualConnection::xmppError()
{
    qDebug("ERROR talking to servers %s", m_domain.toAscii().constData() );
    m_ready = false;
}
