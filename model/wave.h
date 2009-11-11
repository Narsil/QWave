#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QString>
#include <QDateTime>

class Wavelet;
class Environment;
class WaveDigest;

class Wave : public QObject
{
    Q_OBJECT
public:    
    /**
      * Do not create a wave directly. Instead use Environment::createWave.
      */
    Wave(Environment* environment, const QString& domain, const QString &id);

    QString id() const { return this->m_id; }
    QString domain() const { return this->m_domain; }

    WaveDigest* digest() const { return m_digest; }

    Wavelet* wavelet() const;
    Wavelet* wavelet(const QString& id) const;
    Environment* environment() const;

    QDateTime lastChange() const { return m_lastChange; }
    void setLastChange();
    int blipCount() const;
    int unreadBlipCount() const;

signals:
    /**
      * The number of blips or the number or read/unread blips changed
      *
      * Consumed by the UI.
      */
    void blipCountChanged();
    /**
      * Consumed by the UI.
      */
    void dateChanged();

private:
    QString m_id;
    QString m_domain;
    WaveDigest* m_digest;
    QDateTime m_lastChange;
};

#endif // WAVE_H
