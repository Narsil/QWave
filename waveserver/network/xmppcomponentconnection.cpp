#include "xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"
#include "app/settings.h"
#include "model/appliedwaveletdelta.h"
#include "model/signedwaveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/certificatestore.h"
#include "model/waveurl.h"
#include "network/servercertificate.h"
#include "network/xmppstanza.h"

#include <QByteArray>
#include <QXmlStreamAttributes>
#include <QXmlStreamWriter>
#include <QCryptographicHash>
#include <QtGlobal>

XmppComponentConnection* XmppComponentConnection::s_connection = 0;

XmppComponentConnection::XmppComponentConnection(QObject* parent)
        : ActorGroup(parent), m_state( Init ), m_connected(false), m_writer(0), m_idCount(0), m_currentStanza(0), m_currentTag(0)
{
    // Setup the XML reader
    m_reader.setNamespaceProcessing(false);

    s_connection = this;

    // Create a connection to the local XMPP server
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
                // Expect <stream:stream being sent from the server
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
                // Extract the stream ID
                m_streamId = attribs.value("id").toString();

                // Send the handshake
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
                // Expect <handshake> from the server
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
                // Expect </handshake> from the server
                if ( token != QXmlStreamReader::EndElement || m_reader.name() != "handshake" )
                {
                    xmppError();
                    return;
                }

                emit ready();

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
                    {
                        QString name = m_reader.qualifiedName().toString();
                        // Check which kind of message this might be.
                        // This can contain false positives, but only for malformed stanzas.
                        // The code handling the stanzas will check for wellformedness. XML sucks
                        if ( m_currentStanza && m_currentStanza->kind() == XmppStanza::Unknown )
                        {
                            if ( name == "wavelet-update" )
                                m_currentStanza->setKind( XmppStanza::WaveletUpdate );
                            else if ( name == "received" )
                                m_currentStanza->setKind( XmppStanza::MessageReceipt );
                            else if ( name == "delta-history" )
                                m_currentStanza->setKind( XmppStanza::HistoryRequest );
                            else if ( name == "applied-delta" && m_currentStanza->qualifiedName() == "iq" )
                                m_currentStanza->setKind( XmppStanza::HistoryResponse );
                            else if ( name == "signer-request" )
                                m_currentStanza->setKind( XmppStanza::SignerRequest );
                            else if ( name == "signature" && m_currentStanza->type() == "result" )
                                m_currentStanza->setKind( XmppStanza::SignerResponse );
                            else if ( name == "signature" && m_currentStanza->type() == "set" )
                                m_currentStanza->setKind( XmppStanza::PostSigner );
                            else if ( name == "signature-response" && m_currentStanza->type() == "set" )
                                m_currentStanza->setKind( XmppStanza::PostSignerResponse );
                            else if ( name == "submit-request" )
                                m_currentStanza->setKind( XmppStanza::SubmitRequest );
                            else if ( name == "submit-response" )
                                m_currentStanza->setKind( XmppStanza::SubmitResponse );
                            else if ( name == "query" && m_reader.attributes().value("xmlns") == "http://jabber.org/protocol/disco#items" )
                            {
                                if ( m_currentStanza->type() == "get" )
                                    m_currentStanza->setKind( XmppStanza::DiscoItems );
                                else if ( m_currentStanza->type() == "result" )
                                    m_currentStanza->setKind( XmppStanza::DiscoItemsResponse );
                            }
                            else if ( name == "query" && m_reader.attributes().value("xmlns") == "http://jabber.org/protocol/disco#info" )
                            {
                                if ( m_currentStanza->type() == "get" )
                                    m_currentStanza->setKind( XmppStanza::DiscoInfo );
                                else if ( m_currentStanza->type() == "result" )
                                    m_currentStanza->setKind( XmppStanza::DiscoInfoResponse );
                            }
                        }
                        if ( m_currentTag == 0 )
                        {
                            XmppStanza* stanza = new XmppStanza( name, m_reader.attributes() );
                            m_currentStanza = stanza;
                            m_currentTag = stanza;
                            if ( (name == "iq" || name == "message") && stanza->type() == "error" )
                                stanza->setKind( XmppStanza::Error );
                        }
                        else
                        {
                            XmppTag* tag = new XmppTag( name, m_reader.attributes(), m_currentTag );
                            m_currentTag->add( tag );
                            m_currentTag = tag;
                        }
                    }
                    break;
                case QXmlStreamReader::EndElement:
                    if ( m_currentTag == 0 )
                        xmppError();
                    // Did this close the top-level tag? -> Stanza is complete
                    else if ( m_currentTag->parent() == 0 )
                    {
                        XmppStanza* stanza = (XmppStanza*)m_currentTag;
                        // Get the actor group which handles messages from this remote host
                        XmppVirtualConnection* con = virtualConnection(stanza->from(), false);
                        m_currentTag = 0;
                        m_currentStanza = 0;

                        // Send the stanze as an IMessage to the actor group that processes messages from this remote host.
                        // Ownership of the message is transfered -> no delete required.
                        con->enqueue( stanza );
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

bool XmppComponentConnection::send( const QString& stanza )
{
    if ( !m_connected )
        return false;

    qDebug("msg>>> %s", stanza.toAscii().constData() );
    (*m_writer) << stanza;
    m_writer->flush();
    return true;
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
