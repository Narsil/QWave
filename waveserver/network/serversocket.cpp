#include "serversocket.h"
#include "clientconnection.h"
#include <QTcpServer>
#include <QHostAddress>
#include <QTcpSocket>

ServerSocket::ServerSocket(const QString& domain)
        : m_domain(domain)
{
    m_socket = new QTcpServer(this);

    bool check = connect( m_socket, SIGNAL(newConnection()), SLOT(newConnection()));
    Q_ASSERT(check);

    // List on any interface
    check = m_socket->listen( QHostAddress::Any, 9876);
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
