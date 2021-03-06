#ifndef SIGNEDWAVELETDELTA_H
#define SIGNEDWAVELETDELTA_H

#include "model/waveletdelta.h"
#include "model/signature.h"
#include <QString>
#include <QByteArray>
#include <QList>

namespace protocol
{
    class ProtocolSignedDelta;
    class ProtocolWaveletDelta;
}

class SignedWaveletDelta
{
public:
    SignedWaveletDelta();
    SignedWaveletDelta( const protocol::ProtocolSignedDelta* signedDelta, bool* ok = 0 );
    SignedWaveletDelta( const SignedWaveletDelta& delta );
    /**
      * Creates a signed delta by signing the provided delta with the local server certificate.
      */
    SignedWaveletDelta( const protocol::ProtocolWaveletDelta& delta );
    /**
      * Creates a signed delta by signing the provided delta with the local server certificate.
      */
    SignedWaveletDelta( const WaveletDelta& delta );

    const WaveletDelta& delta() const { return m_delta; }
    const QList<Signature>& signatures() const { return m_signatures; }
    const QByteArray& deltaBytes() const { return m_deltaBytes; }

    void toProtobuf(protocol::ProtocolSignedDelta* signedDelta) const;
    QByteArray toBinary() const;
    QString toBase64() const;

    bool isNull() const { return m_null; }

    static SignedWaveletDelta fromBase64( const QString& base64, bool* ok = 0 );

private:
    bool m_null;
    WaveletDelta m_delta;
    QByteArray m_deltaBytes;
    QList<Signature> m_signatures;
};

#endif // SIGNEDWAVELETDELTA_H
