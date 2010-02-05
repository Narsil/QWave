#include "xmppcomponent.h"
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

XmppComponentConnection* XmppComponentConnection::s_connection = 0;

XmppComponentConnection::XmppComponentConnection(QObject* parent)
        : QObject(parent), m_state( Init ), m_connected(false), m_writer(0), m_idCount(0), m_currentTag(0)
{
    m_reader.setNamespaceProcessing(false);

    s_connection = this;

    m_socket = new QTcpSocket(this);

    bool ok = connect( m_socket, SIGNAL(connected()), SLOT(start()));
    Q_ASSERT(ok);
    ok = connect( m_socket, SIGNAL(disconnected()), SLOT(stop()));
    Q_ASSERT(ok);
    ok = connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(stopOnError(QAbstractSocket::SocketError)));
    Q_ASSERT(ok);
    ok = connect( m_socket, SIGNAL(readyRead()), SLOT(readBytes()));
    Q_ASSERT(ok);

    m_socket->connectToHost( Settings::settings()->xmppServerName(), Settings::settings()->xmppComponentPort());
}

XmppComponentConnection::~XmppComponentConnection()
{
    if ( m_writer )
        delete m_writer;
    if ( m_currentTag != 0 )
        delete m_currentTag;
    m_virtualConnections.clear();
    m_socket->deleteLater();
    m_state = Delete;
}

void XmppComponentConnection::stop()
{
    if ( m_state == Delete )
        return;
    m_connected = false;
    qDebug("Disconnected");
}

void XmppComponentConnection::stopOnError(QAbstractSocket::SocketError error)
{
    if ( m_state == Delete )
        return;
    Q_UNUSED(error);
    qDebug("Socket error");
    m_state = Delete;
}

void XmppComponentConnection::xmppError()
{
    if ( m_state == Delete )
        return;
    qDebug("XMPP error");
    m_state = Delete;
}

void XmppComponentConnection::start()
{
    if ( m_state == Delete )
        return;

    m_connected = true;
    qDebug("Connected");
    m_writer = new QTextStream(m_socket);
    m_writer->setCodec("UTF-8");
    QString stream = "<stream:stream xmlns=\"jabber:component:accept\" xmlns:stream=\"http://etherx.jabber.org/streams\" to=\"" + Settings::settings()->xmppComponentName() + "\">";
    qDebug("msg>>> %s", stream.toAscii().constData() );
    (*m_writer) << stream;
    m_writer->flush();
}

