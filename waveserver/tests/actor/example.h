#ifndef EXAMPLE_H
#define EXAMPLE_H

#include "actor.h"
#include <QTimer>
#include <QString>

class MyMessage : public IMessage
{
public:
    MyMessage( const QString& str ) : m_str(str) { }

    QString m_str;
};

class OtherMessage : public IMessage
{
};

class Example : public Actor
{
public:
    Example();

    virtual void EXECUTE();

private:
    int i;
    int k;
    QTimer* timer;
};

#endif // EXAMPLE_H
