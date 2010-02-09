#ifndef REMOTEWAVELET_H
#define REMOTEWAVELET_H

#include "wavelet.h"

class RemoteWavelet : public Wavelet
{
public:
    RemoteWavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );

    /**
      * @return true on success
      */
    bool apply( AppliedWaveletDelta& appliedDelta, QString* errorMessage );

    virtual bool isRemote() const;
    virtual bool isLocal() const;
};

#endif // REMOTEWAVELET_H
