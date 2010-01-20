#ifndef WAVELET_H
#define WAVELET_H

#include <QString>
#include <QHash>
#include <QList>
#include <QSet>
#include <QByteArray>
#include "model/waveletdelta.h"
#include "waveurl.h"

class Wave;
class WaveletDocument;
class ClientConnection;

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

    /**
      * @return operations_applied.
      */
    int receive( const WaveletDelta& delta, QString* errorMessage );

    void subscribe( ClientConnection* connection );
    void unsubscribe( ClientConnection* connection );

private:
    Wave* m_wave;
    QString m_domain;
    QString m_id;
    QHash<QString,WaveletDocument*> m_documents;
    /**
      * History of all transformed and applied deltas.
      */
    QList<WaveletDelta> m_deltas;
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
      * The current digest text.
      */
    QString m_lastDigest;
};

#endif // WAVELET_H
