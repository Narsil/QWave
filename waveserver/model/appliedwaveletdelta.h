#ifndef APPLIEDWAVELETDELTA_H
#define APPLIEDWAVELETDELTA_H

#include "model/waveletdelta.h"
#include "model/signedwaveletdelta.h"
#include <QString>
#include <QByteArray>

class Signature;

namespace protocol
{
    class ProtocolAppliedWaveletDelta;
}

class AppliedWaveletDelta
{
public:
    AppliedWaveletDelta();
    AppliedWaveletDelta( const AppliedWaveletDelta& delta );
    AppliedWaveletDelta( const SignedWaveletDelta& delta, qint64 applicationTime, int m_operationsApplied );
    AppliedWaveletDelta( const protocol::ProtocolAppliedWaveletDelta* protobufDelta, bool* ok );

    /**
      * This is the original signed delta.
      */
    const SignedWaveletDelta& signedDelta() const { return m_signedDelta; }
    const WaveletDelta::HashedVersion& resultingVersion() const;

    const WaveletDelta::HashedVersion& appliedAt() const { return m_appliedAt; }

    /**
      * This is either the same as the signed delta or a transformation thereof.
      */
    const WaveletDelta& transformedDelta() const { if ( m_transformedDelta.isNull() ) return m_signedDelta.delta(); return m_transformedDelta; }
    void setTransformedDelta( const WaveletDelta& delta ) { m_transformedDelta = delta; }

    qint64 applicationTime() const { return m_applicationTime; }
    int operationsApplied() const { return m_operationsApplied; }

    bool isNull() const { return m_null; }

    void toProtobuf(protocol::ProtocolAppliedWaveletDelta* appliedDelta) const;
    QByteArray toBinary() const;
    QString toBase64() const;

    static AppliedWaveletDelta fromBase64( const QString& base64, bool* ok);

private:
    bool m_null;
    SignedWaveletDelta m_signedDelta;
    WaveletDelta::HashedVersion m_resultingVersion;
    qint64 m_applicationTime;
    int m_operationsApplied;
    /**
      * May be null.
      */
    WaveletDelta::HashedVersion m_appliedAt;
    /**
      * May be null.
      */
    WaveletDelta m_transformedDelta;
};

#endif // APPLIEDWAVELETDELTA_H
