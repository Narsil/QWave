#ifndef XMPPSUBMITRESPONSEACTOR_H
#define XMPPSUBMITRESPONSEACTOR_H

#include <QString>
#include "xmppstanza.h"
#include "xmppactor.h"
#include "model/signedwaveletdelta.h"

class LocalWavelet;
class AppliedWaveletDelta;

class XmppSubmitResponseActor : public XmppActor
{
public:
    XmppSubmitResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    XmppStanza m_stanza;
    LocalWavelet* m_wavelet;
    SignedWaveletDelta m_delta;
    const AppliedWaveletDelta* m_applied;
};

#endif // XMPPSUBMITRESPONSEACTOR_H
