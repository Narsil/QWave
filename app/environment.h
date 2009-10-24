#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QObject>
#include <QHash>
#include <QString>

class Wave;
class Wavelet;
class Participant;
class NetworkAdapter;
class WaveList;

class Environment : public QObject
{
public:
    Environment(const QString& address, const QString& name);

    Participant* localUser() const;
    NetworkAdapter* networkAdapter() const;
    Wavelet* wavelet( const QString& id ) const { return m_wavelets[id]; }
    WaveList* inbox() const { return m_inbox; }

    Wave* wave( const QString& id );
    Wave* createWave( const QString& id );

    void addWavelet( Wavelet* wavelet );
    void removeWavelet( Wavelet* wavelet );

private:
    Participant* m_localUser;
    NetworkAdapter* m_networkAdapter;
    QHash<QString,Wavelet*> m_wavelets;
    QHash<QString,Wave*> m_waves;
    WaveList* m_inbox;
};

#endif // ENVIRONMENT_H
