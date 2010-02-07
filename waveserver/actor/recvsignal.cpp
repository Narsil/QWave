#include "recvsignal.h"
#include "actor.h"
#include "actorgroup.h"

RecvSignalImpl::RecvSignalImpl(QObject* obj, const char* signal)
{
    bool ok = connect( obj, signal, SLOT(trigger()));
    Q_ASSERT(ok);
}

void RecvSignalImpl::trigger()
{
    qDebug("Signal");
    IMessage* msg = new SignalMessage( this );
    msg->setReceiver( actor() );
    actor()->actorGroup()->enqueue( msg );
}
