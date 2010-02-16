#include "wave.h"
#include "localwavelet.h"
#include "remotewavelet.h"
#include "wavefolk.h"
#include "app/settings.h"

Wave::Wave(WaveFolk* folk, const QString& domain, const QString& id)
    : ActorGroup( domain + "$" + id, folk ), m_domain(domain), m_id(id)
{
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
}

Wave* Wave::wave(const QString& domain, const QString& id, bool create)
{
    return WaveFolk::instance()->wave( domain, id, create );
}

