#include "xmpphistoryresponseactor.h"
#include "xmppvirtualconnection.h"
#include "xmppcomponentconnection.h"
#include "xmppstanza.h"
#include "app/settings.h"
#include "actor/recvsignal.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/appliedwaveletdelta.h"
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
        WaveUrl url( waveletName );
        if ( url.isNull() ) { XMPPERROR("Malformed wavelet-name"); }

        Wave* wave = Wave::wave( url.waveDomain(), url.waveId() );
        if ( !wave ) { XMPPERROR("Unknown wave"); }
        m_wavelet = wave->wavelet( url.waveletDomain(), url.waveletId() );
        if ( !m_wavelet ) { XMPPERROR("Unknown wavelet"); }
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
        writer.writeAttribute("id", m_stanza.stanzaId() );
        writer.writeAttribute("to", connection()->domain() );
        writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
        writer.writeStartElement("pubsub");
        writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
        writer.writeStartElement("items");

        m_end = qMin( m_wavelet->version(), m_end );
        m_start = qMin( m_wavelet->version(), m_start );
        for( qint64 v = m_start; v <= m_end; ++v )
        {
            const AppliedWaveletDelta* delta = m_wavelet->delta(v);
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
        writer.writeAttribute("version", QString::number(m_wavelet->version()) );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeStartElement("item");
        writer.writeStartElement("history-truncated");
        writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
        writer.writeAttribute("version", QString::number(m_end) );
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();
        writer.writeEndElement();

        connection()->send( send );
    }

    END_EXECUTE;
}
