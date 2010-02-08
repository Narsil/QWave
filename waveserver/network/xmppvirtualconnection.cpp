#include "xmppvirtualconnection.h"
#include "xmppdiscoactor.h"
#include "xmppwaveletupdateactor.h"
#include "xmppdiscoresponseactor.h"
#include "xmppsignerresponseactor.h"
#include "xmpphistoryresponseactor.h"
#include "xmppsubmitresponseactor.h"
#include "xmpppostsignerresponseactor.h"
#include "network/xmppcomponentconnection.h"
#include "app/settings.h"
#include "model/appliedwaveletdelta.h"
#include "model/signedwaveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/certificatestore.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/waveurl.h"
#include "network/servercertificate.h"

#include <QByteArray>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QCryptographicHash>
#include <QtGlobal>
#include <QDateTime>

XmppVirtualConnection::XmppVirtualConnection( XmppComponentConnection* connection, const QString& domain, bool resolve )
        : ActorGroup( connection ), m_state( resolve ? Init : Established ), m_component( connection ), m_domain( domain ), m_signerInfoSent(false), m_ready(false)
{
    if ( !resolve )
        setReady();
    else
        new XmppDiscoActor(this);
}

XmppVirtualConnection::~XmppVirtualConnection()
{
    foreach( XmppStanza* stanza, m_stanzaQueue )
    {
        delete stanza;
    }
}

void XmppVirtualConnection::process( const XmppStanza& stanza )
{
    qDebug("Processing %s", stanza.qualifiedName().toAscii().constData() );

    if ( stanza.qualifiedName() == "iq" && stanza["type"] == "error" )
    {
        xmppError();
        return;
    }

    switch( m_state )
    {
        case Init:
        {
            // The remote server tells us his wave JID?
            if ( stanza.qualifiedName() == "iq" && stanza["type"] == "result" )
            {
                XmppTag* query = stanza.child("query");
                if ( query )
                {
                    foreach( XmppTag* tag, query->children("item") )
                    {
                        if ( tag->hasAttribute("jid" ) )
                        {
                            QString waveHost = (*tag)["jid"];
                            if ( waveHost == m_domain )
                            {
                                m_state = Established;
                                return;
                            }

                            XmppVirtualConnection* con = m_component->renameVirtualConnection( this, m_domain, waveHost );
                            if ( con != this )
                            {
                                // TODO: Can  this happen at all?
                                // Forward all
                                while( !m_stanzaQueue.isEmpty() )
                                {
                                    XmppStanza* stanza = m_stanzaQueue.dequeue();
                                    stanza->setAttribute("to", waveHost);
                                    con->send( stanza );
                                }
                                m_state = Delete;
                                return;
                            }

                            // Find out whether the remote server supports a version of wave that is acceptable
                            m_domain = waveHost;
                            QString send;
                            {
                                QXmlStreamWriter writer( &send );
                                writer.writeStartElement("iq");
                                writer.writeAttribute("type", "get" );
                                writer.writeAttribute("id", m_component->nextId() );
                                writer.writeAttribute("to", waveHost );
                                writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                                writer.writeStartElement("query");
                                writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#info" );
                                writer.writeEndElement();
                                writer.writeEndElement();
                            }
                            m_component->send( send );

                            m_state = DiscoItems;
                            return;
                        }
                    }
                }
            }
        }
        break;
        case DiscoItems:
        {
            if ( stanza.qualifiedName() == "iq" && stanza["type"] == "result" )
            {
                XmppTag* query = stanza.child("query");
                if ( query )
                {
                    XmppTag* ident = query->child("identity");
                    if ( ident && (*ident)["type"] == "google-wave" )
                    {
                        m_state = Established;
                        qDebug("Found a wave server");

                        // Forward all queued messages
                        while( !m_stanzaQueue.isEmpty() )
                        {
                            XmppStanza* stanza = m_stanzaQueue.dequeue();
                            stanza->setAttribute("to", m_domain);
                            send( stanza );
                        }
                    }
                    return;
                }

                qDebug("The server does not support wave.");
                xmppError();
                return;
            }
        }
        break;
        case Established:
            break;
        case Delete:
            return;
        case Error:
            return;
    }

    if ( stanza.qualifiedName() == "iq" )
        processIqGet( stanza );
    else if ( stanza.qualifiedName() == "message" )
        processMessage( stanza );
}

