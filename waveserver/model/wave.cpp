#include "wave.h"

QHash<QString,Wave*>* Wave::s_waves = 0;

Wave::Wave(const QString& domain, const QString& id)
    : m_domain(domain), m_id(id)
{
    if ( s_waves == 0 )
        s_waves = new QHash<QString,Wave*>();
    (*s_waves)[m_domain + "!" + m_id] = this;
}

Wave::~Wave()
{
    s_waves->remove(m_domain + "!" + m_id);
}

Wavelet* Wave::wavelet( const QString& domain, const QString& id, bool create )
{
    QString name = domain + "!" + id;
    if ( m_wavelets.contains(name))
        return m_wavelets[name];
    if ( !create )
        return 0;
    Wavelet* wavelet = new Wavelet(this, domain, id);
    m_wavelets[name] = wavelet;
    return wavelet;
}

Wave* Wave::wave(const QString& domain, const QString& id, bool create)
{
    if ( s_waves != 0 )
    {
        Wave* wave = (*s_waves)[domain + "!" + id];
        if ( wave )
            return wave;
    }
    if ( !create )
        return 0;

    return new Wave( domain, id );
}

