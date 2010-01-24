#ifndef CONVERTER_H
#define CONVERTER_H

class WaveletDelta;

namespace protocol
{
    class ProtocolWaveletDelta;
}

class Converter
{
public:
    /**
    * Convert the Qt-ified representation to the protocol buffer representation.
      */
    static void convert(protocol::ProtocolWaveletDelta* result, const WaveletDelta& delta );
    static WaveletDelta convert( const protocol::ProtocolWaveletDelta& delta );

private:
    Converter();
};

#endif // CONVERTER_H
