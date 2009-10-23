#include "rpc.h"

#include <QTcpSocket>

RPC::RPC(QObject* parent)
        : QObject( parent ), m_counter(0), m_lengthCompleted(0), m_bufferLen(0)
{
    m_socket = 0;
}

RPC::~RPC()
{
}

void RPC::open(const QString& host, quint16 port)
{
    if ( m_socket )
        delete m_socket;

    m_socket = new QTcpSocket(this);

    connect( m_socket, SIGNAL(connected()), SLOT(start()));
    connect( m_socket, SIGNAL(disconnected()), SLOT(stop()));
    connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(stopOnError(QAbstractSocket::SocketError)));
    connect( m_socket, SIGNAL(readyRead()), SLOT(readBytes()));

    m_socket->connectToHost(host, port);
}

void RPC::send(const QString& methodName, const char* data, quint32 size)
{
    QAbstractSocket* dev = m_socket;

    // Attention: This code works only on CPUs with little endian encoding
    IntUnion u;
    // Size of the message
    u.i = 1 + 1 + methodName.length() + 1 + size;
    dev->write(u.str, 4);
    u.i = ++m_counter;
    // TODO: Varint
    dev->write(u.str, 1);
    u.i = methodName.length();
    // TODO: Varint
    dev->write(u.str, 1);
    dev->write(methodName.toAscii().constData(), methodName.length());
    u.i = methodName.length();
    // TODO: Varint
    dev->write(u.str, 1);
    dev->write(data, size);
    dev->flush();
}

void RPC::start()
{
    emit online();
    qDebug("CONNECT");
}

void RPC::stop()
{
    emit offline();
    qDebug("DISCONNECT");
}

void RPC::stopOnError(QAbstractSocket::SocketError)
{
    emit offline();
    qDebug("SOCKET Error");
}

void RPC::readBytes()
{
//    qDebug("READ");
    while( readBytesIntern() ) { }
}

bool RPC::readBytesIntern()
{
//    qDebug("readyRead %i", (int)m_socket->bytesAvailable() );
    if ( m_lengthCompleted != 4 )
    {
        qint64 len = m_socket->read( m_length.str + m_lengthCompleted, 4 - m_lengthCompleted );
        if ( len == -1 )
            return false;
        m_lengthCompleted += len;

        if ( m_lengthCompleted != 4 )
            return false;
    }

//    qDebug("Got len of %i", m_length.i);

    QByteArray arr = m_socket->read( m_length.i - m_bufferLen);
    if ( arr.isEmpty() )
        return false;
    m_bufferLen += arr.length();
    if ( m_buffer.isEmpty() )
    {
        m_buffer = arr;
        arr = QByteArray();
    }
    else    
        m_buffer.append(arr);    

    if ( m_bufferLen != m_length.i )
        return false;

    qDebug("Got message of len %i", m_length.i);

    if ( m_buffer.length() < 4 )
    {
        qDebug("Malformed message");
        m_socket->close();
        return false;
    }

    int offset = 0;
    const char* ptr = m_buffer.constData();
    quint32 number = decodeVarint(ptr, m_bufferLen - offset, offset);
    if ( offset == -1 )
    {
        qDebug("Malformed number varint");
        m_socket->close();
        return false;
    }
    qDebug("Got message number %i", number);

    quint32 strlen = decodeVarint(ptr, m_bufferLen - offset, offset);
    if ( offset == -1 )
    {
        qDebug("Malformed strlen varint");
        m_socket->close();
        return false;
    }
    qDebug("Got message strlen %i", strlen);

    QString methodName = QString::fromAscii(ptr + offset, strlen);
    qDebug("Got message: %s", methodName.toAscii().constBegin() );
    offset += strlen;

    quint32 payload = decodeVarint(ptr, m_bufferLen - offset, offset);
    if ( offset == -1 )
    {
        qDebug("Malformed payload varint");
        m_socket->close();
        return false;
    }

    if ( (int)payload != m_buffer.length() - offset )
    {
        qDebug("Malformed message 2");
        m_socket->close();
        return false;
    }

    qDebug("Got message with payload size %i", payload);

    // Message is complete
    emit messageReceived( methodName, QByteArray::fromRawData(ptr + offset, payload));

    // Prepare for new message
    m_buffer.clear();
    m_bufferLen = 0;
    m_lengthCompleted = 0;

    return true;
}

quint32 RPC::decodeVarint(const char* charptr, quint32 maxSize, int &consumed)
{
    const unsigned char* ptr = (const unsigned char*)charptr;
    quint32 result = 0;
    for( int i = 0; i < (int)qMin(maxSize, (quint32)5); i++, consumed++ )
    {
        if ( ( ptr[consumed] & 0x80 ) == 0 )
        {
            result |= (0x7f & ptr[consumed++]) << i * 7;
            return result;
        }
        result |= (0x7f & ptr[consumed]) << i * 7;
    }
    consumed = -1;
    return 0;
}
