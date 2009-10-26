#include "synchronizeddocument.h"
#include "participant.h"
#include "app/environment.h"

SynchronizedDocument::SynchronizedDocument(Environment* environment, QObject* parent)
        : StructuredDocument( parent ), m_environment(environment)
{
     m_serverMsgCount = 0;
     m_clientMsgCount = 0;
}

void SynchronizedDocument::handleSend( WaveletDelta& outgoing )
{
    // incoming.setLocalVersion(m_clientMsgCount);
    // incoming.setRemoteVersion(m_serverMsgCount);
    outgoing.version().version = m_serverMsgCount;
    // TODO: Hash
    outgoing.setAuthor(m_environment->localUser()->address());
    // TODO: Support multiple docs
    outgoing.setDocumentId("main");
    if ( outgoing.hasMutation() )
        outgoing.mutation()->apply(this);
    m_outgoingDeltas.append(outgoing);
    m_clientMsgCount++;
}

void SynchronizedDocument::handleReceive( const WaveletDelta& incoming )
{
    while( !m_outgoingDeltas.isEmpty() )
    {
        const WaveletDelta& m = m_outgoingDeltas.first();
        if ( m.version().version < incoming.version().version )
            m_outgoingDeltas.removeFirst();
        else
            break;
    }

    Q_ASSERT( incoming.version().version == m_serverMsgCount );
    WaveletDelta msg(incoming);
    for( int i = 0; i < m_outgoingDeltas.count(); ++i )
    {
        const WaveletDelta& m = m_outgoingDeltas[i];
        msg = m.translate(msg);
        m_outgoingDeltas[i] = msg.translate(m);
    }
    if ( msg.mutation() )
        msg.mutation()->apply( this );
    m_serverMsgCount++;
}

