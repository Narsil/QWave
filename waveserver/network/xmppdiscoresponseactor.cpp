#include "xmppdiscoresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include <QXmlStreamWriter>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppDiscoResponseActor::XmppDiscoResponseActor(XmppVirtualConnection* con, const QString& id, XmppStanza::Kind kind)
        : XmppActor(con), m_id(id), m_kind(kind)
{
    con->addActor( this );
}

void XmppDiscoResponseActor::EXECUTE()
{
    qDebug("EXECUTE DiscoResponse");

    BEGIN_EXECUTE;

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    if ( m_kind == XmppStanza::DiscoInfo )
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "result" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("query");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#info" );
        writer.writeStartElement("identity");
        writer.writeAttribute("type", "google-wave" );
        writer.writeAttribute("name", "Heterodyne 0.1" );
        writer.writeAttribute("category", "collaboration" );
        writer.writeEndElement();
        writer.writeStartElement("feature");
        writer.writeAttribute("var", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }
    else if ( m_kind == XmppStanza::DiscoItems )
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "result" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->domain() );
        writer.writeStartElement("query");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#items" );
        writer.writeStartElement("item");
        writer.writeAttribute("jid", Settings::settings()->xmppComponentName() );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }
    else
    {
        Q_ASSERT(false);
    }

    END_EXECUTE;
}
