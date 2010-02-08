#ifndef XMPPWAVELETUPDATEACTOR_H
#define XMPPWAVELETUPDATEACTOR_H

#include <QString>
#include "xmppactor.h"

class AppliedWaveletDelta;

class XmppWaveletUpdateActor : public XmppActor
{
public:
    XmppWaveletUpdateActor(XmppVirtualConnection* con, const QString& waveletName, const AppliedWaveletDelta& waveletDelta);

protected:
    virtual void EXECUTE();

private:
    QString m_waveletName;
    QString m_base64;
    QString m_id;
};

#endif // XMPPWAVELETUPDATEACTOR_H
