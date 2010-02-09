#include "waveletdelta.h"
#include "network/converter.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"

WaveletDelta::WaveletDelta()
{
    m_null = true;
}

WaveletDelta::WaveletDelta(const WaveletDelta& delta)
        : m_null( delta.m_null ),
          m_version( delta.m_version ),
          m_author( delta.m_author ),
          m_operations( delta.m_operations )
{
}

WaveletDelta::~WaveletDelta()
{
}

void WaveletDelta::addOperation(const WaveletDeltaOperation& op)
{
    m_null = false;
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

WaveletDelta WaveletDelta::fromBase64(const QString& base64, bool* ok)
{
    QByteArray ascii = base64.toAscii();
    QByteArray data = QByteArray::fromBase64( ascii );

    protocol::ProtocolWaveletDelta delta;
    bool result = delta.ParseFromArray( data.constData(), data.length() );
    if ( !result )
    {
        if ( ok )
            *ok = false;
        return WaveletDelta();
    }
    if ( ok )
        *ok = true;
    return Converter::convert( delta );
}
