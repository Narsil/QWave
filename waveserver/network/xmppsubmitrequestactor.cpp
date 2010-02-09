#include "xmppsubmitrequestactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "actor/recvxmpp.h"
#include "actor/timeout.h"
#include "model/appliedwaveletdelta.h"
#include "network/servercertificate.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/certificatestore.h"
#include <QXmlStreamWriter>
#include <QList>
#include <QByteArray>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }


XmppSubmitRequestActor::XmppSubmitRequestActor(XmppVirtualConnection* con, const WaveUrl& url, const protocol::ProtocolWaveletDelta& delta)
        : XmppActor(con), m_url( url ), m_delta(delta)
{
    con->addActor( this );
}

void XmppSubmitRequestActor::EXECUTE()
{
    qDebug("EXECUTE WaveletUpdateResponse");

    BEGIN_EXECUTE;

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Post signer information before sending the first submit request
    if ( !connection()->hasPostedSigner() )
    {
        // Send the post-signer message
        {
            m_id = connection()->component()->nextId();
            QString send;
            QXmlStreamWriter writer( &send );
            writer.writeStartElement("iq");
            writer.writeAttribute("type", "set" );
            writer.writeAttribute("id", m_id );
            writer.writeAttribute("to", connection()->domain() );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("pubsub");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
            writer.writeStartElement("publish");
            writer.writeAttribute("node", "signer" );
            writer.writeStartElement("item");
            writer.writeStartElement("signature");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("domain", connection()->domain() );
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
            connection()->send( send );
        }

        // Wait for the receipt
        yield( RecvXmpp<XmppStanza::PostSignerResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
        if ( REASON( RecvXmpp<XmppStanza::PostSignerResponse> ) )
        {
            XMPPLOG("Got receipt for post signature");
            // Do nothing by intention. We got the receipt. Fine.
        }
        else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
        else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }
    }

    // Send the submit-request
    {
        m_id = connection()->component()->nextId();
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "set" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("pubsub");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
        writer.writeStartElement("publish");
        writer.writeAttribute("node", "wavelet" );
        writer.writeStartElement("item");
        writer.writeStartElement("submit-request");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("wavelet-name", m_url.toString() );
        writer.writeStartElement("delta");
        writer.writeCDATA( m_delta.toBase64() );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    // Wait for the receipt
    yield( RecvXmpp<XmppStanza::SubmitResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
    if ( REASON( RecvXmpp<XmppStanza::SubmitResponse> ) )
    {
        XMPPLOG("Got receipt for submit request");
        // Do nothing by intention. We got the receipt. Fine.
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
    else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }

    END_EXECUTE;
}
