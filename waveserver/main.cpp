#include <QCoreApplication>
#include "network/serversocket.h"
#include "network/xmppcomponent.h"
#include "app/settings.h"
#include "persistence/commitlog.h"

#include "model/waveletdelta.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    QString base64 = "CvwBClAKGAgGEhRY+1JsoC5PoOP7mjl5Ia6ocbP1MhIWdDFAd2F2ZTEudnMudW5pLWR1ZS5kZRocChp0b3JiZW5Ad2F2ZTIudnMudW5pLWR1ZS5kZRKnAQqAAaXU++prW9qyJ7x4VQg0ssxqISXOTzk9TqYxVVz0Dz80vaR71LtDfDAiRvLUlZTs4H1t8qAjPCkER+TkueBCiptRBY8OIM2BchfEZefr5RgCkMx6/T0poY+y+UM3ZwjeaLOEr4g2SCxYNZ3M2sYljNTE0ExYWp+2pGD23r1TtbbDEiAyjlXsmK8rOC9NDxmO/GflkIrzMYXTtZOcRXBiU7phNxgBEhgIBhIUWPtSbKAuT6Dj+5o5eSGuqHGz9TIYASCN1eDe5SQ=";
//    QString base64 = "YG48CFlHNDhDRmxITkRoRFJteElUa1JvUkZKdGVFbFVhMUp2VWtaS2RHVkZiRlZoTVVwMlZXdGFTMlJIVmtaaVJsWm9UVlZ3TWxaWGRHRlRNbEpJVm10YWFWSnNXbTlVVmxaM1RXeGFXR1I=";
//    QString base64 = "CjQICBIwd2F2ZTovL3dhdmUxLnZzLnVuaS1kdWUuZGUvdysxOTI0MDE2MjgvY29udityb290Ehp0b3JiZW5Ad2F2ZTEudnMudW5pLWR1ZS5kZRoXGhUKBGIrYjESDQoCKAkKAxIBMwoCKAE=";
//    WaveletDelta delta = WaveletDelta::fromBase64(base64);

    // Get the settings
    QString profile = "./waveserver.conf";
    if ( argc == 2 )
        profile = QString(argv[1]);
    Settings settings( profile );

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

    // Recover by reading the commit log
    bool ok = CommitLog::commitLog()->applyAll();
    if( !ok )
        qDebug("FAILED reading and applying commit log");

    // Listen to clients
    ServerSocket socket;

    // Loop
    return a.exec();
}
