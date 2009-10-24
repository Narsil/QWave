#include "wave.h"
#include "wavelet.h"
#include "synchronizeddocument.h"
#include "documentmutation.h"
#include "app/environment.h"

Wave::Wave(Environment* environment, const QString& domain, const QString &id)
        : QObject(environment), m_id(id), m_domain(domain)
{
    m_digestDoc = new SynchronizedDocument(this);

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

void Wave::setDigest(const QString& digest)
{
    m_digest = digest;
    emit digestChanged();
}

Environment* Wave::environment() const
{
    return (Environment*)parent();
}

void Wave::mutateDigest(const DocumentMutation& mutation)
{
    // m_digestDoc->handleReceive(mutation);
    mutation.apply(m_digestDoc);
    setDigest( m_digestDoc->toPlainText() );
}
