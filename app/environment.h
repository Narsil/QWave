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

/**
  * This class is an anchor for the most important objects in a QWave environment.
  */
class Environment : public QObject
{
public:
    /**
      * @param profile is the name for the instance of QWave.
      *                The name determines the settings object.
      *                This way QWave can be started with different settings.
      */
    Environment(const QString& profile);

    /**
      * Invoke after the settings are correct.
      */
    void configure();

    Participant* localUser() const;
    NetworkAdapter* networkAdapter() const;
    /**
      * Searches for an existing wavelet object.
      * If this function returns 0 this does not imply that such a wavelet does not exist.
      */
    Wavelet* wavelet( const QString& url ) const { return m_wavelets[url]; }
    /**
      * The returned list is always automatically populated with all waves in
      * the user's inbox.
      *
      * The population of this list is done via NetworkAdapter.
      */
    WaveList* inbox() const { return m_inbox; }
    Contacts* contacts() const { return m_contacts; }
    Settings* settings() const { return m_settings; }

    /**
      * Looksup a wave.
      *
      * @return the wave or 0 if the wave has not yet been created.
      */
    Wave* wave( const QString& domain, const QString& id ) const;
    /**
      * Creates a new wave and registers it. Thus, it can be lookedup later on by
      * querying for its id and domain.
      * @see wave
      */
    Wave* createWave( const QString& id );
    /**
      * In this version of the function you can choose the domain, but
      * there are little sensible reasons for doing so.
      * This is only required internally.
      */
    Wave* createWave( const QString& domain, const QString& id );

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
