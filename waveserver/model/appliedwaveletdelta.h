#ifndef APPLIEDWAVELETDELTA_H
#define APPLIEDWAVELETDELTA_H

#include "model/waveletdelta.h"
#include <QString>
#include <QByteArray>

namespace protocol
{
    class ProtocolAppliedWaveletDelta;
}

class AppliedWaveletDelta
{
public:
    AppliedWaveletDelta();
    AppliedWaveletDelta( const AppliedWaveletDelta& delta );
    AppliedWaveletDelta( const WaveletDelta& delta, qint64 applicationTime, int m_operationsApplied );

    const WaveletDelta& delta() const { return m_delta; }
    const WaveletDelta::HashedVersion& resultingVersion() const;

    qint64 applicationTime() const { return m_applicationTime; }
//    void setApplicationTime( qint64 time ) { m_applicationTime = time; }
    int operationsApplied() const { return m_operationsApplied; }
//    void setOperationsApplied(int operationsApplied) { m_operationsApplied = operationsApplied; }

    QByteArray signature() const { return m_signature; }

    bool isNull() const { return m_null; }

    void toProtobuf(protocol::ProtocolAppliedWaveletDelta* appliedDelta) const;
    QByteArray toBinary() const;
    QString toBase64() const;

private:
    bool m_null;
    WaveletDelta m_delta;
    WaveletDelta::HashedVersion m_resultingVersion;
    qint64 m_applicationTime;
    int m_operationsApplied;
    QByteArray m_signature;
};

#endif // APPLIEDWAVELETDELTA_H
