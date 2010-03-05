#include "clientindexwaveactor.h"
#include "network/clientconnection.h"
#include "fcgi/fcgiclientconnection.h"
#include "actor/pbmessage.h"
#include "actor/recvpb.h"
#include "actor/timeout.h"
#include "model/wavefolk.h"

// TODO: Better error management
#define CLIENTERROR(msg) { logErr(msg, __FILE__, __LINE__); TERMINATE(); }
#define CLIENTLOG(msg) { log(msg, __FILE__, __LINE__); }

ClientIndexWaveActor::ClientIndexWaveActor(ClientConnection* con)
        : ClientActor(con)
{
}

ClientIndexWaveActor::ClientIndexWaveActor(FCGIClientConnection* con)
        : ClientActor(con)
{
}

void ClientIndexWaveActor::execute()
{
    qDebug("EXECUTE ClientIndexWaveActor");

    BEGIN_EXECUTE;

    // Ask the store for all wavelets in which this user participates
    {
        m_msgId = nextId();
        PBMessage<messages::QueryParticipantWavelets>* msg = new PBMessage<messages::QueryParticipantWavelets>( ActorId("store", participant()), m_msgId );
        msg->setCreateOnDemand( true );
        msg->set_participant( participant().toStdString() );
        bool ok = post( msg );
        if ( !ok ) { CLIENTERROR("Could not talk to store"); }
    }

    // Wait for the database
    yield( RecvPB<messages::QueryParticipantWaveletsResponse>() | Timeout(10000) );
    if ( REASON( RecvPB<messages::QueryParticipantWaveletsResponse> ) )
    {
        CLIENTLOG("Got query response");
        m_response.MergeFrom( *REASON );
    }
    else if ( REASON( Timeout ) ) { CLIENTERROR("Timeout waiting for response from store"); }

    // Subscribe to all wavelets
    for( i = 0; i < m_response.wavelet_name_size(); ++i )
    {
        WaveUrl url( QString::fromStdString( m_response.wavelet_name(i) ) );
        CLIENTLOG("Subscribing to " + url.toString() );
        // Subscribe to this wavelet
        PBMessage<messages::SubscribeWavelet>* subscribe = new PBMessage<messages::SubscribeWavelet>( WaveFolk::actorId( url ) );
        subscribe->setCreateOnDemand( true );
        subscribe->set_index( true );
        subscribe->set_content( false );
        subscribe->set_subscribe( true );
        subscribe->set_actor_id( connection()->actorId().toString().toStdString() );
        bool ok = post( subscribe );
        if ( !ok ) { CLIENTERROR("Could not subscribe to wavelet"); }
    }

    qDebug("FINISHED ClientIndexWaveActor");

    END_EXECUTE;
}
