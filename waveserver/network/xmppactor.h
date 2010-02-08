#ifndef XMPPACTOR_H
#define XMPPACTOR_H

#include "actor/actor.h"

class XmppVirtualConnection;

class XmppActor : public Actor
{
public:
    XmppActor(XmppVirtualConnection* con);

    XmppVirtualConnection* connection() const { return m_connection; }

    void log( const char* error, const char* file, int line );
    void log( const QString& error, const char* file, int line );
    void logErr( const char* error, const char* file, int line );
    void logErr( const QString& error, const char* file, int line );


private:
    XmppVirtualConnection* m_connection;
};

#endif // XMPPACTOR_H
