#include "wavelet.h"
#include "wave.h"
#include "blip.h"
#include "blipthread.h"
#include "model/structureddocument.h"
#include "participant.h"
#include "model/documentmutation.h"
#include "otprocessor.h"
#include "contacts.h"
#include "app/environment.h"
#include "network/networkadapter.h"
#include "unknowndocument.h"
#include "attachment.h"
#include <QStack>
#include <QtDebug>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QtGlobal>

Wavelet::Wavelet(Wave* wave, const QString& domain, const QString &id)
        : QObject(wave), m_id(id), m_domain(domain), m_wave(wave)
{
    environment()->addWavelet(this);
    m_doc = new StructuredDocument(this);
    m_processor = new OTProcessor(this);

    connect( m_processor, SIGNAL(documentMutation(QString,DocumentMutation,QString)), SLOT(mutateDocument(QString,DocumentMutation,QString)));
    connect( m_processor, SIGNAL(participantAdd(QString)), SLOT(addParticipant(QString)));
    connect( m_processor, SIGNAL(participantRemove(QString)), SLOT(removeParticipant(QString)));
}

Wavelet::~Wavelet()
{
    environment()->removeWavelet(this);
}

WaveUrl Wavelet::url() const
{
    WaveUrl url( m_wave->domain(), m_wave->id(), m_domain, m_id );
    return url;
}

void Wavelet::updateConversation(const QString& author)
{
    QHash<QString,Blip*> blips( m_blips );
    QHash<QString,BlipThread*> blipThreads( m_blipThreads );
    QList<Blip*> newBlips;
    m_blips.clear();
    m_blipThreads.clear();
    m_rootBlips.clear();

    // Parse the conversation document and rebuild the conversation.
    // Keep all blips which already exist.

    Participant* creator = environment()->contacts()->addParticipant(author);
    QObject* currentParent = this;
    int stackCount = 0;
    QStack<QObject*> objectStack;
    for( int i = 0; i < m_doc->count(); ++i )
    {
        switch( m_doc->typeAt(i) )
        {
        case StructuredDocument::Char:
            // Do nothing by intention
            break;
        case StructuredDocument::Start:
            {
                objectStack.push(currentParent);
                stackCount++;
                StructuredDocument::AttributeList attribs = m_doc->attributesAt(i);
                QString id = attribs["id"];
                QString tag = m_doc->tagAt(i);

                if ( tag == "blip" )
                {
                    // Does such a blip already exist?
                    Blip* blip = blips[id];
                    if ( !blip )
                    {
                        // Does a blip document already exist?
                        UnknownDocument* u = m_unknownDocs[id];
                        if ( u )
                        {
                            if ( currentParent == this )
                                blip = new Blip( (Wavelet*)currentParent, id, creator, *(u->document()) );
                            else if ( qobject_cast<BlipThread*>(currentParent) )
                                blip = new Blip( (BlipThread*)currentParent, id, creator, *(u->document()) );
                            else
                            {
                                // Ooooops
                                return;
                            }                            
                            m_unknownDocs.remove(id);
                            delete u;
                        }
                        else
                        {
                            if ( currentParent == this )
                                blip = new Blip( (Wavelet*)currentParent, id, creator );
                            else if ( qobject_cast<BlipThread*>(currentParent) )
                                blip = new Blip( (BlipThread*)currentParent, id, creator );
                            else
                            {
                                // Ooooops
                                return;
                            }
                        }
                        if ( author == m_wave->environment()->localUser()->address() )
                            blip->setUnread(false);
                        else
                            newBlips.append(blip);
                        connect( blip, SIGNAL(unreadChanged()), SIGNAL(blipCountChanged()));
                    }
                    // The blip already exists
                    else
                    {
                        blip->setParent(currentParent);
                        blip->clearThreadList();
                    }
                    blip->setConversationStartIndex(i);
                    if ( currentParent == this )
                        m_rootBlips.append(blip);
                    else if ( qobject_cast<BlipThread*>(currentParent) )
                        ((BlipThread*)currentParent)->addBlip(blip);
                    m_blips[id] = blip;
                    currentParent = blip;
                }
                else if ( tag == "thread" )
                {
                    // Does such a blip already exist?
                    BlipThread* thread = blipThreads[id];
                    if ( !thread )
                    {
                        if ( qobject_cast<Blip*>(currentParent) )
                            thread = new BlipThread( (Blip*)currentParent, id );
                        else
                        {
                            // Ooooops
                            return;
                        }
                    }
                    else
                        thread->clearBlipList();
                    ((Blip*)currentParent)->addThread(thread);
                    m_blipThreads[id] = thread;
                    if ( thread->parent() != currentParent )
                        thread->setParent(currentParent);
                    currentParent = thread;
                }
                else if ( tag == "conversation" )
                {
                    // Do nothing currently
                }
            }
            break;
        case StructuredDocument::End:
            {
                if ( stackCount == 0 )
                    // Ooooops
                    return;
                if ( qobject_cast<Blip*>(currentParent) )
                    qobject_cast<Blip*>(currentParent)->setConversationEndIndex(i);
                stackCount--;
                currentParent = objectStack.pop();
            }
            break;
        }
    }

    // Delete unused threads or blips
    foreach( QString key, blips.keys() )
    {
        if ( !m_blips.contains(key) )
            delete blips[key];
    }
    foreach( QString key, blipThreads.keys() )
    {
        if ( !m_blipThreads.contains(key) )
            delete blipThreads[key];
    }

    emit blipCountChanged();

    foreach (Blip* blip, newBlips){
        emit newBlipAdded(blip);
    }
}

