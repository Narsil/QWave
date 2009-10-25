#ifndef RPC_H
#define RPC_H

#include <QObject>
#include <QByteArray>
#include <QAbstractSocket>

class QTcpSocket;

class RPC : public QObject
{
    Q_OBJECT
public:
    RPC(QObject* parent = 0);
    ~RPC();

    void open(const QString& host, quint16 port);

    void send(const QString& methodName, const char* data, quint32 size);

signals:
    void messageReceived(const QString& methodName, const QByteArray& data);
    void online();
    void offline();

private slots:
    void start();
    void stop();
    void stopOnError(QAbstractSocket::SocketError);
    void readBytes();

private:
    bool readBytesIntern();
    quint32 decodeVarint(const char* ptr, quint32 maxSize, int& consumed);
    void encodeVarint(char* ptr, quint32 value, int &offset );

    union IntUnion
    {
        quint32 i;
        char str[4];
    };

    int m_counter;
    IntUnion m_length;
    int m_lengthCompleted;
    QByteArray m_buffer;
    quint32 m_bufferLen;
    QTcpSocket* m_socket;
};

#endif // RPC_H