void XmppVirtualConnection::processIqGet( const XmppStanza& stanza )
{
    XmppTag* query = stanza.child("query");
    XmppTag* pubsub = stanza.child("pubsub");

    if ( stanza["type"] == "get" )
    {
        //
        // Message REQUEST DISCO#ITEMS
        //
        if ( query && (*query)["xmlns"] == "http://jabber.org/protocol/disco#items" )
        {
            QString sendStr;
            {
                QXmlStreamWriter writer( &sendStr );
                writer.writeStartElement("iq");
                writer.writeAttribute("type", "result" );
                writer.writeAttribute("id", stanza["id"] );
                writer.writeAttribute("to", m_domain );
                writer.writeAttribute("from", Settings::settings()->domain() );
                writer.writeStartElement("query");
                writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#items" );
                writer.writeStartElement("item");
                writer.writeAttribute("jid", Settings::settings()->xmppComponentName() );
                writer.writeEndElement();
                writer.writeEndElement();
                writer.writeEndElement();
            }
            m_component->send( sendStr );
        }
        //
        // Message REQUEST DISCO#INFO
        //
        else if ( query && (*query)["xmlns"] == "http://jabber.org/protocol/disco#info" )
        {
            QString sendStr;
            {
                QXmlStreamWriter writer( &sendStr );
                writer.writeStartElement("iq");
                writer.writeAttribute("type", "result" );
                writer.writeAttribute("id", stanza["id"] );
                writer.writeAttribute("to", m_domain );
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
            }
            m_component->send( sendStr );
        }
        else if ( pubsub && (*pubsub)["xmlns"] == "http://jabber.org/protocol/pubsub" )
        {
            XmppTag* items = pubsub->child( "items" );
            //
            // Message: REQUEST SIGNATURE
            //
            if ( items && (*items)["node"] == "signer" && items->child("signer-request") )
            {
                XmppTag* signerRequest = items->child("signer-request");

                // Find out for which signer a certificate is requested
                QByteArray signerId = QByteArray::fromBase64( signerRequest->attribute("signer-id").toAscii() );
                const ServerCertificate* cert = CertificateStore::store()->certificate( signerId );
                if ( !cert )
                {
                    qDebug("Unknown signerId");
                    xmppError();
                    return;
                }

                QString sendStr;
                {
                    QXmlStreamWriter writer( &sendStr );
                    writer.writeStartElement("iq");
                    writer.writeAttribute("type", "result" );
                    writer.writeAttribute("id", stanza["id"] );
                    writer.writeAttribute("to", m_domain );
                    writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                    writer.writeStartElement("pubsub");
                    writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
                    writer.writeStartElement("items");
                    writer.writeStartElement("signature");
                    writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
                    writer.writeAttribute("domain", Settings::settings()->domain() );
                    writer.writeAttribute("algorithm", "SHA256" );
                    QList<QByteArray> certificates = cert->toBase64();
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
                }
                m_component->send( sendStr );
            }
            //
            // Message: REQUEST DELTA-HISTORY
            //
            else if ( items && (*items)["node"] == "wavelet" && items->child("delta-history") )
            {
                XmppTag* history = items->child("delta-history");
                // TODO: Inspect the hashes that are being sent
                bool ok = true;
                qint64 start = history->attribute("start-version").toLong(&ok);
                if ( !ok || start < 0)
                {
                    qDebug("start-version missing");
                    xmppError();
                    return;
                }
                qint64 end = history->attribute("end-version").toLong(&ok);
                if ( !ok || end < 0 )
                {
                    qDebug("end-version missing");
                    xmppError();
                    return;
                }
                QString waveletName = history->attribute("wavelet-name");
                WaveUrl url( waveletName );
                if ( url.isNull() )
                {
                    qDebug("Malformed wavelet-name");
                    xmppError();
                    return;
                }
                Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
                if ( !wave )
                {
                    qDebug("Unknown wave");
                    xmppError();
                    return;
                }
                Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId() );
                if ( !wavelet )
                {
                    qDebug("Unknown wavelet");
                    xmppError();
                    return;
                }

                QString sendStr;
                {
                    QXmlStreamWriter writer( &sendStr );
                    writer.writeStartElement("iq");
                    writer.writeAttribute("type", "result" );
                    writer.writeAttribute("id", stanza["id"] );
                    writer.writeAttribute("to", m_domain );
                    writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                    writer.writeStartElement("pubsub");
                    writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
                    writer.writeStartElement("items");

                    end = qMin( wavelet->version(), end );
                    start = qMin( wavelet->version(), start );
                    for( qint64 v = start; v <= end; ++v )
                    {
                        const AppliedWaveletDelta* delta = wavelet->delta(v);
                        if ( delta && !delta->isNull() )
                        {
                            QString str64 = delta->toBase64();
                            writer.writeStartElement("item");
                            writer.writeStartElement("applied-delta");
                            writer.writeCDATA( str64 );
                            writer.writeEndElement();
                            writer.writeEndElement();
                        }
                    }

                    writer.writeStartElement("item");
                    writer.writeStartElement("commit-notice");
                    writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
                    writer.writeAttribute("version", QString::number(wavelet->version()) );
                    writer.writeEndElement();
                    writer.writeEndElement();
                    writer.writeStartElement("item");
                    writer.writeStartElement("history-truncated");
                    writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
                    writer.writeAttribute("version", QString::number(end) );
                    writer.writeEndElement();
                    writer.writeEndElement();
                    writer.writeEndElement();
                    writer.writeEndElement();
                    writer.writeEndElement();
                }
                m_component->send( sendStr );
            }
            else
                qDebug(" ... message is unhandled");
        }
        else
            qDebug(" ... message is unhandled");
    }
    else if ( stanza["type"] == "set" )
    {
        if ( pubsub && (*pubsub)["xmlns"] == "http://jabber.org/protocol/pubsub" )
        {
            //
            // Message SUBMIT-REQUEST
            //
            XmppTag* publish = pubsub->child( "publish" );
            if ( publish && (*publish)["node"] == "wavelet" )
            {
//                XmppTag* item = publish->child( "item" );
//                XmppTag* request = item ? item->child("submit-request") : 0;
//                XmppTag* delta = request ? request->child("delta") : 0;
//                if ( item && request && delta )
//                {
//                    QString waveletName = delta->attribute("wavelet-name");
//                    WaveUrl url( waveletName );
//                    if ( url.isNull() )
//                    {
//                        qDebug("Malformed wavelet-name");
//                        xmppError();
//                        return;
//                    }
//                    Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
//                    if ( !wave )
//                    {
//                        qDebug("Unknown wave");
//                        xmppError();
//                        return;
//                    }
//                    Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId() );
//                    if ( !wavelet )
//                    {
//                        qDebug("Unknown wavelet");
//                        xmppError();
//                        return;
//                    }
//
//                    if ( delta->children().count() == 1 && ( delta->childAt(0)->isCData() || delta->childAt(0)->isText() ) )
//                    {
//                        QString base64 = delta->childAt(0)->text();
//                        bool ok;
//                        SignedWaveletDelta wdelta = SignedWaveletDelta::fromBase64( base64, &ok );
//                        if ( !ok )
//                        {
//                            qDebug("Could not deserialize wavelet delta");
//                            xmppError();
//                            return;
//                        }
//                        qDebug("Got wavelet delta");
//
//                        if ( wdelta.signatures().count() == 0 )
//                        {
//                            qDebug("No signature found.");
//                            xmppError();
//                            return;
//                        }
//
//                        // Are all signers known to the certificate store?
//                        bool allKnown = true;
//                        foreach( const Signature& sig, wdelta.signatures() )
//                        {
//                            const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
//                            if ( !cert )
//                            {
//                                qDebug("At least one signer is not known. Move submit-request to the EscrowDeposit.");
//                                allKnown = false;
//
////                                // Send a signer-request
////                                QString sendStr;
////                                {
////                                    QXmlStreamWriter writer( &sendStr );
////                                    writer.writeStartElement("iq");
////                                    writer.writeAttribute("type", "get" );
////                                    writer.writeAttribute("id", m_component->nextId() );
////                                    writer.writeAttribute("to", m_domain );
////                                    writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
////                                    writer.writeStartElement("pubsub");
////                                    writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
////                                    writer.writeStartElement("items");
////                                    writer.writeAttribute("node", "signer");
////                                    writer.writeStartElement("signer-request");
////                                    writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
////                                    writer.writeAttribute("wavelet-name", waveletName );
////                                    writer.writeAttribute("version", QString::number( wdelta.delta().version().version ) );
////                                    writer.writeAttribute("history-hash", QString::fromAscii( wdelta.delta().version().hash.toBase64() ) );
////                                    writer.writeAttribute("signer-id", QString::fromAscii( sig.signerId().toBase64() ) );
////                                    writer.writeEndElement();
////                                    writer.writeEndElement();
////                                    writer.writeEndElement();
////                                    writer.writeEndElement();
////                                }
////                                m_component->send( sendStr );
//                            }
//                            else
//                            {
//                                // Check signature
//                                if ( !cert->verify( wdelta.deltaBytes(), sig.signature() ) )
//                                {
//                                    qDebug("Certificate and signature do not match");
//                                    xmppError();
//                                    return;
//                                }
//                                qDebug("CHECKED Signature successfully");
//                            }
//                        }
//                        if ( !allKnown )
//                        {
//                            // TODO Send an error to indicate the remote that the signer infor is missing.
//                            // TODO: This must wait until the protocol has been updated.
//                            qDebug("Submit-request rejected because of missing singer info.");
//                            xmppError();
//                            return;
//                        }
//
//                        // TODO: AppliedWaveletDelta should be able to carry a list of signatures
//                        Signature signature = wdelta.signatures()[0];
//                        QString err;
//                        int version = wavelet->apply( wdelta.delta(), &err, &signature );
//                        if ( !err.isEmpty() || version < 0 )
//                        {
//                            qDebug("Failed to apply wavelet delta: %s", err.toAscii().constData());
//                            xmppError();
//                            return;
//                        }
//
//                        qDebug("Applied wavelet");
//                        const AppliedWaveletDelta& applied = wavelet->delta(version - 1);
//
//                        // Send a response
//                        QString sendStr;
//                        {
//                            QXmlStreamWriter writer( &sendStr );
//                            writer.writeStartElement("iq");
//                            writer.writeAttribute("type", "result" );
//                            writer.writeAttribute("id", stanza["id"] );
//                            writer.writeAttribute("to", m_domain );
//                            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
//                            writer.writeStartElement("pubsub");
//                            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
//                            writer.writeStartElement("publish");
//                            writer.writeStartElement("item");
//                            writer.writeStartElement("submit-response");
//                            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
//                            writer.writeAttribute("operations-applied", QString::number(applied.operationsApplied()) );
//                            writer.writeAttribute("application-timestamp", QString::number(applied.applicationTime()) );
//                            writer.writeStartElement("hashed-version");
//                            writer.writeAttribute("version", QString::number(applied.resultingVersion().version) );
//                            writer.writeAttribute("history-hash", QString::fromAscii(applied.resultingVersion().hash.toBase64()) );
//                            writer.writeEndElement();
//                            writer.writeEndElement();
//                            writer.writeEndElement();
//                            writer.writeEndElement();
//                            writer.writeEndElement();
//                            writer.writeEndElement();
//                        }
//                        m_component->send( sendStr );
//                    }
//                }
            }
            //
            // Message POST SIGNATURE
            //
            else if ( publish && (*publish)["node"] == "signer" )
            {
                XmppTag* item = publish->child( "item" );
                XmppTag* signature = item ? item->child("signature") : 0;
                if ( item && signature )
                {
                    QString domain = signature->attribute("domain");
                    QString algorithm = signature->attribute("algorithm");
                    if ( algorithm != "SHA256" )
                    {
                        qDebug("Unsupported algorithm %s", algorithm.toAscii().constData() );
                        xmppError();
                        return;
                    }

                    // Extract the certificates
                    QList<QByteArray> certificates;
                    foreach( XmppTag* certificate, signature->children("certificate") )
                    {
                        QString str;
                        // Extract the certificate
                        foreach( XmppTagPtr t, certificate->children() )
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
                    if ( certificates.count() == 0 )
                    {
                        qDebug("No certificate supplied.");
                        xmppError();
                        return;
                    }

                    // Decode the certificate
                    RemoteServerCertificate* cs = new RemoteServerCertificate( certificates );
                    if ( !cs->isValid() )
                    {
                        delete cs;
                        qDebug("Supplied certificate is not valid or malformed.");
                        xmppError();
                        return;
                    }

                    // Store the certificates
                    // TODO: Check the top-level authority of the certificates. Is this certificate acceptable at all?
                    CertificateStore::store()->addCertificate( cs );

                    // Send a response
                    QString sendStr;
                    {
                        QXmlStreamWriter writer( &sendStr );
                        writer.writeStartElement("iq");
                        writer.writeAttribute("type", "result" );
                        writer.writeAttribute("id", stanza["id"] );
                        writer.writeAttribute("to", m_domain );
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
                    }
                    m_component->send( sendStr );
                }
            }
        }
    }
    else
        qDebug(" ... message is unhandled");
    // TODO
}

