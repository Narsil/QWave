#ifndef CLIENTACTOR_H
#define CLIENTACTOR_H

#include "actor/actor.h"
#include "actor/actorid.h"

class ClientConnection;

class ClientActor : public Actor
{
public:
    ClientActor( ClientConnection* con );

    inline ClientConnection* connection() { return m_connection; }

    const ActorId& actorId() const { return m_actorId; }

protected:
    void log( const char* error, const char* file, int line );
    void log( const QString& error, const char* file, int line );
    void logErr( const char* error, const char* file, int line );
    void logErr( const QString& error, const char* file, int line );

private:
    ClientConnection* m_connection;
    ActorId m_actorId;

    static quint64 s_id;
};

#endif // CLIENTACTOR_H