#include "actor.h"
#include "actorgroup.h"

qint64 Actor::s_id = 0;

Actor::Actor()
        : m_group(0)
{
    m_state = 0;
    m_reason = 0;
    m_wait = 0;
}

Actor::~Actor()
{
    deleteWait();
    deleteReason();

    if ( m_group )
        m_group->removeActor(this);
}

bool Actor::run()
{
    if ( m_state == -1 )
        return false;
    execute();
    deleteReason();
    if ( m_state == -1 )
    {
        deleteWait();
        return false;
    }
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

bool Actor::process( const QSharedPointer<IMessage>& message )
{
    if ( m_wait )
    {
        m_reason = m_wait->handleMessage( message );
        if ( m_reason )
        {
            m_reason->m_refCount++;
            deleteWait();
            return true;
        }
    }

    return false;
}

bool Actor::send( const ActorId& destination, IMessage* msg )
{
    msg->setSender( actorId() );
    if ( !m_group )
        return false;
    return m_group->send( destination, msg );
}

qint64 Actor::nextId() const
{
    return ++s_id;
}
