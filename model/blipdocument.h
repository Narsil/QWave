#ifndef BLIPDOCUMENT_H
#define BLIPDOCUMENT_H

#include "structureddocument.h"
#include <QStack>

class DocumentMutation;

class BlipDocument : public StructuredDocument
{
    Q_OBJECT
public:
    BlipDocument(QObject* parent = 0);
    BlipDocument( const StructuredDocument& doc);

protected:
    virtual void onMutationStart();
    virtual void onRetainChar(int index);
    virtual void onRetainElementStart(int index);
    virtual void onRetainElementEnd(int index);
    virtual void onDeleteChars(int index, const QString& chars);
    virtual void onDeleteElementStart(int index);
    virtual void onDeleteElementEnd(int index);
    virtual void onInsertChars(int index, const QString& chars);
    virtual void onInsertElementStart(int index);
    virtual void onInsertElementEnd(int index);
    virtual void onAnnotationUpdate(int index, const QHash<QString,QString>& updates);
    virtual void onMutationEnd();

signals:    
    void mutationStart();
    void insertedText( int lineCount, int inlinePos, const QString& text );
    void deletedText( int lineCount, int inlinePos, const QString& text );
    void deletedLineBreak(int lineCount, int inlinePos);
    void insertedLineBreak(int lineCount, int inlinePos);
    void mutationEnd();

private:
    bool m_inBody;
    bool m_afterLine;
    int m_inlinePos;
    int m_lineCount;
    QStack<QString> m_stack;
};

#endif // BLIPDOCUMENT_H
