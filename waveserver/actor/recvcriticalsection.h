#ifndef RECVCRITICALSECTION_H
#define RECVCRITICALSECTION_H

#include <QObject>
#include <QQueue>
#include "actor/waitingcondition.h"
#include "actor/recvxor.h"
#include "actor/actor.h"

class CriticalSection
{
public:
    CriticalSection() : m_actor(0), m_enabled(true) { }

    /**
      * @internal. Use 'yield( RecvCriticalSection( section ) )' instead.
      */
    void enterIntern(Actor* actor);

    bool tryEnter(Actor* actor );
    void leave(Actor* actor);

    void enable();
    void disable();

private:
    /**
      * The actor that is currently allowed to execute.
      */
    Actor* m_actor;
    QQueue<Actor*> m_queue;
    bool m_enabled;
};

class EnterCriticalSectionMessage : public QEvent
{
public:
    EnterCriticalSectionMessage( CriticalSection* p ) : QEvent( (QEvent::Type)IMessage::EnterCriticalSection), ptr(p) { }
    CriticalSection* ptr;
};

class RecvCriticalSectionImpl : public WaitingConditionImpl
{
public:
    RecvCriticalSectionImpl(CriticalSection* section) { m_section = section; }

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        EnterCriticalSectionMessage* m = dynamic_cast<EnterCriticalSectionMessage*>( event );
        if ( m && m->ptr == m_section )
            return this;
        return 0;
    }

    virtual void setActor( Actor* actor ) { m_section->enterIntern( actor ); this->WaitingConditionImpl::setActor(actor); }

private:
    CriticalSection* m_section;
};

class RecvCriticalSection : public WaitingCondition
{
public:
    RecvCriticalSection( CriticalSection* section) { m_ptr = new RecvCriticalSectionImpl(section); m_ptr->m_refCount = 1; }
    RecvCriticalSection( const RecvCriticalSection& x ) { m_ptr = const_cast<RecvCriticalSectionImpl*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    RecvCriticalSection( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<RecvCriticalSectionImpl*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    ~RecvCriticalSection() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    operator bool() const { return m_ptr != 0; }

    RecvCriticalSection& operator=( const RecvCriticalSection& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const RecvCriticalSection& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const RecvCriticalSection& x) const { return m_ptr != x.m_ptr; }

    RecvCriticalSectionImpl* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

private:
    RecvCriticalSectionImpl* m_ptr;
};

template<class X> RecvXor operator|( const RecvCriticalSection& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }


#endif // RECVCRITICALSECTION_H
