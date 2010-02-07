#ifndef ACTOR_H
#define ACTOR_H

#include <QObject>
#include <QSharedPointer>
#include "waitingcondition.h"
#include "imessage.h"

#define EXECUTE(x) execute()
#define terminate() m_state = -1; return;
#define BEGIN_EXECUTE switch(m_state) { case 0:
#define END_EXECUTE ; } m_state = -1;
#define yield(condition) m_state = __LINE__; m_wait = (condition).donate(); return; case __LINE__:
// #define REASON(t) t* REASON = dynamic_cast<t*>( this->m_reason )
#define REASON(t) t REASON = _REASON<t>( this->m_reason )

template<class R> R _REASON(WaitingConditionImpl* ptr) { return R( ptr ); }

class ActorGroup;

class Actor : public QObject
{
public:
    Actor();
    virtual ~Actor();

    bool run();

    /**
      * @internal
      */
    bool process( const QSharedPointer<IMessage>& message );
    /**
      * @internal
      */
    void setActorGroup( ActorGroup* group ) { m_group = group; }
    ActorGroup* actorGroup() const { return m_group; }

protected:
    virtual void EXECUTE() = 0;

    /**
      * @internal
      *
      * The waiting condition which has completed or 0.
      */
    WaitingConditionImpl* m_reason;
    /**
      * @internal
      */
    WaitingConditionImpl* m_wait;
    /**
      * @internal
      */
    int m_state;

private:
    void deleteWait();
    void deleteReason();

    ActorGroup* m_group;
};

#endif // ACTOR_H
