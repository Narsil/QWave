#ifndef WAVELET_H
#define WAVELET_H

#include <QString>
#include <QHash>
#include <QList>
#include <QSet>
#include <QByteArray>
#include "model/waveletdelta.h"
#include "model/appliedwaveletdelta.h"
#include "waveurl.h"

class Wave;
class WaveletDocument;
class ClientConnection;
class JID;

namespace protocol
{
    class ProtocolWaveletDelta;
}

class Wavelet
{
public:
    Wavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );
    ~Wavelet();

    WaveUrl url() const;

    QString id() const { return m_id; }
    QString domain() const { return m_domain; }

    Wave* wave() const { return m_wave; }

    QString firstRootBlipId() const;
    WaveletDocument* firstRootBlip() const;

    QString digest() const;
    WaveletDelta initialDigest() const;

    bool hasParticipant(const QString& jid) const;
    QSet<QString> participantIds() const { return m_participants; }

    qint64 version() const { return m_version; }

    const AppliedWaveletDelta& delta( int version ) { return m_deltas[version]; }

    /**
      * @return operations_applied.
      */
    int apply( const protocol::ProtocolWaveletDelta& protobufDelta, QString* errorMessage );

    void subscribe( ClientConnection* connection );
    void unsubscribe( ClientConnection* connection );

private:
    void subscribeRemote( const JID& remoteJid );
    void unsubscribeRemote( const JID& remoteJid );

    Wave* m_wave;
    QString m_domain;
    QString m_id;
    QHash<QString,WaveletDocument*> m_documents;
    /**
      * History of all transformed and applied deltas.
      * The position in this array reflects the version number. Since one delta can span multiple versions
      * it is possible that some entries in this list are 0.
      */
    QList<AppliedWaveletDelta> m_deltas;
    /**
      * The latest version.
      */
    qint64 m_version;
    /**
      * The hash of the latest version.
      */
    QByteArray m_hash;
    /**
      * The JIDs of all participants in this wave.
      */
    QSet<QString> m_participants;
    /**
      * A set of ClientConnection Ids.
      */
    QSet<QString> m_subscribers;
    /**
      * A set of all remote wave domains which need updates of this wave.
      * The integer is the number of participants in this domain who are participating in this wave.
      */
    QHash<QString,int> m_remoteSubscribers;
    /**
      * The current digest text.
      */
    QString m_lastDigest;
};

#endif // WAVELET_H
