#ifndef FCGICLIENTCONNECTION_H
#define FCGICLIENTCONNECTION_H

#include <QQueue>
#include "actor/actorgroup.h"
#include "actor/pbmessage.h"
#include "actor/actorid.h"
#include "protocol/webclient.pb.h"

class ClientActorFolk;

namespace waveserver
{
    class ProtocolOpenRequest;
}

class FCGIClientConnection : public ActorGroup
{
public:
    FCGIClientConnection(const QString& sessionId, const QString& participant, ClientActorFolk* parent);
    ~FCGIClientConnection();

    QString participant() const { return m_participant; }
    QString sessionId() const { return m_sessionId; }

protected:
    virtual void customEvent( QEvent* event );

private:
    void handleRequest( const PBMessage<webclient::Request>* request );
    void openRequest( const waveserver::ProtocolOpenRequest* msg );
    void reply( PBMessage<webclient::Response>* response );
    void errorReply( const std::string& msg );

    QString m_participant;
    QString m_sessionId;
    bool m_waveletsOpened;
    qint64 m_clientSequenceNumber;
    qint64 m_serverSequenceNumber;
    qint64 m_ackedSequenceNumber;
    /**
      * Messages which must be sent to the web browser when possible.
      */
    QQueue<PBMessage<webclient::Response>*> m_outQueue;
    ActorId m_pendingRequest;    
};

#endif // FCGICLIENTCONNECTION_H
