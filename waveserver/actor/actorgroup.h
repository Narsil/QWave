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
class ActorFolk;

class ActorGroup : public QObject
{
public:
    ActorGroup( const QString& groupId, ActorFolk* folk );
    ActorGroup( const QString& groupId, ActorGroup* parentFolk );
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
      * @return the parent group or zero if this is a top-level group.
      */
    ActorFolk* folk() const;

    /**
      * @return the parent group or zero if this is a top-level group.
      */
    ActorGroup* parentGroup() const { return m_parentGroup; }

    /**
      * @internal
      *
      * Use Actor::send to send messages. This is only an intermediate help function.
      */
    virtual bool send( const ActorId& id, IMessage* msg );

    /**
      * @return the ID of this group which is unique among its parent-group (or folk if there is no parent-group).
      * In case of a hierarchical folk,
      * the group ID is not the same as the absolute group ID.
      */
    inline QString groupId() const { return objectName(); }
    QString absGroupId() const;
    /**
      * @return an actor ID which specifies this group.
      */
    ActorId actorId() const;

    virtual Actor* actor( const ActorId& id, bool createOnDemand );
    virtual ActorGroup* group( const QString& groupId, bool createOnDemand );

protected:
    /**
      * Overload this function if you want to intercept the dispatching.
      */
    virtual void dispatch( const QSharedPointer<IMessage>& message );

private:
    /**
      * @internal
      */
    void addGroup( ActorGroup* group );
    /**
      * @internal
      */
    void removeGroup( ActorGroup* group );

    /**
      * Dispatches all queued messages to the actors.
      */
    void run();
    /**
      * The object takes over ownership of the message.
      */
    void process( const QSharedPointer<IMessage>& message, Actor* actor );

    bool m_destructed;
    QQueue<IMessagePtr> m_queue;
    QList<Actor*> m_actors;
    /**
      * True, if a member of this group is currently processing a message.
      */
    bool m_active;
    /**
      * The parent group. May be 0.
      */
    ActorGroup* m_parentGroup;
    ActorFolk* m_folk;
    QList<ActorGroup*> m_groups;
};

#endif // ACTORGROUP_H
