#ifndef XMPPDISCORESPONSEACTOR_H
#define XMPPDISCORESPONSEACTOR_H

#include <QString>
#include "xmppstanza.h"
#include "xmppactor.h"

class XmppDiscoResponseActor : public XmppActor
{
public:
    XmppDiscoResponseActor(XmppVirtualConnection* con, const QString& id, XmppStanza::Kind kind);

protected:
    virtual void execute();

private:
    QString m_id;
    XmppStanza::Kind m_kind;
};

#endif // XMPPDISCORESPONSEACTOR_H
