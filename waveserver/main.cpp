#include <QCoreApplication>
#include "network/serversocket.h"
#include "app/settings.h"
#include "persistence/commitlog.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Get the settings
    QString profile = "QWaveServer";
    if ( argc == 2 )
        profile = QString(argv[1]);
    Settings settings( profile );

    // Recover by reading the commit log
    bool ok = CommitLog::commitLog()->applyAll();
    if( !ok )
        qDebug("FAILED reading and applying commit log");

    // Listen to clients
    ServerSocket socket;

    // Loop
    return a.exec();
}
