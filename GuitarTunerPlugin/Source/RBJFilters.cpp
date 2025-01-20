#include "RBJFilters.h"
#include <cmath>

RBJFilter_base::RBJFilter_base()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (float& coeff : coeffs)
        coeff = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

RBJFilter_base::~RBJFilter_base() {}

// ===================== LPF =====================

LPF::LPF()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q  = 0.0f;
    norm = 0.0f;

    for (float& coeff : coeffs)
        coeff = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float LPF::calculateBandwidth (int order)
{
    jlimit (1, 10, order);
    return std::sqrt (std::powf (10.0f, (static_cast<float> (order) * 3.0f) * (1.0f / 20.0f)));
}

void LPF::calculateCoeffs (float fc, float fs, int)
{

    fc = jlimit (20.0f, 24000.0f, fc);
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos (w);
    const float a = std::sin (w) / (2.0f * 0.707f);
    norm = 1.0f / (1.0f + a);

    coeffs[LPF::a0] = (1.0f + a)          * norm;
    coeffs[LPF::a1] = (-2.0f * c)         * norm;
    coeffs[LPF::a2] = (1.0f - a)          * norm;
    coeffs[LPF::b0] = ((1.0f - c) * 0.5f) * norm;
    coeffs[LPF::b1] = (1.0f - c)          * norm;
    coeffs[LPF::b2] = coeffs[LPF::b0];
}

void LPF::process (float& input)
{
    x[2] = x[1];
    x[1] = x[0];
    x[0] = input;
    y[2] = y[1];
    y[1] = y[0];

    y[0] = coeffs[LPF::b0] * x[0]
         + coeffs[LPF::b1] * x[1]
         + coeffs[LPF::b2] * x[2]
         - coeffs[LPF::a1] * y[1]
         - coeffs[LPF::a2] * y[2];

    input = y[0];
}


// ===================== HPF =====================

HPF::HPF()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float HPF::calculateBandwidth (int)
{
    return 0.0;
}

void HPF::calculateCoeffs (float fc, float fs, int)
{
    fc = jlimit (1.0f, 20000.0f, fc);

    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float a = std::sin(w) / (2.0f * 0.707f);

    norm = 1.0f / (1.0f + a);

    coeffs[HPF::a0] = (1.0f + a)          * norm;
    coeffs[HPF::a1] = (-2.0f * c)         * norm;
    coeffs[HPF::a2] = (1.0f - a)          * norm;
    coeffs[HPF::b0] = ((1.0f + c) * 0.5f) * norm;
    coeffs[HPF::b1] = (-(1.0f + c))       * norm;
    coeffs[HPF::b2] = coeffs[HPF::b0];
}

void HPF::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[HPF::b0] * x[0]
         + coeffs[HPF::b1] * x[1]
         + coeffs[HPF::b2] * x[2]
         - coeffs[HPF::a1] * y[1]
         - coeffs[HPF::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}


// ===================== Peak =====================

Peak::Peak()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q  = 0.0f;
    norm = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float Peak::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return std::powf (10.0f, float(order) / 40.0f);
}

void Peak::calculateCoeffs (float fc, float fs, int order)
{
    const float A = calculateBandwidth (order);
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float BW = 3.0f / 2.0f;
    const float a = s * std::sinh (std::log (2.0f) / 2.0f * BW * w / s);

    norm = 1.0f / (1.0f + a / A);

    coeffs[Peak::a0] = (1.0f + a / A)   * norm;
    coeffs[Peak::a1] = (-2.0f * c)      * norm;
    coeffs[Peak::a2] = (1.0f - a / A)   * norm;
    coeffs[Peak::b0] = (1.0f + a * A)   * norm;
    coeffs[Peak::b1] = coeffs[Peak::a1];
    coeffs[Peak::b2] = (1.0f - a * A)   * norm;
}

void Peak::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[Peak::b0] * x[0]
         + coeffs[Peak::b1] * x[1]
         + coeffs[Peak::b2] * x[2]
         - coeffs[Peak::a1] * y[1]
         - coeffs[Peak::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}

// ===================== BPF =====================

BPF::BPF()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float BPF::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float(order) / 40.0f);
}

