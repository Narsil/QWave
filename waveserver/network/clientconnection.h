#ifndef CLIENTCONNECTION_H
#define CLIENTCONNECTION_H

#include <QObject>
#include <QMultiHash>
#include <QHash>
#include <QString>
#include <QList>
#include <QByteArray>
#include "model/waveletdelta.h"
#include "actor/actorgroup.h"

class RPC;
class QByteArray;
class QTcpSocket;
class ClientActorFolk;

namespace waveserver
{
    class ProtocolSubmitResponse;
    class ProtocolWaveletUpdate;
}

namespace protocol
{
    class ProtocolWaveletDelta;
}

class ClientConnection : public ActorGroup
{
    Q_OBJECT
public:
    /**
      * @internal
      *
      * Do not create a connection directly. Use ClientActorFolk instead.
      */
    ClientConnection(QTcpSocket* socket, ClientActorFolk* parent = 0);
    ~ClientConnection();

    QString participant() const { return m_participant; }
    /**
      * The domain of this wave server.
      */
    QString domain() const;

    /**
      * @internal
      *
      * Called from ClientSubmitRequestActor.
      */
    void sendSubmitResponse( const waveserver::ProtocolSubmitResponse& response );
    /**
      * @internal
      *
      * Called from ClientSubmitRequestActor.
      */
    void sendFailedSubmitResponse( const QString& errorMessage );

protected:
    virtual void customEvent( QEvent* event );

private slots:
    /**
      * Connected to the RPC.
      */
    void getOffline();
    /**
      * Connected to the RPC.
      */
    void networkError();
    /**
      * Connected to the RPC.
      */
    void messageReceived(const QString& methodName, const QByteArray& data);

private:
    void sendIndexUpdate(const QString& waveletName, protocol::ProtocolWaveletDelta* delta);
    void sendWaveletUpdate( const waveserver::ProtocolWaveletUpdate& update );

    RPC* m_rpc;
    QString m_participant;
    qint64 m_digestVersion;
    QByteArray m_digestHash;
    bool m_waveletsOpened;
};

#endif // CLIENTCONNECTION_H
