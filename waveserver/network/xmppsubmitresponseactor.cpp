#include "xmppsubmitresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "network/servercertificate.h"
#include "model/wavefolk.h"
#include "model/certificatestore.h"
#include "model/signedwaveletdelta.h"
#include "actor/pbmessage.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"
#include "protocol/messages.pb.h"
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
        m_url = WaveUrl( waveletName );
        if ( m_url.isNull() ) { XMPPERROR("Malformed wavelet-name"); }

        // Extract the delta
        if ( delta->children().count() == 0 || ( !delta->childAt(0)->isCData() && !delta->childAt(0)->isText() ) ) { XMPPERROR("Empty delta in submit-request"); }
        QByteArray ba = QByteArray::fromBase64( delta->childAt(0)->text().toAscii() );
        if ( !m_signedDelta.ParseFromArray( ba.data(), ba.length() ) ) { XMPPERROR("Could not deserialize ProtocolSignedDelta"); }
    }

    // Check signature
    {
        bool ok;
        SignedWaveletDelta delta ( &m_signedDelta, &ok );
        if ( !ok ) { XMPPERROR("Could not deserialize wavelet delta"); }
        if ( delta.signatures().count() == 0 ) { XMPPERROR("No signature found."); }

        // Are all signers known to the certificate store?
        foreach( const Signature& sig, delta.signatures() )
        {
            const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
            if ( !cert ) { XMPPERROR("At least one signer is not known."); }

            // Check signature
            if ( !cert->verify( delta.deltaBytes(), sig.signature() ) ) { XMPPERROR("Certificate and signature do not match"); }
            qDebug("CHECKED Signature successfully");
        }
    }

    // Send a submit request to the wavelet
    {
        m_id = nextId();
        // Construct the request
        PBMessage<messages::LocalSubmitRequest>* msg = new PBMessage<messages::LocalSubmitRequest>( WaveFolk::actorId( m_url ), m_id );
        msg->set_wavelet_name( m_url.toString().toStdString() );
        protocol::ProtocolSignedDelta* signedDelta = msg->mutable_signed_delta();
        signedDelta->MergeFrom( m_signedDelta );
        msg->setCreateOnDemand(false);
        // Send the request
        bool ok = post( msg );
        if ( !ok ) { XMPPERROR("Wavelet is not known"); }
    }

    // Wait for the response
    yield( RecvPB<messages::SubmitResponse>( m_id ) | Timeout(10000) );
    if ( REASON( RecvPB<messages::SubmitResponse> ) )
    {
        XMPPLOG("Got submit response");
        // Send the submit-response
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
        writer.writeAttribute("operations-applied", QString::number(REASON->operations_applied()) );
        writer.writeAttribute("application-timestamp", QString::number(REASON->application_timestamp()) );
        writer.writeStartElement("hashed-version");
        writer.writeAttribute("version", QString::number(REASON->hashed_version_after_application().version()) );
        QByteArray hash = QByteArray( REASON->hashed_version_after_application().history_hash().data(), REASON->hashed_version_after_application().history_hash().length() );
        writer.writeAttribute("history-hash", QString::fromAscii(hash.toBase64()) );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        connection()->send( send );
    }
    else if ( REASON( Timeout ) ) { XMPPERROR("Timeout waiting for response"); }

    END_EXECUTE;
}
