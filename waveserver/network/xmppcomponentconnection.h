#ifndef XMPPCOMPONENTCONNECTION_H
#define XMPPCOMPONENTCONNECTION_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QTcpSocket>
#include <QTextStream>
#include <QList>
#include <QSharedPointer>
#include <QHash>

#include "actor/actorfolk.h"

class XmppVirtualConnection;
class XmppStanza;
class WaveUrl;
class XmppTag;

class XmppComponentConnection : public ActorFolk
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
    bool isReady() const { return m_connected; }

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
    bool send( const QString& stanza );
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

signals:
    void ready();

protected:
    virtual ActorGroup* group( const ActorId& id );

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
    XmppStanza* m_currentStanza;
    XmppTag* m_currentTag;

    static XmppComponentConnection* s_connection;
};

#endif // XMPPCOMPONENTCONNECTION_H
