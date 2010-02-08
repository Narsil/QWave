#ifndef XMPPVIRTUALCONNECTION_H
#define XMPPVIRTUALCONNECTION_H

#include "network/xmppstanza.h"
#include "actor/actorgroup.h"

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

class XmppComponentConnection;
class AppliedWaveletDelta;
class WaveUrl;

namespace protocol
{
    class ProtocolWaveletDelta;
}

class XmppVirtualConnection : public ActorGroup
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
    void sendSubmitRequest( const WaveUrl& url, const protocol::ProtocolWaveletDelta& delta );

    /**
      * During wave server lookup, this property holds a domain such as "wave2.vs.uni-due.de", i.e. the JID of the XMPP server.
      * After lookup, this property is changed to "wave.wave2.vs.uni-due.de", i.e. the JID of the wave server component.
      */
    QString domain() const { return m_domain; }
    void setDomain( const QString& domain );

    XmppComponentConnection* component() const { return m_component; }

    void xmppError();

    bool send( const QString& stanza );

    /**
      * @internal
      */
    void setReady();
    bool isReady() const { return m_ready; }

signals:
    void ready();

protected:
    virtual void dispatch( const QSharedPointer<IMessage>& message );

private:
    /**
      * The function takes ownership of the generated stanza.
      */
    void send( XmppStanza* stanza );

    void processMessage( const XmppStanza& stanza );

    enum State
    {
        Init,
        DiscoItems,
        Established,
        Delete,
        Error
    } m_state;

    XmppComponentConnection* m_component;
    QString m_domain;
    QQueue<XmppStanza*> m_stanzaQueue;
    bool m_signerInfoSent;
    bool m_ready;
};

#endif // XMPPVIRTUALCONNECTION_H
