#ifndef ACTOR_H
#define ACTOR_H

#include <QObject>
#include <QSharedPointer>
#include "waitingcondition.h"
#include "imessage.h"

#define EXECUTE(x) execute()
#define TERMINATE() m_state = -1; return;
#define BEGIN_EXECUTE switch(m_state) { case 0:
#define END_EXECUTE ; } m_state = -1;
#define yield(condition) m_state = __LINE__; m_wait = (condition).donate(); return; case __LINE__:
// #define REASON(t) t* REASON = dynamic_cast<t*>( this->m_reason )
#define REASON(t) t REASON = _REASON<t>( this->m_reason )

template<class R> R _REASON(WaitingConditionImpl* ptr) { return R( ptr ); }

class ActorGroup;

/**
  * An actor can be used much like a C# or Python iterator.
  * Its main function EXECUTE() can return in the middle using the 'yield' statement.
  * When the function is invoked again it continues just below the 'yield' statement which caused it to leave.
  * Please note that calling yield does NOT BLOCK the thread. The EXECUTE function is left and the current thread is assigned to other actors.
  *
  * In the yield statement you must return a WaitingCondition, i.e. "yield( Timeout(100) );"
  * When the waiting condition is satisfied, the actor will become ready and continue execution below the yield statement
  * whenever it can get hold of the CPU.
  *
  * Each Actor must be assigned to one ActorGroup, otherwise it will not receive CPU time from the thread.
  *
  * WARNING: Inside EXECUTE do not use local variables since they do not survive multiple invocations.
  * Instead, promote the variables to member fields.
  */
class Actor : public QObject
{
public:
    Actor(ActorGroup* group);
    Actor();
    virtual ~Actor();

    bool run();

    /**
      * @internal
      */
    bool process( const QSharedPointer<IMessage>& message );
    /**
      * @internal
      *
      * Invoked by ActorGroup. Use ActorGroup::addActor instead.
      */
    void setActorGroup( ActorGroup* group ) { Q_ASSERT( m_group == 0 || m_group == group ); m_group = group; }
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
