#include "waveletdelta.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"

WaveletDelta::WaveletDelta()
{
}

WaveletDelta::WaveletDelta(const WaveletDelta& delta)
        : m_version( delta.m_version ),
          m_author( delta.m_author ),
          m_operations( delta.m_operations )
{
}

WaveletDelta::~WaveletDelta()
{
}

void WaveletDelta::addOperation(const WaveletDeltaOperation& op)
{
    m_operations.append(op);
}

QByteArray WaveletDelta::toBinary() const
{
    protocol::ProtocolWaveletDelta delta;
    Converter::convert( &delta, *this );

    QByteArray ba;
    ba.resize( delta.ByteSize() );
    delta.SerializeToArray( ba.data(), ba.count() );
    return ba;
}

QString WaveletDelta::toBase64() const
{
    protocol::ProtocolWaveletDelta delta;
    Converter::convert( &delta, *this );

    QByteArray ba;
    ba.resize( delta.ByteSize() );
    delta.SerializeToArray( ba.data(), ba.count() );

    QByteArray base64 = ba.toBase64();
    return QString::fromAscii( base64.constData(), base64.length() );
}

WaveletDelta WaveletDelta::fromBinary(const QByteArray& data)
{
    protocol::ProtocolWaveletDelta delta;
    delta.ParseFromArray( data.constData(), data.length() );

    return Converter::convert( delta );
}

WaveletDelta WaveletDelta::fromBase64(const QString& base64)
{
    QByteArray ascii = base64.toAscii();
    QByteArray data = QByteArray::fromBase64( ascii );

//    qDebug("Decode %i bytes", data.length() );
//
//    QFile file("out.data");
//    file.open( QFile::ReadWrite );
//    file.write( data );
//    file.close();

    protocol::ProtocolWaveletDelta delta;
    delta.ParseFromArray( data.constData(), data.length() );

    return Converter::convert( delta );
}
