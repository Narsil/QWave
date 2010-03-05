#ifndef CLIENTSUBMITREQUESTACTOR_H
#define CLIENTSUBMITREQUESTACTOR_H

#include "network/clientactor.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/waveurl.h"

//class Wavelet;
class ClientConnection;
class FCGIClientConnection;

class ClientSubmitRequestActor : public ClientActor
{
public:
    ClientSubmitRequestActor( ClientConnection* con, const QByteArray& data );
    ClientSubmitRequestActor( FCGIClientConnection* con, const waveserver::ProtocolSubmitRequest& request );

protected:
    void execute();

private:
    QByteArray m_data;
    bool m_needsParsing;
    waveserver::ProtocolSubmitRequest m_update;
    WaveUrl m_url;
    qint64 m_id;
};

#endif // CLIENTSUBMITREQUESTACTOR_H