void XmppComponentConnection::readBytes()
{
    if ( m_state == Delete )
        return;

    qint64 len = m_socket->bytesAvailable();
    qDebug("Got %i bytes", (int)len);
    QByteArray data = m_socket->readAll();
    qDebug("<<<msg %s", data.constData() );
    m_reader.addData( data );

    while( !m_reader.atEnd() )
    {
        QXmlStreamReader::TokenType token = m_reader.readNext();
        if ( token == QXmlStreamReader::Invalid || token == QXmlStreamReader::NoToken )
            return;
        if ( token == QXmlStreamReader::StartDocument )
        {
            token = m_reader.readNext();
            if ( token == QXmlStreamReader::Invalid || token == QXmlStreamReader::NoToken )
                return;
        }

        switch( m_state )
        {
        case Init:
            {
                if ( token != QXmlStreamReader::StartElement || m_reader.qualifiedName() != "stream:stream" )
                {
                    xmppError();
                    return;
                }
                QXmlStreamAttributes attribs = m_reader.attributes();
                if ( !attribs.hasAttribute("id") )
                {
                    xmppError();
                    return;
                }
                m_streamId = attribs.value("id").toString();

                // Send the handshak
                QCryptographicHash hash( QCryptographicHash::Sha1 );
                hash.addData( m_streamId.toAscii().constData() );
                hash.addData( Settings::settings()->xmppComponentSecret().toAscii().constData() );
                QByteArray result = hash.result();
                QString key = "";
                QString format = "%1";
                unsigned char* p = (unsigned char*)result.data();
                for( int i = 0; i < result.length(); ++i )
                {
                    key += format.arg( p[i], 2, 16, QLatin1Char('0') );
                }
                qDebug("Answer is %s", key.toAscii().constData() );
                (*m_writer) << "<handshake>" + key + "</handshake>";
                m_writer->flush();
                m_state = HandshakeStart;
            }
            break;
        case HandshakeStart:
            {
                if ( token != QXmlStreamReader::StartElement || m_reader.name() != "handshake" )
                {
                    xmppError();
                    return;
                }
                m_state = HandshakeEnd;
            }
            break;
        case HandshakeEnd:
            {
                if ( token != QXmlStreamReader::EndElement || m_reader.name() != "handshake" )
                {
                    xmppError();
                    return;
                }

                // Send everything that has queued up so far
                while( !m_stanzaQueue.isEmpty() )
                {
                    QString stanza = m_stanzaQueue.dequeue();
                    qDebug("msg>>> %s", stanza.toAscii().constData() );
                    (*m_writer) << stanza;
                }
                m_writer->flush();

                m_state = Established;                
            }
            break;
        case Established:
            // Skip everything until we find a new stanza
            if ( m_currentTag == 0 && token != QXmlStreamReader::StartElement )
                continue;
            switch( token )
            {
                case QXmlStreamReader::NoToken:
                case QXmlStreamReader::Invalid:
                case QXmlStreamReader::StartDocument:
                    xmppError();
                    break;
                case QXmlStreamReader::EndDocument:
                    stop();
                    break;
                case QXmlStreamReader::StartElement:
                    if ( m_currentTag == 0 )
                    {
                        XmppStanza* stanza = new XmppStanza( m_reader.qualifiedName().toString(), m_reader.attributes() );
                        m_currentTag = stanza;
                    }
                    else
                    {
                        XmppTag* tag = new XmppTag( m_reader.qualifiedName().toString(), m_reader.attributes(), m_currentTag );
                        m_currentTag->add( tag );
                        m_currentTag = tag;
                    }
                    break;
                case QXmlStreamReader::EndElement:
                    if ( m_currentTag == 0 )
                        xmppError();
                    else if ( m_currentTag->parent() == 0 )
                    {
                        XmppStanza* stanza = (XmppStanza*)m_currentTag;
                        XmppVirtualConnection* con = virtualConnection(stanza->from(), false);

                        con->process( *stanza );
                        delete stanza;
                        m_currentTag = 0;
                    }
                    else
                        m_currentTag = m_currentTag->parent();
                    break;
                case QXmlStreamReader::Characters:
                    if ( m_currentTag != 0 )
                    {
                        if ( m_reader.isCDATA() )
                            m_currentTag->addCData( m_reader.text().toString() );
                        else
                            m_currentTag->add( m_reader.text().toString() );
                    }
                    break;
                case QXmlStreamReader::Comment:
                case QXmlStreamReader::DTD:
                case QXmlStreamReader::EntityReference:
                case QXmlStreamReader::ProcessingInstruction:
                    // Do nothing by intention
                    break;
            }
            break;
        case Delete:
            break;
        }
    }
    stop();
}

XmppVirtualConnection* XmppComponentConnection::virtualConnection( const QString& domain, bool resolve )
{
    if ( !m_virtualConnections.contains(domain) )
    {
        XmppVirtualConnection* con = new XmppVirtualConnection( this, domain, resolve );
        m_virtualConnections[domain] = con;
    }
    return m_virtualConnections[domain];
}

void XmppComponentConnection::removeVirtualConnection( XmppVirtualConnection* con )
{
    if ( m_state == Delete )
        return;
    m_virtualConnections.remove(con->domain() );
    con->deleteLater();
}

XmppVirtualConnection* XmppComponentConnection::renameVirtualConnection( XmppVirtualConnection* con, const QString& oldName, const QString& newName )
{
    if ( m_virtualConnections.contains( newName ) )
    {
        XmppVirtualConnection* c = m_virtualConnections[ newName ];
        m_virtualConnections[ oldName ] = c;
        return c;
    }
    m_virtualConnections[ newName ] = con;
    return con;
}

void XmppComponentConnection::send( const QString& stanza )
{
    if ( !m_connected )
        m_stanzaQueue.append( stanza );
    else
    {
        qDebug("msg>>> %s", stanza.toAscii().constData() );
        (*m_writer) << stanza;
        m_writer->flush();
    }
}

void XmppComponentConnection::send( const XmppStanza& stanza )
{
    QString result;
    {
        QXmlStreamWriter writer( &result );
        stanza.write( writer );
    }
    send( result );
}

QString XmppComponentConnection::nextId()
{
    int id = qrand() % 10000;
    return QString("%1-%2").arg(id).arg( m_idCount++ );
}

QString XmppComponentConnection::domain() const
{
    return Settings::settings()->domain();
}

QString XmppComponentConnection::host() const
{
    return Settings::settings()->xmppComponentName();
}

/***********************************************
  *
  * XmppVirtualConnection
  *
  **********************************************/

