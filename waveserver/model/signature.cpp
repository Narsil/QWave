#include "signature.h"
#include "protocol/common.pb.h"

Signature::Signature( const protocol::ProtocolSignature* signature )
{
    m_signature = QByteArray( signature->signature_bytes().data(), signature->signature_bytes().length() );
    m_signerId = QByteArray( signature->signer_id().data(), signature->signer_id().length() );
}

void Signature::toProtobuf(protocol::ProtocolSignature* signature) const
{
    signature->set_signature_algorithm( protocol::ProtocolSignature_SignatureAlgorithm_SHA1_RSA );
    signature->set_signature_bytes( m_signature.constData(), m_signature.length() );
    signature->set_signer_id( m_signerId.constData(), m_signerId.length() );
}
