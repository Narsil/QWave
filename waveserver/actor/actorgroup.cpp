#include "actorgroup.h"

ActorGroup::ActorGroup(QObject* parent)
        : QObject( parent ), m_destructed(false), m_active(false)
{
}

ActorGroup::~ActorGroup()
{
    m_destructed = true;
    foreach( Actor* actor, m_actors )
    {
        delete actor;
    }
}

void ActorGroup::process( const QSharedPointer<IMessage>& message )
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

    actor->setActorGroup( this );

    if ( !actor->run() )
    {
        qDebug("Actor finished on the first run");
        actor->deleteLater();
        return;
    }
    m_actors.append( actor );
}

void ActorGroup::removeActor( Actor* actor )
{
    if ( m_destructed )
        return;
    m_actors.removeOne( actor );
}

void ActorGroup::enqueue( IMessage* msg )
{
    if ( !m_active && m_queue.isEmpty() )
    {
        m_active = true;
        QSharedPointer<IMessage> message(msg);
        process( message );
        m_active = false;
        if ( !m_queue.isEmpty() )
            run();
        return;
    }
    m_queue.enqueue( QSharedPointer<IMessage>(msg) );
}

void ActorGroup::run()
{
    m_active = true;

    while( !m_queue.isEmpty() )
    {
        QSharedPointer<IMessage> msg = m_queue.dequeue();
        process( msg );
    }

    m_active = false;
}
