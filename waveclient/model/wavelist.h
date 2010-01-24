#ifndef WAVELIST_H
#define WAVELIST_H

#include <QObject>
#include <QList>

class Wave;

class WaveList : public QObject
{
    Q_OBJECT
public:
    WaveList(QObject* parent = 0);

    void addWave( Wave* wave );
    void removeWave( Wave* wave );
    bool contains( Wave* wave );
    QList<Wave*>::const_iterator begin() const { return m_waves.begin(); }
    QList<Wave*>::const_iterator end() const { return m_waves.end(); }
    const QList<Wave*>& waves() const { return m_waves; }

signals:
    void waveAdded( Wave* wave );
    void waveRemoved( Wave* wave );

private:
    QList<Wave*> m_waves;
};

#endif // WAVELIST_H
