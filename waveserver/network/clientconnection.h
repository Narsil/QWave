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
class Wavelet;
class WaveletDelta;
class AppliedWaveletDelta;
class QByteArray;
class QTcpSocket;
class Participant;
class ClientActorFolk;

namespace waveserver
{
    class ProtocolSubmitResponse;
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

    Participant* participant() const { return m_participant; }
    /**
      * A locally unique ID for this connection.
      */
//    QString id() const { return m_id; }
    /**
      * The domain of this wave server.
      */
    QString domain() const;

    void sendWaveletUpdate( Wavelet* wavelet, const QList<AppliedWaveletDelta>& delta );
    void sendSubmitResponse( const waveserver::ProtocolSubmitResponse& response );
    void sendSubmitResponse( qint32 operations_applied, const WaveletDelta::HashedVersion* hashedVersionAfterApplication, const QString& errorMessage = QString::null );
    void sendIndexUpdate(Wavelet* wavelet, const WaveletDelta& indexDelta);

    static ClientConnection* connectionById( const QString& id );
    static QList<ClientConnection*> connectionsByParticipant( const QString& participant );

private slots:
    void getOffline();
    void networkError();
    void messageReceived(const QString& methodName, const QByteArray& data);

private:
    RPC* m_rpc;
    Participant* m_participant;
//    QString m_id;
    qint64 m_digestVersion;
    QByteArray m_digestHash;

    static QMultiHash<QString,ClientConnection*>* s_connectionsByParticipant;
    static QHash<QString,ClientConnection*>* s_connectionsById;
};

#endif // CLIENTCONNECTION_H
