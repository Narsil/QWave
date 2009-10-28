#include "otprocessor.h"
#include "participant.h"
#include "wavelet.h"
#include "app/environment.h"
#include "network/networkadapter.h"

OTProcessor::OTProcessor(Environment* environment, QObject* parent)
        : QObject( parent ), m_environment(environment), m_wavelet(0)
{
     m_serverMsgCount = 0;
     m_clientMsgCount = 0;
}

OTProcessor::OTProcessor(Wavelet* wavelet)
        : QObject( wavelet ), m_environment(wavelet->environment()), m_wavelet(wavelet)
{
     m_serverMsgCount = 0;
     m_clientMsgCount = 0;
}

void OTProcessor::handleSend( const DocumentMutation& mutation, const QString& documentId )
{
    WaveletDelta delta;
    WaveletDeltaOperation op;
    op.setMutation(mutation);
    op.setDocumentId(documentId);
    delta.addOperation(op);
    handleSend(delta);
}

void OTProcessor::handleSend( WaveletDelta& outgoing )
{
    Q_ASSERT( m_wavelet != 0 );
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

    // Send it to the server via the network
    m_environment->networkAdapter()->sendDelta(outgoing, m_wavelet);
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
