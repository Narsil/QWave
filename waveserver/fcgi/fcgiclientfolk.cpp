#include "fcgiclientfolk.h"
#include "model/jid.h"

FCGIClientActorFolk* FCGIClientActorFolk::s_folk = 0;

FCGIIClientActorFolk::FCGIClientActorFolk(QObject* parent)
        : ActorFolk( "webclient", parent )
{
}

FCGIClientConnection* FCGIClientActorFolk::newClientConnection( const QString& id )
{
    FCGIClientConnection* con = new FCGIClientConnection( id, this );
    return con;
}

ActorGroup* FCGIClientActorFolk::group( const QString& groupId, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    ActorGroup* g = this->ActorFolk::group( groupId, createOnDemand );
    if ( g )
        return g;

    return newClientConnection( groupId );
}

FCGIClientActorFolk* FCGIClientActorFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new FCGIClientActorFolk();
    return s_folk;
}
