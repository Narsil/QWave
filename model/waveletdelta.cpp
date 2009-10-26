#include "waveletdelta.h"
#include "documentmutation.h"

WaveletDelta::WaveletDelta()
        : m_mutation(0)
{
}

WaveletDelta::WaveletDelta(const WaveletDelta& delta)
        : m_documentId(delta.m_documentId),
          m_mutation(delta.m_mutation ? new DocumentMutation(*(delta.m_mutation)) : 0),
          m_addParticipant(delta.m_addParticipant),
          m_removeParticipant(delta.m_removeParticipant),
          m_version( delta.m_version ),
          m_resultingVersion( delta.m_resultingVersion ),
          m_author( delta.m_author )
{
}

WaveletDelta::~WaveletDelta()
{
    if ( m_mutation != 0 )
        delete m_mutation;
}

DocumentMutation* WaveletDelta::createMutation()
{
    if ( m_mutation != 0 )
        delete m_mutation;
    m_mutation = new DocumentMutation;
    return m_mutation;
}

void WaveletDelta::setMutation( const DocumentMutation& mutation )
{
    if ( m_mutation != 0 )
        delete m_mutation;
    m_mutation = new DocumentMutation(mutation);
}

bool WaveletDelta::isNull() const
{
    return m_mutation == 0 && m_addParticipant.isEmpty() && m_removeParticipant.isEmpty();
}

WaveletDelta WaveletDelta::translate(const WaveletDelta& delta) const
{
    WaveletDelta result(*this);
    if ( hasAddParticipant() )
    {
        if ( delta.hasAddParticipant() && delta.addParticipant() == m_addParticipant )
            result.clearAddParticipant();
    }
    if ( hasRemoveParticipant() )
    {
        if ( delta.hasRemoveParticipant() && delta.removeParticipant() == m_removeParticipant )
            result.clearRemoveParticipant();
    }
    if ( mutation() != 0 )
    {
        if ( delta.mutation() != 0 )
            result.setMutation( mutation()->translate(*(delta.mutation())) );
    }
    return result;
}
