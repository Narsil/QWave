#include "clientactorfolk.h"
#include "clientparticipant.h"
#include "model/jid.h"

ClientActorFolk* ClientActorFolk::s_folk = 0;

ClientActorFolk::ClientActorFolk(QObject* parent)
        : ActorFolk( "client", parent )
{
}

ClientConnection* ClientActorFolk::newClientConnection( QTcpSocket* socket )
{
    ClientConnection* con = new ClientConnection( socket, this );
    // m_connections[con->groupId()] = con;
    return con;
}

ActorGroup* ClientActorFolk::group( const QString& groupId, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    // ActorGroup* g = m_connections[ groupId ];
    ActorGroup* g = this->ActorFolk::group( groupId, createOnDemand );
    if ( g )
        return g;

    JID jid( groupId );
    if ( jid.isValid() )
    {
        g = new ClientParticipant( groupId );
        return g;
    }
    return 0;
}

ClientActorFolk* ClientActorFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new ClientActorFolk();
    return s_folk;
}
