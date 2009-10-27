#include "otprocessor.h"
#include "participant.h"
#include "app/environment.h"

OTProcessor::OTProcessor(Environment* environment, QObject* parent)
        : QObject( parent ), m_environment(environment)
{
     m_serverMsgCount = 0;
     m_clientMsgCount = 0;
}

void OTProcessor::handleSend( WaveletDelta& outgoing )
{
    // incoming.setLocalVersion(m_clientMsgCount);
    // incoming.setRemoteVersion(m_serverMsgCount);
    outgoing.version().version = m_serverMsgCount;
    // TODO: Hash
    outgoing.setAuthor(m_environment->localUser()->address());

    // Apply all document mutations locally
    foreach( WaveletDeltaOperation op, outgoing.operations() )
    {
        if ( op.hasMutation() )
            emit documentMutation(op.documentId(), *(op.mutation()));
    }
    // Remember that this message has been sent but not yet acked
    m_outgoingDeltas.append(outgoing);
    m_clientMsgCount++;
}

void OTProcessor::handleReceive( const WaveletDelta& incoming )
{
    // Throw away all delta that we have in common with the server
    while( !m_outgoingDeltas.isEmpty() )
    {
        const WaveletDelta& m = m_outgoingDeltas.first();
        if ( m.version().version < incoming.version().version )
            m_outgoingDeltas.removeFirst();
        else
            break;
    }

    // Transform the received delta and transform the operations which have not
    // yet been acknowledged by the server
    // Disabled because of a bug in the google code: Q_ASSERT( incoming.version().version == m_serverMsgCount );
    WaveletDelta msg(incoming);
    for( int i = 0; i < m_outgoingDeltas.count(); ++i )
    {
        WaveletDelta& m = m_outgoingDeltas[i];
        for( int c = 0; c < m.operations().count(); ++c )
        {
            for( int s = 0; s < msg.operations().count(); ++s )
            {
                WaveletDeltaOperation sop = msg.operations()[s].translate(m.operations()[c]);
                m.operations()[c] = m.operations()[c].translate(msg.operations()[s]);
                msg.operations()[s] = sop;
            }
        }
    }

    // Apply the received delta
    for( int s = 0; s < msg.operations().count(); ++s )
    {
        const WaveletDeltaOperation sop = msg.operations()[s];
        if ( sop.hasMutation() )
            emit documentMutation(sop.documentId(), *(sop.mutation()));
        if ( sop.hasAddParticipant() )
            emit participantAdd( sop.addParticipant() );
        if ( sop.hasRemoveParticipant() )
            emit participantRemove( sop.removeParticipant() );
    }
    m_serverMsgCount++;
}
