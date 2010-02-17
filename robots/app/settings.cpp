#include "settings.h"

Settings::Settings(const QString& profile, QObject* parent)
    : QSettings( profile, QSettings::NativeFormat, parent)
{
}

void Settings::setServerName( const QString& serverName )
{
    setValue( "server/serverName", QVariant( serverName ) );
}

QString Settings::serverName() const
{
    return value("server/serverName", QVariant("") ).toString();
}

void Settings::setServerPort(int port)
{
    setValue( "serverPort", QVariant( port ) );
}

int Settings::serverPort() const
{
    return value("serverPort", QVariant((int)9876) ).toInt();
}

void Settings::setUserAddress( const QString& userAddress )
{
    setValue( "server/userAddress", QVariant( userAddress ) );
}

QString Settings::userAddress() const
{
    return value("server/userAddress", QVariant("") ).toString();
}

void Settings::setUserName( const QString& userName )
{
    setValue( "server/userName", QVariant( userName ) );
}

QString Settings::userName() const
{
    return value("server/userName", QVariant("") ).toString();
}

void Settings::setPassword( const QString& password )
{
    setValue( "server/password", QVariant( password ) );
}

QString Settings::password() const
{
    return value("server/password", QVariant("") ).toString();
}

bool Settings::isConfigured() const
{
    return this->serverPort() > 0 && !this->serverName().isEmpty() && !this->userName().isEmpty() && !this->userAddress().isEmpty();
}

QString Settings::waveDomain() const
{
    QString jid = userAddress();
    qDebug("JID=%s", jid.toAscii().constData());
    int index = jid.indexOf('@');
    if ( index == -1 )
        return QString::null;
    return jid.mid( index + 1 );
}
