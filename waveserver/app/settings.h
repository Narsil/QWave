#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings(const QString& profile, QObject* parent = 0);

    /**
      * @returns the settings object created first, or creates a new default settings object.
      */
    static Settings* settings();

    void setLogFile( const QString& logfile );
    QString logFile() const;

    void setDomain( const QString& domain );
    QString domain() const;

    void setPort( int port );
    int port() const;

private:
    static Settings* s_settings;
};

#endif // SETTINGS_H
