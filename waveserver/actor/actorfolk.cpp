#include "actorfolk.h"
#include "actordispatcher.h"
#include "actorgroup.h"

ActorFolk::ActorFolk(const QString& folkId, QObject* parent)
        : QObject( parent ), m_folkId( folkId ), m_isHierarchical( true )
{
}

void ActorFolk::activate()
{
    ActorDispatcher::dispatcher()->addFolk( this );
}

void ActorFolk::deactivate()
{
    ActorDispatcher::dispatcher()->removeFolk( this );
}

bool ActorFolk::enqueue( const ActorId& actor, const QSharedPointer<IMessage>& message )
{
    Q_ASSERT( actor.folk() == m_folkId );

    ActorGroup* g = 0;
    if ( m_isHierarchical )
    {
        QStringList groups = actor.groups();
        for( int i = 0; i < groups.length(); ++i )
        {
            QString gr = groups[i];
            if ( i == 0 )
                g = group( gr, message->createOnDemand() );
            else
                g = g->group( gr, message->createOnDemand() );
            if ( !g )
                return false;
        }
    }
    else
        g = group( actor.group(), message->createOnDemand() );
    if ( !g )
        return false;
    return g->enqueue( actor, message );
}

