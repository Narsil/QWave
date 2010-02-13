#include "actor/actordispatcher.h"
#include "actor/actorfolk.h"
#include "actor/actorgroup.h"
#include "actor/actor.h"

ActorDispatcher* ActorDispatcher::s_dispatcher = 0;

ActorDispatcher::ActorDispatcher()
{
    for( int i = 0; i < ActorId::MAX_FOLK; ++i )
    {
        m_folks[i] = 0;
    }
}

ActorFolk* ActorDispatcher::folk( ActorId::Folk folk )
{
    return m_folks[folk];
}

void ActorDispatcher::addFolk( ActorFolk* folk )
{
    m_folks[folk->folk()] = folk;
}

void ActorDispatcher::removeFolk( ActorFolk* folk )
{
    if ( m_folks[folk->folk()] == folk )
        m_folks[folk->folk()] = 0;
}

bool ActorDispatcher::send( const ActorId& actor, const QSharedPointer<IMessage>& message )
{
    if ( actor.isNull() )
        return false;
    ActorFolk* folk = m_folks[actor.folk()];
    if ( !folk )
        return false;
    return folk->enqueue( actor, message );
}

bool ActorDispatcher::send( const ActorId& actor, IMessage* msg )
{
    return send( actor, QSharedPointer<IMessage>( msg ) );
}

ActorDispatcher* ActorDispatcher::dispatcher()
{
    if ( s_dispatcher == 0 )
        s_dispatcher = new ActorDispatcher();
    return s_dispatcher;
}
