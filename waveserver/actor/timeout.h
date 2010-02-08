#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <QObject>
#include "waitingcondition.h"
#include "recvxor.h"

class QTimer;
class TimeoutImpl;

/**
  * @internal
  */
class TimeoutMessage : public IMessage
{
public:
    TimeoutMessage( TimeoutImpl* p ) : ptr(p) { }
    TimeoutImpl* ptr;
};

/**
  * @internal
  */
class TimeoutImpl : public QObject, public WaitingConditionImpl
{
    Q_OBJECT
public:
    TimeoutImpl(int timeout);
    ~TimeoutImpl();

    virtual WaitingConditionImpl* handleMessage( const QSharedPointer<IMessage>& msg )
    {
        TimeoutMessage* m = dynamic_cast<TimeoutMessage*>( msg.data() );
        if ( m && m->ptr == this )
            return this;
        return 0;
    }

private slots:
    void timeout();

private:
    QTimer* m_timer;
};

/**
  * This class can be used as a parameter to the yield statement inside an Actor.
  */
typedef WaitingConditionPointer2<TimeoutImpl, int> Timeout;

/**
  * Allows to concatenate a timeout with other WaitingCondition classes.
  */
template<class T, class P1, class X> RecvXor operator|( const WaitingConditionPointer2<T,P1>& x, const X& y ) { return RecvXor(x.donate(), y.donate()); }

#endif // TIMEOUT_H
