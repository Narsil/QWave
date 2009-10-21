#include "blipthread.h"
#include "blip.h"
#include <QtDebug>

BlipThread::BlipThread(Blip* blip, const QString& id)
        : QObject( blip ), m_id(id)
{
    this->setObjectName(id);
}

Blip* BlipThread::parentBlip()
{
    return qobject_cast<Blip*>(parent());
}

Wavelet* BlipThread::wavelet()
{
    return parentBlip()->wavelet();
}

QList<Blip*> BlipThread::blips()
{
    QList<Blip*> result;
    for( QObjectList::const_iterator it = children().begin(); it != children().end(); ++it )
    {
        Blip* blip = qobject_cast<Blip*>(*it);
        if ( blip )
            result.append(blip);
    }
    return result;
}

void BlipThread::print_(int ind)
{
    QString indent;
    indent.fill(' ', ind);
    qDebug() << indent.toAscii().constData() << "Thread id=" << this->id();

    foreach( Blip* b, blips() )
    {
        b->print_(ind+1);
    }
}

