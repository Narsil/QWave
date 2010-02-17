#include "wavelist.h"
#include "wave.h"

WaveList::WaveList(QObject* parent)
        : QObject(parent)
{
}

void WaveList::addWave( Wave* wave )
{
    m_waves.append( wave );
    emit waveAdded(wave);
}

void WaveList::removeWave( Wave* wave )
{
    m_waves.removeAll(wave);
    emit waveRemoved(wave);
}

bool WaveList::contains( Wave* wave )
{
    return m_waves.contains(wave);
}
