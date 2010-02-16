#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QHash>
#include <QString>
#include "wavelet.h"
#include "actor/actorgroup.h"

class WaveFolk;

class Wave : public ActorGroup
{
public:
    /**
      * @internal
      *
      * Use WaveFolk to instantiate new waves.
      */
    Wave( WaveFolk* folk, const QString& domain, const QString& id );
    ~Wave();

    QString domain() const { return m_domain; }
    QString id() const { return m_id; }

    Wavelet* wavelet( const QString& domain, const QString& id, bool create = false );

    // TBR
    static Wave* wave(const QString& domain, const QString& id, bool create = false);

protected:
    virtual ActorGroup* group( const QString& groupId, bool createOnDemand );

private:
    QString m_domain;
    QString m_id;
};

#endif // WAVE_H
