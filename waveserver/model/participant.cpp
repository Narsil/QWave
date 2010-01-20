#include "participant.h"

QHash<QString,Participant*>* Participant::s_participants = 0;

Participant::Participant(const QString& id)
        : m_id(id)
{
    if ( !s_participants )
        s_participants = new QHash<QString,Participant*>();
    s_participants->insert( id, this );
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

Participant* Participant::participant( const QString& id, bool create )
{
    if ( s_participants == 0 )
        s_participants = new QHash<QString,Participant*>();
    if ( s_participants->contains(id) )
        return (*s_participants)[id];
    if ( !create )
        return 0;
    Participant* p = new Participant(id);
    s_participants->insert( id, p );
    return p;
}
