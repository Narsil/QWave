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
    ActorFolk(ActorId::Folk folk, QObject* parent = 0);

    void activate();
    void deactivate();

    inline ActorId::Folk folk() const { return m_folk; }

    bool enqueue( const ActorId& actor, const QSharedPointer<IMessage>& message );

protected:
    virtual ActorGroup* group( const ActorId& id ) = 0;

private:
    ActorId::Folk m_folk;
};

#endif // ACTORFOLK_H