void BPF::calculateCoeffs (float fc, float fs, int)
{
    const float gain = 1.0f;
    const float Q = 0.707f;
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float BW = gain / 2.0f;
    const float a = s * std::sinh (std::log (2.0f) / 2.0f * BW * w / s);

    norm = 1.0f / (1.0f + a);

    coeffs[BPF::a0] = (1.0f + a)   * norm;
    coeffs[BPF::a1] = (-2.0f * c)  * norm;
    coeffs[BPF::a2] = (1.0f - a)   * norm;
    coeffs[BPF::b0] = (Q * a)      * norm;
    coeffs[BPF::b1] = 0.0f         * norm;
    coeffs[BPF::b2] = (-Q * a)     * norm;
}

void BPF::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[BPF::b0] * x[0]
         + coeffs[BPF::b1] * x[1]
         + coeffs[BPF::b2] * x[2]
         - coeffs[BPF::a1] * y[1]
         - coeffs[BPF::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}


// ===================== BPFcQ =====================

BPFcQ::BPFcQ()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float BPFcQ::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float(order) / 40.0f);
}

void BPFcQ::calculateCoeffs (float fc, float fs, int)
{
    const float gain = 1.0f;
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float BW = gain / 2.0f;
    const float a = s * std::sinh (std::log (2.0f) / 2.0f * BW * w / s);

    norm = 1.0f / (1.0f + a);

    coeffs[BPFcQ::a0] = (1.0f + a)   * norm;
    coeffs[BPFcQ::a1] = (-2.0f * c)  * norm;
    coeffs[BPFcQ::a2] = (1.0f - a)   * norm;
    coeffs[BPFcQ::b0] = a            * norm;
    coeffs[BPFcQ::b1] = 0.0f         * norm;
    coeffs[BPFcQ::b2] = -a           * norm;
}

void BPFcQ::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[BPFcQ::b0] * x[0]
         + coeffs[BPFcQ::b1] * x[1]
         + coeffs[BPFcQ::b2] * x[2]
         - coeffs[BPFcQ::a1] * y[1]
         - coeffs[BPFcQ::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}


// ===================== BPFcQ =====================

Notch::Notch()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float Notch::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float(order) / 40.0f);
}

void Notch::calculateCoeffs (float fc, float fs, int)
{
    const float gain = 3.0f;
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float BW = gain / 2.0f;
    const float a = s * std::sinh (std::log (2.0f) / 2.0f * BW * w / s);

    norm = 1.0f / (1.0f + a);

    coeffs[Notch::a0] = (1.0f + a)   * norm;
    coeffs[Notch::a1] = (-2.0f * c)  * norm;
    coeffs[Notch::a2] = (1.0f - a)   * norm;
    coeffs[Notch::b0] = 1.0f         * norm;
    coeffs[Notch::b1] = coeffs[Notch::a1];
    coeffs[Notch::b2] = 1.0f         * norm;
}

void Notch::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[Notch::b0] * x[0]
         + coeffs[Notch::b1] * x[1]
         + coeffs[Notch::b2] * x[2]
         - coeffs[Notch::a1] * y[1]
         - coeffs[Notch::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}

// ===================== APF =====================

APF::APF()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float APF::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float(order) / 40.0f);
}

void APF::calculateCoeffs (float fc, float fs, int)
{
    const float gain = 12.0f;
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float BW = gain / 2.0f;
    const float a = s * std::sinh (std::log (2.0f) / 2.0f * BW * w / s);

    norm = 1.0f / (1.0f + a);

    coeffs[APF::a0] = (1.0f + a)   * norm;
    coeffs[APF::a1] = (-2.0f * c)  * norm;
    coeffs[APF::a2] = (1.0f - a)   * norm;
    coeffs[APF::b0] = (1.0f - a)   * norm;
    coeffs[APF::b1] = coeffs[APF::a1];
    coeffs[APF::b2] = (1.0f + a)   * norm;
}

void APF::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[APF::b0] * x[0]
         + coeffs[APF::b1] * x[1]
         + coeffs[APF::b2] * x[2]
         - coeffs[APF::a1] * y[1]
         - coeffs[APF::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}




// ===================== LowShelf =====================

LowShelf::LowShelf()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float LowShelf::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float(order) / 40.0f);
}

