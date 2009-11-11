#ifndef BLIPTHREAD_H
#define BLIPTHREAD_H

#include <QObject>
#include <QList>

class Blip;
class Wavelet;

class BlipThread : public QObject
{
    Q_OBJECT
public:
    BlipThread(Blip* blip, const QString& id);

    QString id() const { return this->m_id; }
    Blip* parentBlip() const;
    Wavelet* wavelet() const;
    const QList<Blip*>& blips() const { return m_blips; }

    int blipCount() const;
    int unreadBlipCount() const;

    /**
      * @internal
      */
    void addBlip(Blip* blip);
    /**
      * @internal
      */
    void clearBlipList();

    void print_(int indent);

private:
    QString m_id;
    QList<Blip*> m_blips;
};

#endif // THREAD_H
