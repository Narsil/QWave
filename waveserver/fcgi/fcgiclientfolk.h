#ifndef FCGICLIENTFOLK_H
#define FCGICLIENTFOLK_H

#include "actor/actorfolk.h"
#include "fcgi/fcgiclientconnection.h"

class FCGIClientActorFolk : public ActorFolk
{
public:
    FCGIClientActorFolk(QObject* parent = 0);

    FCGIClientConnection* newClientConnection( const QString& id );

    static FCGIClientActorFolk* instance();

    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:

    static FCGIClientActorFolk* s_folk;
};

#endif // FCGICLIENTFOLK_H
