#include "network/clientsubmitrequestactor.h"
#include "network/clientconnection.h"
#include "network/converter.h"
#include "network/xmppcomponentconnection.h"
#include "network/xmppvirtualconnection.h"
#include "model/waveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/waveurl.h"
#include "model/localwavelet.h"
#include "model/remotewavelet.h"
#include "model/participant.h"
#include "app/settings.h"
#include "actor/pbmessage.h"
#include "actor/timeout.h"
#include "actor/recvpb.h"
#include "protocol/messages.pb.h"
#include <QByteArray>

#define CLIENTERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->sendFailedSubmitResponse(msg); TERMINATE(); }
#define CLIENTLOG(msg) { log(msg, __FILE__, __LINE__); }

ClientSubmitRequestActor::ClientSubmitRequestActor( ClientConnection* con, const QByteArray& data )
        : ClientActor( con ), m_data(data)
{
}

void ClientSubmitRequestActor::execute()
{
    qDebug("EXECUTE SubmitRequestActor");

    BEGIN_EXECUTE;

    // Analyze the request
    {
        if ( !m_update.ParseFromArray(m_data.constData(), m_data.length()) ) { CLIENTERROR("Could not parse submit request"); }
        qDebug("msg<< %s", m_update.DebugString().data());

        QString waveletId = QString::fromStdString( m_update.wavelet_name() );

        m_url = waveletId;
        if ( m_url.isNull() ) { CLIENTERROR("Malformed wave url"); }

        // Find the wave
        Wave* wave = Wave::wave( m_url.waveDomain(), m_url.waveId(), (m_url.waveDomain() == connection()->domain()) );
        if ( !wave ) { CLIENTERROR("Could not create wave"); }

        // If the wavelet does not exist -> create it (but only if it is a local wavelet)
        m_wavelet = wave->wavelet( m_url.waveletDomain(), m_url.waveletId(), (m_url.waveletDomain() == connection()->domain()) );
        if ( !m_wavelet ) { CLIENTERROR("Could not create wavelet"); }
    }

    if ( m_wavelet->isRemote() )
    {
        // Send a submit request via the federation protocol
        {
            // Is the delta applicable? If not we can reject it right now
            QString err = "";
            if ( !m_wavelet->checkHashedVersion( m_update.delta(), &err ) ) { CLIENTERROR(QString("Could not apply delta %1. Delta is not sent to remote server.").arg(err)); }

            m_id = nextId();
            IMessage* msg = new PBMessage<waveserver::ProtocolSubmitRequest>( m_update, ActorId( "federation", m_url.waveletDomain() ), m_id );
            msg->setCreateOnDemand( true );
            bool ok = post( msg );
            if ( !ok ) { CLIENTERROR("Internal server error"); }
        }

        // Wait for the response
        yield( RecvPB<waveserver::ProtocolSubmitResponse>( m_id ) | Timeout(10000) );
        if ( REASON( RecvPB<waveserver::ProtocolSubmitResponse> ) )
        {
            CLIENTLOG("Got submit response");
            // Send a response
            connection()->sendSubmitResponse( *REASON );
        }
        else if ( REASON( Timeout ) ) { CLIENTERROR("Timeout"); }    

        TERMINATE();
    }
    else
    {
        // Send a submit request to the wavelet
        {
            m_id = nextId();
            // Sign the delta
            SignedWaveletDelta s( m_update.delta() );
            // Construct the request
            PBMessage<messages::LocalSubmitRequest>* msg = new PBMessage<messages::LocalSubmitRequest>( m_wavelet->actorId(), m_id );
            msg->set_wavelet_name( m_update.wavelet_name() );
            protocol::ProtocolSignedDelta* signedDelta = msg->mutable_signed_delta();
            s.toProtobuf( signedDelta );
            msg->setCreateOnDemand(true);
            // Send the request
            bool ok = post( msg );
            if ( !ok ) { CLIENTERROR("Internal server error"); }
        }

        // Wait for the response
        yield( RecvPB<messages::SubmitResponse>( m_id ) | Timeout(10000) );
        if ( REASON( RecvPB<messages::SubmitResponse> ) )
        {
            CLIENTLOG("Got submit response");
            // Send a response to the client
            waveserver::ProtocolSubmitResponse response;
            response.set_operations_applied( REASON->operations_applied() );
            protocol::ProtocolHashedVersion* version = response.mutable_hashed_version_after_application();
            version->MergeFrom( REASON->hashed_version_after_application() );

            connection()->sendSubmitResponse( response );
        }
        else if ( REASON( Timeout ) ) { CLIENTERROR("Timeout waiting for response"); }
    }
//
//
//    LocalWavelet* localWavelet = dynamic_cast<LocalWavelet*>(m_wavelet);
//    // Apply the delta
//    QString err = "";
//    int version = localWavelet->apply( SignedWaveletDelta( m_update.delta() ), &err );
//    if ( !err.isEmpty() || version < 0 ) { CLIENTERROR(QString("Could not apply delta: %1").arg(err)); }
//
//    const AppliedWaveletDelta* applied = m_wavelet->delta(version - 1);
//    Q_ASSERT(applied);
//    // Send a response
//    connection()->sendSubmitResponse( applied->operationsApplied(), &applied->resultingVersion(), QString::null );

    END_EXECUTE;
}
