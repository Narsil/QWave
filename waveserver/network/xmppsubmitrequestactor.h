#ifndef XMPPSUBMITREQUESTACTOR_H
#define XMPPSUBMITREQUESTACTOR_H

#include <QSharedPointer>
#include <QByteArray>

#include "network/xmppactor.h"
#include "model/signedwaveletdelta.h"
#include "model/waveurl.h"
#include "actor/pbmessage.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"

class XmppSubmitRequestActor : public XmppActor
{
public:
    XmppSubmitRequestActor(XmppVirtualConnection* con, PBMessage<waveserver::ProtocolSubmitRequest>* message );

protected:
    virtual void execute();

private:
    void sendErrorResponse();

    PBMessage<waveserver::ProtocolSubmitRequest> m_message;
    WaveUrl m_url;
    SignedWaveletDelta m_delta;
    QString m_id;
    qint64 m_applicationTime;
    int m_operationsApplied;
    qint64 m_version;
    QByteArray m_hash;
};

#endif // XMPPSUBMITREQUESTACTOR_H
