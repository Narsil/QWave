#ifndef FCGISERVER_H
#define FCGISERVER_H

#include <QObject>

class QTcpServer;

namespace FCGI
{
    class FCGIServer : public QObject
    {
        Q_OBJECT
    public:
        FCGIServer();

    private slots:
        void newConnection();

    private:
        QTcpServer* m_socket;
    };

}

#endif // FCGISERVER_H
