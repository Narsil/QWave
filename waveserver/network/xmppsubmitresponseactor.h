#ifndef XMPPSUBMITRESPONSEACTOR_H
#define XMPPSUBMITRESPONSEACTOR_H

#include <QString>
#include "network/xmppstanza.h"
#include "network/xmppactor.h"
#include "protocol/common.pb.h"
#include "model/waveurl.h"

class LocalWavelet;
class AppliedWaveletDelta;

/**
  * Handles an incoming submit-request and forwards it to a LocalWavelet instance.
  * In addition it checks and verifies the signatures for the submitted delta.
  */
class XmppSubmitResponseActor : public XmppActor
{
public:
    XmppSubmitResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    qint64 m_id;
    XmppStanza m_stanza;
    WaveUrl m_url;
    protocol::ProtocolSignedDelta m_signedDelta;
};

#endif // XMPPSUBMITRESPONSEACTOR_H