void LowShelf::calculateCoeffs (float fc, float fs, int)
{
    const auto gain = -12.0f;
    const auto A = std::powf (10.0f, gain / 40.0f);
    const auto w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const auto c = cos(w);
    const auto s = sin(w);
    const auto S = 1.0f;
    const auto a = (s / 2.0f) * std::sqrt((A + (1.0f / A)) * ((1.0f / S) - 1.0f) + 2.0f);

    norm = 1.0f / ((A + 1.0f) + (A - 1.0f) * c + 2.0f * std::sqrt(A) * a);

    coeffs[LowShelf::a0] = ((A + 1.0f) + (A - 1.0f) * c + 2.0f * std::sqrt(A) * a)     * norm;
    coeffs[LowShelf::a1] = (-2.0f * ((A - 1.0f) + (A + 1.0f) * c))                     * norm;
    coeffs[LowShelf::a2] = ((A + 1.0f) + (A - 1.0f) * c - 2.0f * std::sqrt(A) * a)     * norm;

    coeffs[LowShelf::b0] = (A * ((A + 1.0f) - (A - 1.0f) * c + 2.0f * std::sqrt(A) * a)) * norm;
    coeffs[LowShelf::b1] = (2.0f * A * ((A - 1.0f) - (A + 1.0f) * c))                    * norm;
    coeffs[LowShelf::b2] = (A * ((A + 1.0f) + (A - 1.0f) * c - 2.0f * std::sqrt(A) * a)) * norm;
}

void LowShelf::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[LowShelf::b0] * x[0]
         + coeffs[LowShelf::b1] * x[1]
         + coeffs[LowShelf::b2] * x[2]
         - coeffs[LowShelf::a1] * y[1]
         - coeffs[LowShelf::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}



// ===================== HighShelf =====================

HighShelf::HighShelf()
{
    freqc = 0.0f;
    freqs = 0.0f;
    q     = 0.0f;
    norm  = 0.0f;

    for (int i = 0; i < RBJFilter_base::NUMCOEFFS; ++i)
        coeffs[i] = 0.0f;

    for (int i = 0; i < 3; ++i)
    {
        y[i] = 0.0f;
        x[i] = 0.0f;
    }
}

float HighShelf::calculateBandwidth (int order)
{
    jlimit(-24, 24, order);
    return  std::powf (10.0f, float (order) / 40.0f);
}

void HighShelf::calculateCoeffs (float fc, float fs, int)
{
    const float gain = -12.0f;
    const float A = std::powf (10.0f, gain / 40.0f);
    const float w = 2.0f * static_cast<float> (juce::MathConstants<float>::pi) * (fc / fs);
    const float c = cos(w);
    const float s = sin(w);
    const float S = 1.0f;
    const float a = (s / 2.0f) * std::sqrt((A + (1.0f / A)) * ((1.0f / S) - 1.0f) + 2.0f);

    norm = 1.0f / ((A + 1.0f) - (A - 1.0f) * c + 2.0f * std::sqrt(A) * a);

    coeffs[HighShelf::a0] = ((A + 1.0f) - (A - 1.0f) * c + 2.0f * std::sqrt(A) * a)       * norm;
    coeffs[HighShelf::a1] = (2.0f * ((A - 1.0f) - (A + 1.0f) * c))                        * norm;
    coeffs[HighShelf::a2] = ((A + 1.0f) - (A - 1.0f) * c - 2.0f * std::sqrt(A) * a)       * norm;

    coeffs[HighShelf::b0] = (A * ((A + 1.0f) + (A - 1.0f) * c + 2.0f * std::sqrt(A) * a)) * norm;
    coeffs[HighShelf::b1] = (-2.0f * A * ((A - 1.0f) + (A + 1.0f) * c))                   * norm;
    coeffs[HighShelf::b2] = (A * ((A + 1.0f) + (A - 1.0f) * c - 2.0f * std::sqrt(A) * a)) * norm;
}

void HighShelf::process (float& input)
{
    x[0] = input;

    y[0] = coeffs[HighShelf::b0] * x[0]
         + coeffs[HighShelf::b1] * x[1]
         + coeffs[HighShelf::b2] * x[2]
         - coeffs[HighShelf::a1] * y[1]
         - coeffs[HighShelf::a2] * y[2];

    x[2] = x[1];
    x[1] = x[0];

    y[2] = y[1];
    y[1] = y[0];

    input = y[0];
}