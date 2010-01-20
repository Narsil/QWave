#include <QtGui/QApplication>
#include "mainwindow.h"
#include "network/serversocket.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // TODO: This is a hard coded wave domain name
    ServerSocket socket("localhost");

    MainWindow w;
    w.show();
    return a.exec();
}