XmppVirtualConnection::XmppVirtualConnection( XmppComponentConnection* connection, const QString& domain, bool resolve )
        : QObject( connection ), m_state( resolve ? Init : Established ), m_connection( connection ), m_domain( domain ), m_signerInfoSent(false)
{
    if ( m_state == Init )
    {
        // Ask the remote server for its wave JID
        QString send;
        {
            QXmlStreamWriter writer( &send );
            writer.writeStartElement("iq");
            writer.writeAttribute("type", "get" );
            writer.writeAttribute("id", m_connection->nextId() );
            writer.writeAttribute("to", m_domain );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("query");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#items" );
            writer.writeEndElement();
            writer.writeEndElement();
        }
        m_connection->send( send );
    }
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

                            XmppVirtualConnection* con = m_connection->renameVirtualConnection( this, m_domain, waveHost );
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
                                writer.writeAttribute("id", m_connection->nextId() );
                                writer.writeAttribute("to", waveHost );
                                writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
                                writer.writeStartElement("query");
                                writer.writeAttribute("xmlns", "http://jabber.org/protocol/disco#info" );
                                writer.writeEndElement();
                                writer.writeEndElement();
                            }
                            m_connection->send( send );

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
            m_connection->send( sendStr );
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
            m_connection->send( sendStr );
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
                m_connection->send( sendStr );
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
                        const AppliedWaveletDelta& delta = wavelet->delta(v);
                        if ( !delta.isNull() )
                        {
                            QString str64 = delta.toBase64();
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
                m_connection->send( sendStr );
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
                XmppTag* item = publish->child( "item" );
                XmppTag* request = item ? item->child("submit-request") : 0;
                XmppTag* delta = request ? request->child("delta") : 0;
                if ( item && request && delta )
                {
                    QString waveletName = delta->attribute("wavelet-name");
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

                    if ( delta->children().count() == 1 && ( delta->childAt(0)->isCData() || delta->childAt(0)->isText() ) )
                    {
                        QString base64 = delta->childAt(0)->text();
                        bool ok;
                        SignedWaveletDelta wdelta = SignedWaveletDelta::fromBase64( base64, &ok );
                        if ( !ok )
                        {
                            qDebug("Could not deserialize wavelet delta");
                            xmppError();
                            return;
                        }
                        qDebug("Got wavelet delta");

                        if ( wdelta.signatures().count() == 0 )
                        {
                            qDebug("No signature found.");
                            xmppError();
                            return;
                        }

                        // Are all signers known to the certificate store?
                        bool allKnown = true;
                        foreach( const Signature& sig, wdelta.signatures() )
                        {
                            const ServerCertificate* cert = CertificateStore::store()->certificate( sig.signerId() );
                            if ( !cert )
                            {
                                qDebug("At least one signer is not known. Move submit-request to the EscrowDeposit.");
                                allKnown = false;

//                                // Send a signer-request
//                                QString sendStr;
//                                {
//                                    QXmlStreamWriter writer( &sendStr );
//                                    writer.writeStartElement("iq");
//                                    writer.writeAttribute("type", "get" );
//                                    writer.writeAttribute("id", m_connection->nextId() );
//                                    writer.writeAttribute("to", m_domain );
//                                    writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
//                                    writer.writeStartElement("pubsub");
//                                    writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub" );
//                                    writer.writeStartElement("items");
//                                    writer.writeAttribute("node", "signer");
//                                    writer.writeStartElement("signer-request");
//                                    writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
//                                    writer.writeAttribute("wavelet-name", waveletName );
//                                    writer.writeAttribute("version", QString::number( wdelta.delta().version().version ) );
//                                    writer.writeAttribute("history-hash", QString::fromAscii( wdelta.delta().version().hash.toBase64() ) );
//                                    writer.writeAttribute("signer-id", QString::fromAscii( sig.signerId().toBase64() ) );
//                                    writer.writeEndElement();
//                                    writer.writeEndElement();
//                                    writer.writeEndElement();
//                                    writer.writeEndElement();
//                                }
//                                m_connection->send( sendStr );
                            }
                            else
                            {
                                // Check signature
                                if ( !cert->verify( wdelta.deltaBytes(), sig.signature() ) )
                                {
                                    qDebug("Certificate and signature do not match");
                                    xmppError();
                                    return;
                                }
                                qDebug("CHECKED Signature successfully");
                            }
                        }
                        if ( !allKnown )
                        {
                            // TODO Send an error to indicate the remote that the signer infor is missing.
                            // TODO: This must wait until the protocol has been updated.
                            qDebug("Submit-request rejected because of missing singer info.");
                            xmppError();
                            return;
                        }

                        // TODO: AppliedWaveletDelta should be able to carry a list of signatures
                        Signature signature = wdelta.signatures()[0];
                        QString err;
                        int version = wavelet->apply( wdelta.delta(), &err, &signature );
                        if ( !err.isEmpty() || version < 0 )
                        {
                            qDebug("Failed to apply wavelet delta: %s", err.toAscii().constData());
                            xmppError();
                            return;
                        }

                        qDebug("Applied wavelet");
                        const AppliedWaveletDelta& applied = wavelet->delta(version - 1);

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
                            writer.writeStartElement("submit-response");
                            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
                            writer.writeAttribute("operations-applied", QString::number(applied.operationsApplied()) );
                            writer.writeAttribute("application-timestamp", QString::number(applied.applicationTime()) );
                            writer.writeStartElement("hashed-version");
                            writer.writeAttribute("version", QString::number(applied.resultingVersion().version) );
                            writer.writeAttribute("history-hash", QString::fromAscii(applied.resultingVersion().hash.toBase64()) );
                            writer.writeEndElement();
                            writer.writeEndElement();
                            writer.writeEndElement();
                            writer.writeEndElement();
                            writer.writeEndElement();
                            writer.writeEndElement();
                        }
                        m_connection->send( sendStr );
                    }
                }
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
                    m_connection->send( sendStr );
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

    if ( stanza["type"] == "normale" )
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
            m_connection->send( sendStr );

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

