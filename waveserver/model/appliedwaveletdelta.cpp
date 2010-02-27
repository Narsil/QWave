#include "appliedwaveletdelta.h"
#include "protocol/common.pb.h"
// #include "protocol/waveclient-rpc.pb.h"
// #include "network/servercertificate.h"
#include "network/converter.h"
#include "signature.h"
//#include <openssl/sha.h>

AppliedWaveletDelta::AppliedWaveletDelta()
        : m_null(true), m_operationsApplied(0)
{
}

AppliedWaveletDelta::AppliedWaveletDelta( const AppliedWaveletDelta& delta )
        : m_null(delta.m_null), m_signedDelta( delta.m_signedDelta ) /*, m_resultingVersion( delta.m_resultingVersion )*/ , m_applicationTime( delta.m_applicationTime ), m_operationsApplied( delta.m_operationsApplied ), m_appliedAt( delta.m_appliedAt ), m_transformedDelta( delta.m_transformedDelta )
{
}

AppliedWaveletDelta::AppliedWaveletDelta( const SignedWaveletDelta& delta, qint64 applicationTime, int operationsApplied )
        : m_null(false), m_signedDelta( delta ), m_applicationTime( applicationTime ), m_operationsApplied( operationsApplied )
{
}

AppliedWaveletDelta::AppliedWaveletDelta( const protocol::ProtocolAppliedWaveletDelta* protobufDelta, bool *ok )
        : m_null(false)
{
     m_operationsApplied = protobufDelta->operations_applied();
     m_applicationTime = protobufDelta->application_timestamp();

     if ( protobufDelta->has_hashed_version_applied_at() )
     {
         m_appliedAt.version = protobufDelta->hashed_version_applied_at().version();
         m_appliedAt.hash = QByteArray( protobufDelta->hashed_version_applied_at().history_hash().data(), protobufDelta->hashed_version_applied_at().history_hash().length() );
     }

     m_signedDelta = SignedWaveletDelta( &protobufDelta->signed_original_delta(), ok );
}

void AppliedWaveletDelta::toProtobuf(protocol::ProtocolAppliedWaveletDelta* appliedDelta) const
{
    appliedDelta->set_operations_applied( m_operationsApplied );
    appliedDelta->set_application_timestamp( m_applicationTime );

    if ( !m_appliedAt.isNull() )
    {
        protocol::ProtocolHashedVersion* hashed = appliedDelta->mutable_hashed_version_applied_at();
        hashed->set_version( m_appliedAt.version );
        hashed->set_history_hash( m_appliedAt.hash.constData(), m_appliedAt.hash.length() );
    }

    protocol::ProtocolSignedDelta* signedDelta = appliedDelta->mutable_signed_original_delta();
    m_signedDelta.toProtobuf( signedDelta );
}

QByteArray AppliedWaveletDelta::toBinary() const
{
    protocol::ProtocolAppliedWaveletDelta appliedDelta;
    toProtobuf( &appliedDelta );

    QByteArray ba2;
    ba2.resize( appliedDelta.ByteSize() );
    appliedDelta.SerializeToArray( ba2.data(), ba2.count() );
    return ba2;
}

QString AppliedWaveletDelta::toBase64() const
{
    QByteArray base64 = toBinary().toBase64();
    QString str64 = QString::fromAscii( base64.constData(), base64.length() );
    return str64;
}

AppliedWaveletDelta AppliedWaveletDelta::fromBase64( const QString& base64, bool* ok)
{
    QByteArray data = QByteArray::fromBase64( base64.toAscii() );

    protocol::ProtocolAppliedWaveletDelta appliedDelta;
    if ( !appliedDelta.ParseFromArray( data.data(), data.length() ) )
    {
        if ( ok )
            *ok = false;
        return AppliedWaveletDelta();
    }

    appliedDelta.PrintDebugString();

    return AppliedWaveletDelta( &appliedDelta, ok );
}

//const WaveletDelta::HashedVersion& AppliedWaveletDelta::resultingVersion() const
//{
//    if ( !m_resultingVersion.hash.isNull() )
//        return m_resultingVersion;
//
//    QByteArray ba2 = toBinary();
//    ba2.prepend( m_signedDelta.delta().version().hash );
//
//    QByteArray hashBuffer( 32, 0 );
//    SHA256( (const unsigned char*)ba2.constData(), ba2.length(), (unsigned char*)hashBuffer.data() );
//    // Copy over the first 20 bytes
//    ((AppliedWaveletDelta*)this)->m_resultingVersion.hash.resize(20);
//    for( int i = 0; i < 20; ++i )
//        ((AppliedWaveletDelta*)this)->m_resultingVersion.hash.data()[i] = hashBuffer.data()[i];
//    ((AppliedWaveletDelta*)this)->m_resultingVersion.version = m_signedDelta.delta().version().version + m_operationsApplied;
//    return m_resultingVersion;
//}

void AppliedWaveletDelta::setAppliedAt( qint64 version, const QByteArray& hash )
{
    m_appliedAt.hash = hash;
    m_appliedAt.version = version;
}
