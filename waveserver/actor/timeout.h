#ifndef TIMEOUT_H
#define TIMEOUT_H

#include "actor/actor.h"
#include "actor/waitingcondition.h"
#include "actor/recvxor.h"

/**
  * @internal
  */
class TimeoutImpl : public WaitingConditionImpl
{
public:
    TimeoutImpl(int interval) : m_interval(interval), m_timerId(-1) { }
    ~TimeoutImpl() { if ( m_timerId != -1 ) actor()->killTimer( m_timerId ); }

    virtual WaitingConditionImpl* handleMessage( QEvent* event )
    {
        if ( event->type() == QEvent::Timer && static_cast<QTimerEvent*>(event)->timerId() == m_timerId )
        {
            actor()->killTimer( m_timerId );
            m_timerId = -1;
            event->setAccepted( true );
            return this;
        }
        return 0;
    }

    virtual void setActor( Actor* actor ) { this->WaitingConditionImpl::setActor(actor); m_timerId = actor->startTimer( m_interval ); }

private:
    int m_interval;
    int m_timerId;
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
