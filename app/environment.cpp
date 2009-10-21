#include "environment.h"
#include "model/participant.h"
#include "model/wavelet.h"
#include "network/networkadapter.h"

Environment::Environment(const QString& address, const QString& name)
{
    m_localUser = new Participant(address);
    m_localUser->setName(name);

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
