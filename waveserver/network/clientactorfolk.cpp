#include "clientactorfolk.h"
#include "clientparticipant.h"
#include "fcgi/fcgiclientconnection.h"
#include "network/clientconnection.h"
#include "model/jid.h"
#include <QTimer>

ClientActorFolk* ClientActorFolk::s_folk = 0;

ClientActorFolk::ClientActorFolk(QObject* parent)
        : ActorFolk( "client", parent )
{
    // This timer runs the cleanup process periodically
    QTimer* timer = new QTimer(this);
    timer->setInterval( 1000 * 30 );
    bool ok = connect( timer, SIGNAL(timeout()), SLOT(periodicCleanup()));
    timer->start();
    Q_ASSERT(ok);
}

ClientConnection* ClientActorFolk::newClientConnection( QTcpSocket* socket )
{
    ClientConnection* con = new ClientConnection( socket, this );
    return con;
}

FCGIClientConnection* ClientActorFolk::newFCGIClientConnection( const QString& sessionId, const QString& jid )
{
    FCGIClientConnection* con = new FCGIClientConnection( sessionId, jid, this );
    return con;
}

ActorGroup* ClientActorFolk::group( const QString& groupId, bool createOnDemand )
{
    Q_UNUSED( createOnDemand )

    ActorGroup* g = this->ActorFolk::group( groupId, createOnDemand );
    if ( g )
        return g;

    JID jid( groupId );
    if ( jid.isValid() )
    {
        g = new ClientParticipant( groupId );
        return g;
    }
    return 0;
}

ClientActorFolk* ClientActorFolk::instance()
{
    if ( s_folk == 0 )
        s_folk = new ClientActorFolk();
    return s_folk;
}

void ClientActorFolk::periodicCleanup()
{
    for( int i = 0; i < children().length(); ++i )
    {
        FCGIClientConnection* f = dynamic_cast<FCGIClientConnection*>( children().at(i) );
        if ( f )
        {
            if ( f->isDead() )
                f->deleteLater();
        }
    }
}
