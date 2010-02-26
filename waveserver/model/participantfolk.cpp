#include "participantfolk.h"
#include "participant.h"
#include "jid.h"
#include "app/settings.h"

ParticipantFolk* ParticipantFolk::s_instance = 0;

ParticipantFolk::ParticipantFolk()
        : ActorFolk("user")
{
}

ParticipantFolk* ParticipantFolk::instance()
{
    if ( s_instance == 0 )
        s_instance = new ParticipantFolk();
    return s_instance;
}

ActorGroup* ParticipantFolk::group( const QString& id, bool createOnDemand )
{
    ActorGroup* g = this->ActorFolk::group( id, createOnDemand );
    if ( g )
        return g;

    JID jid(id);
    if ( !jid.isValid() )
    {
        qDebug("Malformed JID");
        return 0;
    }
    if ( jid.domain() != Settings::settings()->domain() )
    {
        qDebug("Participant jas a remote JID");
        return 0;
    }

    return new Participant( jid, this );
}

