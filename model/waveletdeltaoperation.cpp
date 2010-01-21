#include "waveletdeltaoperation.h"

WaveletDeltaOperation::WaveletDeltaOperation()
{
}

WaveletDeltaOperation::WaveletDeltaOperation(const WaveletDeltaOperation& delta)
        : m_documentId(delta.m_documentId),
          m_mutation(delta.m_mutation),
          m_addParticipant(delta.m_addParticipant),
          m_removeParticipant(delta.m_removeParticipant)
{
}

void WaveletDeltaOperation::setMutation( const DocumentMutation& mutation )
{
    m_mutation = mutation;
}

bool WaveletDeltaOperation::isNull() const
{
    return m_mutation.isEmpty() && m_addParticipant.isEmpty() && m_removeParticipant.isEmpty();
}

QPair<WaveletDeltaOperation,WaveletDeltaOperation> WaveletDeltaOperation::xform( const WaveletDeltaOperation& o1, const WaveletDeltaOperation& o2, bool* ok )
{
    WaveletDeltaOperation r1;
    r1.setDocumentId( o1.documentId() );
    WaveletDeltaOperation r2;
    r2.setDocumentId( o2.documentId() );
    *ok = true;

    if ( o1.hasAddParticipant() )
    {
        if ( !o2.hasAddParticipant() || o2.addParticipant() != o1.addParticipant() )
            r1.setAddParticipant( o1.addParticipant() );
    }
    if ( o2.hasAddParticipant() )
    {
        if ( !o1.hasAddParticipant() || o1.addParticipant() != o2.addParticipant() )
            r2.setAddParticipant( o2.addParticipant() );
    }
    if ( o1.hasRemoveParticipant() )
    {
        if ( !o2.hasRemoveParticipant() || o2.removeParticipant() != o1.removeParticipant() )
            r1.setRemoveParticipant( o1.removeParticipant() );
    }
    if ( o2.hasRemoveParticipant() )
    {
        if ( !o1.hasRemoveParticipant() || o1.removeParticipant() != o2.removeParticipant() )
            r2.setRemoveParticipant( o2.removeParticipant() );
    }
    if ( o1.hasMutation() && !o2.hasMutation() )
        r1.setMutation(o1.mutation());
    else if ( o2.hasMutation() && !o1.hasMutation() )
        r2.setMutation(o2.mutation());
    else if ( o1.hasMutation() && o2.hasMutation() )
    {
        if ( o1.documentId() == o2.documentId() )
        {
            QPair<DocumentMutation,DocumentMutation> pair = DocumentMutation::xform( o1.mutation(), o2.mutation(), ok );
            r1.setMutation(pair.first);
            r2.setMutation(pair.second);
        }
        else
        {
            r1.setMutation( o1.mutation() );
            r2.setMutation( o2.mutation() );
        }
    }

    return QPair<WaveletDeltaOperation,WaveletDeltaOperation>(r1,r2);
}
