// MidpointOS.cpp
// W K Werkowicz
// uses the SawOS from OversamplingOscillators by Sam Pluta the VariableOversampling class from Jatin Chowdhurry's ChowDSP library in the ported plugins library by Mads Kjeldgaard
// edit the c_cpp_properties.json and CMakeLists.txt files to correctly point to these files on your system

#include "MidpointOS.hpp"

static InterfaceTable *ft;

namespace MidpointOS {

MidpointOSNext::MidpointOSNext() {
}

MidpointOSNext::~MidpointOSNext() {}

float MidpointOSNext::next(float freq, float phaseIn, float freqMul, float depth, float spread, float reduction, float lerp, float* buf, float* buf_, int* mSize, int* mSize_) {
  float phaseDiff = (phaseIn - lastPhase);
  lastPhase = phaseIn;

  phase += (phaseDiff);
  phase += freq * freqMul;

  phase = sc_wrap(phase, -1.f, 2.f)
  if (phase >= 1.f) {
    phase -= 1.f;
    buf[0] = 0;
    buf[1] = 1;
    buf[2] = 0;
    buf[3] = -1;
    buf[4] = 0;
    *mSize = 5;
    // mSize_ = 4;
    // Copy(mSize, buf_, buf);
    for (int i = 0; i < ceilf(depth); ++i) {
      parent->subdiv(buf, buf_, *mSize, spread * powf(reduction, i));
    }
  }

  if (phase <= 0.f) {
    phase += 1.f;
    buf[0] = 0;
    buf[1] = 1;
    buf[2] = 0;
    buf[3] = -1;
    buf[4] = 0;
    *mSize = 5;
    // mSize_ = 5;
    // Copy(mSize, buf_, buf);
    for (int i = 0; i < ceilf(depth); ++i) {
      parent->subdiv(buf, buf_, *mSize, spread * powf(reduction, i));
    }
  }
  
  int ix = int(phase * *mSize);
  float z = buf[ix];
  int ix_ = int(phase * *mSize_);
  float z_ = buf_[ix_];

  if (lerp) {
    float frac = sc_frac(phase * *mSize);
    float frac_ = sc_frac(phase * *mSize_);
    // yo dawg
    z = lininterp(lerp, z, lininterp(frac, buf[ix], buf[(ix + 1 ) % *mSize]));
    z_ = lininterp(lerp, z_, lininterp(frac_, buf_[ix_], buf_[(ix_ + 1 ) % *mSize_]));
  }

  float out = lininterp(sc_frac(depth), z_, z);
  return out;
}

MidpointOS::MidpointOS() {
  const float SR = sampleRate();
  oversample.reset(SR);
  mOversamplingIndex = sc_clip(int(in0(6)), 0, 4);
  oversample.setOversamplingIndex(mOversamplingIndex);
  osBuffer = oversample.getOSBuffer();
  mCalcFunc = make_calc_function<MidpointOS, &MidpointOS::next_aa>();
  // initialize a constant for multiplying the frequency
  // mFreqMul = 2.0f/(float)sampleRate();
  mFreqMul = sampleDur();
  // get depth param
  mDepth = sc_clip(in0(1), 0, 24);
  // get spread param
  mSpread = in0(2);
  // get reduction param
  mReduction = in0(3);
  // allocate and fill the buffers
  buf = (float*) RTAlloc(mWorld, 4096 * sizeof(float));
  buf_ = (float*) RTAlloc(mWorld, 4096 * sizeof(float));

  if (buf == NULL) {
      mCalcFunc = make_calc_function<MidpointOS, &MidpointOS::clear>();
      clear(1);
      if(mWorld->mVerbosity > -2) {
          Print("Failed to allocate memory for Midpoint ugen.\n");
      }
      return;
  }
  if (buf_ == NULL) {
      mCalcFunc = make_calc_function<MidpointOS, &MidpointOS::clear>();
      clear(1);
      if(mWorld->mVerbosity > -2) {
          Print("Failed to allocate memory for Midpoint ugen.\n");
      }
      return;
  }
  memset(buf, 0, 4096 * sizeof(float));
  memset(buf_, 0, 4096 * sizeof(float));
  // Initialize first and last elements
  buf[0] = 0;
  buf[1] = 1;
  buf[2] = 0;
  buf[3] = -1;
  buf[4] = 0;
  // init size
  mSize = 5;
  mSize_ = 5;
  Copy(mSize, buf_, buf);
  // run midpoint displacement
  for (int i = 0; i < ceilf(mDepth); ++i) {
      subdiv(buf, buf_, mSize, mSpread * powf(mReduction, i));
  }

  midpoint.parent = this;

  // output a single sample
  next_aa(1);
}

MidpointOS::~MidpointOS() {
  // Free the memory.
  RTFree(mWorld, buf);
  RTFree(mWorld, buf_);
}

void MidpointOS::clear(int inNumSamples) {
  ClearUnitOutputs(this, inNumSamples);
}

void MidpointOS::next_aa(int nSamples) {
  // get the pointer to the output buffer
  float *outBuf = out(0);
  const float *freq = in(0);;
  const float *phase = in(1);
  const float *depth = in(2);
  const float *spread = in(3);
  const float *reduction = in(4);
  const float *lerp = in(5);
  double freqmul = mFreqMul;

  for (int i=0; i < nSamples; ++i) {
    float output = 0.f;
    for (int k = 0; k < oversample.getOversamplingRatio(); ++k) {
      mFreq = freq[(inRate(0) != calc_ScalarRate) * i];
      mPhase = phase[(inRate(1) != calc_ScalarRate) * i];
      mDepth = sc_clip(depth[(inRate(2) != calc_ScalarRate) * i], 0, 24);
      mSpread = spread[(inRate(3) != calc_ScalarRate) * i];
      mReduction = reduction[(inRate(4) != calc_ScalarRate) * i];
      mLerp = lerp[(inRate(5) != calc_ScalarRate) * i];
      osBuffer[k] = midpoint.next(mFreq, mPhase, mFreqMul / oversample.getOversamplingRatio(), mDepth, mSpread, mReduction, mLerp, buf, buf_, &mSize, &mSize_);
    }
    if (mOversamplingIndex != 0) output = oversample.downsample();
    else output = osBuffer[0];
    outBuf[i] = output;
  }
}

void MidpointOS::subdiv(float* b, float* b_, int size, float spread) {
  if (size * 2 - 1 > 4096) {
      // Print("Max size reached\n");
      return;
  }

  // Copy the previous state of the buffer into buf_
  Copy(size, b_, b);

  float* tmp = (float*) RTAlloc(mWorld, (4096) * sizeof(float));
  memset(tmp, 0, 4096 * sizeof(float));

  // Calculate midpoints with spread and store them in tmp
  for (int i = 0; i < size - 1; ++i) {
      float mid = (b[i] + b[(i + 1) % size]) * 0.5f;
      mid += rgen.frand2() * spread;
      mid = sc_fold1(mid);
      tmp[i] = mid;
  }

  // Make space in b
  for (int i = size - 1; i > 0; --i) {
      b[2 * i] = b[i];
  }

  // Interleave tmp into b
  for (int i = 0; i < size - 1; ++i) {
      b[2 * i + 1] = tmp[i];
  }

  // Free tmp
  RTFree(mWorld, tmp);

  // Update mSize
  mSize_ = mSize;
  mSize = size * 2 - 1;
}

}

PluginLoad(MidpointOS)
{
  ft = inTable;
  registerUnit<MidpointOS::MidpointOS>(ft, "MidpointOS", false);
}
