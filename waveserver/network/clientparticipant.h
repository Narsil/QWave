#ifndef CLIENTPARTICIPANT_H
#define CLIENTPARTICIPANT_H

#include "actor/actorgroup.h"

class ClientConnection;

class ClientParticipant : public ActorGroup
{
public:
    ClientParticipant(const QString& jid);

protected:
    virtual void customEvent( QEvent* event );
};

#endif // CLIENTPARTICIPANT_H
