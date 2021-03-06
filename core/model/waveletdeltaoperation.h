#ifndef WAVELETDELTAOPERATION_H
#define WAVELETDELTAOPERATION_H

#include <QString>
#include <QPair>

#include "documentmutation.h"

class WaveletDeltaOperation
{
public:
    WaveletDeltaOperation();
    WaveletDeltaOperation(const WaveletDeltaOperation& op);

    bool hasMutation() const { return !m_mutation.isEmpty(); }
    bool hasAddParticipant() const { return !m_addParticipant.isEmpty(); }
    bool hasRemoveParticipant() const { return !m_removeParticipant.isEmpty(); }
    void setMutation( const DocumentMutation& mutation );
    DocumentMutation& mutation() { return m_mutation; }
    const DocumentMutation& mutation() const { return m_mutation; }
    QString documentId() const { return m_documentId; }
    void setDocumentId(const QString& documentId) { m_documentId = documentId; }
    void clearDocumentId() { m_documentId = QString::null; }
    QString addParticipant() const { return m_addParticipant; }
    void setAddParticipant(const QString& participant) { m_addParticipant = participant; }
    void clearAddParticipant() { m_addParticipant = QString::null; }
    QString removeParticipant() const { return m_removeParticipant; }
    void setRemoveParticipant(const QString& participant) { m_removeParticipant = participant; }
    void clearRemoveParticipant() { m_removeParticipant = QString::null; }

//    WaveletDeltaOperation translate(const WaveletDeltaOperation& delta) const;

    static QPair<WaveletDeltaOperation,WaveletDeltaOperation> xform( const WaveletDeltaOperation& o1, const WaveletDeltaOperation& o2, bool* ok );

    bool isNull() const;

private:
    QString m_documentId;
    DocumentMutation m_mutation;
    QString m_addParticipant;
    QString m_removeParticipant;
};

#endif // WAVELETDELTAOPERATION_H
