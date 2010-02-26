#ifndef REMOTEWAVELET_H
#define REMOTEWAVELET_H

#include <QSet>
#include <QString>
#include <QByteArray>

#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"
#include "model/wavelet.h"
#include "model/appliedwaveletdelta.h"

class RemoteWavelet : public Wavelet
{
public:
    RemoteWavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );

    virtual bool isRemote() const;
    virtual bool isLocal() const;

protected:
    virtual void customEvent( QEvent* event );
    virtual void onAddParticipant( const JID& jid );
    virtual void onRemoveParticipant( const JID& jid );

private:
    class WaveletUpdateActor : public WaveletActor
    {
    public:
        WaveletUpdateActor( RemoteWavelet* wavelet, PBMessage<messages::RemoteWaveletUpdate>* message );

    protected:
        void execute();

    private:
        inline RemoteWavelet* wavelet() { return static_cast<RemoteWavelet*>( m_wavelet ); }

        PBMessage<messages::RemoteWaveletUpdate> m_message;
        QByteArray m_binary;
        AppliedWaveletDelta m_appliedDelta;
        qint64 m_msgId;
        QSet<QString> m_addLocalUser;
        QSet<QString> m_removeLocalUser;
    };

};

#endif // REMOTEWAVELET_H
