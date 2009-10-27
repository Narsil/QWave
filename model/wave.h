#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QString>

class Wavelet;
class Environment;
class WaveDigest;

class Wave : public QObject
{
    Q_OBJECT
public:    
    Wave(Environment* environment, const QString& domain, const QString &id);

    QString id() const { return this->m_id; }
    QString domain() const { return this->m_domain; }

    WaveDigest* digest() const { return m_digest; }

    Wavelet* wavelet() const;
    Wavelet* wavelet(const QString& id) const;
    Environment* environment() const;

private:
    QString m_id;
    QString m_domain;
    WaveDigest* m_digest;
};

#endif // WAVE_H
