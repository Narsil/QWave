#ifndef SERVERSOCKET_H
#define SERVERSOCKET_H

#include <QObject>

class QTcpServer;

/**
  * Accepts incoming TCP connections.
  */
class ServerSocket : public QObject
{
    Q_OBJECT
public:
    ServerSocket();

private slots:
    void newConnection();

private:
    QTcpServer* m_socket;
    QString m_domain;
};

#endif // SERVERSOCKET_H
