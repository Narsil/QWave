#ifndef XmppComponentConnection_H
#define XmppComponentConnection_H

#include "xmppstanza.h"

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QTcpSocket>
#include <QStringList>
#include <QTextStream>
#include <QXmlStreamAttributes>
#include <QList>
#include <QSharedPointer>
#include <QHash>
#include <QQueue>

typedef QSharedPointer<XmppStanza> XmppStanzaPtr;

class XmppVirtualConnection;
class AppliedWaveletDelta;

class XmppComponentConnection : public QObject
{
    Q_OBJECT
public:
    XmppComponentConnection(QObject* parent = 0);
    ~XmppComponentConnection();

    /**
      * Something like "mycompany.com".
      */
    QString domain() const;
    /**
      * Something like "wave.mycompany.com".
      */
    QString host() const;
    bool isConnected() const { return m_connected; }

    XmppVirtualConnection* virtualConnection( const QString& domain, bool resolve = true );
    /**
      * @internal
      */
    void removeVirtualConnection( XmppVirtualConnection* con );
    /**
      * @internal
      */
    XmppVirtualConnection* renameVirtualConnection( XmppVirtualConnection* con, const QString& oldName, const QString& newName );

    /**
      * @internal
      */
    void send( const QString& stanza );
    /**
      * @internal
      */
    void send( const XmppStanza& stanza );

    /**
      * @internal
      */
    QString nextId();

    /**
      * @return null if no XMPP is configured.
      */
    static XmppComponentConnection* connection() { return s_connection; }

private slots:
    void start();
    void stop();
    void stopOnError(QAbstractSocket::SocketError);
    void readBytes();

private:
    void xmppError();

    enum State
    {
        Init = 0,
        HandshakeStart = 1,
        HandshakeEnd = 2,
        Established = 3,
        Delete = 4
    } m_state;

    bool m_connected;
    QTcpSocket* m_socket;
    QXmlStreamReader m_reader;
    QTextStream* m_writer;
    QString m_streamId;
    QHash<QString,XmppVirtualConnection*> m_virtualConnections;
    int m_idCount;
    QQueue<QString> m_stanzaQueue;    
    XmppTag* m_currentTag;

    static XmppComponentConnection* s_connection;
};

class XmppVirtualConnection : public QObject
{
    Q_OBJECT
public:
    XmppVirtualConnection( XmppComponentConnection* connection, const QString& domain, bool resolve = true );
    /**
      * Do not delete a virtual connection directly. Always go via the XmppComponentConnection::removeVirtualConnection.
      */
    ~XmppVirtualConnection();

    /**
      * Sends a stanza to the remote wave server indicating a wavelet update.
      */
    void sendWaveletUpdate(const QString& waveletName, const AppliedWaveletDelta& delta);

    /**
      * During wave server lookup, this property holds a domain such as "wave2.vs.uni-due.de", i.e. the JID of the XMPP server.
      * After lookup, this property is changed to "wave.wave2.vs.uni-due.de", i.e. the JID of the wave server component.
      */
    QString domain() const { return m_domain; }
    /**
      * @return true if the virtual connection could not be established, i.e. the remote wave server did not answer or
      * is incompatible.
      */
    bool hasError() const { return m_state == Error; }

    void process( const XmppStanza& stanza );

private:
    /**
      * The function takes ownership of the generated stanza.
      */
    void send( XmppStanza* stanza );

    void xmppError();

    void processIqGet( const XmppStanza& stanza );
    void processMessage( const XmppStanza& stanza );

    enum State
    {
        Init,
        DiscoItems,
        Established,
        Delete,
        Error
    } m_state;

    XmppComponentConnection* m_connection;
    QString m_domain;
    QQueue<XmppStanza*> m_stanzaQueue;
};

#endif // XmppComponentConnection_H
