#include "xmppwaveletupdateresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "actor/recvxmpp.h"
#include "actor/timeout.h"
#include "network/servercertificate.h"
#include "model/waveurl.h"
#include "model/wave.h"
#include "model/remotewavelet.h"
#include "model/certificatestore.h"
#include <QXmlStreamWriter>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppWaveletUpdateResponseActor::XmppWaveletUpdateResponseActor(XmppVirtualConnection* con, XmppStanza* stanza)
        : XmppActor(con), m_stanza(*stanza)
{
}

void XmppWaveletUpdateResponseActor::execute()
{
    qDebug("EXECUTE WaveletUpdateResponse");

    BEGIN_EXECUTE;

    // Analyze the received stanza
    {
        XmppTag* event = m_stanza.child("event");
        XmppTag* items = event ? event->child("items") : 0;
        XmppTag* item = items ? items->child("item") : 0;
        XmppTag* update = item ? item->child("wavelet-update") : 0;
        if ( !update ) { XMPPERROR("Malformed history response. Missing wavelet-update."); }

        WaveUrl url( update->attribute("wavelet-name") );
        if ( url.isNull() ) { XMPPERROR("Malformed wavelet name " + url.toString() ); }

        Wave* wave = Wave::wave( url.waveDomain(), url.waveId(), true );
        if ( !wave ) { XMPPERROR("Unknown wave"); }
        Wavelet* w = wave->wavelet( url.waveletDomain(), url.waveletId(), true );
        if ( !w ) { XMPPERROR("Unknown wavelet"); }
        if ( !w->isRemote() ) { XMPPERROR("Wavelet update requests for local wavelets cannot be accepted"); }
        m_wavelet = dynamic_cast<RemoteWavelet*>(w);

        QList<XmppTag*> lst = update->children("applied-delta");
        for( i = 0; i < lst.count(); ++i )
        {
            XmppTag* appliedDelta = lst[i];
            QString str;
            // Extract the base64 encoded delta
            foreach( XmppTag* t, appliedDelta->children() )
            {
                if ( t->isText() || t->isCData() )
                   str += t->text();
            }
            bool ok;
            AppliedWaveletDelta d = AppliedWaveletDelta::fromBase64( str, &ok );
            if ( !ok ) { XMPPERROR("Could not decode applied wavelet delta in history response"); }

            m_deltas.append( d );
        }

        // TODO: What about a comit notice?
    }

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Send a response to tell that we got the stanza
    {
        QString send;
        QXmlStreamWriter writer( &send );
        writer.writeStartElement("message");
        writer.writeAttribute("id", m_stanza.stanzaId() );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("received");
        writer.writeAttribute("xmlns", "urn:xmpp:receipts" );
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

//    // Are all signers known ?
//    count = m_delta.signatures().count();
//    for( i = 0; i < count; ++i )
//    {
//        m_signature = m_delta.signatures()[i];
//        m_cert = CertificateStore::store()->certificate( m_signature.signerId() );
//        if ( !m_cert )
//        {
//            // Ask for signer info, i.e. get a certificate for this signer ID.
//            {
//                m_id = connection()->component()->nextId();
//                QString send;
//                QXmlStreamWriter writer( &send );
//                writer.writeStartElement("iq");
//                writer.writeAttribute("type", "get" );
//                writer.writeAttribute("id", m_id );
//                writer.writeAttribute("to", connection()->domain() );
//                writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
//                writer.writeStartElement("pubsub");
//                writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
//                writer.writeStartElement("items");
//                writer.writeAttribute("node", "signer" );
//                writer.writeStartElement("signer-request");
//                writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
//                writer.writeAttribute("wavelet-name", m_wavelet->url().toString() );
//                writer.writeAttribute("history-hash", QString::fromAscii( m_delta.delta().version().hash.toBase64() ) );
//                writer.writeAttribute("version", QString::number( m_delta.delta().version().version ) );
//                writer.writeEndElement();
//                writer.writeEndElement();
//                writer.writeEndElement();
//                writer.writeEndElement();
//                connection()->send( send );
//            }
//
//            // Wait for the response
//            yield( RecvXmpp<XmppStanza::SignerResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
//            if ( REASON( RecvXmpp<XmppStanza::SignerResponse> ) )
//            {
//                XMPPLOG("Got signer info");
//                XmppTag* pubsub = REASON->child("pubsub");
//                XmppTag* publish = pubsub ? pubsub->child("publish") : 0;
//                XmppTag* item = publish ? publish->child( "item" ) : 0;
//                XmppTag* signature = item ? item->child("signature") : 0;
//                if ( !signature ) { XMPPERROR("Malformed post-signer"); }
//
//                QString domain = signature->attribute("domain");
//                QString algorithm = signature->attribute("algorithm");
//                if ( algorithm != "SHA256" ) { XMPPERROR("Unsupported algorithm " + algorithm ); }
//
//                // Extract the certificates
//                QList<QByteArray> certificates;
//                foreach( XmppTag* certificate, signature->children("certificate") )
//                {
//                    QString str;
//                    // Extract the certificate
//                    foreach( XmppTagPtr t, certificate->children() )
//                    {
//                        if ( t->isText() || t->isCData() )
//                            str += t->text();
//                    }
//                    // Convert it to PEM format
//                    QString pem( "-----BEGIN CERTIFICATE-----\n");
//                    int i = 0;
//                    while( i < str.length() )
//                    {
//                        int l = qMin(64, str.length() - i);
//                        pem += str.mid( i, l ) + "\n";
//                        i += l;
//                    }
//                    pem += "-----END CERTIFICATE-----\n";
//                    QByteArray ba = pem.toAscii();
//                    certificates.append(ba);
//                }
//                if ( certificates.count() == 0 ) { XMPPERROR("No certificate supplied."); }
//
//                // Decode the certificate
//                RemoteServerCertificate* cs = new RemoteServerCertificate( certificates );
//                if ( !cs->isValid() ) { delete cs; XMPPERROR("Supplied certificate is not valid or malformed."); }
//
//                // Store the certificates
//                // TODO: Check the top-level authority of the certificates. Is this certificate acceptable at all?
//                CertificateStore::store()->addCertificate( cs );
//            }
//            else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
//            else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }
//        }
//
//        // Check signature
//        if ( !m_cert->verify( m_delta.deltaBytes(), m_signature.signature() ) ) { XMPPERROR("Certificate and signature do not match"); }
//        qDebug("CHECKED Signature successfully");
//    }

    // We are missing some deltas? Then issue a history-request
    if ( m_deltas.count() > 0 && m_deltas[0].signedDelta().delta().version().version > m_wavelet->version() )
    {
        // Ask for the history
        {
            m_id = connection()->component()->nextId();
            QString send;
            QXmlStreamWriter writer( &send );
            writer.writeStartElement("iq");
            writer.writeAttribute("type", "get" );
            writer.writeAttribute("id", m_id );
            writer.writeAttribute("to", connection()->domain() );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("pubsub");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
            writer.writeStartElement("items");
            writer.writeAttribute("node", "wavelet" );
            writer.writeStartElement("delta-history");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("wavelet-name", m_wavelet->url().toString() );
            writer.writeAttribute("response-length-limit", QString::number( 300000 ) );
            writer.writeAttribute("start-version", QString::number( m_wavelet->version() ) );
            writer.writeAttribute("start-version-hash", QString::fromAscii( m_wavelet->hash().toBase64() ) );
            writer.writeAttribute("end-version", QString::number( m_deltas[0].signedDelta().delta().version().version ) );
            writer.writeAttribute("end-version-hash", QString::fromAscii( m_deltas[0].signedDelta().delta().version().hash.toBase64() ) );
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            connection()->send( send );
        }

        // Wait for the response
        yield( RecvXmpp<XmppStanza::HistoryResponse>( m_id ) | Timeout(10000) | RecvXmpp<XmppStanza::Error>( m_id ) );
        if ( REASON( RecvXmpp<XmppStanza::HistoryResponse> ) )
        {
            XMPPLOG("Got delta history");
            XmppTag* pubsub = REASON->child("pubsub");
            XmppTag* items = pubsub ? pubsub->child("items") : 0;
            if ( !items ) { XMPPERROR("Malformed history response"); }

            int len = m_deltas.count();
            QList<XmppTag*> lst = items->children("item");
            for( i = 0; i < lst.count(); ++i )
            {
                XmppTag* item = lst[i];
                XmppTag* appliedDelta = item->child("applied-delta");
                if ( appliedDelta )
                {
                    QString str;
                    // Extract the base64 encoded delta
                    foreach( XmppTag* t, appliedDelta->children() )
                    {
                        if ( t->isText() || t->isCData() )
                            str += t->text();
                    }
                    bool ok;
                    AppliedWaveletDelta d = AppliedWaveletDelta::fromBase64( str, &ok );
                    if ( !ok ) { XMPPERROR("Could not decode applied wavelet delta in history response"); }

                    m_deltas.insert( m_deltas.count() - len, d );
                }
                // ELSE: TODO: What to do about commit notices ?
            }

        }
        else if ( REASON( Timeout ) ) { XMPPERROR("Timeout"); }
        else if ( REASON( RecvXmpp<XmppStanza::Error> ) ) { XMPPERROR("Peer reported an error"); }
    }

    // Check that we have a signer info for all deltas. If we don't have it, request it. Then verify the delta's signature.
    for( i = 0; i < m_deltas.count(); ++i )
    {
        m_delta = m_deltas[i];
        if ( m_delta.signedDelta().signatures().count() == 0 ) { XMPPERROR("No signature in SignedWaveltDelta"); }

        // TODO: Check the signer id and signature

        XMPPLOG(QString("Applying delta at version %1").arg( m_delta.signedDelta().delta().version().version ) );

        // Apply the delta to the wavelet
        {
            QString err;
            bool ok = m_wavelet->apply( m_delta, &err );
            if ( !ok ) { XMPPERROR("Error applying delta: " + err ); }
        }
    }

    qDebug("Everthing has been applied");

    END_EXECUTE;
}
