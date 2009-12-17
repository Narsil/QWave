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
    /**
      * Called when the local client creates a delta.
      * This function will apply the delta locally and queue it for sending to the server.
      */
    void handleSend( WaveletDelta& outgoing );
    /**
      * Called upon receipt of a delta from the server.
      */
    void handleReceive( const WaveletDelta& incoming );
    /**
      * After a sequence of handleReceive calls, this method is called to submit the resulting version and hash.
      */
    void setResultingHash(int version, const QByteArray& hash);

    /**
      * The number of deltas which have been applied locally but not applied by the server yet.
      * This can include at most one delta which has been submitted to the server but has not yet been applied.
      */
    int queuedDeltaCount() const { return m_outgoingDeltas.count(); }
    /**
      * @return true if a delta has been sent to the server but has not yet been applied by the server.
      */
    bool isSubmitPending() const { return m_submitPending; }

    /**
      * The latest version reported by the server. Note that the local copy of the document may have some deltas applied already which
      * are not applied by the server yet.
      */
    int serverVersion() const { return m_serverVersion; }

    /**
     * You can turn on and off the gathering of pending deltas with this.
     */
    void setGatheringDeltas(bool gather);
    void setSuspendSending(bool suspend);
    bool isSuspendSending() const { return m_suspendSending; }

public slots:
    /**
     * Consumed by the UI
     */
    void handleSendRemoveParticipant(const QString &address);

signals:
    /**
      * Connected to the wavelet.
      */
    void participantAdd( const QString& address );
    /**
      * Connected to the wavelet.
      */
    void participantRemove( const QString& address );
    /**
      * Connected to the wavelet.
      */
    void documentMutation( const QString& documentId, const DocumentMutation& mutation, const QString& author );

private:
    void setup();
    void submitNext();
    /**
     * Gathers pending operations while waiting for server acknowledgment.
     */
    void gatherOutgoingDeltas();

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
    bool m_suspendSending;
    bool m_gatherDeltas;
};

#endif // OTPROCESSOR_H
