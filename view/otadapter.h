#ifndef OTADAPTER_H
#define OTADAPTER_H

#include <QHash>
#include <QObject>
#include <QDateTime>
#include <QTextCursor>

class BlipGraphicsItem;
class Blip;
class GraphicsTextItem;
class Environment;
class DocumentMutation;
class Participant;
class QTimer;

/**
  * This adapter connectes QTextDocument with BlipDocument.
  * This is required because the GUI edits a QTextDocument while the operation transformation (OT)
  * works on a SynchronizedDocument. The task of this adapter is to keep these two synchronized.
  */
class OTAdapter : public QObject
{
    Q_OBJECT
public:
    OTAdapter(BlipGraphicsItem* parent );
    ~OTAdapter();

    Blip* blip() const;
    GraphicsTextItem* textItem() const;
    BlipGraphicsItem* blipItem() const;

    void onContentsChange( int position, int charsRemoved, int charsAdded );
    void onStyleChange( int position, int charsFormatted, const QString& style, const QString& value );
    void suspendContentsChange( bool suspend ) { m_suspendContentsChange = suspend; }

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

private:
    class Cursor
    {
    public:
        Cursor( Participant* participant, const QDateTime& timestamp ) { m_participant = participant; m_timestamp = timestamp; }

        Participant* m_participant;
        QDateTime m_timestamp;
        QTextCursor m_textCursor;
    };

signals:
    void titleChanged(const QString& title);

private slots:
    void mutationStart();
    void insertText( int inlinePos, const QString& text );
    void deleteText( int inlinePos, const QString& text );
    void deleteLineBreak(int inlinePos);
    void insertLineBreak(int inlinePos);
    void setCursor(int inlinePos, const QString& author);
    void insertImage( int inlinePos, const QString& attachmentId, const QImage& image, const QString& caption );
    void setStyle( const QString& style, const QString& value, int startPos, int endPos );
    void mutationEnd();
    void removeOldCursors();

private:
    void startOldCursorsTimer();

    bool m_suspendContentsChange;
    QString m_authorNames;
    /**
      * This flag avoids that edit actions made by the user come back as DocumentMutation.
      */
    bool m_blockUpdate;
    QHash<QString,Cursor*> m_cursors;
    QTimer* m_timer;
};

#endif // OTADAPTER_H
