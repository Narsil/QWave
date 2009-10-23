#ifndef RPC_H
#define RPC_H

#include <QObject>

class QTcpSocket;
class QByteArray;

class RPC : public QObject
{
    Q_OBJECT
public:
    RPC(const QString& host, quint16 port );
    ~RPC();

    void send(const QString& methodName, const char* data, quint64 size);

signals:
    void messageReceived(const QString& methodName, const QByteArray& data);

private slots:
    void start();
    void stop();
    void readBytes();

private:
    int m_counter;
    union
    {
        quint32 i;
        char[4] str;
    } m_length;

    QTcpSocket* m_socket;
};

#endif // RPC_H
