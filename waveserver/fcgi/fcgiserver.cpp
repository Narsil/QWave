#include "fcgiserver.h"
#include "fcgiprotocol.h"
#include "app/settings.h"
#include <QTcpServer>
#include <QHostAddress>
#include <QTcpSocket>

using namespace FCGI;

FCGIServer::FCGIServer()
    : ActorFolk("fcgi")
{
    setHierarchical(true);

    m_socket = new QTcpServer(this);

    bool check = connect( m_socket, SIGNAL(newConnection()), SLOT(newConnection()));
    Q_ASSERT(check);

    // Listen on any interface
    check = m_socket->listen( QHostAddress::Any, Settings::settings()->fcgiPort());
    if ( !check )
    {
        qDebug("Failed to bind to port");
        Q_ASSERT(false);
    }
}

void FCGIServer::newConnection()
{
    QTcpSocket* sock = m_socket->nextPendingConnection();
    // new ClientConnection(sock, this);

    new FCGIProtocol( sock, this );
}
