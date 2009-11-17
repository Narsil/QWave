#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings(const QString& profile, QObject* parent = 0);

    void setServerName( const QString& serverName );
    QString serverName() const;
    void setServerPort(int port);
    int serverPort() const;
    void setUserAddress( const QString& userAddress );
    /**
      * @return the JID of the user, i.e. user@host.
      */
    QString userAddress() const;
    void setUserName( const QString& userName );
    /**
      * @return the display name of the user.
      */
    QString userName() const;
    void setPassword( const QString& password );
    QString password() const;

    QString waveDomain() const;

    bool isConfigured() const;
};

#endif // SETTINGS_H
