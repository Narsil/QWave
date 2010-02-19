#ifndef XMPPPOSTSIGNERRESPONSEACTOR_H
#define XMPPPOSTSIGNERRESPONSEACTOR_H

#include "xmppstanza.h"
#include "xmppactor.h"

class ServerCertificate;

class XmppPostSignerResponseActor : public XmppActor
{
public:
    XmppPostSignerResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    XmppStanza m_stanza;
};

#endif // XMPPPOSTSIGNERRESPONSEACTOR_H
