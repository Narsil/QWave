#ifndef RECVXOR_H
#define RECVXOR_H

#include "waitingcondition.h"

class RecvXorImpl : public WaitingConditionImpl
{
public:
    RecvXorImpl(WaitingConditionImpl* wait1, WaitingConditionImpl* wait2) : m_wait1(wait1), m_wait2(wait2) {  }
    ~RecvXorImpl() { if ( m_wait1 ) { m_wait1->m_refCount--; if ( m_wait1->m_refCount == 0 ) delete m_wait1; } if ( m_wait1) { m_wait2->m_refCount--; if ( m_wait2->m_refCount == 0 ) delete m_wait2; } }

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        WaitingConditionImpl* result = m_wait1->handleMessage( event );
        if ( result )
            return result;
        result = m_wait2->handleMessage( event );
        if ( result )
            return result;
        return 0;
    }

    virtual void setActor( Actor* actor ) { this->WaitingConditionImpl::setActor( actor ); m_wait1->setActor(actor); m_wait2->setActor(actor); }

private:
    WaitingConditionImpl* m_wait1;
    WaitingConditionImpl* m_wait2;
};

class RecvXor : public WaitingCondition
{
public:
    RecvXor(WaitingConditionImpl* wait1, WaitingConditionImpl* wait2 ) { m_ptr = new RecvXorImpl(wait1, wait2); m_ptr->m_refCount = 1; }
    RecvXor( const RecvXor& x ) { m_ptr = const_cast<RecvXorImpl*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    ~RecvXor() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    RecvXorImpl* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

private:
    RecvXorImpl* m_ptr;
};

template<class X> RecvXor operator|( const RecvXor& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

#endif // RECVXOR_H
