#include "actorfolk.h"
#include "actordispatcher.h"
#include "actorgroup.h"
#include "imessage.h"

ActorFolk::ActorFolk(const QString& folkId, QObject* parent)
        : QObject( parent ), m_isHierarchical( false )
{
    setObjectName( folkId );
    ActorDispatcher::dispatcher()->addFolk( this );
}

ActorFolk::~ActorFolk()
{
    ActorDispatcher::dispatcher()->removeFolk( this );
}

ActorGroup* ActorFolk::group( const ActorId& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    Q_ASSERT( id.folk() == folkId() );

    if ( m_isHierarchical )
    {
        QStringList groups = id.groups();
        ActorGroup* g = 0;
        for( int i = 0; i < groups.length(); ++i )
        {
            QString gr = groups[i];
            if ( i == 0 )
                g = group( gr, createOnDemand );
            else
                g = g->group( gr, createOnDemand );
            if ( !g )
            {
                qDebug("Could not resolve group %s", gr.toAscii().constData() );
                return 0;
            }
        }
        return g;
    }

    return group(id.group(), createOnDemand );
}

ActorGroup* ActorFolk::group( const QString& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    return findDirectChild<ActorGroup>( id );
}

template<class T> T* ActorFolk::findDirectChild( const QString& name )
{
    for( int i = 0; i < children().length(); ++i )
    {
        QObject* c = children().at(i);
        if ( c->objectName() == name )
            return dynamic_cast<T*>( c );
    }
    return 0;
}
