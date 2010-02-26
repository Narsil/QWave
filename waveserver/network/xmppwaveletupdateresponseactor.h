#ifndef XMPPWAVELETUPDATERESPONSEACTOR_H
#define XMPPWAVELETUPDATERESPONSEACTOR_H

#include <QList>
#include "network/xmppstanza.h"
#include "network/xmppactor.h"
#include "model/appliedwaveletdelta.h"
#include "model/signature.h"
#include "model/waveurl.h"

class RemoteWavelet;
class ServerCertificate;

class XmppWaveletUpdateResponseActor : public XmppActor
{
public:
    XmppWaveletUpdateResponseActor(XmppVirtualConnection* con, XmppStanza* stanza);

protected:
    virtual void execute();

private:
    XmppStanza m_stanza;
    RemoteWavelet* m_wavelet;
    int i;
//    int count;
//    const ServerCertificate* m_cert;
//    Signature m_signature;
    QString m_id;    
    QList<AppliedWaveletDelta> m_deltas;
    AppliedWaveletDelta m_delta;
    WaveUrl m_url;
};

#endif // XMPPWAVELETUPDATERESPONSE_H
