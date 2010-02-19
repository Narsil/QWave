#ifndef CLIENTSUBMITREQUESTACTOR_H
#define CLIENTSUBMITREQUESTACTOR_H

#include "network/clientactor.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/waveurl.h"

class Wavelet;

class ClientSubmitRequestActor : public ClientActor
{
public:
    ClientSubmitRequestActor( ClientConnection* con, const QByteArray& data );

protected:
    void execute();

private:
    QByteArray m_data;
    Wavelet* m_wavelet;
    waveserver::ProtocolSubmitRequest m_update;
    WaveUrl m_url;
    qint64 m_id;
};

#endif // CLIENTSUBMITREQUESTACTOR_H
