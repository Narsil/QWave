#ifndef BLIPDOCUMENT_H
#define BLIPDOCUMENT_H

#include "structureddocument.h"

class DocumentMutation;

class BlipDocument : public StructuredDocument
{
    Q_OBJECT
public:
    BlipDocument(QObject* parent = 0);
    BlipDocument( const StructuredDocument& doc);

//    virtual bool apply(const DocumentMutation& mutation);

signals:
    void insertedText( int lineCount, int inlinePos, const QString& text );
    void deletedText( int lineCount, int inlinePos, const QString& text );
    void deletedLineBreak(int lineCount, int inlinePos);
    void insertedLineBreak(int lineCount, int inlinePos);
};

#endif // BLIPDOCUMENT_H
