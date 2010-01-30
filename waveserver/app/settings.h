#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings(const QString& filename, QObject* parent = 0);

    /**
      * @returns the settings object created first, or creates a new default settings object.
      */
    static Settings* settings();

    void setLogFile( const QString& logfile );
    QString logFile() const;

    void setDomain( const QString& domain );
    /**
      * Something like mycompany.com, i.e. the JID of your users is user@mycompany.com.
      */
    QString domain() const;

    void setClientPort( int port );
    /**
      * Usually "9876"
      */
    int clientPort() const;

    void setXmppComponentPort( int port );
    /**
      * The port mentioned here is used to connect to the component port the XMPP server.
      *
      * Usually "5275"
      */
    int xmppComponentPort() const;
    void setXmppServerName( const QString& domain );
    /**
      * The host mentioned here is used to connect to the component port the XMPP server.
      *
      * Usually "localhost" or the same as xmppComponentName.
      */
    QString xmppServerName() const;
    void setXmppComponentSecret( const QString& secret );
    /**
      * The shared secret between the wave server and the local XMPP server.
      */
    QString xmppComponentSecret() const;
    void setXmppComponentName( const QString& name );
    /**
      * Usually wave.{domain}.
      */
    QString xmppComponentName() const;

    bool federationEnabled() const;
    void setFederationEabled( bool enabled );

    QString certificateFile() const;
    void setCertificateFile( const QString& file );

    QString privateKeyFile() const;
    void setPrivateKeyFile( const QString& file );

private:
    static Settings* s_settings;
};

#endif // SETTINGS_H
