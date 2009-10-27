#include "wavedigest.h"
#include "wave.h"
#include "documentmutation.h"
#include "structureddocument.h"

WaveDigest::WaveDigest(Wave* parent)
        : QObject(parent)
{
    m_doc = new StructuredDocument(this);
}

void WaveDigest::addParticipant( Participant* participant)
{
    if ( !m_participants.contains(participant) )
        m_participants.append(participant);
    emit participantAdded(participant);
}

void WaveDigest::removeParticipant( Participant* participant)
{
    m_participants.removeAll(participant);
    emit participantRemoved(participant);
}

void WaveDigest::mutate(const DocumentMutation& mutation)
{
    // m_digestDoc->handleReceive(mutation);
    mutation.apply(m_doc);
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
