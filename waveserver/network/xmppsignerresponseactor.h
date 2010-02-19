#ifndef XMPPSIGNERRESPONSEACTOR_H
#define XMPPSIGNERRESPONSEACTOR_H

#include "xmppstanza.h"
#include "xmppactor.h"

class ServerCertificate;

class XmppSignerResponseActor : public XmppActor
{
public:
    XmppSignerResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    XmppStanza m_stanza;
    const ServerCertificate* m_cert;
};

#endif // XMPPSIGNERRESPONSEACTOR_H
