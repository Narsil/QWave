#ifndef BLIPTHREAD_H
#define BLIPTHREAD_H

#include <QObject>

class Blip;
class Wavelet;

class BlipThread : public QObject
{
    Q_OBJECT
public:
    BlipThread(Blip* blip, const QString& id);

    QString id() const { return this->m_id; }
    Blip* parentBlip();
    Wavelet* wavelet();
    QList<Blip*> blips();

    void print_(int indent);

private:
    QString m_id;
};

#endif // THREAD_H
