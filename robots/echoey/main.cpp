#include <QCoreApplication>
#include "app/environment.h"
#include "model/participant.h"
#include "echoey.h"
#include <QDebug>

int main(int argc, char ** argv)
{
    QCoreApplication a(argc,argv);
    QString profile = "./echoey.conf";
    Environment* en = new Environment(profile);
    qDebug()<<"Starting echoey as "<<en->localUser()->address();
    Echoey echoey(en);
    return a.exec();
}
