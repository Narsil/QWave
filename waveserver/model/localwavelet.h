#ifndef LOCALWAVELET_H
#define LOCALWAVELET_H

#include <QSharedPointer>

#include "wavelet.h"
#include "actor/actor.h"
#include "actor/actorid.h"
#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"

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
    int apply( const SignedWaveletDelta& clientDelta, QString* errorMessage, int operationsApplied = -1, qint64 applicationTime = -1 );

    virtual bool isRemote() const;
    virtual bool isLocal() const;

protected:
    virtual void dispatch( const QSharedPointer<IMessage>& message );

private:
    class WaveletActor : public Actor
    {
    public:
        WaveletActor( LocalWavelet* wavelet ) : m_wavelet( wavelet ) { }

       virtual const ActorId& actorId() const { return m_actorId; }

   protected:
       void log( const char* error, const char* file, int line );
       void log( const QString& error, const char* file, int line );
       void logErr( const char* error, const char* file, int line );
       void logErr( const QString& error, const char* file, int line );

   private:
       LocalWavelet* m_wavelet;
       ActorId m_actorId;
    };

    class SubmitRequestActor : public WaveletActor
    {
    public:
        SubmitRequestActor( LocalWavelet* wavelet, const QSharedPointer<PBMessage<messages::LocalSubmitRequest> >& message ) : WaveletActor( wavelet ), m_message( message ) { wavelet->addActor( this ); }

    protected:
        void EXECUTE();

    private:
        qint64 timeStamp();
        void sendFailedSubmitResponse(const QString& err);

        QSharedPointer<PBMessage<messages::LocalSubmitRequest> > m_message;
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
