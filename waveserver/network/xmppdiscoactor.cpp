#include "xmppdiscoactor.h"
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

XmppDiscoActor::XmppDiscoActor(XmppVirtualConnection* con)
        : XmppActor(con)
{
    con->addActor( this );
}

void XmppDiscoActor::EXECUTE()
{
    qDebug("EXECUTE Disco");

    BEGIN_EXECUTE;

    // Wait until the component is connected
    if ( !connection()->component()->isReady() )
        yield( RecvSignal( connection()->component(), SIGNAL(ready())) );

    qDebug("Start disco ...");

    // Ask the remote server for its JIDs
    m_id = connection()->component()->nextId();
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "get" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("query");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#items" );
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    // Check that at least one JID is available.
    // TODO: The server could report multiple JIDs? Iterate them all
    yield( RecvXmpp<XmppStanza::DiscoItemsResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
    if ( REASON( RecvXmpp<XmppStanza::DiscoItemsResponse> ) )
    {
        XmppTag* query = REASON->child("query");
        if ( !query ) { XMPPERROR("Malformed query"); }

        bool found = true;
        foreach( XmppTag* tag, query->children("item") )
        {
            if ( tag->hasAttribute("jid" ) )
            {
                QString domain = (*tag)["jid"];
                connection()->setDomain( domain );
                found = true;
            }
        }
        if ( !found ) { XMPPERROR("No wave component available"); }
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
    else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }

    // Ask the remote server for wave support
    m_id = connection()->component()->nextId();
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "get" );
        writer.writeAttribute("id", m_id );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("query");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#info" );
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    qDebug("YIELD1");
    // Find out whether the remote server supports a version of wave that is acceptable
    yield( RecvXmpp<XmppStanza::DiscoInfoResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
    qDebug("YIELD2");
    if ( REASON( RecvXmpp<XmppStanza::DiscoInfoResponse> ) )
    {
        XmppTag* query = REASON->child("query");
        if ( !query ) { XMPPERROR("Malformed query"); }

        XmppTag* ident = query->child("identity");
        if ( !ident || (*ident)["type"] != "google-wave" ) { XMPPERROR("Wave is not supported"); }

        XMPPLOG("Found a wave server");
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
    else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }

    END_EXECUTE;

    connection()->setReady();
}
