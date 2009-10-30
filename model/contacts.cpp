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

    emit participantAdded(p);

    return p;
}

QList<Participant*> Contacts::participants() const
{
    QList<Participant*> result;
    foreach( Participant* p, m_participants.values() )
    {
        result.append(p);
    }
    return result;
}
