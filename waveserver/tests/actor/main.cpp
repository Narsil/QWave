#include <QtCore/QCoreApplication>
#include "example.h"
#include "actorgroup.h"
//#include <iostream>
//#include <typeinfo>

namespace Foo
{
class A
{
public:
    virtual void dummy() { }
};

class B : public A
{
};
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ActorGroup group;
    Example* e = new Example();
    group.addActor( e );

    return a.exec();

//    for( int i = 0; i < 40; ++i )
//    {
//        group.process( new MyMessage() );
//    }
//
//    return 1;
}
