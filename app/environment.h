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
class Contacts;
class Settings;

class Environment : public QObject
{
public:
    Environment();

    /**
      * Invoke after the settings are correct.
      */
    void configure();

    Participant* localUser() const;
    NetworkAdapter* networkAdapter() const;
    Wavelet* wavelet( const QString& id ) const { return m_wavelets[id]; }
    WaveList* inbox() const { return m_inbox; }
    Contacts* contacts() const { return m_contacts; }
    Settings* settings() const { return m_settings; }

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
    Contacts* m_contacts;
    Settings* m_settings;
};

#endif // ENVIRONMENT_H
