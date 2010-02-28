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
#include "actor/recvcriticalsection.h"
#include "actor/pbmessage.h"
#include "protocol/messages.pb.h"

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

namespace waveserver
{
    class ProtocolWaveletUpdate;
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

    /**
      * The JID of the user who sent the first delta for this wave.
      */
    QString creator() const { return m_creator; }

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
    const QByteArray& hash() const { return m_hash; }

    // TBR
    //const AppliedWaveletDelta* delta( int version ) { return &(m_deltas[version]); }

    /**
      * Checks that the version and history hash are ok. It does not do OT, i.e. the delta may still fail
      * when being transformed or applied because its operations are not applicable.
      *
      * The purpose of this function is to roughly check deltas before they are submitted to remote wave servers.
      */
    bool checkHashedVersion( const protocol::ProtocolWaveletDelta& protobufDelta, QString* errorMessage );
    bool checkHashedVersion( const WaveletDelta& clientDelta, QString* errorMessage );

    CriticalSection* criticalSection() { return &m_criticalSection; }

protected:
    /**
      * Abstract base class for actors.
      */
    class WaveletActor : public Actor
    {
    public:
       WaveletActor( Wavelet* wavelet );

   protected:
       void log( const char* error, const char* file, int line );
       void log( const QString& error, const char* file, int line );
       void logErr( const char* error, const char* file, int line );
       void logErr( const QString& error, const char* file, int line );

       Wavelet* m_wavelet;

    private:
       static qint64 s_id;
    };

    /**
      * Initializes the wavelet from the store.
      */
    class InitActor : public WaveletActor
    {
    public:
        InitActor( Wavelet* wavelet);

    protected:
        void execute();

    private:
        qint64 m_msgId;
    };

protected:
    /**
      * Called from 'apply' when a participant has been added.
      */
    virtual void onAddParticipant( const JID& jid ) = 0;
    /**
      * Called from 'apply' when a participant has been removed.
      */
    virtual void onRemoveParticipant( const JID& jid ) = 0;

    /**
      * This function must be called when a local participant is removed from the wavelet.
      * It unsubscribes all client connections.
      */
    void unsubscribeAllClients( const QString& participant );
    /**
      * This function must be called when a local participant is added to the wavelet.
      * It sends a messages::WaveletNotify message to all client connections of this participant.
      */
    void notifyAllClients( const QString& participant );

    virtual void customEvent( QEvent* event );

    /**
      * A list of all wavelet documents in this wavelet.
      */
    QHash<QString,WaveletDocument*> m_documents;
    /**
      * The JIDs of all participants in this wave.
      */
    QSet<QString> m_participants;
    /**
      * The current digest text.
      */
    QString m_lastDigest;

    /**
      * Tests and translates a delta. The result can be applied to the wavelet. This function does not change the wavelet itself.
      * If the returned delta is null, some error has occured and the delta must be rejected.
      */
    AppliedWaveletDelta process( const protocol::ProtocolSignedDelta* signed_delta, const protocol::ProtocolAppliedWaveletDelta* applied_delta, QSet<QString>* addedLocalUsers, QSet<QString>* removedLocalUser, QString* errorMessage );
    /**
      * Applies the delta to the wavelet. This does NOT persist the change. It just applied it to the in-memory representation of the wave.
      */
    bool apply( const AppliedWaveletDelta& appliedDelta, QString* errorMessage );

private:
    /**
      * Transforms a delta such that it can be applied to the wavelet.
      */
    bool transform( WaveletDelta& clientDelta, QString* errorMessage, bool* ok );
    void broadcast( const WaveletDelta& applied_delta );
    void broadcastDigest(const WaveletDelta& digest );
    void toWaveletUpdate( waveserver::ProtocolWaveletUpdate* update );

    /**
      * Subscribes or unsubscribes a client connection.
      */
    class SubscribeActor : public WaveletActor
    {
    public:
        SubscribeActor( Wavelet* wavelet, const PBMessage<messages::SubscribeWavelet>& message);

    protected:
        void execute();

        PBMessage<messages::SubscribeWavelet> m_message;
    };

    /**
      * The latest version.
      */
    qint64 m_version;
    /**
      * The hash of the latest version.
      */
    QByteArray m_hash;
    Wave* m_wave;
    /**
      * Wavelet domain.
      */
    QString m_domain;
    /**
      * Wavelet ID.
      */
    QString m_id;
    /**
      * A set of ClientConnection ActorIds.
      */
    QSet<QByteArray> m_contentSubscribers;
    /**
      * A set of ClientConnection ActorIds.
      */
    QSet<QByteArray> m_indexSubscribers;
    /**
      * History of all transformed and applied deltas.
      * The position in this array reflects the version number. Since one delta can span multiple versions
      * it is possible that some entries in this list are 0.
      */
    QList<AppliedWaveletDelta> m_deltas;
    /**
      * Used to synchronize actors. Only one is allowed to modify the wavelet at a time.
      */
    CriticalSection m_criticalSection;
    QString m_creator;
};

#endif // WAVELET_H
