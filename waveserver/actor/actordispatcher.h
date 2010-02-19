#ifndef ACTORDISPATCHER_H
#define ACTORDISPATCHER_H

#include <QObject>
#include <QHash>

#include "actor/actorid.h"

class IMessage;
class ActorFolk;

class ActorDispatcher : public QObject
{
public:
    /**
      * @internal
      */
    void addFolk( ActorFolk* folk );
    /**
      * @internal
      */
    void removeFolk( ActorFolk* folk );

    ActorFolk* folk( const QString& folk );

    bool send( IMessage* msg );
    bool post( IMessage* msg );

    QObject* lookup( const ActorId& id, bool createOnDemand );

    static ActorDispatcher* dispatcher();

private:
    ActorDispatcher();

    QHash<QString,ActorFolk*> m_folks;
    static ActorDispatcher* s_dispatcher;
};

#endif // ACTORDISPATCHER_H
