#include <QtCore/QCoreApplication>
#include "example.h"
#include "actorgroup.h"
#include "actorfolk.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ActorFolk* folk = new ActorFolk("myfolk", &a );
    ActorGroup* group = new ActorGroup( "mygroup", folk );
    Example* e = new Example( "myactor", group );

    return a.exec();
}
