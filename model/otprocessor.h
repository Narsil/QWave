#ifndef OTPROCESSOR_H
#define OTPROCESSOR_H

#include <QObject>
#include <QList>

#include "waveletdelta.h"

class Environment;
class DocumentMutation;
class Wavelet;
class Participant;

class OTProcessor : public QObject
{
    Q_OBJECT
public:
    /**
      * This constructor is used for OT processing of the wavelet digest.
      */
    OTProcessor(Environment* environment, QObject* parent = 0);
    OTProcessor(Wavelet* wavelet);

    void handleSendAddParticipant( Participant* p );
    void handleSend( const DocumentMutation& mutation, const QString& documentId );
    void handleSend( WaveletDelta& outgoing );
    void handleReceive( const WaveletDelta& incoming );

    void setResultingHash(int version, const QByteArray& hash);

signals:
    void participantAdd( const QString& address );
    void participantRemove( const QString& address );
    void documentMutation( const QString& documentId, const DocumentMutation& mutation );

private:
    void setup();
    void submitNext();

    int m_serverMsgCount;
    int m_clientMsgCount;
    QList<WaveletDelta> m_outgoingDeltas;
    Environment* m_environment;
    /**
      * May be null if used for the digest.
      */
    Wavelet* m_wavelet;
    bool m_submitPending;
    int m_serverVersion;
    QByteArray m_serverHash;
};

#endif // OTPROCESSOR_H
