#include "clientconnection.h"
#include "serversocket.h"
#include "network/rpc.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "network/converter.h"
#include "model/waveletdelta.h"
#include "model/wavelet.h"
#include "model/wave.h"
#include "model/waveurl.h"
#include "model/participant.h"
#include "app/settings.h"
#include "persistence/commitlog.h"

#include <QTcpSocket>
#include <QByteArray>
#include <QUuid>
// TODO: Why do we still use QUrl here?
#include <QUrl>

#include <sstream>
#include <string>

QMultiHash<QString,ClientConnection*>* ClientConnection::s_connectionsByParticipant = 0;
QHash<QString,ClientConnection*>* ClientConnection::s_connectionsById = 0;

ClientConnection::ClientConnection(QTcpSocket* socket, ServerSocket* parent)
        : QObject(parent), m_participant(0), m_digestVersion(0)
{
    m_id = QUuid::createUuid().toString();

    if ( s_connectionsByParticipant == 0 )
        s_connectionsByParticipant = new QMultiHash<QString,ClientConnection*>();
    if ( s_connectionsById == 0 )
        s_connectionsById = new QMultiHash<QString,ClientConnection*>();
    (*s_connectionsById)[ m_id ] = this;

    m_rpc = new RPC(socket, this);
    bool check = connect( m_rpc, SIGNAL(offline()), SLOT(getOffline()));
    Q_ASSERT(check);
    check = connect( m_rpc, SIGNAL(socketError()), SLOT(networkError()));
    Q_ASSERT(check);
    check = connect( m_rpc, SIGNAL(messageReceived(QString,QByteArray)), SLOT(messageReceived(QString,QByteArray)));
    Q_ASSERT(check);
}

ClientConnection::~ClientConnection()
{
    if ( m_participant )
        s_connectionsByParticipant->remove( m_participant->toString(), this );
    s_connectionsById->remove( m_id );
}

void ClientConnection::messageReceived(const QString& methodName, const QByteArray& data)
{
    if ( methodName == "waveserver.ProtocolOpenRequest" )
    {
        waveserver::ProtocolOpenRequest open;
        open.ParseFromArray(data.constData(), data.length());
        // TODO: Check for parsing errors

        qDebug("msg<< %s", open.DebugString().data());

        // This is a chance of getting the client participant ID. A bit strange place, but well ...
        QString participantId = QString::fromStdString( open.participant_id() );
        JID jid( participantId );
        if ( !jid.isValid() )
        {
            qDebug("Malformed JID");
            getOffline();
            return;
        }
        if ( !jid.isLocal() )
        {
            qDebug("Not a local user");
            getOffline();
            return;
        }

        // No participant or a different one than before? -> not allowed
        if ( m_participant && m_participant->toString() != participantId )
        {
            qDebug("Cannot change user for open connection");
            getOffline();
            return;
        }

        // This is the first time to encounter the JID of our client?
        if ( !m_participant )
        {
            m_participant = Participant::participant( participantId, true );
            s_connectionsByParticipant->insert( m_participant->toString(), this );
        }

        QString waveId = QString::fromStdString( open.wave_id() );

        if ( waveId == "!indexwave" )
        {
            // Send a digest for all waves in which the user participates
            foreach( Wavelet* wavelet, m_participant->wavelets() )
            {
                this->sendIndexUpdate( wavelet, wavelet->initialDigest() );
            }
            return;
        }

        for( int w = 0; w < open.wavelet_id_prefix_size(); ++w )
        {
            QString waveletId = QString::fromStdString( open.wavelet_id_prefix(w) );
            int index = waveId.indexOf( '!' );
            if ( index == -1 )
            {
                qDebug("Malformed wave id");
                getOffline();
                return;
            }
            QString waveDomain = waveId.left(index);
            waveId = waveId.mid( index + 1 );
            index = waveletId.indexOf( '!' );
            if ( index == -1 )
            {
                qDebug("Malformed wavelet id");
                getOffline();
                return;
            }
            QString waveletDomain = waveletId.left(index);
            waveletId = waveletId.mid( index + 1 );

            WaveUrl url( waveDomain, waveId, waveletDomain, waveletId );
            Wave* wave = Wave::wave( waveDomain, waveId, (waveDomain == domain()) );
            if ( !wave )
            {
                qDebug("Could not find or create wave %s", url.toString().toAscii().constData() );
                continue;
            }
            Wavelet* wavelet = wave->wavelet( waveletDomain, waveletId, (waveletDomain == domain()) );
            if ( !wavelet )
            {
                qDebug("Could not find or create wavelet %s", url.toString().toAscii().constData() );
                continue;
            }

            wavelet->subscribe(this);
        }
    }
    else if ( methodName == "waveserver.ProtocolSubmitRequest" )
    {
        waveserver::ProtocolSubmitRequest update;
        update.ParseFromArray(data.constData(), data.length());
        qDebug("msg<< %s", update.DebugString().data());

        // Write it to the commit log
        CommitLog::commitLog()->write(update);

        QString waveletId = QString::fromStdString( update.wavelet_name() );

        WaveUrl url( waveletId );
        if ( url.isNull() )
        {
            sendSubmitResponse( 0, 0, "Malformed wave url");
            return;
        }

        // Find the wave
        Wave* wave = Wave::wave( url.waveDomain(), url.waveId(), (url.waveDomain() == domain()) );
        if ( !wave )
        {
            sendSubmitResponse( 0, 0, "Could not create wave");
            return;
        }

        // TODO: What about remote wavelets?
        Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), (url.waveletDomain() == domain()) );
        if ( !wavelet )
        {
            sendSubmitResponse( 0, 0, "Could not create wavelet");
            return;
        }

        // Apply the delta
        QString err = "";
        int version = wavelet->apply(update.delta(), &err );
        if ( !err.isEmpty() || version < 0 )
        {
            sendSubmitResponse( 0, 0, err );
        }
        else
        {
            const AppliedWaveletDelta& applied = wavelet->delta(version - 1);

            // Send a response
            sendSubmitResponse( applied.operationsApplied(), &applied.resultingVersion(), QString::null );
        }
    }
}

