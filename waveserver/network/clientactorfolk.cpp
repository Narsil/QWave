#include "clientactorfolk.h"

ClientActorFolk* ClientActorFolk::s_folk = 0;

ClientActorFolk::ClientActorFolk(QObject* parent)
        : ActorFolk( "client", parent )
{
    // Start dispatching messages
    activate();
}

ClientConnection* ClientActorFolk::newClientConnection( QTcpSocket* socket )
{
    ClientConnection* con = new ClientConnection( socket, this );
    m_connections[con->groupId()] = con;
    return con;
}

ActorGroup* ClientActorFolk::group( const QString& groupId, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    return m_connections[ groupId ];
}

ClientActorFolk* ClientActorFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new ClientActorFolk();
    return s_folk;
}
