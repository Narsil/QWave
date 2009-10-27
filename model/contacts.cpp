#include "contacts.h"
#include "participant.h"
#include "app/environment.h"

Contacts::Contacts(Environment* environment, QObject* parent)
        : QObject( parent ), m_environment(environment)
{
}

Participant* Contacts::participant( const QString& address ) const
{
    Participant* p = m_participants[address];
    if ( p )
        return p;
    if ( address == m_environment->localUser()->address() )
        return m_environment->localUser();
    return 0;
}

Participant* Contacts::addParticipant( const QString& address )
{
    Participant* p = participant(address);
    if ( p )
        return p;
    p = new Participant(address);
    m_participants[address] = p;
    return p;
}
