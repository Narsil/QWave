#include "xmpphistoryresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"
#include "protocol/messages.pb.h"
#include <QXmlStreamWriter>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppHistoryResponseActor::XmppHistoryResponseActor(XmppVirtualConnection* con, XmppStanza* stanza)
        : XmppActor(con), m_stanza(*stanza)
{
}

void XmppHistoryResponseActor::execute()
{
    qDebug("EXECUTE SignerResponse");

    BEGIN_EXECUTE;

    // Analyze the request
    {
        XmppTag* pubsub = m_stanza.child("pubsub");
        XmppTag* items = pubsub ? pubsub->child( "items" ) : 0;
        XmppTag* history = items ? items->child("delta-history") : 0;
        if ( !history ) { XMPPERROR("Malformed delta-hiistory request"); }

        // TODO: Inspect the hashes that are being sent
        bool ok = true;
        m_start = history->attribute("start-version").toLong(&ok);
        if ( !ok || m_start < 0) { XMPPERROR("start-version missing"); }

        m_end = history->attribute("end-version").toLong(&ok);
        if ( !ok || m_end < 0 ) { XMPPERROR("end-version missing"); }

        QString waveletName = history->attribute("wavelet-name");
        m_url = WaveUrl( waveletName );
        if ( m_url.isNull() ) { XMPPERROR("Malformed wavelet-name"); }
    }

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Send a query to the database asking for the deltas
    {
        m_msgId = nextId();
        PBMessage<messages::QueryWaveletUpdates>* query = new PBMessage<messages::QueryWaveletUpdates>( ActorId("store", m_url.toString() ), m_msgId );
        query->setCreateOnDemand( true );
        query->set_wavelet_name(m_url.toString().toStdString() );
        query->set_start_version( m_start );
        query->set_end_version( m_end );
        bool ok = post( query );
        if ( !ok ) { XMPPERROR("Internal server error. Could not talk to database."); }
    }

    // Wait for a response from the database
    yield( RecvPB<messages::QueryWaveletUpdatesResponse>(m_msgId) | Timeout(10000) );
    if ( REASON(RecvPB<messages::QueryWaveletUpdatesResponse>) )
    {
        if ( !REASON->ok() ) { XMPPERROR("Data base reported an error:" + QString::fromStdString( REASON->error() )); }

        // Send the requested deltas
        {
            QString send;
            QXmlStreamWriter writer( &send );

            writer.writeStartElement("iq");
            writer.writeAttribute("type", "result" );
            writer.writeAttribute("id", m_stanza.stanzaId() );
            writer.writeAttribute("to", connection()->domain() );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("pubsub");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
            writer.writeStartElement("items");

            for( i = 0; i < REASON->applied_delta_size(); ++i )
            {
                QByteArray ba = QByteArray::fromRawData( REASON->applied_delta(i).data(), REASON->applied_delta(i).length() );
                QString str64 = QString::fromAscii( ba.toBase64() );
                writer.writeStartElement("item");
                writer.writeStartElement("applied-delta");
                writer.writeCDATA( str64 );
                writer.writeEndElement();
                writer.writeEndElement();
            }

            writer.writeStartElement("item");
            writer.writeStartElement("commit-notice");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("version", QString::number(REASON->end_version()) );
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeStartElement("item");
            writer.writeStartElement("history-truncated");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("version", QString::number(REASON->end_version()) );
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();

            connection()->send( send );
        }
    }
    else { XMPPERROR("Timeout waiting for database"); }

    END_EXECUTE;
}
