#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <QByteArray>

namespace protocol
{
    class ProtocolSignature;
}

class Signature
{
public:
    Signature() { }
    Signature( const protocol::ProtocolSignature* signature );
    Signature( const Signature& signature ) : m_signature( signature.m_signature ), m_signerId( signature.m_signerId ) { }
    Signature( const QByteArray& signature, const QByteArray& signerId ) : m_signature( signature ), m_signerId( signerId ) { }

    QByteArray signature() const { return m_signature; }
    QByteArray signerId() const { return m_signerId; }

    void toProtobuf(protocol::ProtocolSignature* signature) const;

private:
    QByteArray m_signature;
    QByteArray m_signerId;

};

#endif // SIGNATURE_H
