#include "timeout.h"
#include "actor.h"
#include "actorgroup.h"

#include <QTimer>

TimeoutImpl::TimeoutImpl(int timeout)
    : m_timeout(timeout)
{
    QTimer::singleShot( timeout, this, SLOT(timeout()));
}

void TimeoutImpl::timeout()
{
    qDebug("Timeout %i", m_timeout);
    IMessage* msg = new TimeoutMessage( this );
    msg->setReceiver( actor() );
    actor()->actorGroup()->enqueue( msg );
}
