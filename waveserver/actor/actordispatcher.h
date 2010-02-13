#ifndef ACTORDISPATCHER_H
#define ACTORDISPATCHER_H

#include <QObject>
#include <QSharedPointer>
#include "actor/imessage.h"
#include "actor/actorid.h"

class ActorFolk;

class ActorDispatcher : public QObject
{
public:
    /**
      * @internal Use ActorFolk::activate instead.
      */
    void addFolk( ActorFolk* folk );
    /**
      * @internal Use ActorFolk::deactivate instead.
      */
    void removeFolk( ActorFolk* folk );
    ActorFolk* folk( ActorId::Folk folk );

    bool send( const ActorId& actor, const QSharedPointer<IMessage>& message );
    bool send( const ActorId& actor, IMessage* msg );

    static ActorDispatcher* dispatcher();

private:
    ActorDispatcher();

    ActorFolk* m_folks[ActorId::MAX_FOLK];
    static ActorDispatcher* s_dispatcher;
};

#endif // ACTORDISPATCHER_H
