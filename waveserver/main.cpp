#include <QCoreApplication>
#include "network/serversocket.h"
#include "network/xmppcomponentconnection.h"
#include "network/servercertificate.h"
#include "app/settings.h"
#include "persistence/storefolk.h"
#include "model/wavefolk.h"
#include "fcgi/fcgiserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Get the settings
    QString profile = "./waveserver.conf";
    if ( argc == 2 )
        profile = QString(argv[1]);
    Settings settings( profile );

    // Enfore the creation of a server certificate
    if ( Settings::settings()->certificateFile().isEmpty() )
    {
        qDebug("You must specify a server certificate file.");
        return 2;
    }
    if ( Settings::settings()->privateKeyFile().isEmpty() )
    {
        qDebug("You must specify a server certificate private key file.");
        return 2;
    }
    // Enfore the loading of the certificate and private key
    LocalServerCertificate::certificate();

    // Instantiate the store
    StoreFolk::store();
    // Instantiate the wave folk
    WaveFolk::instance();

    if ( Settings::settings()->federationEnabled() )
    {
        // Check that XMPP parameters are set correctly
        if ( Settings::settings()->xmppComponentPort() == 0 )
        {
            qDebug("You must specify a xmpp component port.");
            return 2;
        }
        if ( Settings::settings()->xmppServerName().isEmpty() )
        {
            qDebug("You must specify a xmpp server name.");
            return 2;
        }
        if ( Settings::settings()->domain().isEmpty() )
        {
            qDebug("You must specify a domain name.");
            return 2;
        }
        if ( Settings::settings()->xmppComponentName().isEmpty() )
        {
            qDebug("You must specify a xmpp component name.");
            return 2;
        }
        if ( Settings::settings()->xmppComponentSecret().isEmpty() )
        {
            qDebug("You must specify a xmpp component secret.");
            return 2;
        }

        // Connect to the XMPP component
        new XmppComponentConnection( &a );
    }

    if ( Settings::settings()->fcgiEnabled() )
        new FCGI::FCGIServer();

    // Listen to clients
    ServerSocket socket;

    qDebug("Waveserver started ...");

    // Loop
    return a.exec();
}
