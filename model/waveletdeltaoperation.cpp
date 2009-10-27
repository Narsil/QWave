#include "waveletdeltaoperation.h"
#include "documentmutation.h"

WaveletDeltaOperation::WaveletDeltaOperation()
        : m_mutation(0)
{
}

WaveletDeltaOperation::WaveletDeltaOperation(const WaveletDeltaOperation& delta)
        : m_documentId(delta.m_documentId),
          m_mutation(delta.m_mutation ? new DocumentMutation(*(delta.m_mutation)) : 0),
          m_addParticipant(delta.m_addParticipant),
          m_removeParticipant(delta.m_removeParticipant)
{
}

WaveletDeltaOperation::~WaveletDeltaOperation()
{
    if ( m_mutation != 0 )
        delete m_mutation;
}

DocumentMutation* WaveletDeltaOperation::createMutation()
{
    if ( m_mutation != 0 )
        delete m_mutation;
    m_mutation = new DocumentMutation;
    return m_mutation;
}

void WaveletDeltaOperation::setMutation( const DocumentMutation& mutation )
{
    if ( m_mutation != 0 )
        delete m_mutation;
    m_mutation = new DocumentMutation(mutation);
}

bool WaveletDeltaOperation::isNull() const
{
    return m_mutation == 0 && m_addParticipant.isEmpty() && m_removeParticipant.isEmpty();
}

WaveletDeltaOperation WaveletDeltaOperation::translate(const WaveletDeltaOperation& delta) const
{
    WaveletDeltaOperation result(*this);
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
