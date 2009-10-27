#include "wave.h"
#include "wavelet.h"
#include "synchronizeddocument.h"
#include "wavedigest.h"
#include "app/environment.h"

Wave::Wave(Environment* environment, const QString& domain, const QString &id)
        : QObject(environment), m_id(id), m_domain(domain)
{
    m_digest = new WaveDigest(this);

    new Wavelet(this, domain, "conv+root");
}

Wavelet* Wave::wavelet() const
{
    for( QObjectList::const_iterator it = children().begin(); it != children().end(); ++it )
    {
        Wavelet* w = qobject_cast<Wavelet*>(*it);
        if ( w )
            return w;
    }
    return 0;
}

Wavelet* Wave::wavelet(const QString& id) const
{
    for( QObjectList::const_iterator it = children().begin(); it != children().end(); ++it )
    {
        Wavelet* w = qobject_cast<Wavelet*>(*it);
        if ( w && w->id() == id )
            return w;
    }
    return 0;
}

Environment* Wave::environment() const
{
    return (Environment*)parent();
}
