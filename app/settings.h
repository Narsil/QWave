#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings(QObject* parent = 0);

    void setServerName( const QString& serverName );
    QString serverName() const;
    void setServerPort(int port);
    int serverPort() const;
    void setUserAddress( const QString& userAddress );
    QString userAddress() const;
    void setUserName( const QString& userName );
    QString userName() const;
    void setPassword( const QString& password );
    QString password() const;

    bool isConfigured() const;
};

#endif // SETTINGS_H
