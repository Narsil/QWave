#ifndef PARTICIPANTFOLK_H
#define PARTICIPANTFOLK_H

#include <QHash>

#include "actor/actorfolk.h"
#include "participant.h"

class ParticipantFolk : public ActorFolk
{
public:
    ParticipantFolk();

    static ParticipantFolk* instance();

protected:
    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:
    static ParticipantFolk* s_instance;
};

#endif // PARTICIPANTFOLK_H
