#include "xmppcomponent.h"
#include "app/settings.h"
#include "model/waveletdelta.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"

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
                        m_currentTag->add( m_reader.text().toString() );
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
        : QObject( connection ), m_state( resolve ? Init : Established ), m_connection( connection ), m_domain( domain )
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

    if ( stanza.qualifiedName() == "iq" && stanza["type"] == "get" )
        processIqGet( stanza );
    else if ( stanza.qualifiedName() == "message" )
        processMessage( stanza );
    else if ( stanza.qualifiedName() == "iq" && stanza["type"] == "error" )
    {
        xmppError();
        return;
    }
    else
    {
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
                        }

                        if ( m_state != Established )
                        {
                            xmppError();
                            return;
                        }
                    }
                }
                break;
            case Established:
                break;
            case Delete:
                break;
            case Error:
                break;
        }
    }
}

void XmppVirtualConnection::processIqGet( const XmppStanza& stanza )
{
    XmppTag* query = stanza.child("query");
    XmppTag* pubsub = stanza.child("pubsub");

    if ( stanza["type"] == "get" )
    {
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
            if ( items && (*items)["node"] == "signer" )
            {
                // TODO: Check the remainder of this message for plausability

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
                    QList<QByteArray> certificates = m_connection->certificate().toBase64();
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
            else
                qDebug(" ... message is unhandled");
        }
        else
            qDebug(" ... message is unhandled");
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

    // TODO
}

void XmppVirtualConnection::xmppError()
{
    qDebug("Error talking to servers %s", m_domain.toAscii().constData() );
    m_state = Error;
}

void XmppVirtualConnection::sendWaveletUpdate(const QString& waveletName, const WaveletDelta& waveletDelta)
{
    protocol::ProtocolWaveletDelta delta;
    Converter::convert( &delta, waveletDelta);
    QByteArray ba;
    ba.resize( delta.ByteSize() );
    delta.SerializeToArray( ba.data(), ba.count() );

    protocol::ProtocolAppliedWaveletDelta appliedDelta;
    appliedDelta.set_operations_applied( waveletDelta.operations().count() );
    appliedDelta.set_application_timestamp( QDateTime::currentDateTime().toTime_t() );
    protocol::ProtocolHashedVersion* hashed = appliedDelta.mutable_hashed_version_applied_at();
    hashed->set_version( waveletDelta.version().version );
    QByteArray hash = waveletDelta.version().hash;
    hashed->set_history_hash( hash.constData(), hash.length() );
    protocol::ProtocolSignedDelta* signedDelta = appliedDelta.mutable_signed_original_delta();
    signedDelta->set_delta( ba.constData(), ba.length() );
    protocol::ProtocolSignature* signature = signedDelta->add_signature();
    signature->set_signature_algorithm( protocol::ProtocolSignature_SignatureAlgorithm_SHA1_RSA );
    QByteArray signerInfo = m_connection->certificate().signerInfo();
    signature->set_signer_id( signerInfo.constData(), signerInfo.length() );
    QByteArray sig = m_connection->certificate().sign(ba);
    signature->set_signature_bytes( sig.constData(), sig.length() );

    QByteArray ba2;
    ba2.resize( appliedDelta.ByteSize() );
    appliedDelta.SerializeToArray( ba2.data(), ba2.count() );

    QByteArray base64 = ba2.toBase64();
    QString str64 = QString::fromAscii( base64.constData(), base64.length() );

    if ( m_state != Established )
    {
        XmppStanza* stanza = new XmppStanza("message");
        stanza->setAttribute("type", "normal");
        stanza->setAttribute("id", m_connection->nextId());
        stanza->setAttribute("from", Settings::settings()->xmppComponentName());
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
    }
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

