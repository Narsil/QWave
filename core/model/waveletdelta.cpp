#include "waveletdelta.h"

WaveletDelta::WaveletDelta()
{
}

WaveletDelta::WaveletDelta(const WaveletDelta& delta)
        : m_version( delta.m_version ),
          m_author( delta.m_author ),
          m_operations( delta.m_operations )
{
}

WaveletDelta::~WaveletDelta()
{
}

void WaveletDelta::addOperation(const WaveletDeltaOperation& op)
{
    m_operations.append(op);
}
