#include "wave.h"
#include "wavelet.h"
#include "app/environment.h"

Wave::Wave(Environment* environment, const QString& domain, const QString &id)
        : QObject(environment), m_id(id), m_domain(domain)
{
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

Environment* Wave::environment() const
{
    return (Environment*)parent();
}
