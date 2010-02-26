#ifndef LOCALWAVELET_H
#define LOCALWAVELET_H

#include <QHash>
#include <QSet>
#include <QString>

#include "wavelet.h"
#include "actor/actor.h"
#include "actor/actorid.h"
#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"
#include "model/appliedwaveletdelta.h"

/**
  * Hosts a local wavelet.
  */
class LocalWavelet : public Wavelet
{
public:
    LocalWavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );

    virtual bool isRemote() const;
    virtual bool isLocal() const;

protected:
    virtual void customEvent( QEvent* event );
    virtual void onAddParticipant( const JID& jid );
    virtual void onRemoveParticipant( const JID& jid );

private:

//    /**
//      * Initializes the wavelet from the store.
//      */
//    class InitActor : public WaveletActor
//    {
//    public:
//        InitActor( LocalWavelet* wavelet);
//
//    protected:
//        void execute();
//
//    private:
//        inline LocalWavelet* wavelet() { return static_cast<LocalWavelet*>( m_wavelet ); }
//
//        qint64 m_msgId;
//    };

    /**
      * Submits a new delta to the wavelet, stores it in the database, and sends out wavelet updates, digest updates etc.
      */
    class SubmitRequestActor : public WaveletActor
    {
    public:
        SubmitRequestActor( LocalWavelet* wavelet, PBMessage<messages::LocalSubmitRequest>* message );

    protected:
        void execute();

    private:
        inline LocalWavelet* wavelet() { return static_cast<LocalWavelet*>( m_wavelet ); }

        qint64 timeStamp();
        void sendFailedSubmitResponse(const QString& err);

        PBMessage<messages::LocalSubmitRequest> m_message;
//        SignedWaveletDelta m_signedDelta;
//        WaveletDelta m_delta;
//        bool m_transformed;
        /**
          * The binary represenation of the applied wavelet.
          */
        QByteArray m_binary;
//        int m_operationsApplied;
//        qint64 m_applicationTime;
//        QByteArray m_resultingHash;
//        qint64 m_resultingVersion;
        AppliedWaveletDelta m_appliedDelta;

        qint64 m_msgId;
        QSet<QString> m_addLocalUser;
        QSet<QString> m_removeLocalUser;
    };

    void subscribeRemote( const JID& remoteJid );
    void unsubscribeRemote( const JID& remoteJid );

    /**
      * A set of all remote wave domains which need updates of this wave.
      * The integer is the number of participants in this domain who are participating in this wave.
      */
    QHash<QString,int> m_remoteSubscribers;
};

#endif // LOCALWAVELET_H
