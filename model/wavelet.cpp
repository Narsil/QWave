#include "wavelet.h"
#include "wave.h"
#include "blip.h"
#include "blipthread.h"
#include "structureddocument.h"
#include "participant.h"
#include "documentmutation.h"
#include "otprocessor.h"
#include "contacts.h"
#include "app/environment.h"
#include "network/networkadapter.h"
#include "unknowndocument.h"
#include <QStack>
#include <QtDebug>
#include <QImage>
#include <QByteArray>
#include <QBuffer>

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

QUrl Wavelet::url() const
{
    QUrl url;
    url.setScheme("wave");
    url.setHost( domain() );
    // TODO: This is only true for local wavelets!
    url.setPath( "/" + wave()->id() + "/" + m_id );
    return url;
}

void Wavelet::updateConversation(const QString& author)
{
    QHash<QString,Blip*> blips( m_blips );
    QHash<QString,BlipThread*> blipThreads( m_blipThreads );
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
}

void Wavelet::print_()
{
    qDebug() << "Wavelet id=" << this->id();
    foreach( Blip* blip, rootBlips() )
    {
        blip->print_(1);
    }
}

Blip* Wavelet::blip(const QString& id)
{
    return m_blips[id];
}

void Wavelet::addParticipant( Participant* participant)
{
    if ( !m_participants.contains(participant) )
        m_participants.append(participant);
    emit participantAdded(participant);
}

void Wavelet::removeParticipant( Participant* participant)
{
    m_participants.removeAll(participant);
    emit participantRemoved(participant);
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
            return;
        }
    }
}

void Wavelet::mutateDocument( const QString& documentId, const DocumentMutation& mutation, const QString& author )
{
    if ( documentId == "conversation" )
    {
        m_doc->apply(mutation, author);
        this->updateConversation(author);
        emit conversationChanged();
    }
    else
    {
        Blip* b = blip(documentId);
        if ( b )
            b->receive(mutation, author);
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

QString Wavelet::insertImageAttachment(const QUrl& url, int width, int height, const QImage& thumbnail)
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
    m1.insertStart("thumbnail");
    m1.insertChars( QString( ba.toBase64() ) );
    m1.insertEnd();
    attribs.clear();
    attribs["width"] = width;
    attribs["height"] = height;
    m1.insertStart("image", attribs);
    m1.insertEnd();
    m1.insertEnd();
    processor()->handleSend( m1, rand );

    return rand;
}
