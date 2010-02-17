#include "actorgroup.h"
#include "actorid.h"
#include "actordispatcher.h"
#include "actorfolk.h"

ActorGroup::ActorGroup(const QString& groupId, ActorFolk* folk)
        : QObject( folk ), m_destructed(false), m_active(false), m_parentGroup(0), m_folk(folk)
{
    setObjectName( groupId );
}

ActorGroup::ActorGroup(const QString& groupId, ActorGroup* parentGroup)
        : QObject( parentGroup ), m_destructed(false), m_active(false), m_parentGroup(parentGroup), m_folk(0)
{
    setObjectName( groupId );
    m_parentGroup->addGroup(this);
}

ActorGroup::~ActorGroup()
{
    if ( m_parentGroup )
        m_parentGroup->removeGroup(this);
    m_destructed = true;
    foreach( Actor* actor, m_actors )
    {
        delete actor;
    }
}

void ActorGroup::dispatch( const QSharedPointer<IMessage>& message )
{
    Actor* actor = message->receiver();
    if ( actor )
    {
        if ( m_actors.contains( actor ) )
            process( message, actor );
    }
    else
    {
        for( int i = 0; i < m_actors.count(); ++i )
        {
            Actor* actor = m_actors[i];
            process( message, actor );
        }
    }
}

void ActorGroup::process( const QSharedPointer<IMessage>& message, Actor* actor )
{
    if ( actor->process( message) )
    {
        bool result = actor->run();
        if ( !result )
        {
            qDebug("Delete actor");
            actor->deleteLater();
        }
    }
}

void ActorGroup::addActor( Actor* actor )
{
    Q_ASSERT( !m_destructed );
    Q_ASSERT( folk() != 0 );

    qDebug("Add actor %s to %s", actor->actorId().toString().toAscii().constData(), this->actorId().toString().toAscii().constData() );

    actor->setActorGroup( this );
    m_actors.append( actor );

    bool old = m_active;
    m_active = true;
    if ( !actor->run() )
    {
        m_active = old;
        qDebug("Actor finished on the first run");
        actor->deleteLater();
        return;
    }
    m_active = old;

    if ( !m_active )
        run();
}

void ActorGroup::removeActor( Actor* actor )
{
    qDebug("Remove actor");

    if ( m_destructed )
        return;
    m_actors.removeOne( actor );
}

void ActorGroup::addGroup( ActorGroup* group )
{
    Q_ASSERT( !m_destructed );
    Q_ASSERT( folk() != 0 );

    m_groups.append( group );
}

void ActorGroup::removeGroup( ActorGroup* group )
{
    if ( m_destructed )
        return;
    m_groups.removeOne( group );
}

void ActorGroup::enqueue( IMessage* msg )
{
    QSharedPointer<IMessage> message(msg);
    enqueue( message );
}

void ActorGroup::enqueue( const QSharedPointer<IMessage>& message )
{
    if ( !m_active && m_queue.isEmpty() )
    {
        m_active = true;
        dispatch( message );
        m_active = false;
        if ( !m_queue.isEmpty() )
            run();
        return;
    }
    m_queue.enqueue( message );
}

bool ActorGroup::enqueue( const ActorId& actorId, const QSharedPointer<IMessage>& message )
{
    if ( actorId.hasActor() )
    {
        Actor* a = actor( actorId, false );
        if ( !a )
        {
            qDebug("Unknown actor %s in %s", actorId.toString().toAscii().constData(), this->actorId().toString().toAscii().constData() );
            return false;
        }
        message->setReceiver( a );
    }
    enqueue(message);
    return true;
}

void ActorGroup::run()
{
    m_active = true;

    while( !m_queue.isEmpty() )
    {
        QSharedPointer<IMessage> msg = m_queue.dequeue();
        dispatch( msg );
    }

    m_active = false;
}

Actor* ActorGroup::actor( const ActorId& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    for( int i = 0; i < m_actors.count(); ++i )
    {
        Actor* a = m_actors[i];
        if ( a->actorId().actor() == id.actor() )
            return a;
    }
    return 0;
}

ActorGroup* ActorGroup::group( const QString& id, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    for( int i = 0; i < m_groups.count(); ++i )
    {
        ActorGroup* a = m_groups[i];
        if ( a->groupId() == id )
            return a;
    }
    return 0;
}

bool ActorGroup::send( const ActorId& id, IMessage* msg )
{
    return ActorDispatcher::dispatcher()->send( id, msg );
}

QString ActorGroup::absGroupId() const
{
    if ( m_parentGroup )
    {
        QString id = m_parentGroup->groupId();
        id += "/";
        id += objectName();
        return id;
    }
    return objectName();
}

ActorId ActorGroup::actorId() const
{
    ActorFolk* f = folk();
    if ( !f )
        return ActorId();
    return ActorId( f->folkId(), absGroupId() );
}

ActorFolk* ActorGroup::folk() const
{
    if ( m_folk )
        return m_folk;
    if ( m_parentGroup )
        return m_parentGroup->folk();
    return 0;
}
