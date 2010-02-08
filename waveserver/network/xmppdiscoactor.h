#ifndef XMPPDISCOACTOR_H
#define XMPPDISCOACTOR_H

#include "xmppactor.h"

class XmppDiscoActor : public XmppActor
{
public:
    XmppDiscoActor(XmppVirtualConnection* con);

protected:
    virtual void EXECUTE();

private:
    QString m_id;
};

#endif // XMPPDISCOACTOR_H
