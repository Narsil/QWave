#include "appliedwaveletdelta.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "network/servercertificate.h"
#include "network/converter.h"
#include "signature.h"
#include <openssl/sha.h>

AppliedWaveletDelta::AppliedWaveletDelta()
        : m_null(true), m_operationsApplied(0)
{
}

AppliedWaveletDelta::AppliedWaveletDelta( const AppliedWaveletDelta& delta )
        : m_null(delta.m_null), m_delta( delta.m_delta ), m_resultingVersion( delta.m_resultingVersion ), m_applicationTime( delta.m_applicationTime ), m_operationsApplied( delta.m_operationsApplied ), m_signature( delta.m_signature ), m_signerId( delta.m_signerId )
{
}

AppliedWaveletDelta::AppliedWaveletDelta( const WaveletDelta& delta, qint64 applicationTime, int operationsApplied, const Signature* signature )
        : m_null(false), m_delta( delta ), m_applicationTime( applicationTime ), m_operationsApplied( operationsApplied )
{
    if ( signature )
    {
        m_signature = signature->signature();
        m_signerId = signature->signerId();
    }
}

void AppliedWaveletDelta::toProtobuf(protocol::ProtocolAppliedWaveletDelta* appliedDelta) const
{
    protocol::ProtocolWaveletDelta delta;
    Converter::convert( &delta, m_delta);
    QByteArray ba;
    ba.resize( delta.ByteSize() );
    delta.SerializeToArray( ba.data(), ba.count() );

    appliedDelta->set_operations_applied( m_operationsApplied );
    appliedDelta->set_application_timestamp( m_applicationTime );

//    protocol::ProtocolHashedVersion* hashed = appliedDelta.mutable_hashed_version_applied_at();
//    hashed->set_version( waveletDelta.delta().version().version );
//    QByteArray hash = waveletDelta.delta().version().hash;
//    hashed->set_history_hash( hash.constData(), hash.length() );

    protocol::ProtocolSignedDelta* signedDelta = appliedDelta->mutable_signed_original_delta();
    signedDelta->set_delta( ba.constData(), ba.length() );
    protocol::ProtocolSignature* signature = signedDelta->add_signature();
    signature->set_signature_algorithm( protocol::ProtocolSignature_SignatureAlgorithm_SHA1_RSA );
    if ( m_signerId.isNull() )
    {
        QByteArray signerId = LocalServerCertificate::certificate()->signerId();
        signature->set_signer_id( signerId.constData(), signerId.length() );
    }
    else
        signature->set_signer_id( m_signerId.constData(), m_signerId.length() );
    if ( m_signature.isNull() )
        ((AppliedWaveletDelta*)this)->m_signature = LocalServerCertificate::certificate()->sign(ba);
    signature->set_signature_bytes( m_signature.constData(), m_signature.length() );
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

const WaveletDelta::HashedVersion& AppliedWaveletDelta::resultingVersion() const
{
    if ( !m_resultingVersion.hash.isNull() )
        return m_resultingVersion;

    QByteArray ba2 = toBinary();
    ba2.prepend( m_delta.version().hash );

    QByteArray hashBuffer( 32, 0 );
    SHA256( (const unsigned char*)ba2.constData(), ba2.length(), (unsigned char*)hashBuffer.data() );
    // Copy over the first 20 bytes
    ((AppliedWaveletDelta*)this)->m_resultingVersion.hash.resize(20);
    for( int i = 0; i < 20; ++i )
        ((AppliedWaveletDelta*)this)->m_resultingVersion.hash.data()[i] = hashBuffer.data()[i];
    ((AppliedWaveletDelta*)this)->m_resultingVersion.version = m_delta.version().version + m_operationsApplied;
    return m_resultingVersion;
}
