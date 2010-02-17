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
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include <QXmlStreamWriter>
#include <QList>
#include <QByteArray>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); sendErrorResponse(); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }


XmppSubmitRequestActor::XmppSubmitRequestActor(XmppVirtualConnection* con, const QSharedPointer<PBMessage<waveserver::ProtocolSubmitRequest> >& message )
        : XmppActor(con), m_message( message )
{
    con->addActor( this );
}

void XmppSubmitRequestActor::EXECUTE()
{
    qDebug("EXECUTE SubmitRequest Actor");

    BEGIN_EXECUTE;

    m_delta = SignedWaveletDelta( m_message->delta() );
    m_url = WaveUrl( QString::fromStdString( m_message->wavelet_name() ) );

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
            connection()->setPostedSigner( true );
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
        writer.writeStartElement("delta");
        writer.writeAttribute("wavelet-name", m_url.toString() );
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
        XmppTag* pubsub = REASON->child("pubsub");
        XmppTag* publish = pubsub ? pubsub->child("publish") : 0;
        XmppTag* item = publish ? publish->child("item") : 0;
        XmppTag* resp = item ? item->child("submit-response") : 0;
        XmppTag* hash = resp ? resp->child("hashed-version") : 0;
        if ( !hash ) { XMPPERROR("Malformed submit-response (missing tags)"); }

        bool ok;
        m_applicationTime = resp->attribute("application-timestamp").toLongLong(&ok);
        if ( !ok ) { XMPPERROR("Malformed submit-response (timestamp)"); }

        m_operationsApplied = resp->attribute("operations-applied").toInt(&ok);
        if ( !ok ) { XMPPERROR("Malformed submit-response (operations applied)"); }

        m_version = hash->attribute("version").toInt(&ok);
        if ( !ok ) { XMPPERROR("Malformed submit-response (version)"); }

        m_hash = QByteArray::fromBase64( hash->attribute("history-hash").toAscii() );
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
    else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }

    // Send information back
    if ( !m_message->sender().isNull() )
    {
        XMPPLOG("Sending response to " + m_message->sender().toString() );
        waveserver::ProtocolSubmitResponse response;
        response.set_operations_applied( m_operationsApplied );
        response.mutable_hashed_version_after_application()->set_history_hash( m_hash.data(), m_hash.length() );
        response.mutable_hashed_version_after_application()->set_version( m_version );
        send( m_message->sender(), new PBMessage<waveserver::ProtocolSubmitResponse>( response, m_message->Id() ) );
    }

    END_EXECUTE;
}

void XmppSubmitRequestActor::sendErrorResponse()
{
    if ( !m_message->sender().isNull() )
    {
        waveserver::ProtocolSubmitResponse response;
        response.set_operations_applied( 0 );
        send( m_message->sender(), new PBMessage<waveserver::ProtocolSubmitResponse>( response, m_message->Id() ) );
    }
}