void XmppVirtualConnection::processMessage( const XmppStanza& stanza )
{
    if ( stanza["type"] == "error" )
    {
        xmppError();
        return;
    }

    if ( stanza["type"] == "normal" )
    {
        XmppTag* event = stanza.child("event");
        XmppTag* items = event ? event->child("items") : 0;
        XmppTag* item = items ? items->child("item") : 0;
        XmppTag* update = item ? item->child("wavelet-update") : 0;
        XmppTag* applied = update ? update->child("applied-delta") : 0;
        XmppTag* text = applied ? applied->childAt(0) : 0;
        if ( text && ( text->isCData() || text->isText() ) )
        {
            WaveUrl url( update->attribute("wavelet-name") );
            if ( url.isNull() )
            {
                qDebug("Malformed wavelet name %s", url.toString().toAscii().constData() );
                xmppError();
                return;
            }

            bool ok;
            SignedWaveletDelta delta = SignedWaveletDelta::fromBase64( text->text(), &ok );
            if ( !ok )
            {
                qDebug("Could not deserialize signed wavelet delta.");
                xmppError();
                return;
            }

            if ( delta.signatures().count() == 0 )
            {
                qDebug("No signature in SignedWaveltDelta");
                xmppError();
                return;
            }

            Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
            if ( !wave )
            {
                qDebug("Unknown wave");
                xmppError();
                return;
            }
            Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), true );
            if ( !wavelet )
            {
                qDebug("Unknown wavelet");
                xmppError();
                return;
            }

            // Send a response
            QString sendStr;
            {
                QXmlStreamWriter writer( &sendStr );
                writer.writeStartElement("message");
                writer.writeAttribute("id", stanza["id"] );
                writer.writeAttribute("to", m_domain );
                writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                writer.writeStartElement("received");
                writer.writeAttribute("xmlns", "urn:xmpp:receipts" );
                writer.writeEndElement();
                writer.writeEndElement();
            }
            m_component->send( sendStr );

            // Are all signers known ?
            foreach( const Signature& sig, delta.signatures() )
            {
                const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
                if ( !cert )
                {
                    // TODO Ask for signer info, i.e. get a certificate for this signer ID.
                }
            }

            // We are missing some deltas? Then issue a history-request
            if ( delta.delta().version().version > wavelet->version() )
            {
                // TODO: return
            }

            QString err;
            // TODO: What if there are multuple signatures
            int version = wavelet->apply( delta.delta(), &err, &delta.signatures()[0] );
            if ( version == -1 || !err.isEmpty() )
            {
                qDebug("Error applying delta: %s", err.toAscii().constData() );
                xmppError();
                return;
            }
        }
        else
            qDebug("... message unhandled");
    }
    else
        qDebug("... message unhandled");
}

