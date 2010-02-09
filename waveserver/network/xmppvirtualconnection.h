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

/**
  * Handles all XMPP stanzas exchanged between this wave server and another remote wave server.
  */
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

    /**
      * Used by the actors to signal an error.
      */
    void xmppError();

    /**
      * Used by the actors to send a message.
      */
    bool send( const QString& stanza );

    /**
      * @internal
      */
    void setReady();
    bool isReady() const { return m_ready; }

    /**
      * Tells whether the post-signer message has already been sent.
      */
    bool hasPostedSigner() const { return m_postedSigner; }
    /**
      * Used by the actor after the post-signer message has been sent and acknowledged.
      */
    void setPostedSigner( bool posted ) { m_postedSigner = posted; }

signals:
    /**
      * Emitted after we verified that the remote xmpp server supports wave.
      */
    void ready();

protected:
    /**
      * Creates new actors based on incoming messages.
      */
    virtual void dispatch( const QSharedPointer<IMessage>& message );

private:
    XmppComponentConnection* m_component;
    QString m_domain;
    bool m_postedSigner;
    bool m_ready;
};

#endif // XMPPVIRTUALCONNECTION_H
