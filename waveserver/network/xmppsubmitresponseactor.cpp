#include "xmppsubmitresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "model/appliedwaveletdelta.h"
#include "network/servercertificate.h"
#include "model/waveurl.h"
#include "model/wave.h"
#include "model/localwavelet.h"
#include "model/certificatestore.h"
#include <QXmlStreamWriter>

#define XMPPERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->xmppError(); TERMINATE(); }
#define XMPPLOG(msg) { log(msg, __FILE__, __LINE__); }

XmppSubmitResponseActor::XmppSubmitResponseActor(XmppVirtualConnection* con, XmppStanza* stanza)
        : XmppActor(con), m_stanza(*stanza)
{
}

void XmppSubmitResponseActor::execute()
{
    qDebug("EXECUTE SubmitResponse");

    BEGIN_EXECUTE;

    // Analyze the request and try to apply the submitted delta
    {
        // Check the stanza
        XmppTag* pubsub = m_stanza.child("pubsub");
        XmppTag* publish = pubsub ? pubsub->child("publish") : 0;
        XmppTag* item = publish ? publish->child( "item" ) : 0;
        XmppTag* request = item ? item->child("submit-request") : 0;
        XmppTag* delta = request ? request->child("delta") : 0;
        if ( !delta ) { XMPPERROR("Malformed submit-request"); }

        // Find the wavelet
        QString waveletName = delta->attribute("wavelet-name");
        WaveUrl url( waveletName );
        if ( url.isNull() ) { XMPPERROR("Malformed wavelet-name"); }
        Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
        if ( !wave ) { XMPPERROR("Unknown wave"); }
        Wavelet* w = wave->wavelet( url.waveletDomain(), url.waveletId() );
        if ( !w ) { XMPPERROR("Unknown wavelet"); }
        if ( !w->isLocal() ) { XMPPERROR("Submit requests for remote wavelets cannot be accepted"); }
        m_wavelet = dynamic_cast<LocalWavelet*>(w);

        // Extract the delta
        if ( delta->children().count() == 0 || ( !delta->childAt(0)->isCData() && !delta->childAt(0)->isText() ) ) { XMPPERROR("Empty delta in submit-request"); }
        QString base64 = delta->childAt(0)->text();
        bool ok;
        m_delta = SignedWaveletDelta::fromBase64( base64, &ok );
        if ( !ok ) { XMPPERROR("Could not deserialize wavelet delta"); }
        if ( m_delta.signatures().count() == 0 ) { XMPPERROR("No signature found."); }

        // Are all signers known to the certificate store?
        foreach( const Signature& sig, m_delta.signatures() )
        {
            const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
            if ( !cert ) { XMPPERROR("At least one signer is not known. Move submit-request to the EscrowDeposit."); }

            // Check signature
            if ( !cert->verify( m_delta.deltaBytes(), sig.signature() ) ) { XMPPERROR("Certificate and signature do not match"); }
            qDebug("CHECKED Signature successfully");
        }

        // Apply the delta
        QString err;
        int version = m_wavelet->apply( m_delta, &err );
        if ( !err.isEmpty() || version < 0 ) { XMPPERROR("Failed to apply submitted wavelet delta"); }
        qDebug("Applied wavelet");
        m_applied = m_wavelet->delta(version - 1);
    }

    // Wait until the connection is ready
    if ( !connection()->isReady() )
        yield( RecvSignal( connection(), SIGNAL(ready())) );

    // Send the submit-response
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
        writer.writeStartElement("submit-response");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("operations-applied", QString::number(m_applied->operationsApplied()) );
        writer.writeAttribute("application-timestamp", QString::number(m_applied->applicationTime()) );
        writer.writeStartElement("hashed-version");
        writer.writeAttribute("version", QString::number(m_applied->resultingVersion().version) );
        writer.writeAttribute("history-hash", QString::fromAscii(m_applied->resultingVersion().hash.toBase64()) );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }

    END_EXECUTE;
}