void XmppVirtualConnection::sendSubmitRequest( const WaveUrl& url, const protocol::ProtocolWaveletDelta& delta )
{
    // Post signer information before sending the first submit request
    if ( !m_signerInfoSent )
    {
        m_signerInfoSent = true;
        QString sendStr;
        {
            QXmlStreamWriter writer( &sendStr );
            writer.writeStartElement("iq");
            writer.writeAttribute("type", "set" );
            writer.writeAttribute("id", m_component->nextId() );
            writer.writeAttribute("to", m_domain );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("pubsub");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
            writer.writeStartElement("publish");
            writer.writeAttribute("node", "signer" );
            writer.writeStartElement("item");
            writer.writeStartElement("signature");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("domain", m_domain );
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
        }
        m_component->send( sendStr );
    }

    SignedWaveletDelta sdelta( delta );
    QString str64 = sdelta.toBase64();

    QString sendStr;
    {
        QXmlStreamWriter writer( &sendStr );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "set" );
        writer.writeAttribute("id", m_component->nextId() );
        writer.writeAttribute("to", m_domain );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("pubsub");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
        writer.writeStartElement("publish");
        writer.writeAttribute("node", "wavelet" );
        writer.writeStartElement("item");
        writer.writeStartElement("submit-request");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("wavelet-name", url.toString() );
        writer.writeStartElement("delta");
        writer.writeCDATA( str64 );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
    }
    m_component->send( sendStr );

    // TODO: Wait for the response
}

