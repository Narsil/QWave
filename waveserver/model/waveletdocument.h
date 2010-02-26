#ifndef WAVELETDOCUMENT_H
#define WAVELETDOCUMENT_H

#include "model/structureddocument.h"

namespace protocol
{
    class ProtocolDocumentOperation;
}

class Wavelet;

class WaveletDocument : public StructuredDocument
{
public:
    WaveletDocument(Wavelet* wavelet, const QString& name);

    inline QString name() const { return m_name; }

    void toDocumentOperation( protocol::ProtocolDocumentOperation* operation );

private:
    Wavelet* m_wavelet;
    QString m_name;
};

#endif // WAVELETDOCUMENT_H
