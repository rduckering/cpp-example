#include "GuitarPitchDetectionFX.h"
#include "VectorOps.h"

void GuitarPitchDetectionFX::init()
{
    clearBuffers();
    lpf1.calculateCoeffs (1000.0f, sampleRate, 2);
    lpf2.calculateCoeffs (1000.0f, sampleRate, 2);
    hpf1.calculateCoeffs (60.0f, sampleRate, 2);
    hpf2.calculateCoeffs (60.0f, sampleRate, 2);
}

void GuitarPitchDetectionFX::update()
{

}

void GuitarPitchDetectionFX::process (float* audioStream, int numSamples)
{
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = audioStream[i];
        lpf1.process (sample);
        lpf2.process (sample);

        hpf1.process (sample);
        hpf2.process (sample);

        sigBuffer[bufferSampleCnt] = sample;
        bufferSampleCnt++;

        if (bufferSampleCnt >= halfBufferSize)
        {
            pitch.store (detectPitch(), std::memory_order::memory_order_release);
            clearBuffers();
            bufferSampleCnt = 0;
        }
    }
}

void GuitarPitchDetectionFX::clearBuffers()
{
    memset (&diffBuffer[0], 0, sizeof (float) * bufferSize);
    memset (&sigBuffer[0], 0, sizeof (float) * bufferSize);
    memset (&cumulativeBuffer[0], 0, sizeof (float) * bufferSize);
}

void GuitarPitchDetectionFX::computeDifference()
{
    float delta = 0.0f;

    for (size_t tau = 0; tau < halfBufferSize; ++tau)
    {
        diffBuffer[tau] = 0.0f;

        for (size_t j = 0; j < halfBufferSize; ++j)
        {
            delta = sigBuffer[j] - sigBuffer[j + tau];
            diffBuffer[tau] += delta * delta;
        }
    }
}

// Vector operations made this 13 times faster
// I suspect we can do this using a frequency domain version, for better performance
void GuitarPitchDetectionFX::computeDifferenceV2()
{
    juce::FloatVectorOperations::clear (&scratch[0], halfBufferSize);

    for (size_t tau = 0; tau < halfBufferSize; ++tau)
    {
        juce::FloatVectorOperations::subtract (&scratch[0], &sigBuffer[0], &sigBuffer[tau], halfBufferSize);
        juce::FloatVectorOperations::multiply (&scratch[0], &scratch[0], &scratch[0], halfBufferSize);

        VectorOps::sum (&scratch[0], &diffBuffer[tau], halfBufferSize);
    }
}

void GuitarPitchDetectionFX::computeCumulativeMean()
{
    float sum = 0.0f;

    for (size_t tau = 0; tau < halfBufferSize; ++tau)
    {
        if (tau == 0 || diffBuffer[tau] == 0.0f)
        {
            cumulativeBuffer[tau] = 1.0f;
            continue;
        }

        sum += diffBuffer[tau];
        cumulativeBuffer[tau] = diffBuffer[tau] / ((1.0f / static_cast<float> (tau)) * sum);
    }
}

int GuitarPitchDetectionFX::absoluteThreshold()
{
    for (size_t tau = 0; tau < halfBufferSize; ++tau)
    {
        if (cumulativeBuffer[tau] < threshold)
        {
            while (tau + 1 < halfBufferSize && cumulativeBuffer[tau + 1] < cumulativeBuffer[tau])
            {
                tau += 1;
            }

            return static_cast<int> (tau);
        }
    }

    return -1;
}

float GuitarPitchDetectionFX::parabolicInterpolation (int tau)
{
    if (tau < 1 || tau >= static_cast <int> (cumulativeBuffer.size()) - 1)
        return static_cast<float> (tau);

    const auto index = static_cast<size_t> (tau);
    const float s0 = cumulativeBuffer[index - 1];
    const float s1 = cumulativeBuffer[index];
    const float s2 = cumulativeBuffer[index + 1];

    return static_cast <float> (index) + (s2 - s0) / (2.0f * (2.0f * s1 - s2 - s0));
}

float GuitarPitchDetectionFX::detectPitch()
{
    computeDifferenceV2();
    computeCumulativeMean();
    const int tau = absoluteThreshold();

    if (tau == -1)
        return static_cast<float> (tau);

    return sampleRate / parabolicInterpolation (tau);
}