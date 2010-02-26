#include "clientparticipant.h"
#include "clientactorfolk.h"
#include "clientconnection.h"

ClientParticipant::ClientParticipant(const QString& jid)
        : ActorGroup( jid, ClientActorFolk::instance() )
{
}

void ClientParticipant::customEvent( QEvent* event )
{
    // Broadcast the event
    QObject* o = folk();
    foreach( QObject* obj, o->children() )
    {
        ClientConnection* con = dynamic_cast<ClientConnection*>(obj);
        if ( con && con->participant() == groupId() )
            con->event( event );
    }
}
