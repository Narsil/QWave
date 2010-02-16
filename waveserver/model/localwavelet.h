#ifndef LOCALWAVELET_H
#define LOCALWAVELET_H

#include "wavelet.h"

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

private:
    void subscribeRemote( const JID& remoteJid );
    void unsubscribeRemote( const JID& remoteJid );

    /**
      * A set of all remote wave domains which need updates of this wave.
      * The integer is the number of participants in this domain who are participating in this wave.
      */
    QHash<QString,int> m_remoteSubscribers;
};

#endif // LOCALWAVELET_H
