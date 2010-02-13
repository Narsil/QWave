#ifndef XMPPSUBMITREQUESTACTOR_H
#define XMPPSUBMITREQUESTACTOR_H

#include <QSharedPointer>
#include <QByteArray>

#include "network/xmppactor.h"
#include "model/signedwaveletdelta.h"
#include "model/waveurl.h"
#include "actor/pbmessage.h"

namespace waveserver
{
    class ProtocolSubmitRequest;
}

class XmppSubmitRequestActor : public XmppActor
{
public:
    XmppSubmitRequestActor(XmppVirtualConnection* con, const QSharedPointer<PBMessage<waveserver::ProtocolSubmitRequest> >& message );

protected:
    virtual void EXECUTE();

private:
    void sendErrorResponse();

    QSharedPointer<PBMessage<waveserver::ProtocolSubmitRequest> > m_message;
    WaveUrl m_url;
    SignedWaveletDelta m_delta;
    QString m_id;
    qint64 m_applicationTime;
    int m_operationsApplied;
    qint64 m_version;
    QByteArray m_hash;
};

#endif // XMPPSUBMITREQUESTACTOR_H
