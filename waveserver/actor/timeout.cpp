#include "timeout.h"
#include "actor.h"
#include "actorgroup.h"

#include <QTimer>

TimeoutImpl::TimeoutImpl(int timeout)
{
    // QTimer::singleShot( timeout, this, SLOT(timeout()));
    m_timer = new QTimer();
    m_timer->setSingleShot( true );
    m_timer->setInterval( timeout );
    connect( m_timer, SIGNAL(timeout()), SLOT(timeout()) );
    m_timer->start();
}

TimeoutImpl::~TimeoutImpl()
{
    delete m_timer;
}

void TimeoutImpl::timeout()
{
    qDebug("Timeout");
    IMessage* msg = new TimeoutMessage( this );
    msg->setReceiver( actor() );
    actor()->actorGroup()->enqueue( msg );
}
