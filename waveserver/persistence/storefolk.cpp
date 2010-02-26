#include "storefolk.h"
#include "store.h"

StoreFolk* StoreFolk::s_store = 0;

StoreFolk::StoreFolk()
        : ActorFolk( "store")
{
    m_store = new Store( "main", this );
}

StoreFolk* StoreFolk::store()
{
    if ( s_store == 0 )
        s_store = new StoreFolk();
    return s_store;
}

ActorGroup* StoreFolk::group( const QString& id, bool createOnDemand )
{
    Q_UNUSED( id )
    Q_UNUSED( createOnDemand )

    // TODO: This is a shortcut. There should be multiple stores
    return m_store;
}
