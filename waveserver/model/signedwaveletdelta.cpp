#include "signedwaveletdelta.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "network/servercertificate.h"
#include "network/converter.h"
#include <openssl/sha.h>
#include <string>

SignedWaveletDelta::SignedWaveletDelta()
{
}

SignedWaveletDelta::SignedWaveletDelta( const SignedWaveletDelta& delta )
        : m_delta( delta.m_delta ), m_deltaBytes( delta.m_deltaBytes ), m_signatures( delta.m_signatures )
{
}

//SignedWaveletDelta::SignedWaveletDelta( const WaveletDelta& delta )
//        : m_delta( delta )
//{
//}
//
SignedWaveletDelta::SignedWaveletDelta( const protocol::ProtocolWaveletDelta& delta )
{
    m_delta = Converter::convert( delta );
    m_deltaBytes.resize( delta.ByteSize() );
    delta.SerializeToArray( m_deltaBytes.data(), m_deltaBytes.count() );

    m_signatures.append( Signature( LocalServerCertificate::certificate()->sign(m_deltaBytes), LocalServerCertificate::certificate()->signerId() ) );
}

SignedWaveletDelta::SignedWaveletDelta( const protocol::ProtocolSignedDelta* signedDelta, bool* ok )
{
    const ::std::string& str = signedDelta->delta();
    m_deltaBytes = QByteArray( str.data(), str.length() );
    protocol::ProtocolWaveletDelta pdelta;
    bool result = pdelta.ParseFromArray( str.data(), str.length() );
    if ( !result )
    {
        if ( ok )
            *ok = false;
        return;
    }

    m_delta = Converter::convert( pdelta );
    for( int i = 0; i < signedDelta->signature_size(); ++i )
    {
        const protocol::ProtocolSignature& sig = signedDelta->signature(i);
        m_signatures.append( Signature( &sig ) );
    }

    if ( ok )
        *ok = true;
}

void SignedWaveletDelta::toProtobuf(protocol::ProtocolSignedDelta* signedDelta) const
{
    signedDelta->set_delta( m_deltaBytes.constData(), m_deltaBytes.length() );
    foreach( const Signature& sig, m_signatures )
    {
        protocol::ProtocolSignature* signature = signedDelta->add_signature();
        sig.toProtobuf( signature );
    }
}

QByteArray SignedWaveletDelta::toBinary() const
{
    protocol::ProtocolSignedDelta signedDelta;
    toProtobuf( &signedDelta );

    QByteArray ba2;
    ba2.resize( signedDelta.ByteSize() );
    signedDelta.SerializeToArray( ba2.data(), ba2.count() );
    return ba2;
}

QString SignedWaveletDelta::toBase64() const
{
    QByteArray base64 = toBinary().toBase64();
    QString str64 = QString::fromAscii( base64.constData(), base64.length() );
    return str64;
}

SignedWaveletDelta  SignedWaveletDelta::fromBase64( const QString& base64, bool* ok)
{
    QByteArray data = QByteArray::fromBase64( base64.toAscii() );

    protocol::ProtocolSignedDelta signedDelta;
    if ( !signedDelta.ParseFromArray( data.data(), data.length() ) )
    {
        if ( ok )
            *ok = false;
        return SignedWaveletDelta();
    }

    return SignedWaveletDelta( &signedDelta, ok );
}
