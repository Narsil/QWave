#include "environment.h"
#include "model/participant.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/wavelist.h"
#include "model/contacts.h"
#include "network/networkadapter.h"
#include "settings.h"

Environment::Environment(const QString& profile)
{
    m_settings = new Settings(profile, this);
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
    m_wavelets[wavelet->url().toString()] = wavelet;
}

void Environment::removeWavelet( Wavelet* wavelet )
{
    m_wavelets.remove(wavelet->url().toString());
}

Wave* Environment::wave( const QString& domain, const QString& id ) const
{
    return m_waves[domain + "$" + id];
}

Wave* Environment::createWave( const QString& id )
{
    return createWave( settings()->waveDomain(), id );
}

Wave* Environment::createWave( const QString& domain, const QString& id )
{
    Wave* wave = m_waves[id];
    if ( wave )
        return wave;
    wave = new Wave(this, domain, id);
    m_waves[domain + "$" + id] = wave;
    return wave;
}
