#ifndef XMPPPOSTSIGNERRESPONSEACTOR_H
#define XMPPPOSTSIGNERRESPONSEACTOR_H

#include <QSharedPointer>
#include "xmppstanza.h"
#include "xmppactor.h"

class ServerCertificate;

class XmppPostSignerResponseActor : public XmppActor
{
public:
    XmppPostSignerResponseActor(XmppVirtualConnection* con, const QSharedPointer<XmppStanza>& stanza);

protected:
    virtual void EXECUTE();

private:
    QSharedPointer<XmppStanza> m_stanza;
};

#endif // XMPPPOSTSIGNERRESPONSEACTOR_H
