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

    QString author() const { return m_author; }
    void setAuthor( const QString& author ) { m_author = author; }
    HashedVersion& version() { return m_version; }
    const HashedVersion& version() const { return m_version; }
    HashedVersion& resultingVersion() { return m_resultingVersion; }
    const HashedVersion& resultingVersion() const { return m_resultingVersion; }
    const QList<WaveletDeltaOperation>& operations() const { return m_operations; }
    QList<WaveletDeltaOperation>& operations() { return m_operations; }
    void addOperation(const WaveletDeltaOperation& op);

private:
    HashedVersion m_version;
    HashedVersion m_resultingVersion;
    QString m_author;
    QList<WaveletDeltaOperation> m_operations;
};

#endif // WAVELETDELTA_H
