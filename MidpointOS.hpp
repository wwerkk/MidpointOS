// OversamplingOscillators.hpp

#pragma once

#include "SC_PlugIn.hpp"
#include "VariableOversampling.hpp"
#include "SC_RGen.h"
#include <array>

namespace MidpointOS{

class MidpointOS; // Forward declaration

class MidpointOSNext {
  public:
    MidpointOSNext();
    ~MidpointOSNext();
    float next(float freq, float phaseIn, float freqMul, float depth, float spread, float reduction, float lerp, float* buf, float* buf_, int mSize, int mSize_);
    float phase{0.f};

    float* buf;
    float* buf_;
    int mSize;
    int mSize_;
    MidpointOS* parent;
    float lastPhase{0.f};
    private:

};

class MidpointOS : public SCUnit {
public:
    MidpointOS();

    // Destructor
    ~MidpointOS();

    MidpointOSNext midpoint;
    
    VariableOversampling<> oversample;
    float mFreq{in0(0)};
    float mPhase{in0(1)};
    float mDepth{in0(2)};
    float mSpread{in0(3)};
    float mReduction{in0(4)};
    float mLerp{in0(5)};
    float* buf;
    float* buf_; // copy to keep past version of the buffer
    int mSize;
    int mSize_;

    void subdiv(float* b, float* b_, int size, float spread);
private:

    // Clear function
    void clear(int inNumSamples);
    
    // Calc function
    void next_aa(int nSamples);

    // void rSubdiv(World *mWorld, const float* input, float* output, int inputSize, int depth);

    // Member variables
    RGen & rgen = *mParent -> mRGen;

    float *osBuffer;
    int mOversamplingIndex;
    double mFreqMul; // a constant for multiplying frequency
};

} // namespace MidpointOS