#ifndef XMPPSUBMITRESPONSEACTOR_H
#define XMPPSUBMITRESPONSEACTOR_H

#include <QSharedPointer>
#include <QString>
#include "xmppstanza.h"
#include "xmppactor.h"
#include "model/signedwaveletdelta.h"

class Wavelet;
class AppliedWaveletDelta;

class XmppSubmitResponseActor : public XmppActor
{
public:
    XmppSubmitResponseActor(XmppVirtualConnection* con, const QSharedPointer<XmppStanza>& stanza);

protected:
    virtual void EXECUTE();

private:
    QSharedPointer<XmppStanza> m_stanza;
    Wavelet* m_wavelet;
    SignedWaveletDelta m_delta;
    const AppliedWaveletDelta* m_applied;
};

#endif // XMPPSUBMITRESPONSEACTOR_H
