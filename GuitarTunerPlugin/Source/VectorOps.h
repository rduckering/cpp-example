#pragma once
#define VIMAGE_H

#if !IS_WINDOWS
#include <Accelerate/Accelerate.h>
#elif IS_WINDOWS && WITH_IPP
#include <ipp.h>
#endif

namespace VectorOps
{
#if !IS_WINDOWS
    static inline void sum (float* input, float* output, int size)
    {
        vDSP_sve (input, 1, output, static_cast<size_t> (size));
    }
#elif IS_WINDOWS && WITH_IPP
    static inline void sum (float* input, float* output, int size)
    {
        auto result = ippsSum_32f (static_cast<Ipp32f*> (input), size, static_cast<Ipp32f*> (output), static_cast<IppHintAlgorithm>(1));
    }
#else
    static inline void sum (float* input, float* output, int size)
    {
        *output = std::accumulate (input, input + size, 0.0f);
    }
#endif

}