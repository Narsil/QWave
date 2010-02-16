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
#include "actor/actorgroup.h"

class Wave;
class WaveletDocument;
class ClientConnection;
class JID;
class Signature;
class SignedWaveletDelta;

namespace protocol
{
    class ProtocolWaveletDelta;
    class ProtocolSignedDelta;
}

class Wavelet : public ActorGroup
{
public:
    Wavelet( Wave* wave, const QString& waveletDomain, const QString& waveletId );
    ~Wavelet();

    WaveUrl url() const;

    QString id() const { return m_id; }
    QString domain() const { return m_domain; }
    virtual bool isRemote() const = 0;
    virtual bool isLocal() const = 0;

    Wave* wave() const { return m_wave; }

    QString firstRootBlipId() const;
    WaveletDocument* firstRootBlip() const;

    QString digestText() const;
    WaveletDelta initialDigest() const;

    bool hasParticipant(const QString& jid) const;
    QSet<QString> participantIds() const { return m_participants; }

    /**
      * The current version of the wavelet.
      */
    qint64 version() const { return m_version; }
    /**
      * The resuling rolling hash of the current version.
      */
    QByteArray hash() const { return m_hash; }

    const AppliedWaveletDelta* delta( int version ) { return &(m_deltas[version]); }

    /**
      * Checks that the version and history hash are ok. It does not do OT, i.e. the delta may still fail
      * when being transformed or applied because its operations are not applicable.
      *
      * The purpose of this function is to roughly check deltas before they are submitted to remote wave servers.
      */
    bool checkHashedVersion( const protocol::ProtocolWaveletDelta& protobufDelta, QString* errorMessage );
    bool checkHashedVersion( const WaveletDelta& clientDelta, QString* errorMessage );

    void subscribe( ClientConnection* connection );
    void unsubscribe( ClientConnection* connection );

protected:
    QHash<QString,WaveletDocument*> m_documents;
    /**
      * The JIDs of all participants in this wave.
      */
    QSet<QString> m_participants;
    /**
      * The current digest text.
      */
    QString m_lastDigest;

    void commit( const AppliedWaveletDelta& appliedDelta, bool restore );
    bool transform( WaveletDelta& clientDelta, QString* errorMessage, bool* ok );

private:
    void broadcast( const AppliedWaveletDelta& delta );
    void broadcastDigest(const WaveletDelta& digest );

    /**
      * The latest version.
      */
    qint64 m_version;
    /**
      * The hash of the latest version.
      */
    QByteArray m_hash;
    Wave* m_wave;
    QString m_domain;
    QString m_id;
    /**
      * A set of ClientConnection Ids.
      */
    QSet<QString> m_subscribers;
    /**
      * History of all transformed and applied deltas.
      * The position in this array reflects the version number. Since one delta can span multiple versions
      * it is possible that some entries in this list are 0.
      */
    QList<AppliedWaveletDelta> m_deltas;
};

#endif // WAVELET_H
