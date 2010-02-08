#include "xmppvirtualconnection.h"
#include "xmppdiscoactor.h"
#include "xmppwaveletupdateactor.h"
#include "xmppdiscoresponseactor.h"
#include "xmppsignerresponseactor.h"
#include "xmpphistoryresponseactor.h"
#include "xmppsubmitresponseactor.h"
#include "xmpppostsignerresponseactor.h"
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

#include <QByteArray>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QCryptographicHash>
#include <QtGlobal>
#include <QDateTime>

XmppVirtualConnection::XmppVirtualConnection( XmppComponentConnection* connection, const QString& domain, bool resolve )
        : ActorGroup( connection ), m_state( resolve ? Init : Established ), m_component( connection ), m_domain( domain ), m_signerInfoSent(false), m_ready(false)
{
    if ( !resolve )
        setReady();
    else
        new XmppDiscoActor(this);
}

XmppVirtualConnection::~XmppVirtualConnection()
{
    foreach( XmppStanza* stanza, m_stanzaQueue )
    {
        delete stanza;
    }
}

void XmppVirtualConnection::processMessage( const XmppStanza& stanza )
{
    if ( stanza["type"] == "error" )
    {
        xmppError();
        return;
    }

    if ( stanza["type"] == "normal" )
    {
        XmppTag* event = stanza.child("event");
        XmppTag* items = event ? event->child("items") : 0;
        XmppTag* item = items ? items->child("item") : 0;
        XmppTag* update = item ? item->child("wavelet-update") : 0;
        XmppTag* applied = update ? update->child("applied-delta") : 0;
        XmppTag* text = applied ? applied->childAt(0) : 0;
        if ( text && ( text->isCData() || text->isText() ) )
        {
            WaveUrl url( update->attribute("wavelet-name") );
            if ( url.isNull() )
            {
                qDebug("Malformed wavelet name %s", url.toString().toAscii().constData() );
                xmppError();
                return;
            }

            bool ok;
            SignedWaveletDelta delta = SignedWaveletDelta::fromBase64( text->text(), &ok );
            if ( !ok )
            {
                qDebug("Could not deserialize signed wavelet delta.");
                xmppError();
                return;
            }

            if ( delta.signatures().count() == 0 )
            {
                qDebug("No signature in SignedWaveltDelta");
                xmppError();
                return;
            }

            Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
            if ( !wave )
            {
                qDebug("Unknown wave");
                xmppError();
                return;
            }
            Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), true );
            if ( !wavelet )
            {
                qDebug("Unknown wavelet");
                xmppError();
                return;
            }

            // Send a response
            QString sendStr;
            {
                QXmlStreamWriter writer( &sendStr );
                writer.writeStartElement("message");
                writer.writeAttribute("id", stanza["id"] );
                writer.writeAttribute("to", m_domain );
                writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                writer.writeStartElement("received");
                writer.writeAttribute("xmlns", "urn:xmpp:receipts" );
                writer.writeEndElement();
                writer.writeEndElement();
            }
            m_component->send( sendStr );

            // Are all signers known ?
            foreach( const Signature& sig, delta.signatures() )
            {
                const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
                if ( !cert )
                {
                    // TODO Ask for signer info, i.e. get a certificate for this signer ID.
                }
            }

            // We are missing some deltas? Then issue a history-request
            if ( delta.delta().version().version > wavelet->version() )
            {
                // TODO: return
            }

            QString err;
            // TODO: What if there are multuple signatures
            int version = wavelet->apply( delta.delta(), &err, &delta.signatures()[0] );
            if ( version == -1 || !err.isEmpty() )
            {
                qDebug("Error applying delta: %s", err.toAscii().constData() );
                xmppError();
                return;
            }
        }
        else
            qDebug("... message unhandled");
    }
    else
        qDebug("... message unhandled");
}

void XmppVirtualConnection::sendSubmitRequest( const WaveUrl& url, const protocol::ProtocolWaveletDelta& delta )
{
    // Post signer information before sending the first submit request
    if ( !m_signerInfoSent )
    {
        m_signerInfoSent = true;
        QString sendStr;
        {
            QXmlStreamWriter writer( &sendStr );
            writer.writeStartElement("iq");
            writer.writeAttribute("type", "set" );
            writer.writeAttribute("id", m_component->nextId() );
            writer.writeAttribute("to", m_domain );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("pubsub");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
            writer.writeStartElement("publish");
            writer.writeAttribute("node", "signer" );
            writer.writeStartElement("item");
            writer.writeStartElement("signature");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("domain", m_domain );
            writer.writeAttribute("algorithm", "SHA256" );
            QList<QByteArray> certificates = LocalServerCertificate::certificate()->toBase64();
            foreach(QByteArray ba, certificates )
            {
                writer.writeStartElement("certificate");
                QString str = QString::fromAscii(ba);
                QStringList lst = str.split('\n', QString::SkipEmptyParts );
                lst.removeFirst();
                lst.removeLast();
                writer.writeCDATA( lst.join("") );
                writer.writeEndElement();
            }
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
        }
        m_component->send( sendStr );
    }

    SignedWaveletDelta sdelta( delta );
    QString str64 = sdelta.toBase64();

    QString sendStr;
    {
        QXmlStreamWriter writer( &sendStr );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "set" );
        writer.writeAttribute("id", m_component->nextId() );
        writer.writeAttribute("to", m_domain );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("pubsub");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
        writer.writeStartElement("publish");
        writer.writeAttribute("node", "wavelet" );
        writer.writeStartElement("item");
        writer.writeStartElement("submit-request");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("wavelet-name", url.toString() );
        writer.writeStartElement("delta");
        writer.writeCDATA( str64 );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
    }
    m_component->send( sendStr );

    // TODO: Wait for the response
}

void XmppVirtualConnection::send( XmppStanza* stanza )
{
    switch( m_state )
    {
        case Init:
        case DiscoItems:
            m_stanzaQueue.enqueue(stanza);
            break;
        case Established:
            m_component->send( *stanza );
            delete stanza;
            break;
        case Delete:
        case Error:
            // Do nothing by intention
            break;
    }
}

////////
/// NEW
////////

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
                    // TODO
                    break;
                case XmppStanza::HistoryRequest:
                    new XmppHistoryResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::SignerRequest:
                    new XmppSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::PostSigner:
                    new XmppPostSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::SubmitRequest:
                    new XmppSubmitResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::DiscoInfo:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoInfo );
                    break;
                case XmppStanza::DiscoItems:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoItems );
                    break;
                default:
                    break;
            }
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

