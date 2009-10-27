#include "environment.h"
#include "model/participant.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/wavelist.h"
#include "model/contacts.h"
#include "network/networkadapter.h"

Environment::Environment(const QString& address, const QString& name)
{
    m_localUser = new Participant(address);
    m_localUser->setName(name);

    m_contacts = new Contacts(this, this);
    m_inbox = new WaveList(this);
    m_networkAdapter = new NetworkAdapter(this);    
}

Participant* Environment::localUser() const
{
    return m_localUser;
}

NetworkAdapter* Environment::networkAdapter() const
{
    return m_networkAdapter;
}

void Environment::addWavelet( Wavelet* wavelet )
{
    m_wavelets[wavelet->id()] = wavelet;
}

void Environment::removeWavelet( Wavelet* wavelet )
{
    m_wavelets.remove(wavelet->id());
}

Wave* Environment::wave( const QString& id )
{
    return m_waves[id];
}

Wave* Environment::createWave( const QString& id )
{
    Wave* wave = m_waves[id];
    if ( wave )
        return wave;
    wave = new Wave(this, m_networkAdapter->serverName(), id);
    m_waves[id] = wave;
    return wave;
}
