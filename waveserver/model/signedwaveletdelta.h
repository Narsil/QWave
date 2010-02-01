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
}

class SignedWaveletDelta
{
public:
    SignedWaveletDelta();
    SignedWaveletDelta( const protocol::ProtocolSignedDelta* signedDelta, bool* ok = 0 );
    SignedWaveletDelta( const SignedWaveletDelta& delta );
    SignedWaveletDelta( const WaveletDelta& delta );
    SignedWaveletDelta( const WaveletDelta& delta, const QList<Signature>& signatures );

    const WaveletDelta& delta() const { return m_delta; }
    QList<Signature> signatures() const { return m_signatures; }

    void toProtobuf(protocol::ProtocolSignedDelta* signedDelta) const;
    QByteArray toBinary() const;
    QString toBase64() const;
    
    static SignedWaveletDelta fromBase64( const QString& base64, bool* ok = 0 );

private:
    WaveletDelta m_delta;
    QList<Signature> m_signatures;
};

#endif // SIGNEDWAVELETDELTA_H
