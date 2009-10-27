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

#include <QStack>
#include <QtDebug>

Wavelet::Wavelet(Wave* wave, const QString& domain, const QString &id)
        : QObject(wave), m_id(id), m_domain(domain), m_wave(wave)
{
    environment()->addWavelet(this);
    m_doc = new StructuredDocument(this);
    m_processor = new OTProcessor(environment(), this);

    connect( m_processor, SIGNAL(documentMutation(QString,DocumentMutation)), SLOT(mutateDocument(QString,DocumentMutation)));
    connect( m_processor, SIGNAL(participantAdd(QString)), SLOT(addParticipant(QString)));
    connect( m_processor, SIGNAL(participantRemove(QString)), SLOT(removeParticipant(QString)));
}

Wavelet::~Wavelet()
{
    environment()->removeWavelet(this);
}

void Wavelet::updateConversation()
{
    QHash<QString,Blip*> blips( m_blips );
    QHash<QString,BlipThread*> blipThreads( m_blipThreads );
    m_blips.clear();
    m_blipThreads.clear();

    QObject* currentParent = this;
    QStack<StructuredDocument::Item> stack;
    QStack<QObject*> objectStack;
    for( QList<StructuredDocument::Item>::const_iterator it = m_doc->begin(); it != m_doc->end(); ++it )
    {
        if ( (*it).type == StructuredDocument::Char )
        {
            // Do nothing by intention
        }
        else if ( (*it).type == StructuredDocument::Start )
        {
            objectStack.push(currentParent);
            stack.push(*it);
            QString id = (*it).data.map->value("id");

            if ( (*it).data.map->value("type") == "blip" )
            {
                // Does such a blip already exist?
                Blip* blip = blips[id];
                if ( !blip )
                {
                    if ( currentParent == this )                    
                        blip = new Blip( (Wavelet*)currentParent, id );
                    else if ( qobject_cast<BlipThread*>(currentParent) )
                        blip = new Blip( (BlipThread*)currentParent, id );
                    else
                    {
                        // Ooooops
                        return;
                    }
                }
                m_blips[id] = blip;
                if ( blip->parent() != currentParent )
                    blip->setParent(currentParent);
                currentParent = blip;
            }
            else if ( (*it).data.map->value("type") == "thread" )
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
                m_blipThreads[id] = thread;
                if ( thread->parent() != currentParent )
                    thread->setParent(currentParent);
                currentParent = thread;
            }
            else if ( (*it).data.map->value("type") == "conversation" )
            {
                // Do nothing currently
            }
        }
        else if ( (*it).type == StructuredDocument::End )
        {
            if ( stack.count() == 0 )
                // Ooooops
                return;
            stack.pop();
            currentParent = objectStack.pop();
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

QList<Blip*> Wavelet::rootBlips() const
{
    QList<Blip*> result;
    for( QObjectList::const_iterator it = children().begin(); it != children().end(); ++it )
    {
        Blip* blip = qobject_cast<Blip*>(*it);
        if ( blip )
            result.append( blip );
    }
    return result;
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

void Wavelet::mutateDocument( const QString& documentId, const DocumentMutation& mutation )
{
    if ( documentId == "conversation" )
    {
        mutation.apply(m_doc);
    }
    else
    {
        Blip* b = blip(documentId);
        if ( b )
            b->receive(mutation);
    }
}
