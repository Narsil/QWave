#ifndef CLIENTINDEXWAVEACTOR_H
#define CLIENTINDEXWAVEACTOR_H

#include "network/clientconnection.h"
#include "network/clientactor.h"
#include "protocol/common.pb.h"
#include "model/waveurl.h"
#include "protocol/messages.pb.h"

/**
  * Sends the initial index wave to a client.
  */
class ClientIndexWaveActor : public ClientActor
{
public:
    ClientIndexWaveActor( ClientConnection* con );

protected:
    void execute();

private:
    qint64 m_msgId;
    messages::QueryParticipantWaveletsResponse m_response;
    int i;
};

#endif // CLIENTINDEXWAVEACTOR_H
