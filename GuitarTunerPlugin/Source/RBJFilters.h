#pragma once
#include <JuceHeader.h>

// RBJ EQ implementation https://www.w3.org/TR/audio-eq-cookbook/
// TODO - change process function from taking a float& to block

class RBJFilter_base
{
public:
    enum coeffs {
        a0,
        a1,
        a2,
        b0,
        b1,
        b2,
        NUMCOEFFS
    };

    RBJFilter_base();
    virtual ~RBJFilter_base();

    virtual float  calculateBandwidth (int order) = 0;
    virtual void   calculateCoeffs (float fc, float fs, int order) = 0;
    virtual void  process (float& input) = 0;

    float freqc = 100.0f;
    float q = 0.707f;

    float norm;
    float coeffs[NUMCOEFFS];
    float y[3];
    float x[3];
};

class LPF : public RBJFilter_base
{
public:

    LPF();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class HPF : public RBJFilter_base
{
public:

    HPF();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class Peak : public RBJFilter_base
{
public:

    Peak();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class BPF : public RBJFilter_base
{
public:

    BPF();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class BPFcQ : public RBJFilter_base
{
public:

    BPFcQ();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class Notch : public RBJFilter_base
{
public:

    Notch();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class APF : public RBJFilter_base
{
public:

    APF();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class LowShelf : public RBJFilter_base
{
public:

    LowShelf();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};



class HighShelf : public RBJFilter_base
{
public:

    HighShelf();

    void calculateCoeffs (float fc, float fs, int order) override;
    void  process (float& input) override;

private:
    float calculateBandwidth (int order) override;
};