void XmppVirtualConnection::send( XmppStanza* stanza )
{
    switch( m_state )
    {
        case Init:
        case DiscoItems:
            m_stanzaQueue.enqueue(stanza);
            break;
        case Established:
            m_component->send( *stanza );
            delete stanza;
            break;
        case Delete:
        case Error:
            // Do nothing by intention
            break;
    }
}

////////
/// NEW
////////

void XmppVirtualConnection::setDomain( const QString& domain )
{
    m_component->renameVirtualConnection( this, m_domain, domain );
    m_domain = domain;
}

bool XmppVirtualConnection::send( const QString& stanza )
{
    return m_component->send( stanza );
}

void XmppVirtualConnection::setReady()
{
    qDebug("Starting service");
    m_ready = true;
    emit ready();
}

void XmppVirtualConnection::dispatch( const QSharedPointer<IMessage>& message )
{
    if ( m_ready )
    {
        XmppStanza* stanza = dynamic_cast<XmppStanza*>( message.data() );
        if ( stanza )
        {
            switch( stanza->kind() )
            {
                case XmppStanza::WaveletUpdate:
                    // TODO
                    break;
                case XmppStanza::HistoryRequest:
                    new XmppHistoryResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::SignerRequest:
                    new XmppSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::PostSigner:
                    new XmppPostSignerResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::SubmitRequest:
                    new XmppSubmitResponseActor( this, message.dynamicCast<XmppStanza>() );
                    break;
                case XmppStanza::DiscoInfo:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoInfo );
                    break;
                case XmppStanza::DiscoItems:
                    new XmppDiscoResponseActor( this, stanza->id(), XmppStanza::DiscoItems );
                    break;
                default:
                    break;
            }
        }
    }

    this->ActorGroup::dispatch( message );
}

void XmppVirtualConnection::sendWaveletUpdate(const QString& waveletName, const AppliedWaveletDelta& waveletDelta)
{
    new XmppWaveletUpdateActor( this, waveletName, waveletDelta );
}

void XmppVirtualConnection::xmppError()
{
    qDebug("ERROR talking to servers %s", m_domain.toAscii().constData() );
    m_ready = false;
}

