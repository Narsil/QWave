#ifndef SYNCHRONIZEDDOCUMENT_H
#define SYNCHRONIZEDDOCUMENT_H

#include <QList>

#include "waveletdelta.h"
#include "structureddocument.h"

class Environment;

class SynchronizedDocument : public StructuredDocument
{
    Q_OBJECT
public:
    SynchronizedDocument(Environment* environment, QObject* parent = 0);

    void handleSend( WaveletDelta& outgoing );
    void handleReceive( const WaveletDelta& incoming );

private:
    int m_serverMsgCount;
    int m_clientMsgCount;
    QList<WaveletDelta> m_outgoingDeltas;
    Environment* m_environment;
};

#endif // SYNCHRONIZEDDOCUMENT_H
