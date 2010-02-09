#ifndef XMPPSUBMITREQUESTACTOR_H
#define XMPPSUBMITREQUESTACTOR_H

#include "network/xmppactor.h"
#include "model/signedwaveletdelta.h"
#include "model/waveurl.h"

namespace protocol
{
    class ProtocolWaveletDelta;
}

class XmppSubmitRequestActor : public XmppActor
{
public:
    XmppSubmitRequestActor(XmppVirtualConnection* con, const WaveUrl& url, const protocol::ProtocolWaveletDelta& delta);

protected:
    virtual void EXECUTE();

private:
    WaveUrl m_url;
    SignedWaveletDelta m_delta;
    QString m_id;
};

#endif // XMPPSUBMITREQUESTACTOR_H
