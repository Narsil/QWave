#ifndef UNKNOWNDOCUMENT_H
#define UNKNOWNDOCUMENT_H

#include <QString>

class StructuredDocument;
class DocumentMutation;

/**
  * An UnknownDocument is created for every document in a wavelet which has no corresponding Blip tag in the
  * conversation document. The client performs all OT operations on this document, but its content is
  * not displayed or interpreted.
  */
class UnknownDocument
{
public:
    UnknownDocument(const QString& id);
    ~UnknownDocument();

    QString id() const { return m_id; }

    /**
      * Strips the StructuredDocument from the object.
      * Usually you may want to delete the object in the next step.
      */
    StructuredDocument* releaseDocument();

    /**
      * Applies OT operations.
      */
    void receive( const DocumentMutation& mutation );

private:
    StructuredDocument* m_doc;
    QString m_id;
};

#endif // UNKNOWNDOCUMENT_H
