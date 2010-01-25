#include "serversocket.h"
#include "clientconnection.h"
#include "app/settings.h"
#include <QTcpServer>
#include <QHostAddress>
#include <QTcpSocket>

ServerSocket::ServerSocket()
{
    m_socket = new QTcpServer(this);

    bool check = connect( m_socket, SIGNAL(newConnection()), SLOT(newConnection()));
    Q_ASSERT(check);

    // Listen on any interface
    check = m_socket->listen( QHostAddress::Any, Settings::settings()->clientPort());
    if ( !check )
    {
        qDebug("Failed to bind to port");
        Q_ASSERT(false);
    }
}

void ServerSocket::newConnection()
{
    QTcpSocket* sock = m_socket->nextPendingConnection();
    new ClientConnection(sock, this);
}
