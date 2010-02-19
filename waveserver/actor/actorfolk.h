#ifndef ACTORFOLK_H
#define ACTORFOLK_H

#include <QObject>
#include <QSharedPointer>
#include "actor/actorid.h"
#include "actor/imessage.h"

class ActorGroup;

class ActorFolk : public QObject
{
public:
    ActorFolk(const QString& folk, QObject* parent = 0);
    ~ActorFolk();

    inline QString folkId() const { return objectName(); }

    bool isHierarchical() const { return m_isHierarchical; }
    void setHierarchical(bool enable) { m_isHierarchical = enable; }

    ActorGroup* group( const ActorId& id, bool createOnDemand );

    template<class T> T* findDirectChild( const QString& name );

protected:
    /**
      * The id is not a hierarchical name.
      */
    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:
    bool m_isHierarchical;
};

#endif // ACTORFOLK_H
