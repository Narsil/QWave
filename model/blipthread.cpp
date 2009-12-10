#include "blipthread.h"
#include "blip.h"
#include <QtDebug>

BlipThread::BlipThread(Blip* blip, const QString& id)
        : QObject( blip ), m_id(id)
{
    this->setObjectName(id);
}

Blip* BlipThread::parentBlip() const
{
    return qobject_cast<Blip*>(parent());
}

Wavelet* BlipThread::wavelet() const
{
    return parentBlip()->wavelet();
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

void BlipThread::addBlip(Blip* blip)
{
    m_blips.append(blip);
}

void BlipThread::clearBlipList()
{
    m_blips.clear();
}

int BlipThread::blipCount() const
{
    int result = 0;

    foreach( Blip* b, blips() )
    {
        result++;
        result += b->childBlipCount();
    }
    return result;
}

int BlipThread::unreadBlipCount() const
{
    int result = 0;

    foreach( Blip* b, blips() )
    {
        if ( b->isUnread() )
            result++;
        result += b->unreadChildBlipCount();
    }
    return result;
}

void BlipThread::setUnread(bool unread)
{
	foreach( Blip* b, blips() )
	{
		if ( b->isUnread()!=unread )
			b->setUnread(unread);
		b->setChildrenUnread(unread);
	}
}
