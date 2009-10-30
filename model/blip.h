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
    Blip(Wavelet* wavelet, const QString& id);
    Blip(BlipThread* thread, const QString& id);

    QList<BlipThread*> threads() const;
    QString id() const { return this->m_id; }
    StructuredDocument* document() { return m_doc; }
    BlipThread* parentThread() const;
    Wavelet* wavelet() const;
    Environment* environment() const;
    bool isRootBlip() const;
    bool isFirstRootBlip() const;
    bool isLastBlipInThread() const;
    const QList<Participant*>& authors() const;

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
    QList<Participant*> m_authors;
};

#endif // BLIP_H
