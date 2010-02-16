#ifndef XMPPWAVELETUPDATERESPONSEACTOR_H
#define XMPPWAVELETUPDATERESPONSEACTOR_H

#include <QSharedPointer>
#include <QList>
#include "network/xmppstanza.h"
#include "network/xmppactor.h"
#include "model/appliedwaveletdelta.h"
#include "model/signature.h"

class RemoteWavelet;
class ServerCertificate;

class XmppWaveletUpdateResponseActor : public XmppActor
{
public:
    XmppWaveletUpdateResponseActor(XmppVirtualConnection* con, const QSharedPointer<XmppStanza>& stanza);

protected:
    virtual void EXECUTE();

private:
    QSharedPointer<XmppStanza> m_stanza;
    RemoteWavelet* m_wavelet;
    int i;
//    int count;
//    const ServerCertificate* m_cert;
//    Signature m_signature;
    QString m_id;    
    QList<AppliedWaveletDelta> m_deltas;
    AppliedWaveletDelta m_delta;
};

#endif // XMPPWAVELETUPDATERESPONSE_H