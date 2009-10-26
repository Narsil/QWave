#ifndef BLIP_H
#define BLIP_H

#include <QObject>
#include <QList>

class BlipThread;
class Wavelet;
class SynchronizedDocument;
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
    SynchronizedDocument* document() { return m_doc; }
    BlipThread* parentThread() const;
    Wavelet* wavelet() const;
    Environment* environment() const;
    bool isRootBlip() const;
    bool isFirstRootBlip() const;
    bool isLastBlipInThread() const;
    QList<Participant*> authors() const;

    void print_(int indent);

    void receive( const DocumentMutation& mutation );

signals:
    void update( const DocumentMutation& mutation );

private:
    void setup();

    QString m_id;
    SynchronizedDocument* m_doc;
};

#endif // BLIP_H
