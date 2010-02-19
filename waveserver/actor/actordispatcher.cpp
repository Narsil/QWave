#include "actor/actordispatcher.h"
#include "actor/actorfolk.h"
#include "actor/actorgroup.h"
#include "actor/actor.h"
#include "actor/imessage.h"

#include <QCoreApplication>

ActorDispatcher* ActorDispatcher::s_dispatcher = 0;

ActorDispatcher::ActorDispatcher()
{
}

ActorFolk* ActorDispatcher::folk( const QString& folk )
{
    return m_folks[folk];
}

void ActorDispatcher::addFolk( ActorFolk* folk )
{
    m_folks[folk->folkId()] = folk;
}

void ActorDispatcher::removeFolk( ActorFolk* folk )
{
    m_folks.remove( folk->folkId() );
}

QObject* ActorDispatcher::lookup( const ActorId& id, bool createOnDemand )
{
    if ( id.isNull() )
        return 0;
    ActorFolk* folk = m_folks[id.folk()];
    if ( !folk )
    {
        qDebug("Could not resolve folk %s", id.folk().toAscii().constData() );
        return 0;
    }
    ActorGroup* group = folk->group( id, createOnDemand );
    if ( !group )
        return 0;
    if ( !id.hasActor() )
        return group;
    return group->actor( id.actor(), createOnDemand );
}

bool ActorDispatcher::send( IMessage* msg )
{
    QObject* receiver = lookup( msg->receiver(), msg->createOnDemand() );
    if ( !receiver )
        return false;
    return QCoreApplication::sendEvent( receiver, msg );
}

bool ActorDispatcher::post( IMessage* msg )
{
    QObject* receiver = lookup( msg->receiver(), msg->createOnDemand() );
    if ( !receiver )
        return false;
    QCoreApplication::postEvent( receiver, msg );
    return true;
}

ActorDispatcher* ActorDispatcher::dispatcher()
{
    if ( s_dispatcher == 0 )
        s_dispatcher = new ActorDispatcher();
    return s_dispatcher;
}
