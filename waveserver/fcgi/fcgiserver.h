#ifndef FCGISERVER_H
#define FCGISERVER_H

#include <QObject>
#include "actor/actorfolk.h"

class QTcpServer;

namespace FCGI
{
    class FCGIServer : public ActorFolk
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
