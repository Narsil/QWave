#include "wave.h"
#include "localwavelet.h"
#include "remotewavelet.h"
#include "wavefolk.h"
#include "app/settings.h"

// QHash<QString,Wave*>* Wave::s_waves = 0;

Wave::Wave(const QString& domain, const QString& id)
    : ActorGroup( domain + "$" + id ), m_domain(domain), m_id(id)
{
//    if ( s_waves == 0 )
//        s_waves = new QHash<QString,Wave*>();
//    (*s_waves)[m_domain + "!" + m_id] = this;
}

Wave::~Wave()
{
//    s_waves->remove(m_domain + "!" + m_id);
}

ActorGroup* Wave::group( const QString& groupId, bool createOnDemand )
{
    // Does the required group exist already?
    ActorGroup* g = this->ActorGroup::group( groupId, createOnDemand );
    if ( g || !createOnDemand )
        return g;

    int index = groupId.indexOf('$');
    if ( index == -1 )
    {
        qDebug("Malformed wavelet group ID, $ is missing in %s", groupId.toAscii().constData());
        return 0;
    }
    QString domain = groupId.left(index);
    QString id = groupId.mid(index + 1);

    // TODO: Check for well-formedness of domain and id

    // Local wave or remote wavelet? Create it.
    Wavelet* wavelet;
    if ( domain == Settings::settings()->domain() )
        wavelet = new LocalWavelet(this, domain, id);
    else
        wavelet = new RemoteWavelet(this, domain, id);
    return wavelet;
}

Wavelet* Wave::wavelet( const QString& domain, const QString& id, bool create )
{
    QString name = domain + "$" + id;
    return dynamic_cast<Wavelet*>(group( name, create ));

//    Wavelet* wavelet = dynamic_cast<Wavelet*>(group( name, create ));
//    if ( wavelet )
//        return wavelet;
//    if ( !create )
//        return 0;
//
//    if ( domain == Settings::settings()->domain() )
//        wavelet = new LocalWavelet(this, domain, id);
//    else
//        wavelet = new RemoteWavelet(this, domain, id);
//    // m_wavelets[name] = wavelet;
//    return wavelet;
}

Wave* Wave::wave(const QString& domain, const QString& id, bool create)
{
    return WaveFolk::instance()->wave( domain, id, create );
//    if ( s_waves != 0 )
//    {
//        Wave* wave = (*s_waves)[domain + "!" + id];
//        if ( wave )
//            return wave;
//    }
//    if ( !create )
//        return 0;
//
//    return new Wave( domain, id );
}

