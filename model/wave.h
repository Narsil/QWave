#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QString>

class Wavelet;
class Environment;

class Wave : public QObject
{
    Q_OBJECT
public:    
    Wave(Environment* environment, const QString& domain, const QString &id);

    QString id() const { return this->m_id; }
    QString domain() const { return this->m_domain; }
    QString digest() const { return m_digest; }
    void setDigest(const QString& digest);

    Wavelet* wavelet() const;
    Environment* environment() const;

signals:
    void digestChanged();

private:
    QString m_id;
    QString m_domain;
    QString m_digest;
};

#endif // WAVE_H
