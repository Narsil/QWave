#ifndef WAVEFOLK_H
#define WAVEFOLK_H

#include <QHash>

#include "actor/actorfolk.h"
#include "wave.h"

class WaveFolk : public ActorFolk
{
public:
    WaveFolk(QObject* parent = 0);

    Wave* wave( const QString& domain, const QString& id, bool createOnDemand );

    static WaveFolk* instance();

protected:
    virtual ActorGroup* group( const QString& id, bool createOnDemand );

private:
    QHash<QString,Wave*> m_waves;

    static WaveFolk* s_folk;
};

#endif // WAVEFOLK_H
