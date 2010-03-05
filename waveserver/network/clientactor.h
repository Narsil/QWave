#ifndef CLIENTACTOR_H
#define CLIENTACTOR_H

#include "actor/actor.h"
#include "actor/actorid.h"

class ClientConnection;
class FCGIClientConnection;

class ClientActor : public Actor
{
public:
    ClientActor( ClientConnection* con );
    ClientActor( FCGIClientConnection* con );

    ActorGroup* connection();

    inline ClientConnection* pbConnection() { return m_pbConnection; }
    inline FCGIClientConnection* fcgiConnection() { return m_fcgiConnection; }

    QString participant() const;

protected:
    void log( const char* error, const char* file, int line );
    void log( const QString& error, const char* file, int line );
    void logErr( const char* error, const char* file, int line );
    void logErr( const QString& error, const char* file, int line );

private:
    ClientConnection* m_pbConnection;
    FCGIClientConnection* m_fcgiConnection;

    static quint64 s_id;
};

#endif // CLIENTACTOR_H
