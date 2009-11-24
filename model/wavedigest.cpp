#include "wavedigest.h"
#include "wave.h"
#include "documentmutation.h"
#include "structureddocument.h"
#include "otprocessor.h"
#include "contacts.h"
#include "participant.h"
#include "app/environment.h"

WaveDigest::WaveDigest(Wave* parent)
        : QObject(parent)
{
    m_doc = new StructuredDocument(this);
    m_processor = new OTProcessor(parent->environment(), this);

    connect( m_processor, SIGNAL(documentMutation(QString,DocumentMutation,QString)), SLOT(mutateDocument(QString,DocumentMutation,QString)));
    connect( m_processor, SIGNAL(participantAdd(QString)), SLOT(addParticipant(QString)));
    connect( m_processor, SIGNAL(participantRemove(QString)), SLOT(removeParticipant(QString)));
}

void WaveDigest::addParticipant( const QString& address )
{
    Participant* participant = wave()->environment()->contacts()->addParticipant(address);
    if ( !m_participants.contains(participant) )
    {
        m_participants.append(participant);
        emit participantAdded(participant);
    }
}

void WaveDigest::removeParticipant( const QString& address )
{
    Participant* p = wave()->environment()->contacts()->participant(address);
    m_participants.removeAll(p);
    emit participantRemoved(p);
}

void WaveDigest::mutateDocument( const QString&, const DocumentMutation& mutation, const QString& author )
{
    m_doc->apply(mutation, author);
    emit digestChanged();
}

QString WaveDigest::toPlainText() const
{
    return m_doc->toPlainText();
}

Wave* WaveDigest::wave() const
{
    return (Wave*)parent();
}
