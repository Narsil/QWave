#ifndef RECVSIGNAL_H
#define RECVSIGNAL_H

#include <QObject>
#include "waitingcondition.h"
#include "recvxor.h"

class RecvSignalImpl;

class SignalMessage : public IMessage
{
public:
    SignalMessage( RecvSignalImpl* p ) : ptr(p) { }
    RecvSignalImpl* ptr;
};

class RecvSignalImpl : public QObject, public WaitingConditionImpl
{
    Q_OBJECT
public:
    RecvSignalImpl(QObject* obj, const char* signal);

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        SignalMessage* m = dynamic_cast<SignalMessage*>( event );
        if ( m && m->ptr == this )
            return this;
        return 0;
    }

private slots:
    void trigger();
};

// typedef WaitingConditionPointer<RecvSignalImpl> RecvSignal;

class RecvSignal : public WaitingCondition
{
public:
    RecvSignal(QObject* sender, const char* signal) { m_ptr = new RecvSignalImpl(sender, signal); m_ptr->m_refCount = 1; }
    RecvSignal( const RecvSignal& x ) { m_ptr = const_cast<RecvSignalImpl*>(x.m_ptr); if ( m_ptr ) m_ptr->m_refCount++; }
    RecvSignal( WaitingConditionImpl* x ) { m_ptr = dynamic_cast<RecvSignalImpl*>( x ); if ( m_ptr ) m_ptr->m_refCount++; }
    ~RecvSignal() { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } }

    operator bool() const { return m_ptr != 0; }

    RecvSignal& operator=( const RecvSignal& x ) { if ( m_ptr ) { m_ptr->m_refCount--; if ( m_ptr->m_refCount == 0 ) delete m_ptr; } m_ptr = x.m_ptr; if ( m_ptr ) m_ptr->m_refCount++; return *this; }
    bool operator==( const RecvSignal& x ) const { return m_ptr == x.m_ptr; }
    bool operator!=( const RecvSignal& x) const { return m_ptr != x.m_ptr; }

    RecvSignalImpl* donate() const { if ( m_ptr ) { m_ptr->m_refCount++; } return m_ptr; }

    typedef RecvSignalImpl ImplType;

private:
    RecvSignalImpl* m_ptr;
};

template<class X> RecvXor operator|( const RecvSignal& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

#endif // RECVSIGNAL_H
