#ifndef ACTORGROUP_H
#define ACTORGROUP_H

#include <QObject>
#include <QQueue>
#include <QSharedPointer>
#include <QList>

#include "imessage.h"
#include "actor.h"

typedef QSharedPointer<IMessage> IMessagePtr;

class ActorId;

class ActorGroup : public QObject
{
public:
    ActorGroup( QObject* parent = 0 );
    virtual ~ActorGroup();

    void enqueue( IMessage* msg );
    void enqueue( const QSharedPointer<IMessage>& message );
    /**
      * If an actor has been specified but could not been found, the function returns false, otherwise true.
      */
    bool enqueue( const ActorId& actorId, const QSharedPointer<IMessage>& message );

    void addActor( Actor* actor );
    void removeActor( Actor* actor );

    /**
      * @internal
      *
      * Use Actor::send to send messages. This is only an intermediate help function.
      */
    virtual bool send( const ActorId& id, IMessage* msg );

    const QList<Actor*>& actors() const { return m_actors; }

protected:
    /**
      * Overload this function if you want to intercept the dispatching.
      */
    virtual void dispatch( const QSharedPointer<IMessage>& message );

    virtual Actor* actor( const ActorId& id );

private:
    void run();
    /**
      * The object takes over ownership of the message.
      */
    void process( const QSharedPointer<IMessage>& message, Actor* actor );

    bool m_destructed;
    QQueue<IMessagePtr> m_queue;
    QList<Actor*> m_actors;
    bool m_active;
};

#endif // ACTORGROUP_H
