#include "actor.h"
#include "actorgroup.h"
#include "actordispatcher.h"
#include <QEvent>
#include <QTimerEvent>
#include <QCoreApplication>

qint64 Actor::s_id = 0;

Actor::Actor(ActorGroup* parent)
    : QObject( parent ), m_reason(0), m_wait(0), m_state(0), m_id( parent->actorId() )
{
    QCoreApplication::postEvent( this, new QEvent( (QEvent::Type)IMessage::Create ) );
}

Actor::Actor(const QString& id, ActorGroup* parent)
    : QObject( parent ), m_reason(0), m_wait(0), m_state(0), m_id( parent, id)
{
    setObjectName( id );
    QCoreApplication::postEvent( this, new QEvent( (QEvent::Type)IMessage::Create ) );
}

Actor::~Actor()
{
    deleteWait();
    deleteReason();
}

bool Actor::run()
{
    // Already terminated?
    if ( m_state == -1 )
        return false;
    // Run
    execute();
    // Forget the reason why the actor has run
    deleteReason();
    // Terminated now?
    if ( m_state == -1 )
    {
        // Garbage collect the actor
        deleteLater();
        return false;
    }
    // Tell the wait statement who is waiting
    if ( m_wait )
        m_wait->setActor(this);
    return true;
}

void Actor::deleteWait()
{
    if ( m_wait )
    {
        m_wait->m_refCount--;
        if ( m_wait->m_refCount == 0 )
            delete m_wait;
        m_wait = 0;
    }
}

void Actor::deleteReason()
{
    if ( m_reason )
    {
        m_reason->m_refCount--;
        if ( m_reason->m_refCount == 0 )
            delete m_reason;
        m_reason = 0;
    }
}

void Actor::customEvent( QEvent* event )
{
    if ( event->type() == (QEvent::Type)IMessage::Create )
    {
        run();
        return;
    }

    // Do we wait for an event
    if ( m_wait )
    {
        // Process the event
        m_reason = m_wait->handleMessage( event );
        // We have a reason to continue?
        if ( m_reason )
        {
            // Remember the reason for continuing
            m_reason->m_refCount++;
            // Forget the reason for waiting
            deleteWait();
            // Do something
            run();
        }
    }
}

void Actor::timerEvent( QTimerEvent* event )
{
    customEvent( event );
}

bool Actor::send( IMessage* msg )
{
    if ( msg->sender().isNull() )
        msg->setSender( actorId() );
    return ActorDispatcher::dispatcher()->send( msg );
}

bool Actor::post( IMessage* msg )
{
    if ( msg->sender().isNull() )
        msg->setSender( actorId() );
    return ActorDispatcher::dispatcher()->post( msg );
}

qint64 Actor::nextId()
{
    return ++s_id;
}

ActorGroup* Actor::group() const
{
    return dynamic_cast<ActorGroup*>(parent());
}
