#ifndef BLIP_H
#define BLIP_H

#include <QObject>
#include <QList>

class BlipThread;
class Wavelet;
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
    Blip(Wavelet* wavelet, const QString& id, Participant* creator, StructuredDocument* doc);
    Blip(BlipThread* thread, const QString& id, Participant* creator, StructuredDocument* doc);

    /**
      * A ordered list of all thread belonging to the blip.
      */
    const QList<BlipThread*>& threads() const { return m_threads; }
    QString id() const { return this->m_id; }
    StructuredDocument* document() { return m_doc; }
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
    const QList<Participant*>& authors() const;
    /**
      * The user who created the blip.
      */
    Participant* creator() const { return m_creator; }

    /**
      * @internal
      */
    void clearThreadList();
    /**
      * @internal
      */
    void addThread(BlipThread* thread);

    void print_(int indent);

    void receive( const DocumentMutation& mutation );

signals:
    /**
      * Emitted when a mutation has been applied to the blip.
      * This signal is therefore connected to the GUI to show the update.
      */
    void update( const DocumentMutation& mutation );

private:
    void setup();

    QString m_id;
    StructuredDocument* m_doc;
    /**
      * Resembles the authors as given by the contributor tags.
      * Enlisting an author here is not mandatory. The list can even be empty.
      */
    QList<Participant*> m_authors;
    /**
      * The user who created the blip.
      */
    Participant* m_creator;
    /**
      * A ordered list of all thread belonging to the blip.
      */
    QList<BlipThread*> m_threads;
};

#endif // BLIP_H
