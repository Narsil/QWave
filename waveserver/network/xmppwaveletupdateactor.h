#ifndef XMPPWAVELETUPDATEACTOR_H
#define XMPPWAVELETUPDATEACTOR_H

#include <QString>
#include "network/xmppactor.h"
#include "protocol/messages.pb.h"
#include "actor/pbmessage.h"

class XmppWaveletUpdateActor : public XmppActor
{
public:
    XmppWaveletUpdateActor(XmppVirtualConnection* con, PBMessage<messages::WaveletUpdate>* event);

protected:
    virtual void execute();

private:
    QString m_waveletName;
    QString m_base64;
    QString m_id;
};

#endif // XMPPWAVELETUPDATEACTOR_H
