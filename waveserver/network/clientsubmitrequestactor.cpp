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
#include <QByteArray>

#define CLIENTERROR(msg) { logErr(msg, __FILE__, __LINE__); connection()->sendSubmitResponse(0, 0, msg); TERMINATE(); }
#define CLIENTLOG(msg) { log(msg, __FILE__, __LINE__); }

ClientSubmitRequestActor::ClientSubmitRequestActor( ClientConnection* con, const QByteArray& data )
        : ClientActor( con ), m_data(data)
{
    con->addActor( this );
}

void ClientSubmitRequestActor::EXECUTE()
{
    BEGIN_EXECUTE;

    // Analyze the request
    {
        if ( !m_update.ParseFromArray(m_data.constData(), m_data.length()) ) { CLIENTERROR("Could not parse submit request"); }
        qDebug("msg<< %s", m_update.DebugString().data());

        QString waveletId = QString::fromStdString( m_update.wavelet_name() );

        WaveUrl url( waveletId );
        if ( url.isNull() ) { CLIENTERROR("Malformed wave url"); }

        // Find the wave
        Wave* wave = Wave::wave( url.waveDomain(), url.waveId(), (url.waveDomain() == connection()->domain()) );
        if ( !wave ) { CLIENTERROR("Could not create wave"); }

        // If the wavelet does not exist -> create it (but only if it is a local wavelet)
        m_wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), (url.waveletDomain() == connection()->domain()) );
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
            bool ok = send( ActorId( ActorId::Federation, m_wavelet->domain() ), new PBMessage<waveserver::ProtocolSubmitRequest>( m_update, m_id ) );
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

    Q_ASSERT( m_wavelet->isLocal() );

    LocalWavelet* localWavelet = dynamic_cast<LocalWavelet*>(m_wavelet);
    // Apply the delta
    QString err = "";
    int version = localWavelet->apply( SignedWaveletDelta( m_update.delta() ), &err );
    if ( !err.isEmpty() || version < 0 ) { CLIENTERROR(QString("Could not apply delta: %1").arg(err)); }

    const AppliedWaveletDelta* applied = m_wavelet->delta(version - 1);
    Q_ASSERT(applied);
    // Send a response
    connection()->sendSubmitResponse( applied->operationsApplied(), &applied->resultingVersion(), QString::null );

    END_EXECUTE;
}
