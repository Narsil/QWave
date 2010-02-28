#ifndef XMPPHISTORYRESPONSEACTOR_H
#define XMPPHISTORYRESPONSEACTOR_H

#include "network/xmppstanza.h"
#include "network/xmppactor.h"
#include "model/waveurl.h"

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
    WaveUrl m_url;
    qint64 m_msgId;
    int i;
};

#endif // XMPPHISTORYRESPONSEACTOR_H
