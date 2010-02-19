#ifndef ACTORGROUP_H
#define ACTORGROUP_H

#include <QObject>
#include <QString>

#include "actorid.h"

class Actor;
class ActorFolk;

class ActorGroup : public QObject
{
public:
    ActorGroup( const QString& groupId, ActorFolk* folk );
    ActorGroup( const QString& groupId, ActorGroup* parentGroup );
    virtual ~ActorGroup();

    /**
      * @return the folk to which this group belongs.
      */
    ActorFolk* folk() const;

    /**
      * @return the parent group or zero if this is a top-level group.
      */
    ActorGroup* parentGroup() const { return dynamic_cast<ActorGroup*>( parent() ); }

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

    virtual Actor* actor( const QString& id, bool createOnDemand );
    virtual ActorGroup* group( const QString& groupId, bool createOnDemand );        

    template<class T> T* findDirectChild( const QString& name );

protected:
    /**
      * By default all events not consumed by the ActorGroup itself are broadcasted to all
      * actors in the group. The event is not broadcasted to sub groups.
      */
    void customEvent( QEvent* event );
};

#endif // ACTORGROUP_H
