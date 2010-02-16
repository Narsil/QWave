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
    ActorGroup( const QString& groupId, QObject* parent = 0 );
    virtual ~ActorGroup();

    void enqueue( IMessage* msg );
    void enqueue( const QSharedPointer<IMessage>& message );
    /**
      * If an actor has been specified but could not been found, the function returns false, otherwise true.
      */
    bool enqueue( const ActorId& actorId, const QSharedPointer<IMessage>& message );

    void addActor( Actor* actor );
    void removeActor( Actor* actor );

    void addGroup( ActorGroup* group );
    void removeGroup( ActorGroup* group );

    /**
      * @internal
      *
      * Invoked by ActorGroup. Use ActorGroup::addGroup instead.
      */
    void setParentGroup( ActorGroup* group ) { Q_ASSERT( m_parentGroup == 0 || m_parentGroup == group ); m_parentGroup = group; }
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
      * The ID of this group which is unique among its folk. In case of a hierarchical folk,
      * the group ID is not the same as the absolute group ID.
      */
    inline QString groupId() const { return objectName(); }
    QString absGroupId() const;

    virtual Actor* actor( const ActorId& id, bool createOnDemand );
    virtual ActorGroup* group( const QString& groupId, bool createOnDemand );

protected:
    /**
      * Overload this function if you want to intercept the dispatching.
      */
    virtual void dispatch( const QSharedPointer<IMessage>& message );

private:
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
    QList<ActorGroup*> m_groups;
};

#endif // ACTORGROUP_H
