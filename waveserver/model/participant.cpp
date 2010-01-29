#include "participant.h"

QHash<QString,Participant*>* Participant::s_participants = 0;

Participant::Participant(const JID& jid)
        : m_jid(jid)
{
    if ( !s_participants )
        s_participants = new QHash<QString,Participant*>();
    s_participants->insert( jid.toString(), this );
}

Participant::~Participant()
{
}

void Participant::addWavelet( Wavelet* wavelet )
{
    m_wavelets.insert(wavelet);
}

void Participant::removeWavelet( Wavelet* wavelet )
{
    m_wavelets.remove(wavelet);
}

QSet<Wavelet*> Participant::wavelets() const
{
    return m_wavelets;
}

Participant* Participant::participant( const QString& jid, bool create )
{
    // Check the input
    JID j( jid );
    if ( !j.isValid() )
        return 0;

    if ( s_participants == 0 )
        s_participants = new QHash<QString,Participant*>();
    if ( s_participants->contains(jid) )
        return (*s_participants)[jid];
    if ( !create )
        return 0;
    // Create the participant if it does not yet exist.
    Participant* p = new Participant(j);
    s_participants->insert( jid, p );
    return p;
}
