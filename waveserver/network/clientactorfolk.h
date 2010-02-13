#ifndef CLIENTACTORFOLK_H
#define CLIENTACTORFOLK_H

#include <QHash>

#include "actor/actorfolk.h"
#include "network/clientconnection.h"

class QTcpSocket;

class ClientActorFolk : public ActorFolk
{
public:
    ClientActorFolk(QObject* parent = 0);

    ClientConnection* newClientConnection( QTcpSocket* socket );

    static ClientActorFolk* instance();

protected:
    virtual ActorGroup* group( const ActorId& id );

private:
    QHash<QString,ClientConnection*> m_connections;

    static ClientActorFolk* s_folk;
};

#endif // CLIENTACTORFOLK_H
