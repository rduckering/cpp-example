#pragma once
#include <JuceHeader.h>
#include "RBJFilters.h"

// Yin method pitch detection, uses difference method and averaging

class GuitarPitchDetectionFX
{
public:
    GuitarPitchDetectionFX() = default;
    ~GuitarPitchDetectionFX() = default;

    void init();
    void update();
    void process (float* audioStream, int numSamples);
    static std::string info() { return "GuitarPitchDetection"; }

    float getPitch() const noexcept { return pitch.load (std::memory_order::memory_order_relaxed); }

    void setSampleRate (float sr) noexcept { sampleRate = sr; }
    void setThreshold (float value) { threshold = juce::jlimit (0.0f, 1.0f, value); }

private:
    static constexpr int bufferSize = 4096;
    static constexpr int halfBufferSize = bufferSize / 2;

    float threshold = 0.3f;

    std::array<float, 4096> sigBuffer;
    std::array<float, 4096> diffBuffer;
    std::array<float, 4096> cumulativeBuffer;

    std::array<float, halfBufferSize> scratch;

    size_t bufferSampleCnt = 0;

    void clearBuffers();
    [[maybe_unused]] void computeDifference();
    void computeDifferenceV2();
    int absoluteThreshold();
    float parabolicInterpolation (int tau);
    void computeCumulativeMean();
    float detectPitch();

    float sampleRate = 48000.0f;
    std::atomic<float> pitch = 0.0f;

    LPF lpf1, lpf2;
    HPF hpf1, hpf2;
};