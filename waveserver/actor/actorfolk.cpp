#include "actorfolk.h"
#include "actordispatcher.h"
#include "actorgroup.h"

ActorFolk::ActorFolk(ActorId::Folk folk, QObject* parent)
        : QObject( parent ), m_folk( folk )
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
    Q_ASSERT( actor.folk() == m_folk );

    ActorGroup* g = group( actor );
    if ( !g )
        return false;
    return g->enqueue( actor, message );
}

