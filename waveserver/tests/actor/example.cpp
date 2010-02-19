#include "example.h"
#include "timeout.h"
#include "imessage.h"
#include "recvxmpp.h"
#include "network/xmppstanza.h"
#include "recv.h"
#include "recvxor.h"
#include "recvsignal.h"
#include "actorgroup.h"
#include <QDateTime>

Example::Example(const QString& id, ActorGroup* group)
        : Actor( id, group )
{
}

void Example::execute()
{
    BEGIN_EXECUTE;

    timer = new QTimer();
    timer->setInterval( 25000 );
    timer->start();

    for( i = 0; i < 10; ++i )
    {
        qDebug("Hello 1");
        k = 3;
        yield( Timeout(1000) );
        // yield( Recv<MyMessage>() );
        while( k > 0 )
        {
            qDebug("Hello 2");
            // Send message to ourself
            post( new MyMessage( actorId(), "Hallo Welt") );

            yield( Recv<MyMessage>() | Timeout(300) );
            if ( REASON( Timeout ) )
            {
                qDebug("Got a timeout");
            }
            else if ( REASON( RecvWaveletUpdate ) )
            {
            }
            else if ( REASON( Recv<MyMessage> ) )
            {
                qDebug("Got my message '%s' at %s", REASON->m_str.toAscii().constData(), QDateTime::currentDateTime().toString().toAscii().constData() );
            }
            k--;
        }
    }

    qDebug("Waiting for timer");
    yield( RecvSignal( timer, SIGNAL(timeout()) ) );
    qDebug("All done");

    END_EXECUTE;
}
