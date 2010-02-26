#include "recvcriticalsection.h"
#include <QCoreApplication>

void CriticalSection::enterIntern(Actor* actor)
{
    if ( m_enabled && m_actor == 0 && m_queue.isEmpty() )
    {
        m_actor = actor;
        QCoreApplication::postEvent( actor, new EnterCriticalSectionMessage(this) );
    }
    else
        m_queue.append( actor );
}

void CriticalSection::leave(Actor* actor)
{
    if ( m_actor == actor )
    {
        if ( !m_queue.isEmpty() && m_enabled )
        {
            m_actor = m_queue.dequeue();
            QCoreApplication::postEvent( m_actor, new EnterCriticalSectionMessage(this) );
        }
        else
            m_actor = 0;
    }
}

void CriticalSection::enable()
{
    if ( m_enabled )
        return;
    m_enabled = true;
    if ( m_actor == 0 && !m_queue.isEmpty() )
    {
        m_actor = m_queue.dequeue();
        QCoreApplication::postEvent( m_actor, new EnterCriticalSectionMessage(this) );
    }
}

void CriticalSection::disable()
{
    m_enabled = false;
}

bool CriticalSection::tryEnter(Actor* actor )
{
    if ( m_enabled && m_actor == 0 && m_queue.isEmpty() )
    {
        m_actor = actor;
        return true;
    }
    return false;
}
