#ifndef WAVELETDELTA_H
#define WAVELETDELTA_H

#include <QString>
#include <QByteArray>
#include <QList>

#include "waveletdeltaoperation.h"

class DocumentMutation;

class WaveletDelta
{
public:
    struct HashedVersion
    {
        qint64 version;
        QByteArray hash;
    };

    WaveletDelta();
    WaveletDelta(const WaveletDelta& delta);
    ~WaveletDelta();

    /**
      * The JID of the user who created this delta.
      */
    QString author() const { return m_author; }
    void setAuthor( const QString& author ) { m_author = author; }
    /**
      * @return the version of which this delta must be applied.
      */
    HashedVersion& version() { return m_version; }
    /**
      * @return the version at which this delta must be applied.
      */
    const HashedVersion& version() const { return m_version; }
    const QList<WaveletDeltaOperation>& operations() const { return m_operations; }
    QList<WaveletDeltaOperation>& operations() { return m_operations; }
    void addOperation(const WaveletDeltaOperation& op);

    /**
      * @return the serialized represenation of this wavelet. The format follows Google's protobufs specifications.
      */
    QByteArray toBinary() const;
    QString toBase64() const;
    /**
      * Deserializes a wavelet delta. The format follows Google's protobufs specifications.
      */
    static WaveletDelta fromBinary(const QByteArray& data);
    static WaveletDelta fromBase64(const QString& data, bool* ok = 0);

private:
    HashedVersion m_version;
    QString m_author;
    QList<WaveletDeltaOperation> m_operations;
};

#endif // WAVELETDELTA_H
