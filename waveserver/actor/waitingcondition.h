#ifndef WAITINGCONDITION_H
#define WAITINGCONDITION_H

#include <QSharedPointer>

#include "imessage.h"

class Actor;

/**
  * An abstract base class.
  */
class WaitingConditionImpl
{
public:
    WaitingConditionImpl() : m_refCount(0), m_actor(0) { }
    virtual ~WaitingConditionImpl() { }

    virtual WaitingConditionImpl* handleMessage( const QSharedPointer<IMessage>& msg ) { Q_UNUSED(msg); return 0; }

    Actor* actor() const { return m_actor; }
    /**
      * @internal
      */
    virtual void setActor( Actor* actor ) { m_actor = actor; }

    /**
      * @internal
      */
    int m_refCount;

private:
    Actor* m_actor;
};

class WaitingCondition
{
};

template<class T> class WaitingConditionPointer : public WaitingCondition
{
public:
    WaitingConditionPointer() { m_ptr = new T(); m_ptr->m_refCount = 1; }
    WaitingConditionPointer( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<T*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    WaitingConditionPointer( const WaitingConditionPointer<T>& x ) { m_ptr = const_cast<T*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    ~WaitingConditionPointer() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    operator bool() const { return m_ptr != 0; }

    WaitingConditionPointer<T>& operator=( const WaitingConditionPointer<T>& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const WaitingConditionPointer<T>& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const WaitingConditionPointer<T>& x) const { return m_ptr != x.m_ptr; }

    T* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    typedef T ImplType;

private:
    T* m_ptr;
};

template<class T, class P1> class WaitingConditionPointer2 : public WaitingCondition
{
public:
    WaitingConditionPointer2( P1 p1 ) { m_ptr = new T(p1); m_ptr->m_refCount = 1; }
    WaitingConditionPointer2( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<T*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    WaitingConditionPointer2( const WaitingConditionPointer2<T,P1>& x ) { m_ptr = const_cast<T*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    ~WaitingConditionPointer2() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    T* operator->() const { return m_ptr; }
    T& operator*() const { return *m_ptr; }
    operator bool() const { return m_ptr != 0; }

    WaitingConditionPointer2<T,P1>& operator=( const WaitingConditionPointer2<T,P1>& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const WaitingConditionPointer2<T,P1>& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const WaitingConditionPointer2<T,P1>& x) const { return m_ptr != x.m_ptr; }

    T* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    typedef T ImplType;

private:
    T* m_ptr;
};

#endif // WAITINGCONDITION_H