void XmppVirtualConnection::xmppError()
{
    qDebug("Error talking to servers %s", m_domain.toAscii().constData() );
    m_state = Error;
}

void XmppVirtualConnection::sendWaveletUpdate(const QString& waveletName, const AppliedWaveletDelta& waveletDelta)
{
    QString str64 = waveletDelta.toBase64();

    if ( m_state != Established )
    {
        XmppStanza* stanza = new XmppStanza("message");
        stanza->setAttribute("type", "normal");
        stanza->setAttribute("id", m_connection->nextId());
        stanza->setAttribute("from", Settings::settings()->xmppComponentName());
        // Do not set this property yet
        //        stanza->setAttribute("to", m_domain);
        XmppTag* request = new XmppTag( "request", XmppTag::Element );
        stanza->add(request);
        request->setAttribute("xmlns", "urn:xmpp:receipts");
        XmppTag* event = new XmppTag( "event", XmppTag::Element );
        stanza->add(event);
        event->setAttribute("xmlns", "http://jabber.org/protocol/pubsub#event");
        XmppTag* items = new XmppTag( "items", XmppTag::Element );
        event->add(items);
        XmppTag* item = new XmppTag( "item", XmppTag::Element );
        items->add(item);
        XmppTag* update = new XmppTag( "wavelet-update", XmppTag::Element );
        item->add(update);
        update->setAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver");
        update->setAttribute("wavelet-name", waveletName);
        XmppTag* delta = new XmppTag( "applied-delta", XmppTag::Element );
        update->add(delta);
        XmppTag* cdata = new XmppTag( str64, XmppTag::CData );
        delta->add(cdata);

        send( stanza );
    }
    else
    {
        QString sendStr;
        {
            QXmlStreamWriter writer( &sendStr );
            writer.writeStartElement("message");
            writer.writeAttribute("type", "normal" );
            writer.writeAttribute("id", m_connection->nextId() );
            writer.writeAttribute("to", m_domain );
            writer.writeAttribute("from", Settings::settings()->xmppComponentName() );
            writer.writeStartElement("request");
            writer.writeAttribute("xmlns", "urn:xmpp:receipts" );
            writer.writeEndElement();
            writer.writeStartElement("event");
            writer.writeAttribute("xmlns", "http://jabber.org/protocol/pubsub#event" );
            writer.writeStartElement("items");
            writer.writeStartElement("item");
            writer.writeStartElement("wavelet-update");
            writer.writeAttribute("xmlns", "http://waveprotocol.org/protocol/0.2/waveserver" );
            writer.writeAttribute("wavelet-name", waveletName );
            writer.writeStartElement("applied-delta");
            writer.writeCDATA( str64 );
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
            writer.writeEndElement();
        }
        m_connection->send( sendStr );

        // TODO: Wait for the response and resend if response does not arrive
    }
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
            writer.writeAttribute("id", m_connection->nextId() );
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
        m_connection->send( sendStr );
    }

    SignedWaveletDelta sdelta( delta );
    QString str64 = sdelta.toBase64();

    QString sendStr;
    {
        QXmlStreamWriter writer( &sendStr );
        writer.writeStartElement("iq");
        writer.writeAttribute("type", "set" );
        writer.writeAttribute("id", m_connection->nextId() );
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
    m_connection->send( sendStr );

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
            m_connection->send( *stanza );
            delete stanza;
            break;
        case Delete:
        case Error:
            // Do nothing by intention
            break;
    }    
}

