#include "digest.h"
#include "otprocessor.h"
#include "wave.h"
#include "wavedigest.h"
#include "contacts.h"
#include "app/environment.h"

Digest::Digest(Environment* environment, QObject* parent)
        : QObject(parent), m_environment(environment)
{
    m_processor = new OTProcessor(environment, this);

    connect( m_processor, SIGNAL(documentMutation(QString,DocumentMutation)), SLOT(mutateDocument(QString,DocumentMutation)));
    connect( m_processor, SIGNAL(participantAdd(QString)), SLOT(addParticipant(QString)));
    connect( m_processor, SIGNAL(participantRemove(QString)), SLOT(removeParticipant(QString)));
}

void Digest::addParticipant( const QString& address, const QString& waveid )
{
    Wave* w = m_environment->wave(waveid);
    if ( w )
        w->digest()->addParticipant(m_environment->contacts()->addParticipant(address));
}

void Digest::removeParticipant( const QString& address, const QString& waveid )
{
    Wave* w = m_environment->wave(waveid);
    if ( w )
    {
        Participant* p = m_environment->contacts()->participant(address);
        if ( p )
            w->digest()->removeParticipant(p);
    }
}

void Digest::mutateDocument( const QString&, const DocumentMutation& mutation, const QString& waveid )
{
    Wave* w = m_environment->wave(waveid);
    if ( w )
        w->digest()->mutate(mutation);
}
