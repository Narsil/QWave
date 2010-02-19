#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "actor.h"
#include <QTimer>
#include <QString>

class MyMessage : public IMessage
{
public:
    MyMessage( const ActorId& actorId, const QString& str ) : IMessage( actorId ), m_str(str) { }

    QString m_str;
};

class OtherMessage : public IMessage
{
};

class Example : public Actor
{
public:
    Example(const QString& id, ActorGroup* group);

    virtual void execute();

private:
    int i;
    int k;
    QTimer* timer;
};

#endif // EXAMPLE_H
