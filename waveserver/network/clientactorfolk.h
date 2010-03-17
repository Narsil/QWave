#ifndef CLIENTACTORFOLK_H
#define CLIENTACTORFOLK_H

#include "actor/actorfolk.h"

class QTcpSocket;
class ClientConnection;
class FCGIClientConnection;

class ClientActorFolk : public ActorFolk
{
    Q_OBJECT
public:
    ClientActorFolk(QObject* parent = 0);

    ClientConnection* newClientConnection( QTcpSocket* socket );
    FCGIClientConnection* newFCGIClientConnection( const QString& sessionId, const QString& jid );

    static ClientActorFolk* instance();

    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private slots:
    /**
      * Delete dead client connections, i.e. dead sessions.
      */
    void periodicCleanup();

private:
    static ClientActorFolk* s_folk;
};

#endif // CLIENTACTORFOLK_H
