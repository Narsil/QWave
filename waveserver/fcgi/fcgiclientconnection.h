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
    class ProtocolSubmitRequest;
}

/**
  * For each connected web browser (i.e. for each session) there is
  * a FCGIClientConnection which handles the session.
  */
class FCGIClientConnection : public ActorGroup
{
public:
    FCGIClientConnection(const QString& sessionId, const QString& participant, ClientActorFolk* parent);
    ~FCGIClientConnection();

    QString participant() const { return m_participant; }
    QString sessionId() const { return m_sessionId; }

    /**
      * @internal
      *
      * Called from ClientSubmitRequestActor.
      */
    void sendSubmitResponse( const waveserver::ProtocolSubmitResponse& response );
    /**
      * @internal
      *
      * Called from ClientSubmitRequestActor.
      */
    void sendFailedSubmitResponse( const QString& errorMessage );

protected:
    virtual void customEvent( QEvent* event );

private:
    /**
      * Handles requests received from the web browser.
      */
    void handleRequest( const PBMessage<webclient::Request>* request );
    /**
      * Called by handleRequest.
      */
    void openRequest( const waveserver::ProtocolOpenRequest* msg );
    /**
      * Called by handleRequest.
      */
    void submitRequest( const waveserver::ProtocolSubmitRequest* msg );
    /**
      * Sends or enqueue a message for the web browser.
      */
    void reply( PBMessage<webclient::Response>* response );
    /**
      * Sends or enqueues an error message for the web browser.
      */
    void errorReply( const std::string& msg );
    /**
      * Used to close a pending HTTP connection.
      */
    void emptyReply();

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