void ClientConnection::sendSubmitResponse( qint32 operationsApplied, const WaveletDelta::HashedVersion* hashedVersionAfterApplication, const QString& errorMessage )
{
    waveserver::ProtocolSubmitResponse response;
    response.set_operations_applied( operationsApplied );
    if ( !errorMessage.isNull() )
        response.set_error_message( errorMessage.toStdString() );
    if ( hashedVersionAfterApplication )
    {
        protocol::ProtocolHashedVersion* version = response.mutable_hashed_version_after_application();
        version->set_history_hash( hashedVersionAfterApplication->hash.data(), hashedVersionAfterApplication->hash.length() );
        version->set_version( hashedVersionAfterApplication->version );
    }

    qDebug("SubmitResponse>> %s", response.DebugString().data());

    QByteArray buffer( response.ByteSize(), 0 );
    response.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolSubmitResponse", buffer.constData(), buffer.length());
}

void ClientConnection::sendWaveletUpdate( Wavelet* wavelet, const QList<AppliedWaveletDelta>& deltas )
{
    // Do nothing if there is nothing to send
    if ( deltas.count() == 0 )
        return;

    waveserver::ProtocolWaveletUpdate update;
    update.set_wavelet_name( wavelet->url().toString().toStdString() );
    update.mutable_resulting_version()->set_version( deltas.last().resultingVersion().version );
    QByteArray resultingHash = deltas.last().resultingVersion().hash;
    update.mutable_resulting_version()->set_history_hash( resultingHash.constData(), resultingHash.length() );
    for( int i = 0; i < deltas.count(); ++i )
    {
        const AppliedWaveletDelta& appliedDelta = deltas[i];
        if ( appliedDelta.isNull() )
            continue;
        protocol::ProtocolWaveletDelta* delta = update.add_applied_delta();
        Converter::convert( delta, appliedDelta.delta() );
    }

    qDebug("WaveletUpdate>> %s", update.DebugString().data());

    QByteArray buffer( update.ByteSize(), 0 );
    update.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolWaveletUpdate", buffer.constData(), buffer.length());
}

void ClientConnection::sendIndexUpdate(Wavelet* wavelet, const WaveletDelta& indexDelta)
{
    // Fix the version number and hash
    WaveletDelta sendDelta( indexDelta );
    sendDelta.version().version = m_digestVersion;
    sendDelta.version().hash = m_digestHash;

    // TODO: Building the indexwave ID is very strange with respect to the domain used
    QUrl url;
    url.setScheme("wave");
    url.setHost(wavelet->domain() );
    url.setPath( "/!indexwave/" + wavelet->wave()->id() );
    waveserver::ProtocolWaveletUpdate update;
    update.set_wavelet_name( url.toString().toStdString() );
    update.mutable_resulting_version()->set_version( ++m_digestVersion );
    update.mutable_resulting_version()->set_history_hash( m_digestHash.constData(), m_digestHash.length() );

    protocol::ProtocolWaveletDelta* delta = update.add_applied_delta();
    Converter::convert( delta, sendDelta );

    qDebug("Index WaveletUpdate>> %s", update.DebugString().data());

    QByteArray buffer( update.ByteSize(), 0 );
    update.SerializeToArray( buffer.data(), buffer.length() );

    m_rpc->send("waveserver.ProtocolWaveletUpdate",buffer.constData(), buffer.length());
}

void ClientConnection::getOffline()
{
    this->deleteLater();
}

void ClientConnection::networkError()
{    
    this->deleteLater();
}

ClientConnection* ClientConnection::connectionById( const QString& id )
{
    if ( s_connectionsById == 0 )
        return 0;
    return (*s_connectionsById)[id];
}

QList<ClientConnection*> ClientConnection::connectionsByParticipant( const QString& participant )
{
    QList<ClientConnection*> result;
    if ( s_connectionsByParticipant )
    {
        QMultiHash<QString,ClientConnection*>::iterator it = s_connectionsByParticipant->find( participant );
        for( ; it != s_connectionsByParticipant->end() && it.key() == participant; ++it )
            result.append(*it);
    }
    return result;
}

QString ClientConnection::domain() const
{
    return Settings::settings()->domain();
}
