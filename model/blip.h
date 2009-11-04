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

    void print_(int indent);

    void receive( const DocumentMutation& mutation, const QString& author );

    void createFollowUpBlip();
    void createReplyBlip();

signals:
    /**
      * Emitted when a mutation has been applied to the blip.
      * This signal is therefore connected to the GUI to show the update.
      */
    void update( const DocumentMutation& mutation );

private:
    void setup();

    QString m_id;
    BlipDocument* m_doc;
    /**
      * The user who created the blip.
      */
    Participant* m_creator;
    /**
      * A ordered list of all thread belonging to the blip.
      */
    QList<BlipThread*> m_threads;
    int m_convStartIndex;
    int m_convEndIndex;
};

#endif // BLIP_H
