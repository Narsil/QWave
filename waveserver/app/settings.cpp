#include "settings.h"
#include <QVariant>

Settings* Settings::s_settings = 0;

Settings::Settings(const QString& filename, QObject* parent)
        : QSettings(filename, QSettings::NativeFormat, parent)
{
}

Settings* Settings::settings()
{
    if ( !s_settings )
        s_settings = new Settings( "./waveserver.conf" );
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

void Settings::setClientPort( int port )
{
    setValue( "clientPort", QVariant( port ) );
}

int Settings::clientPort() const
{
    return value("clientPort", QVariant((int)9876) ).toInt();
}

void Settings::setXmppComponentPort( int port )
{
    setValue( "xmppComponentPort", QVariant( port ) );
}

int Settings::xmppComponentPort() const
{
    return value("xmppComponentPort", QVariant((int)5275) ).toInt();
}

void Settings::setXmppServerName( const QString& domain )
{
    setValue( "xmppServerName", QVariant( domain ) );
}

QString Settings::xmppServerName() const
{
    QString name = value("xmppServerName", QVariant("localhost") ).toString();
    if ( name.isEmpty() )
        return xmppComponentName();
    return name;
}

void Settings::setXmppComponentSecret( const QString& secret )
{
    setValue( "xmppComponentSecret", QVariant( secret ) );
}

QString Settings::xmppComponentSecret() const
{
    return value("xmppComponentSecret", QVariant("") ).toString();
}

void Settings::setXmppComponentName( const QString& name )
{
    setValue( "xmppComponentName", QVariant( name ) );
}

QString Settings::xmppComponentName() const
{
    QString name = value("xmppComponentName", QVariant("") ).toString();
    if ( name.isEmpty() )
        return "wave." + domain();
    return name;
}

bool Settings::federationEnabled() const
{
    return value("federationEnabled", QVariant(false) ).toBool();
}

void Settings::setFederationEabled( bool enabled )
{
    setValue( "federationEnabled", QVariant( enabled ) );
}

QString Settings::certificateFile() const
{
    return value("certificateFile", QVariant("waveserver.crt") ).toString();
}

void Settings::setCertificateFile( const QString& file )
{
    setValue( "certificateFile", QVariant( file ) );
}
