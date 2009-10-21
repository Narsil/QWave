#include "synchronizeddocument.h"

SynchronizedDocument::SynchronizedDocument(QObject* parent)
        : StructuredDocument( parent )
{
     m_serverMsgCount = 0;
     m_clientMsgCount = 0;
}

void SynchronizedDocument::handleSend( DocumentMutation& incoming )
{
    incoming.setLocalVersion(m_clientMsgCount);
    incoming.setRemoteVersion(m_serverMsgCount);
    incoming.apply(this);    
    m_outgoingMutations.append(incoming);
    m_clientMsgCount++;
}

void SynchronizedDocument::handleReceive( const DocumentMutation& incoming )
{
    while( !m_outgoingMutations.isEmpty() )
    {
        const DocumentMutation& m = m_outgoingMutations.first();
        if ( m.localVersion() < incoming.remoteVersion() )
            m_outgoingMutations.removeFirst();
        else
            break;
    }

    Q_ASSERT( incoming.localVersion() == m_serverMsgCount );
    DocumentMutation msg(incoming);
    for( int i = 0; i < m_outgoingMutations.count(); ++i )
    {
        const DocumentMutation& m = m_outgoingMutations[i];
        msg = m.translate(msg);
        m_outgoingMutations[i] = msg.translate(m);
    }
    msg.apply( this );
    m_serverMsgCount++;
}

