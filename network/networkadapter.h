#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <QObject>

class DocumentMutation;
class Environment;
class RPC;

class NetworkAdapter : public QObject
{
    Q_OBJECT
public:
    NetworkAdapter(QObject* parent = 0);

    void send( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );
    void receive( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );

    Environment* environment() const;

    void sendOpenWave();

private slots:
    void getOnline();
    void getOffline();

private:
    RPC* m_rpc;

    static NetworkAdapter* s1;
    static NetworkAdapter* s2;
};

#endif // NETWORKADAPTER_H
