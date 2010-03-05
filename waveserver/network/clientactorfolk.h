#ifndef CLIENTACTORFOLK_H
#define CLIENTACTORFOLK_H

#include "actor/actorfolk.h"

class QTcpSocket;
class ClientConnection;
class FCGIClientConnection;

class ClientActorFolk : public ActorFolk
{
public:
    ClientActorFolk(QObject* parent = 0);

    ClientConnection* newClientConnection( QTcpSocket* socket );
    FCGIClientConnection* newFCGIClientConnection( const QString& sessionId, const QString& jid );

    static ClientActorFolk* instance();

    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:
    static ClientActorFolk* s_folk;
};

#endif // CLIENTACTORFOLK_H
