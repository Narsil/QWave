#include "blip.h"
#include "blipthread.h"
#include "wavelet.h"
#include "structureddocument.h"
#include "app/environment.h"
#include "wave.h"
#include "documentmutation.h"
#include "contacts.h"
#include <QtDebug>

Blip::Blip(Wavelet* wavelet, const QString& id)
        : QObject(wavelet), m_id(id), m_doc(0)
{
    setup();
}

Blip::Blip(BlipThread* thread, const QString& id)
        : QObject(thread), m_id(id), m_doc(0)
{
    setup();
}

Blip::Blip(Wavelet* wavelet, const QString& id, StructuredDocument* doc)
       : QObject(wavelet), m_id(id), m_doc(doc)
{
    setup();
}

Blip::Blip(BlipThread* thread, const QString& id, StructuredDocument* doc)
      : QObject(thread), m_id(id), m_doc(doc)
{
    setup();
}

void Blip::setup()
{
     setObjectName(m_id);
     if ( !m_doc )
        m_doc = new StructuredDocument(this);
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

const QList<Participant*>& Blip::authors() const
{
    return m_authors;
//
//    // TODO: Bogus implementation
//    Environment* en = wavelet()->wave()->environment();
//    QList<Participant*> result;
//    result.append( en->localUser() );
//    return result;
}

void Blip::receive( const DocumentMutation& mutation )
{
    mutation.apply(m_doc);

    // Find authors
    m_authors.clear();
    for( QList<StructuredDocument::Item>::const_iterator it = m_doc->begin(); it != m_doc->end(); ++it )
    {
        if ( (*it).type == StructuredDocument::Start && (*it).data.map )
        {
            QString key = (*it).data.map->value("type");
            if ( key == "contributor" )
            {
                QString name = (*it).data.map->value("name");
                if ( !name.isEmpty() )
                {
                    Participant* p = wavelet()->environment()->contacts()->addParticipant(name);
                    if ( !m_authors.contains(p) )
                        m_authors.append(p);
                }
            }
        }
    }

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
