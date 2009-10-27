#ifndef OTPROCESSOR_H
#define OTPROCESSOR_H

#include <QObject>
#include <QList>

#include "waveletdelta.h"

class Environment;
class DocumentMutation;

class OTProcessor : public QObject
{
    Q_OBJECT
public:
    OTProcessor(Environment* environment, QObject* parent = 0);

    void handleSend( WaveletDelta& outgoing );
    void handleReceive( const WaveletDelta& incoming );

signals:
    void participantAdd( const QString& address );
    void participantRemove( const QString& address );
    void documentMutation( const QString& documentId, const DocumentMutation& mutation );

private:
    int m_serverMsgCount;
    int m_clientMsgCount;
    QList<WaveletDelta> m_outgoingDeltas;
    Environment* m_environment;
};

#endif // OTPROCESSOR_H
