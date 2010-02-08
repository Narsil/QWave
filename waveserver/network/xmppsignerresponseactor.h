#ifndef XMPPSIGNERRESPONSEACTOR_H
#define XMPPSIGNERRESPONSEACTOR_H

#include <QSharedPointer>
#include "xmppstanza.h"
#include "xmppactor.h"

class ServerCertificate;

class XmppSignerResponseActor : public XmppActor
{
public:
    XmppSignerResponseActor(XmppVirtualConnection* con, const QSharedPointer<XmppStanza>& stanza);

protected:
    virtual void EXECUTE();

private:
    QSharedPointer<XmppStanza> m_stanza;
    const ServerCertificate* m_cert;
};

#endif // XMPPSIGNERRESPONSEACTOR_H
