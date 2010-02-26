#include "xmppwaveletupdateactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvxmpp.h"
#include "actor/timeout.h"
#include "actor/recvsignal.h"
#include <QXmlStreamWriter>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppWaveletUpdateActor::XmppWaveletUpdateActor(XmppVirtualConnection* con, PBMessage<messages::WaveletUpdate>* event)
        : XmppActor(con)
{
    m_waveletName = QString::fromStdString( event->wavelet_name() );
    QByteArray ba( event->applied_delta().data(), event->applied_delta().length() );
    m_base64 = QString::fromAscii( ba.toBase64() );
}

void XmppWaveletUpdateActor::execute()
{
    qDebug("EXECUTE WaveletUpdateActor");

    BEGIN_EXECUTE;

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    qDebug("Start send wavelet update ...");

    // Send the wavelet update to the remote server
    m_id = connection()->component()->nextId();
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("message");
        writer.writeAttribute("type", "normal" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("request");
        writer.writeAttribute("xmlns", "urn:xmpp:receipts" );
        writer.writeEndElement();
        writer.writeStartElement("event");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub#event" );
        writer.writeStartElement("items");
        writer.writeStartElement("item");
        writer.writeStartElement("wavelet-update");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("wavelet-name", m_waveletName );
        writer.writeStartElement("applied-delta");
        writer.writeCDATA( m_base64 );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    // Wait for the receipt
    yield( RecvXmpp<XmppStanza::MessageReceipt>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
    if ( REASON( RecvXmpp<XmppStanza::MessageReceipt> ) )
    {
        XMPPLOG("Got receipt for wavelet-update");
        // Do nothing by intention. We got the receipt. Fine.
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
    else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }

    END_EXECUTE;
}
