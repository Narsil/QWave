#ifndef WAVELETDELTA_H
#define WAVELETDELTA_H

#include <QString>
#include <QByteArray>

#include "documentmutation.h"

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

    bool hasMutation() const { return m_mutation != 0; }
    bool hasAddParticipant() const { return !m_addParticipant.isEmpty(); }
    bool hasRemoveParticipant() const { return !m_removeParticipant.isEmpty(); }
    void setMutation( const DocumentMutation& mutation );
    DocumentMutation* mutation() { return m_mutation; }
    const DocumentMutation* mutation() const { return m_mutation; }
    DocumentMutation* createMutation();
    QString documentId() const { return m_documentId; }
    void setDocumentId(const QString& documentId) { m_documentId = documentId; }
    void clearDocumentId() { m_documentId = QString::null; }
    QString addParticipant() const { return m_addParticipant; }
    void setAddParticipant(const QString& participant) { m_addParticipant = participant; }
    void clearAddParticipant() { m_addParticipant = QString::null; }
    QString removeParticipant() const { return m_removeParticipant; }
    void setRemoveParticipant(const QString& participant) { m_removeParticipant = participant; }
    void clearRemoveParticipant() { m_removeParticipant = QString::null; }
    QString author() const { return m_author; }
    void setAuthor( const QString& author ) { m_author = author; }
    HashedVersion& version() { return m_version; }
    const HashedVersion& version() const { return m_version; }
    HashedVersion& resultingVersion() { return m_resultingVersion; }
    const HashedVersion& resultingVersion() const { return m_resultingVersion; }

    bool isNull() const;

    WaveletDelta translate(const WaveletDelta& delta) const;

private:
    QString m_documentId;
    DocumentMutation* m_mutation;
    QString m_addParticipant;
    QString m_removeParticipant;
    HashedVersion m_version;
    HashedVersion m_resultingVersion;
    QString m_author;
};

#endif // WAVELETDELTA_H
