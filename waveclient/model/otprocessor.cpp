#include "otprocessor.h"
#include "participant.h"
#include "wavelet.h"
#include "app/environment.h"
#include "network/networkadapter.h"

#include <QDebug>

OTProcessor::OTProcessor(Environment* environment, QObject* parent)
        : QObject( parent ), m_environment(environment), m_wavelet(0), m_gatherDeltas(true)
{
     setup();
}

OTProcessor::OTProcessor(Wavelet* wavelet)
        : QObject( wavelet ), m_environment(wavelet->environment()), m_wavelet(wavelet), m_gatherDeltas(true)
{
     setup();
}

void OTProcessor::setup()
{
    m_suspendSending = false;
    m_submitPending = false;
    m_serverMsgCount = 0;
    m_clientMsgCount = 0;
    m_serverVersion = 0;
    if ( m_wavelet )
        m_serverHash = m_wavelet->url().toString().toAscii();
}

void OTProcessor::setResultingHash(int version, const QByteArray& hash)
{
    if ( version <= m_serverVersion )
    {
        qDebug("Oooops, got an old version");
        return;
    }
    qDebug("Got resulting version %i", version);
    m_serverVersion = version;
    m_serverHash = hash;

    if ( m_outgoingDeltas.length() > 0 && !m_submitPending )
        submitNext();
}

void OTProcessor::submitNext()
{
    // TODO: This is a strange way of handling offline scenarios
    if ( !m_environment->networkAdapter()->isOnline() )
        return;
    Q_ASSERT( !m_submitPending );
    if (m_outgoingDeltas.length() == 0)
        return;
    if (m_suspendSending )
        return;

    m_submitPending = true;
    m_outgoingDeltas[0].version().version = m_serverVersion;
    m_outgoingDeltas[0].version().hash = m_serverHash;
    m_environment->networkAdapter()->submit(m_outgoingDeltas[0], m_wavelet);
}

void OTProcessor::handleSendAddParticipant( Participant* p )
{
    Q_ASSERT(m_wavelet != 0);
    WaveletDelta delta;
    WaveletDeltaOperation op;
    op.setAddParticipant(p->address());
    delta.addOperation(op);
    handleSend(delta);
}

void OTProcessor::handleSendRemoveParticipant(const QString& address)
{
    Q_ASSERT(m_wavelet !=0);
    WaveletDelta delta;
    WaveletDeltaOperation op;
    op.setRemoveParticipant(address);
    delta.addOperation(op);
    handleSend(delta);
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
    outgoing.setAuthor(m_environment->localUser()->address());

    // Apply all document mutations locally
    foreach( WaveletDeltaOperation op, outgoing.operations() )
    {
        if ( op.hasMutation() )
            emit documentMutation(op.documentId(), op.mutation(), outgoing.author());
        if ( op.hasAddParticipant() )
            emit participantAdd( op.addParticipant() );
        if ( op.hasRemoveParticipant() )
            emit participantRemove( op.removeParticipant() );
    }
    // Remember that this message has been sent but not yet acked
    m_outgoingDeltas.append(outgoing);
    m_clientMsgCount++;

    // Send it to the server via the network
    if ( !m_submitPending && !m_suspendSending )
        submitNext();
    else if(m_gatherDeltas)
    	gatherOutgoingDeltas();
}

void OTProcessor::gatherOutgoingDeltas()
{
    // Nothing to gather here
    if (m_outgoingDeltas.size() <= 1 )
        return;
    // First element is waiting for acknowledgment so
    // we gather the two last deltas. The number of deltas should not
    // go above 3 as when a third delta is given it is automatically gathered
    if (m_outgoingDeltas.size() <= 2 && m_submitPending )
        return;
    else if (m_outgoingDeltas.size()>3)
        qDebug("Ooops we shouldn't have accumulated more than 3 deltas");
    WaveletDelta lastDelta = m_outgoingDeltas.takeLast();
    WaveletDelta remainingDelta = m_outgoingDeltas.takeLast();
    foreach( WaveletDeltaOperation op, lastDelta.operations() )
    {
        remainingDelta.addOperation(op);
    }
    m_clientMsgCount--;
    m_outgoingDeltas.append(remainingDelta);
}

void OTProcessor::handleReceive( const WaveletDelta& incoming )
{
    // Check whether this delta actually acks a delta we have sent    
    if ( m_submitPending && incoming.author() == m_environment->localUser()->address() )
    {
        qDebug("Got ACK for my submit");
        m_outgoingDeltas.removeFirst();
        m_submitPending = false;
        return;
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
                bool ok;
                QPair<WaveletDeltaOperation,WaveletDeltaOperation> pair = WaveletDeltaOperation::xform(msg.operations()[s], m.operations()[c], &ok);
                if ( !ok )
                {
                    qDebug("This wave is blown up");
                    // TODO: Good error handling
                    Q_ASSERT(false);
                    return;
                }
                msg.operations()[s] = pair.first;
                m.operations()[c] = pair.second;
            }
        }
    }

    // Apply the received delta
    for( int s = 0; s < msg.operations().count(); ++s )
    {
        const WaveletDeltaOperation sop = msg.operations()[s];
        if ( sop.hasMutation() )
            emit documentMutation(sop.documentId(), sop.mutation(), msg.author());
        if ( sop.hasAddParticipant() )
            emit participantAdd( sop.addParticipant() );
        if ( sop.hasRemoveParticipant() )
            emit participantRemove( sop.removeParticipant() );
    }
    m_serverMsgCount++;
}

void OTProcessor::setGatheringDeltas(bool gather){
	if ( m_gatherDeltas == gather )
		return;
	m_gatherDeltas = gather;
	qDebug()<<"Changed gathering deltas flag to "<<m_gatherDeltas;
}

void OTProcessor::setSuspendSending(bool suspend)
{
    if ( m_suspendSending == suspend )
        return;
    m_suspendSending = suspend;
    if ( !m_suspendSending && !m_submitPending )
        submitNext();
}
