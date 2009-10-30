#include "environment.h"
#include "model/participant.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/wavelist.h"
#include "model/contacts.h"
#include "network/networkadapter.h"
#include "settings.h"

Environment::Environment()
{
    m_settings = new Settings(this);
    m_localUser = new Participant(m_settings->userAddress());
    m_localUser->setName(m_settings->userName());

    m_contacts = new Contacts(this, this);
    m_inbox = new WaveList(this);
    m_networkAdapter = new NetworkAdapter(this);    
}

Participant* Environment::localUser() const
{
    return m_localUser;
}

void Environment::configure()
{
    m_localUser->setAddress(m_settings->userAddress());
    m_localUser->setName(m_settings->userName());
    m_networkAdapter->setServer( m_settings->serverName(), m_settings->serverPort() );
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
