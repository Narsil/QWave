#include "clientactorfolk.h"

ClientActorFolk* ClientActorFolk::s_folk = 0;

ClientActorFolk::ClientActorFolk(QObject* parent)
        : ActorFolk( ActorId::Client, parent )
{
    // Start dispatching messages
    activate();
}

ClientConnection* ClientActorFolk::newClientConnection( QTcpSocket* socket )
{
    ClientConnection* con = new ClientConnection( socket, this );
    m_connections[con->id()] = con;
    return con;
}

ActorGroup* ClientActorFolk::group( const ActorId& id )
{
    return m_connections[ id.group() ];
}

ClientActorFolk* ClientActorFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new ClientActorFolk();
    return s_folk;
}
