#ifndef RECV_H
#define RECV_H

#include "waitingcondition.h"
#include "recvxor.h"

template<class T> class RecvImpl : public WaitingConditionImpl
{
public:
    RecvImpl() : m_message(0) {  }
    ~RecvImpl() {  }

    inline T* message() const { return m_message; }

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        if ( m_message )
            return this;
        if ( event->type() != (QEvent::Type)IMessage::Message )
            return 0;
        m_message = dynamic_cast<T*>( event );
        if ( m_message )
            return this;
        return 0;
    }

private:
    T* m_message;
};

template<class M> class Recv : public WaitingCondition
{
public:
    Recv() { m_ptr = new RecvImpl<M>(); m_ptr->m_refCount = 1; }
    Recv( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<RecvImpl<M>*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    Recv( const Recv<M>& x ) { m_ptr = const_cast<RecvImpl<M>*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    ~Recv() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    M* operator->() const { return m_ptr->message(); }
    M& operator*() const { return *(m_ptr->message()); }
    operator bool() const { return m_ptr != 0; }

    Recv<M>& operator=( const Recv<M>& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const Recv<M>& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const Recv<M>& x) const { return m_ptr != x.m_ptr; }

    RecvImpl<M>* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    const M& message() const { Q_ASSERT( m_ptr ); return *(m_ptr->message()); }

private:
    RecvImpl<M>* m_ptr;
};

template<class M, class X> RecvXor operator|( const Recv<M>& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

#endif // RECV_H
