#ifndef ACTORGROUP_H
#define ACTORGROUP_H

#include <QObject>
#include <QQueue>
#include <QSharedPointer>
#include <QList>

#include "imessage.h"
#include "actor.h"

typedef QSharedPointer<IMessage> IMessagePtr;

class ActorGroup : public QObject
{
public:
    ActorGroup( QObject* parent = 0 );
    virtual ~ActorGroup();

    void enqueue( IMessage* msg );

    void addActor( Actor* actor );
    void removeActor( Actor* actor );

private:
    void run();
    /**
      * The object takes over ownership of the message.
      */
    void process( const QSharedPointer<IMessage>& message );
    void process( const QSharedPointer<IMessage>& message, Actor* actor );

    bool m_destructed;
    QQueue<IMessagePtr> m_queue;
    QList<Actor*> m_actors;
    bool m_active;
};

#endif // ACTORGROUP_H
