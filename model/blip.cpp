#include "blip.h"
#include "blipthread.h"
#include "wavelet.h"
#include "synchronizeddocument.h"
#include "app/environment.h"
#include "wave.h"
#include <QtDebug>

Blip::Blip(Wavelet* wavelet, const QString& id)
        : QObject(wavelet), m_id(id)
{
    setup();
}

Blip::Blip(BlipThread* thread, const QString& id)
        : QObject(thread), m_id(id)
{
    setup();
}

void Blip::setup()
{
     setObjectName(m_id);
     m_doc = new SynchronizedDocument(environment(), this);
}

BlipThread* Blip::parentThread() const
{
    return qobject_cast<BlipThread*>(parent());
}

Wavelet* Blip::wavelet() const
{
    Wavelet* w = qobject_cast<Wavelet*>(parent());
    if ( w )
        return w;
    return parentThread()->wavelet();
}

Environment* Blip::environment() const
{
    return wavelet()->environment();
}

QList<BlipThread*> Blip::threads() const
{
    QList<BlipThread*> result;
    for( QObjectList::const_iterator it = children().begin(); it != children().end(); ++it )
    {
        BlipThread* thread = qobject_cast<BlipThread*>(*it);
        if ( thread )
            result.append(thread);
    }
    return result;
}

bool Blip::isRootBlip() const
{
    return !this->parentThread();
}

bool Blip::isFirstRootBlip() const
{
    Wavelet* w = wavelet();
    return w && w->rootBlips().first() == this;
}

bool Blip::isLastBlipInThread() const
{
    BlipThread* t = parentThread();
    if ( !t )
    {
        Wavelet* w = wavelet();
        return w && w->rootBlips().last() == this;
    }

    return ( t->blips().last() == this );
}

QList<Participant*> Blip::authors() const
{
    // TODO: Bogus implementation
    Environment* en = wavelet()->wave()->environment();
    QList<Participant*> result;
    result.append( en->localUser() );
    return result;
}

void Blip::receive( const DocumentMutation& mutation )
{
    mutation.apply(m_doc);
    emit update(mutation);
}

void Blip::print_(int indent)
{
    QString ind;
    ind.fill(' ', indent);
    qDebug() << ind.toAscii().constData() << "Blip id=" << this->id();

    foreach( BlipThread* t, threads() )
    {
        t->print_(indent+1);
    }
}
