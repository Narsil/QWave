#ifndef BLIP_H
#define BLIP_H

#include <QObject>
#include <QList>

class BlipThread;
class Wavelet;
class BlipDocument;
class StructuredDocument;
class Participant;
class DocumentMutation;
class Environment;
class QUrl;

class Blip : public QObject
{
    Q_OBJECT
public:
    Blip(Wavelet* wavelet, const QString& id, Participant* creator);
    Blip(BlipThread* thread, const QString& id, Participant* creator);
    Blip(Wavelet* wavelet, const QString& id, Participant* creator, const StructuredDocument& doc);
    Blip(BlipThread* thread, const QString& id, Participant* creator, const StructuredDocument& doc);

    /**
      * A ordered list of all thread belonging to the blip.
      */
    const QList<BlipThread*>& threads() const { return m_threads; }
    QString id() const { return this->m_id; }
    BlipDocument* document() { return m_doc; }
    BlipThread* parentThread() const;
    Wavelet* wavelet() const;
    Environment* environment() const;
    bool isRootBlip() const;
    bool isFirstRootBlip() const;
    bool isLastBlipInThread() const;
    /**
      * Resembles the authors as given by the contributor tags.
      * Enlisting an author here is not mandatory. The list can even be empty.
      */
    const QList<QString>& authors() const;
    /**
      * The user who created the blip.
      */
    Participant* creator() const { return m_creator; }
    /**
      * The position in the conversation document where this blip is located.
      */
    void setConversationStartIndex( int index ) { m_convStartIndex = index; }
    void setConversationEndIndex( int index ) { m_convEndIndex = index; }
    /**
      * @internal
      */
    void clearThreadList();
    /**
      * @internal
      */
    void addThread(BlipThread* thread);

    int childBlipCount() const;
    int unreadChildBlipCount() const;
    bool isUnread() const { return m_unread; }
    void setUnread( bool unread );
    void setChildrenUnread(bool unread);

    void print_(int indent);

    void receive( const DocumentMutation& mutation, const QString& author );

    void createFollowUpBlip();
    void createReplyBlip();
    /**
      * Inserts an image into the blip, i.e. it inserts am <image> and <caption> tag. Before calling this function you must add the image itself
      * as an attachmen to the wavelet.
      */
    void insertImage(int index, const QString& attachmentId, const QString& caption);
    /**
      * Inserts a <gadget> tag into the blip.
      */
    void insertGadget( int index, const QUrl& url );
    /**
      * Inserts a <state> tag into the blip.
      */
    void insertGadgetState( int gadgetIndex, const QString& name, const QString& value );

    /**
      * @return the index in the BlipDocument where the gadget with the required id is or -1.
      */
    int gadgetIndex(const QString& gadgetId) const;

signals:
    /**
      * Emitted when a mutation has been applied to the blip.
      * This signal is therefore connected to the GUI to show the update.
      *
      * A more fine granular way of receiving these updates is to hook into the
      * signals emitted by the blip's BlipDocument.
      */
    void update( const DocumentMutation& mutation );
    void unreadChanged();

private:
    void setup();

    /**
      * Usually of the form b+XYZ
      */
    QString m_id;
    /**
      * The contents of the blip.
      */
    BlipDocument* m_doc;
    /**
      * The user who created the blip.
      */
    Participant* m_creator;
    /**
      * A ordered list of all thread belonging to the blip.
      */
    QList<BlipThread*> m_threads;
    /**
     * The position in the wavelet's conversation document where the blip start element is located.
     */
    int m_convStartIndex;
    /**
     * The position in the wavelet's conversation document where the blip end element is located.
     */
    int m_convEndIndex;
    bool m_unread;
};

#endif // BLIP_H
