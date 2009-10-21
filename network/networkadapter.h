#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <QObject>

class DocumentMutation;
class Environment;

class NetworkAdapter : public QObject
{
public:
    NetworkAdapter(QObject* parent = 0);

    void send( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );
    void receive( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );

    Environment* environment() const;

private:
    static NetworkAdapter* s1;
    static NetworkAdapter* s2;
};

#endif // NETWORKADAPTER_H
