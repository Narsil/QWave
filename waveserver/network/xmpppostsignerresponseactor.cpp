#include "xmpppostsignerresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "network/servercertificate.h"
#include "model/certificatestore.h"
#include <QXmlStreamWriter>
#include <QList>
#include <QByteArray>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppPostSignerResponseActor::XmppPostSignerResponseActor(XmppVirtualConnection* con, XmppStanza* stanza)
        : XmppActor(con), m_stanza(*stanza)
{
}

void XmppPostSignerResponseActor::execute()
{
    qDebug("EXECUTE PostSignerResponse");

    // Find the corresponding certificate
    {
        XmppTag* pubsub = m_stanza.child("pubsub");
        XmppTag* publish = pubsub ? pubsub->child("publish") : 0;
        XmppTag* item = publish ? publish->child( "item" ) : 0;
        XmppTag* signature = item ? item->child("signature") : 0;
        if ( !signature ) { XMPPERROR("Malformed post-signer"); }

        QString domain = signature->attribute("domain");
        QString algorithm = signature->attribute("algorithm");
        if ( algorithm != "SHA256" ) { XMPPERROR("Unsupported algorithm " + algorithm ); }

        // Extract the certificates
        QList<QByteArray> certificates;
        foreach( XmppTag* certificate, signature->children("certificate") )
        {
            QString str;
            // Extract the certificate
            foreach( XmppTag* t, certificate->children() )
            {
                if ( t->isText() || t->isCData() )
                    str += t->text();
            }
            // Convert it to PEM format
            QString pem( "-----BEGIN CERTIFICATE-----\n");
            int i = 0;
            while( i < str.length() )
            {
                int l = qMin(64, str.length() - i);
                pem += str.mid( i, l ) + "\n";
                i += l;
            }
            pem += "-----END CERTIFICATE-----\n";
            QByteArray ba = pem.toAscii();
            certificates.append(ba);
        }
        if ( certificates.count() == 0 ) { XMPPERROR("No certificate supplied."); }

        // Decode the certificate
        RemoteServerCertificate* cs = new RemoteServerCertificate( certificates );
        if ( !cs->isValid() ) { delete cs; XMPPERROR("Supplied certificate is not valid or malformed."); }

        // Store the certificates
        // TODO: Check the top-level authority of the certificates. Is this certificate acceptable at all?
        CertificateStore::store()->addCertificate( cs );
    }

    BEGIN_EXECUTE;

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Send the response
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
        writer.writeStartElement("publish");
        writer.writeStartElement("item");
        writer.writeAttribute("node", "signer" );
        writer.writeStartElement("signature-response");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    END_EXECUTE;
}
