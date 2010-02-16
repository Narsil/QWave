#ifndef ACTORDISPATCHER_H
#define ACTORDISPATCHER_H

#include <QObject>
#include <QSharedPointer>
#include <QHash>
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
    ActorFolk* folk( const QString& folk );

    bool send( const ActorId& actor, const QSharedPointer<IMessage>& message );
    bool send( const ActorId& actor, IMessage* msg );

    static ActorDispatcher* dispatcher();

private:
    ActorDispatcher();

    QHash<QString,ActorFolk*> m_folks;
    static ActorDispatcher* s_dispatcher;
};

#endif // ACTORDISPATCHER_H
