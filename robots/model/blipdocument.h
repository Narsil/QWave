#ifndef BLIPDOCUMENT_H
#define BLIPDOCUMENT_H

#include "model/structureddocument.h"
#include <QStack>
#include <QHash>

class DocumentMutation;
class Blip;
class QImage;

/**
  * Represents the content of a blip.
  */
class BlipDocument : public StructuredDocument
{
    Q_OBJECT
public:
    BlipDocument(Blip* parent);
    BlipDocument( const StructuredDocument& doc);

    Blip* blip() const { return (Blip*)parent(); }

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
    virtual void onAnnotationUpdate(int index, const AnnotationChange& updates);
    virtual void onUpdateAttributes(int index, const AttributeList& updates);
    virtual void onUpdatedAttributes(int index);
    virtual void onReplaceAttributes(int index, const AttributeList& updates);
    virtual void onReplacedAttributes(int index);
    virtual void onMutationEnd();

signals:    
    void mutationStart();
    void insertedText( int pos, const QString& text );
    void deletedText( int pos, const QString& text );
    void deletedLineBreak( int pos);
    void insertedLineBreak(int pos);
    void insertImage( int pos, const QString& attachmentId, const QImage& image, const QString& caption );
    void insertGadget( int pos, const QString& url, const QString& author, const QString& id );
    void setGadgetState( int pos, const QString& gadgetId, const QString& name, const QString& value );
    void setStyle( const QString& style, const QString& value, int startPos, int endPos );
    void setCursor(int pos, const QString& author);
    void mutationEnd();

private:
    struct PendingStyle
    {
        int startPos;
        QString value;
    };

    void applyPendingStyleChange( const QString& style, int endPos );

    bool m_inBody;
    bool m_afterLine;
    int m_pos;
    QStack<QString> m_stack;
    QString m_currentAuthor;
    int m_cursorpos;
    /**
      * Required for the <caption> tag.
      */
    bool m_inCaption;
    /**
      * Required for the <caption> tag.
      */
    QString m_caption;
    /**
      * Required for the <image> tag.
      */
    QString m_attachmentId;
    /**
      * Required for the <gadget> tag.
      */
    QString m_gadgetUrl;
    /**
      * Required for the <gadget> tag.
      */
    QString m_gadgetAuthor;
    /**
      * Required for the <gadget> tag.
      */
    QString m_gadgetId;
    QHash<QString,PendingStyle> m_pendingStyles;
};

#endif // BLIPDOCUMENT_H
