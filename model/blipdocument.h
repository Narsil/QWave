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
    virtual void onMutationStart(const QString& author);
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
    void insertedText( int pos, const QString& text );
    void deletedText( int pos, const QString& text );
    void deletedLineBreak( int pos);
    void insertedLineBreak(int pos);
    void setCursor(int pos, const QString& author);
    void mutationEnd();

private:
    bool m_inBody;
    bool m_afterLine;
    int m_pos;
    QStack<QString> m_stack;
    QString m_currentAuthor;
    int m_cursorpos;
};

#endif // BLIPDOCUMENT_H
