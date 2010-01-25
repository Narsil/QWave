#include "rpc.h"

#include <QTcpSocket>

RPC::RPC(QObject* parent)
        : QObject( parent ), m_counter(0), m_lengthCompleted(0), m_bufferLen(0)
{
    m_socket = 0;
}

RPC::RPC(QTcpSocket* socket, QObject* parent)
        : QObject( parent ), m_counter(0), m_lengthCompleted(0), m_bufferLen(0)
{
    m_socket = socket;
    connect( m_socket, SIGNAL(disconnected()), SLOT(stop()));
    connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(stopOnError(QAbstractSocket::SocketError)));
    connect( m_socket, SIGNAL(readyRead()), SLOT(readBytes()));
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
    QByteArray buffer( 100 + methodName.length() + size, 0 );
    char* ptr = buffer.data();
    // Skip the size. We will set it later
    int offset = 4;

    encodeVarint(ptr, ++m_counter, offset);
    encodeVarint(ptr, methodName.length(), offset);
    buffer.insert(offset, methodName);
    ptr = buffer.data();
    offset += methodName.length();
    encodeVarint(ptr, size, offset);
    buffer.insert( offset, QByteArray::fromRawData(data, size) );
    ptr = buffer.data();
    IntUnion u;
    u.i = offset - 4 + size;
    ptr[0] = u.str[0];
    ptr[1] = u.str[1];
    ptr[2] = u.str[2];
    ptr[3] = u.str[3];

    qDebug("Sending %i bytes, thereof %i payload", 4 + u.i, size);
    m_socket->write(ptr, 4 + u.i);
    m_socket->flush();

//    // Attention: This code works only on CPUs with little endian encoding
//    IntUnion u;
//    // Size of the message
//    u.i = 1 + 1 + methodName.length() + 1 + size;
//    dev->write(u.str, 4);
//
//    char buf[10];
//    int offset = 0;
//    encodeVarint(buf, ++m_counter, offset);
//    dev->write(buf, offset);
//
//    offset = 0;
//    encodeVarint(buf, methodName.length(), offset);
//    dev->write(buf, offset);
//
//    dev->write(methodName.toAscii().constData(), methodName.length());
//
//    offset = 0;
//    encodeVarint(buf, size, offset);
//    dev->write(buf, offset);
//
//    dev->write(data, size);
//    dev->flush();
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
    emit socketError();
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

void RPC::encodeVarint(char* ptr, quint32 value, int &offset )
{
    do
    {
        ptr[offset] = (value & 0x7f) | 0x80;
        value >>= 7;
        if ( value == 0 )
            break;
        offset++;
    } while( true );
    ptr[offset] = ptr[offset] & 0x7f;
    offset++;
}
