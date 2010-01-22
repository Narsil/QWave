#include "settings.h"
#include <QVariant>

Settings* Settings::s_settings = 0;

Settings::Settings(const QString& profile, QObject* parent)
        : QSettings( "T.Weis", profile, parent)
{
}

Settings* Settings::settings()
{
    if ( !s_settings )
        s_settings = new Settings( "QWaveServer" );
    return s_settings;
}

void Settings::setLogFile( const QString& logfile )
{
    setValue( "logFile", QVariant( logfile ) );
}

QString Settings::logFile() const
{
    return value("logFile", QVariant("commit.log") ).toString();
}

void Settings::setDomain( const QString& domain )
{
    setValue( "domain", QVariant( domain ) );
}

QString Settings::domain() const
{
    return value("domain", QVariant("localhost") ).toString();
}

void Settings::setPort( int port )
{
    setValue( "serverPort", QVariant( port ) );
}

int Settings::port() const
{
    return value("serverPort", QVariant((int)9876) ).toInt();
}

