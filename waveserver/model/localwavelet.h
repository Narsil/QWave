#ifndef LOCALWAVELET_H
#define LOCALWAVELET_H

#include <QSharedPointer>

#include "wavelet.h"
#include "actor/actor.h"
#include "actor/actorid.h"
#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"
#include "model/signedwaveletdelta.h"

class LocalWavelet : public Wavelet
{
public:
    LocalWavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );

    /**
      * @return operations_applied or -1 on error.
      */
    int apply( const protocol::ProtocolSignedDelta& protobufDelta, QString* errorMessage, int operationsApplied = -1, qint64 applicationTime = -1 );
    /**
      * @return operations_applied or -1 on error.
      */
    int apply( const SignedWaveletDelta& signedDelta, QString* errorMessage, int operationsApplied = -1, qint64 applicationTime = -1 );

    virtual bool isRemote() const;
    virtual bool isLocal() const;

protected:
    virtual void customEvent( QEvent* event );

private:
    class WaveletActor : public Actor
    {
    public:
       WaveletActor( LocalWavelet* wavelet );

   protected:
       void log( const char* error, const char* file, int line );
       void log( const QString& error, const char* file, int line );
       void logErr( const char* error, const char* file, int line );
       void logErr( const QString& error, const char* file, int line );

       LocalWavelet* m_wavelet;

    private:
       static qint64 s_id;
    };

    class SubmitRequestActor : public WaveletActor
    {
    public:
        SubmitRequestActor( LocalWavelet* wavelet, PBMessage<messages::LocalSubmitRequest>* message ) : WaveletActor( wavelet ), m_message( *message ) { }

    protected:
        void execute();

    private:
        qint64 timeStamp();
        void sendFailedSubmitResponse(const QString& err);

        PBMessage<messages::LocalSubmitRequest> m_message;
        SignedWaveletDelta m_signedDelta;
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