void Wavelet::print_()
{
    qDebug() << "Wavelet id=" << this->id();
    foreach( Blip* blip, rootBlips() )
    {
        blip->print_(1);
    }
}

Blip* Wavelet::blip(const QString& id) const
{
    return m_blips[id];
}

Blip* Wavelet::rootBlip( const QString& id ) const
{
    Blip* b = blip(id);
    if ( b && b->isRootBlip() )
        return b;
    return 0;
}

void Wavelet::addParticipant( Participant* participant)
{
    if ( !m_participants.contains(participant) ){
        m_participants.append(participant);
        emit participantAdded(participant,this);
    }
}

void Wavelet::removeParticipant( Participant* participant)
{
    m_participants.removeAll(participant);
    emit participantRemoved(participant,this);
}

Environment* Wavelet::environment() const
{
    return wave()->environment();
}

Participant* Wavelet::participant( const QString& address )
{
    foreach( Participant* p, m_participants)
    {
        if ( p->address() == address )
            return p;
    }
    return 0;
}

void Wavelet::addParticipant( const QString& address )
{
    this->addParticipant( environment()->contacts()->addParticipant(address) );
}

void Wavelet::removeParticipant( const QString& address )
{
    foreach( Participant* p, m_participants)
    {
        if ( p->address() == address )
        {
            m_participants.removeAll(p);
            emit participantRemoved(p,this);
            return;
        }
    }

}

void Wavelet::mutateDocument( const QString& documentId, const DocumentMutation& mutation, const QString& author )
{
    m_wave->setLastChange();

    if ( documentId == "conversation" )
    {
        m_doc->apply(mutation, author);
        this->updateConversation(author);
        if(author!=environment()->localUser()->address())
            emit conversationChanged(this);
    }
    else
    {
        Blip* b = blip(documentId);
        if ( b )
        {
            b->receive(mutation, author);
            // If the blip is authored by the local user, it can be considered being read
            if ( author == m_wave->environment()->localUser()->address() )
                b->setUnread(false);
            else
                b->setUnread(true);
        }
        else if ( documentId.left(2) == "a+" )
        {
            Attachment* a = m_attachments[documentId];
            if ( !a )
            {
                a = new Attachment(this);
                m_attachments[documentId] = a;
            }
            a->apply(mutation, author);
        }
        else
        {
            UnknownDocument* d = m_unknownDocs[documentId];
            if ( !d )
            {
                d = new UnknownDocument(documentId);
                m_unknownDocs[documentId] = d;
            }
            d->receive(mutation, author);
        }
    }
}

QString Wavelet::insertImageAttachment(const QUrl& url, const QImage& image, const QImage& thumbnail)
{
    QString rand;
    rand.setNum( qrand() );
    rand = "a+" + rand;

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    thumbnail.save(&buffer, "PNG");

    DocumentMutation m1;
    StructuredDocument::AttributeList attribs;
    attribs["attachmentId"] = rand;
    attribs["src"] = url.toString();
    m1.insertStart("attachment", attribs);
    attribs.clear();
    attribs["width"] = QString::number(thumbnail.width());
    attribs["height"] = QString::number(thumbnail.height());
    m1.insertStart("thumbnail", attribs);
    m1.insertChars( QString( ba.toBase64() ) );
    m1.insertEnd();
    attribs.clear();
    attribs["width"] = QString::number(image.width());
    attribs["height"] = QString::number(image.height());
    m1.insertStart("image", attribs);
    m1.insertEnd();
    m1.insertEnd();
    processor()->handleSend( m1, rand );

    return rand;
}

Attachment* Wavelet::attachment( const QString& id ) const
{
    return m_attachments[id];
}

int Wavelet::blipCount() const
{
    int result = 0;

    foreach( Blip* blip, rootBlips() )
    {
        result++;
        result += blip->childBlipCount();
    }

    return result;
}

int Wavelet::unreadBlipCount() const
{
    int result = 0;

    foreach( Blip* blip, rootBlips() )
    {
        if ( blip->isUnread() )
            result++;
        result += blip->unreadChildBlipCount();
    }

    return result;
}

void Wavelet::setUnread(bool unread)
{
    foreach( Blip* blip, rootBlips() )
    {
        if ( blip->isUnread()!=unread )
            blip->setUnread(unread);
        blip->setChildrenUnread(unread);
    }
}

Blip* Wavelet::createRootBlip(const QString& text)
{
    if ( m_rootBlips.count() == 0 )
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
        if ( !text.isNull() )
            m2.insertChars( text );
        m2.insertEnd();
        processor()->handleSend( m2, "b+" + rand );

        DocumentMutation m1;
        m1.retain(1);
        map.clear();
        map["id"] = "b+" + rand;
        m1.insertStart("blip", map);
        m1.insertEnd();
        m1.retain( document()->count() - 1 );
        processor()->handleSend( m1, "conversation" );

        Q_ASSERT( m_rootBlips.count() > 0 );
        return this->rootBlip("b+" + rand);
    }
    return m_rootBlips.last()->createFollowUpBlip(text);
}
