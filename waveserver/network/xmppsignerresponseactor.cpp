#include "xmppsignerresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "network/servercertificate.h"
#include "model/certificatestore.h"
#include <QXmlStreamWriter>
#include <QList>
#include <QStringList>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppSignerResponseActor::XmppSignerResponseActor(XmppVirtualConnection* con, const QSharedPointer<XmppStanza>& stanza)
        : XmppActor(con), m_stanza(stanza), m_cert(0)
{
    con->addActor( this );
}

void XmppSignerResponseActor::EXECUTE()
{
    qDebug("EXECUTE SignerResponse");

    // Find the corresponding certificate
    {
        XmppTag* pubsub = m_stanza->child("pubsub");
        XmppTag* items = pubsub ? pubsub->child( "items" ) : 0;
        XmppTag* signerRequest = items ? items->child("signer-request") : 0;
        if ( !signerRequest ) { XMPPERROR("Malformed signer-request"); }

        // Find out for which signer a certificate is requested
        QByteArray signerId = QByteArray::fromBase64( signerRequest->attribute("signer-id").toAscii() );
        m_cert = CertificateStore::store()->certificate( signerId );
        if ( !m_cert ) { XMPPERROR("Unknown signer ID"); }
    }

    BEGIN_EXECUTE;

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Send the requestes certificate
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "result" );
        writer.writeAttribute("id", m_stanza->id() );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("pubsub");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
        writer.writeStartElement("items");
        writer.writeStartElement("signature");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("domain", Settings::settings()->domain() );
        writer.writeAttribute("algorithm", "SHA256" );
        QList<QByteArray> certificates = m_cert->toBase64();
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
        connection()->send( send );
    }

    END_EXECUTE;
}
