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

    void activate();
    void deactivate();

    inline QString folk() const { return m_folk; }

    bool enqueue( const ActorId& actor, const QSharedPointer<IMessage>& message );

    bool isHierarchical() const { return m_isHierarchical; }
    void setHierarchical(bool enable) { m_isHierarchical = enable; }

protected:
    virtual ActorGroup* group( const QString& id, bool createOnDemand ) = 0;

private:
    QString m_folk;
    bool m_isHierarchical;
};

#endif // ACTORFOLK_H
