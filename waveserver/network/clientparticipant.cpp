#include "clientparticipant.h"
#include "clientactorfolk.h"
#include "clientconnection.h"
#include "fcgi/fcgiclientconnection.h"

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
        FCGIClientConnection* fcon = dynamic_cast<FCGIClientConnection*>(obj);
        if ( fcon && fcon->participant() == groupId() )
            fcon->event( event );
    }
}
