#ifndef FCGISERVER_H
#define FCGISERVER_H

#include <QObject>
#include "actor/actorfolk.h"

class QTcpServer;

namespace FCGI
{
    /**
      * The root object for FCGI support.
      * It listens on the FCGI port and creates new FCGIProtocol
      * objects for every incoming TCP connection.
      */
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
