#ifndef OTADAPTER_H
#define OTADAPTER_H

#include <QObject>

class BlipGraphicsItem;
class Blip;
class GraphicsTextItem;
class Environment;
class DocumentMutation;

/**
  * This adapter connectes QTextDocument with SynchronizedDocument.
  * This is required because the GUI edits a QTextDocument while the operation transformation (OT)
  * works on a SynchronizedDocument. The task of this adapter is to keep these two synchronized.
  */
class OTAdapter : public QObject
{
    Q_OBJECT
public:
    OTAdapter(BlipGraphicsItem* parent );

    Blip* blip() const;
    GraphicsTextItem* textItem() const;
    BlipGraphicsItem* blipItem() const;

    void onContentsChange( int position, int charsRemoved, int charsAdded );
    /**
      * Puts the text stored in the Blip into the GraphicsTextItem.
      */
    void setGraphicsText();

    /**
      * charIndex is the position of a character in the QTextDocument.
      * For this character this function calculates the corresponding position in the Blip's StructuredDocument.
      */
    int mapToBlip(int charIndex);

    Environment* environment() const;

signals:
    void titleChanged(const QString& title);

private slots:
    void update( const DocumentMutation& mutation );

    void mutationStart();
    void insertText( int lineCount, int inlinePos, const QString& text );
    void deleteText( int lineCount, int inlinePos, const QString& text );
    void deleteLineBreak(int lineCount, int inlinePos);
    void insertLineBreak(int lineCount, int inlinePos);
    void mutationEnd();

private:
    bool m_suspendContentsChange;
    QString m_authorNames;
    /**
      * This flag avoids that edit actions made by the user come back as DocumentMutation.
      */
    bool m_blockUpdate;
};

#endif // OTADAPTER_H
