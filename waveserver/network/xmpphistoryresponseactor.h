#ifndef XMPPHISTORYRESPONSEACTOR_H
#define XMPPHISTORYRESPONSEACTOR_H

#include "xmppstanza.h"
#include "xmppactor.h"

class Wavelet;

class XmppHistoryResponseActor : public XmppActor
{
public:
    XmppHistoryResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    XmppStanza m_stanza;
    qint64 m_start;
    qint64 m_end;
    Wavelet* m_wavelet;
};

#endif // XMPPHISTORYRESPONSEACTOR_H
