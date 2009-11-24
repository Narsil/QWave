#include "wave.h"
#include "wavelet.h"
#include "wavedigest.h"
#include "app/environment.h"

Wave::Wave(Environment* environment, const QString& domain, const QString &id)
        : QObject(environment), m_id(id), m_domain(domain)
{


    Wavelet* wavelet = new Wavelet(this, domain, "conv+root");
    connect( wavelet, SIGNAL(blipCountChanged()), SIGNAL(blipCountChanged()));
    m_digest = new WaveDigest(this);
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

void Wave::setLastChange()
{
    m_lastChange = QDateTime::currentDateTime();
    emit dateChanged();
}

int Wave::blipCount() const
{
    return wavelet()->blipCount();
}

int Wave::unreadBlipCount() const
{
    return wavelet()->unreadBlipCount();
}

