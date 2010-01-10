#include "blip.h"
#include "blipthread.h"
#include "wavelet.h"
#include "blipdocument.h"
#include "app/environment.h"
#include "wave.h"
#include "documentmutation.h"
#include "contacts.h"
#include "participant.h"
#include "otprocessor.h"
#include <QtDebug>

Blip::Blip(Wavelet* wavelet, const QString& id, Participant* creator)
        : QObject(wavelet), m_id(id), m_doc(0), m_creator(creator)
{
    setup();
}

Blip::Blip(BlipThread* thread, const QString& id, Participant* creator)
        : QObject(thread), m_id(id), m_doc(0), m_creator(creator)
{
    setup();
}

Blip::Blip(Wavelet* wavelet, const QString& id, Participant* creator, const StructuredDocument& doc)
       : QObject(wavelet), m_id(id), m_doc(new BlipDocument(doc)), m_creator(creator)
{
    setup();
}

Blip::Blip(BlipThread* thread, const QString& id, Participant* creator, const StructuredDocument& doc)
      : QObject(thread), m_id(id), m_doc(new BlipDocument(doc)), m_creator(creator)
{
    setup();
}

void Blip::setup()
{
    m_unread = true;
    setObjectName(m_id);
    if ( !m_doc )
       m_doc = new BlipDocument(this);
    else
       m_doc->setParent(this);
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

QList<QString> Blip::authors() const
{
    return m_doc->authors();
}

void Blip::receive( const DocumentMutation& mutation, const QString& author )
{
    m_doc->apply(mutation, author);
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

void Blip::clearThreadList()
{
    m_threads.clear();
}

void Blip::addThread(BlipThread* thread)
{
    m_threads.append(thread);
}

void Blip::createFollowUpBlip()
{
    QString rand;
    rand.setNum( qrand() );

    DocumentMutation m2;
    QHash<QString,QString> map;
    map["name"] = environment()->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet()->processor()->handleSend( m2, "b+" + rand );

    DocumentMutation m1;
    m1.retain(m_convEndIndex + 1);
    map.clear();
    map["id"] = "b+" + rand;
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.retain( wavelet()->document()->count() - m_convEndIndex - 1 );
    wavelet()->processor()->handleSend( m1, "conversation" );
}

void Blip::createReplyBlip()
{
    QString rand;
    rand.setNum( qrand() );
    QString rand2;
    rand2.setNum( qrand() );

    DocumentMutation m2;
    QHash<QString,QString> map;
    map["name"] = environment()->localUser()->address();
    m2.insertStart("contributor", map);
    m2.insertEnd();
    map.clear();
    m2.insertStart("body", map);
    m2.insertStart("line", map);
    m2.insertEnd();
    m2.insertEnd();
    wavelet()->processor()->handleSend( m2, "b+" + rand );

    DocumentMutation m1;
    m1.retain(m_convStartIndex + 1);
    map.clear();
    map["id"] = "t+" + rand2;
    m1.insertStart("thread", map);
    map.clear();
    map["id"] = "b+" + rand;
    m1.insertStart("blip", map);
    m1.insertEnd();
    m1.insertEnd();
    m1.retain( wavelet()->document()->count() - m_convStartIndex - 1 );
    wavelet()->processor()->handleSend( m1, "conversation" );
}

void Blip::insertImage(int index, const QString& attachmentId, const QString& caption)
{
    DocumentMutation m1;
    m1.retain( index );
    int remain = m_doc->count() - index;
    StructuredDocument::AttributeList attribs;
    attribs[ "attachment" ] = attachmentId;
    m1.insertStart( "image", attribs );
    m1.insertStart( "caption", StructuredDocument::AttributeList() );
    m1.insertChars( caption );
    m1.insertEnd();
    m1.insertEnd();
    m1.retain( remain );
    wavelet()->processor()->handleSend( m1, id() );
}

void Blip::insertGadget( int index, const QUrl& url )
{
    DocumentMutation m1;
    m1.retain( index );
    int remain = m_doc->count() - index;
    StructuredDocument::AttributeList attribs;
    attribs["url"] = url.toString();
    attribs["author"] = environment()->localUser()->address();
    m1.insertStart( "gadget", attribs );
    m1.insertEnd();
    m1.retain( remain );
    wavelet()->processor()->handleSend( m1, id() );
}

void Blip::insertGadgetState( int gadgetIndex, const QString& name, const QString& value )
{
    // First, search whether a state tag with the same name attribute exists already
    int index = gadgetIndex + 1;
    int tagCount = 1;
    bool found = false;
    // Iterate over the contents of the gadget tag
    while( tagCount > 0 )
    {
        switch( m_doc->typeAt(index) )
        {
            case StructuredDocument::Start:
                tagCount++;
                if ( m_doc->tagAt(index) == "state" && m_doc->attributesAt(index)["name"] == name )
                {
                    found = true;
                    // This terminate the while loop
                    tagCount = 0;
                }
                else
                    index++;
                break;
            case StructuredDocument::End:
                index++;
                tagCount--;
                break;
            case StructuredDocument::Char:
                index++;
                break;
        }
    }

    DocumentMutation m1;
    if ( found )
    {
        m1.retain( index );
        int remain = m_doc->count() - index;
        if ( value.isNull() )
        {
            m1.deleteStart( "state", m_doc->attributesAt(index) );
            m1.deleteEnd();
            m1.retain( remain - 2 );
        }
        else
        {
            StructuredDocument::AttributeList attribs = m_doc->attributesAt(index);
            QHash<QString,StructuredDocument::StringPair> changes;
            changes["value"].first = attribs["value"];
            changes["value"].second = value;
            m1.updateAttributes(changes);
            m1.retain( remain - 1 );
        }
    }
    else
    {
        m1.retain( gadgetIndex + 1 );
        StructuredDocument::AttributeList attribs;
        attribs["name"] = name;
        attribs["value"] = value;
        m1.insertStart( "state", attribs );
        m1.insertEnd();
        m1.retain( m_doc->count() - gadgetIndex - 1 );
    }
    wavelet()->processor()->handleSend( m1, id() );
}

int Blip::gadgetIndex(const QString& gadgetId) const
{
    for( int i = 0; i < m_doc->count(); ++i )
    {
        if ( m_doc->typeAt(i) == StructuredDocument::Start && m_doc->tagAt(i) == "gadget" && m_doc->attributesAt(i)["**id"] == gadgetId )
            return i;
    }
    return -1;
}

int Blip::childBlipCount() const
{
    int result = 0;
    foreach( BlipThread* t, m_threads )
    {
        result += t->blipCount();
    }
    return result;
}

void Blip::setUnread( bool unread )
{
    if ( unread == m_unread )
        return;
    m_unread = unread;
    emit unreadChanged();
}

void Blip::setChildrenUnread(bool unread)
{
	foreach( BlipThread* t, m_threads )
	{
		t->setUnread(unread);
	}
}

int Blip::unreadChildBlipCount() const
{
    int result = 0;
    foreach( BlipThread* t, m_threads )
    {
        result += t->unreadBlipCount();
    }
    return result;
}
