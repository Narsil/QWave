#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QHash>
#include <QString>
#include "wavelet.h"

class Wave : public QObject
{
public:
    Wave( const QString& domain, const QString& id );
    ~Wave();

    QString domain() const { return m_domain; }
    QString id() const { return m_id; }

    Wavelet* wavelet( const QString& domain, const QString& id, bool create = false );

    static Wave* wave(const QString& domain, const QString& id, bool create = false);

private:
    /**
      * The key is of the form "waveletDomain!waveletId"
      */
    QHash<QString,Wavelet*> m_wavelets;
    QString m_domain;
    QString m_id;

    static QHash<QString,Wave*>* s_waves;
};

#endif // WAVE_H
