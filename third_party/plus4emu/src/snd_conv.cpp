
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "snd_conv.hpp"
#include <cmath>

namespace Plus4Emu {

  inline float AudioConverter::DCBlockFilter::process(float inputSignal)
  {
    // y[n] = x[n] - x[n - 1] + (c * y[n - 1])
    float   outputSignal = (inputSignal - this->xnm1) + (this->c * this->ynm1);
    // avoid denormals
#if defined(__i386__) || defined(__x86_64__)
    unsigned char e = ((unsigned char *) &outputSignal)[3] & 0x7F;
    if (e < 0x08 || e >= 0x78)
      outputSignal = 0.0f;
#else
    outputSignal = (outputSignal < -1.0e-20f || outputSignal > 1.0e-20f ?
                    outputSignal : 0.0f);
#endif
    this->xnm1 = inputSignal;
    this->ynm1 = outputSignal;
    return outputSignal;
  }

  void AudioConverter::DCBlockFilter::setCutoffFrequency(float frq)
  {
    float   tpfdsr = (2.0f * 3.14159265f * frq / sampleRate);
    c = 1.0f - (tpfdsr > 0.0003f ?
                (tpfdsr < 0.125f ? tpfdsr : 0.125f) : 0.0003f);
  }

  AudioConverter::DCBlockFilter::DCBlockFilter(float sampleRate_,
                                               float cutoffFreq)
  {
    sampleRate = sampleRate_;
    c = 1.0f;
    xnm1 = 0.0f;
    ynm1 = 0.0f;
    setCutoffFrequency(cutoffFreq);
  }

  AudioConverter::ParametricEqualizer::ParametricEqualizer()
    : mode(-1),
      xnm1(0.0), xnm2(0.0), ynm1(0.0), ynm2(0.0),
      a1da0(0.0), a2da0(0.0), b0da0(1.0), b1da0(0.0), b2da0(0.0)
  {
  }

  void AudioConverter::ParametricEqualizer::setParameters(int mode_,
                                                          float omega_,
                                                          float level_,
                                                          float q_)
  {
    mode = ((mode_ >= 0 && mode_ <= 2) ? mode_ : -1);
    xnm1 = 0.0;
    xnm2 = 0.0;
    ynm1 = 0.0;
    ynm2 = 0.0;
    omega_ = (omega_ > 0.0005f ? (omega_ < 3.14f ? omega_ : 3.14f) : 0.0005f);
    level_ = (level_ > 0.0001f ? (level_ < 100.0f ? level_ : 100.0f) : 0.0001f);
    q_ = (q_ > 0.001f ? (q_ < 100.0f ? q_ : 100.0f) : 0.001f);
    // the following code is based on formulas by Robert Bristow-Johnson
    double  a = std::sqrt(level_);
    double  cosw0 = std::cos(omega_);
    double  alpha = std::sin(omega_) / (2.0f * q_);
    double  a0;
    switch (mode) {
    case -1:                                    // disabled
      a1da0 = 0.0;
      a2da0 = 0.0;
      b0da0 = 1.0;
      b1da0 = 0.0;
      b2da0 = 0.0;
      break;
    case 0:                                     // peaking EQ
      a0 = 1.0 + (alpha / a);
      a1da0 = (-2.0 * cosw0) / a0;
      a2da0 = (1.0 - (alpha / a)) / a0;
      b0da0 = (1.0 + (alpha * a)) / a0;
      b1da0 = (-2.0 * cosw0) / a0;
      b2da0 = (1.0 - (alpha * a)) / a0;
      break;
    case 1:                                     // low shelf
      {
        double  am1cosw0 = (a - 1.0) * cosw0;
        double  twoSqrtAAlpha = 2.0 * std::sqrt(a) * alpha;
        a0 = (a + 1.0) + am1cosw0 + twoSqrtAAlpha;
        a1da0 = (-2.0 * ((a - 1.0) + ((a + 1.0) * cosw0))) / a0;
        a2da0 = ((a + 1.0) + am1cosw0 - twoSqrtAAlpha) / a0;
        b0da0 = a * ((a + 1.0) - am1cosw0 + twoSqrtAAlpha) / a0;
        b1da0 = 2.0 * a * ((a - 1.0) - ((a + 1.0) * cosw0)) / a0;
        b2da0 = a * ((a + 1.0) - am1cosw0 - twoSqrtAAlpha) / a0;
      }
      break;
    case 2:                                     // high shelf
      {
        double  am1cosw0 = (a - 1.0) * cosw0;
        double  twoSqrtAAlpha = 2.0 * std::sqrt(a) * alpha;
        a0 = (a + 1.0) - am1cosw0 + twoSqrtAAlpha;
        a1da0 = (2.0 * ((a - 1.0) - ((a + 1.0) * cosw0))) / a0;
        a2da0 = ((a + 1.0) - am1cosw0 - twoSqrtAAlpha) / a0;
        b0da0 = a * ((a + 1.0) + am1cosw0 + twoSqrtAAlpha) / a0;
        b1da0 = -2.0 * a * ((a - 1.0) + ((a + 1.0) * cosw0)) / a0;
        b2da0 = a * ((a + 1.0) + am1cosw0 - twoSqrtAAlpha) / a0;
      }
      break;
    }
  }

  inline float AudioConverter::ParametricEqualizer::process(float inputSignal)
  {
    if (mode >= 0) {
      double  yn = (inputSignal * b0da0) + (xnm1 * b1da0) + (xnm2 * b2da0)
                   - (ynm1 * a1da0) - (ynm2 * a2da0);
      // avoid denormals
      volatile double tmp = 1.0e-32;
      yn = (yn + 1.0e-32) - tmp;
      xnm2 = xnm1;
      xnm1 = inputSignal;
      ynm2 = ynm1;
      ynm1 = yn;
      return float(yn);
    }
    else
      return inputSignal;
  }

  inline void AudioConverter::sendOutputSignal(float audioSignal)
  {
    // scale, clip, and convert to 16 bit integer format
    float   outputSignal = audioSignal * ampScale;
    if (outputSignal < 0.0f) {
      outputSignal =
          (outputSignal > -32767.0f ? outputSignal - 0.5f : -32767.5f);
    }
    else {
      outputSignal =
          (outputSignal < 32767.0f ? outputSignal + 0.5f : 32767.5f);
    }
#if defined(__linux) || defined(__linux__)
    int16_t outputSignal_i = int16_t(outputSignal);
    // hack to work around clicks in ALSA sound output
    outputSignal_i = (outputSignal_i != 0 ? outputSignal_i : 1);
    this->audioOutput(outputSignal_i);
#else
    this->audioOutput(int16_t(outputSignal));
#endif
  }

  AudioConverter::AudioConverter(float inputSampleRate_,
                                 float outputSampleRate_,
                                 float dcBlockFreq1, float dcBlockFreq2,
                                 float ampScale_)
    : inputSampleRate(inputSampleRate_),
      outputSampleRate(outputSampleRate_),
      dcBlock1(outputSampleRate_, dcBlockFreq1),
      dcBlock2(outputSampleRate_, dcBlockFreq2)
  {
    setOutputVolume(ampScale_);
    setEqualizerParameters(2, 15000.0f, 0.5f, 0.5f);
  }

  AudioConverter::~AudioConverter()
  {
  }

  void AudioConverter::setInputSampleRate(float sampleRate_)
  {
    inputSampleRate = sampleRate_;
  }

  void AudioConverter::setOutputSampleRate(float sampleRate_)
  {
    outputSampleRate = sampleRate_;
  }

  void AudioConverter::setDCBlockFilters(float frq1, float frq2)
  {
    dcBlock1.setCutoffFrequency(frq1);
    dcBlock2.setCutoffFrequency(frq2);
  }

  void AudioConverter::setEqualizerParameters(int mode_, float freq_,
                                              float level_, float q_)
  {
    float   omega = 2.0f * 3.1415927f * freq_ / outputSampleRate;
    eq.setParameters(mode_, omega, level_, q_);
  }

  void AudioConverter::setOutputVolume(float ampScale_)
  {
    if (ampScale_ > 0.01f && ampScale_ < 1.0f)
      ampScale = 1.17f * ampScale_;
    else if (ampScale_ > 0.99f)
      ampScale = 1.17f;
    else
      ampScale = 0.017f;
  }

  void AudioConverterLowQuality::sendInputSignal(int32_t audioInput)
  {
    float   audioInput_f = float(audioInput);
    phs += 1.0f;
    if (phs < nxtPhs) {
      outputSignal += (prvInput + audioInput_f);
    }
    else {
      float   phsFrac = nxtPhs - (phs - 1.0f);
      float   tmp = prvInput + ((audioInput_f - prvInput) * phsFrac);
      outputSignal += ((prvInput + tmp) * phsFrac);
      outputSignal /= (downsampleRatio * 2.0f);
      float   tmp2 =
          eq.process(dcBlock2.process(dcBlock1.process(outputSignal)));
      sendOutputSignal(tmp2);
      outputSignal = (tmp + audioInput_f) * (1.0f - phsFrac);
      nxtPhs = (nxtPhs + downsampleRatio) - phs;
      phs = 0.0f;
    }
    prvInput = audioInput_f;
  }

  AudioConverterLowQuality::AudioConverterLowQuality(float inputSampleRate_,
                                                     float outputSampleRate_,
                                                     float dcBlockFreq1,
                                                     float dcBlockFreq2,
                                                     float ampScale_)
    : AudioConverter(inputSampleRate_, outputSampleRate_,
                     dcBlockFreq1, dcBlockFreq2, ampScale_)
  {
    prvInput = 0.0f;
    phs = 0.0f;
    downsampleRatio = inputSampleRate_ / outputSampleRate_;
    nxtPhs = downsampleRatio;
    outputSignal = 0.0f;
  }

  AudioConverterLowQuality::~AudioConverterLowQuality()
  {
  }

  void AudioConverterLowQuality::setInputSampleRate(float sampleRate_)
  {
    inputSampleRate = sampleRate_;
    downsampleRatio = inputSampleRate / outputSampleRate;
  }

  void AudioConverterLowQuality::setOutputSampleRate(float sampleRate_)
  {
    outputSampleRate = sampleRate_;
    downsampleRatio = inputSampleRate / outputSampleRate;
  }

  inline void AudioConverterHighQuality::ResampleWindow::processSample(
      float inputSignal, float *outBuf, int outBufSize, float bufPos)
  {
    int      writePos = int(bufPos);
    float    posFrac = bufPos - writePos;
    float    winPos = (1.0f - posFrac) * float(windowSize / 12);
    int      winPosInt = int(winPos);
    float    winPosFrac = winPos - winPosInt;
    writePos -= 5;
    while (writePos < 0)
      writePos += outBufSize;
    do {
      float   w = windowTable[winPosInt]
                  + ((windowTable[winPosInt + 1] - windowTable[winPosInt])
                     * winPosFrac);
      outBuf[writePos] += inputSignal * w;
      if (++writePos >= outBufSize)
        writePos = 0;
      winPosInt += (windowSize / 12);
    } while (winPosInt < windowSize);
  }

  AudioConverterHighQuality::ResampleWindow::ResampleWindow()
  {
    double  pi = std::atan(1.0) * 4.0;
    double  phs = -(pi * 6.0);
    double  phsInc = 12.0 * pi / windowSize;
    for (int i = 0; i <= windowSize; i++) {
      if (i == (windowSize / 2))
        windowTable[i] = 1.0f;
      else
        windowTable[i] = float((std::cos(phs / 6.0) * 0.5 + 0.5)  // von Hann
                               * (std::sin(phs) / phs));
      phs += phsInc;
    }
  }

  AudioConverterHighQuality::ResampleWindow AudioConverterHighQuality::window;

  void AudioConverterHighQuality::sendInputSignal(int32_t audioInput)
  {
    window.processSample(float(audioInput), buf, bufSize, bufPos);
    bufPos += resampleRatio;
    if (bufPos >= nxtPos) {
      if (bufPos >= float(bufSize))
        bufPos -= float(bufSize);
      nxtPos = float(int(bufPos) + 1);
      int     readPos = int(bufPos) - 6;
      while (readPos < 0)
        readPos += bufSize;
      float   tmp = buf[readPos] * resampleRatio;
      buf[readPos] = 0.0f;
      float   tmp2 = eq.process(dcBlock2.process(dcBlock1.process(tmp)));
      sendOutputSignal(tmp2);
    }
  }

  AudioConverterHighQuality::AudioConverterHighQuality(float inputSampleRate_,
                                                       float outputSampleRate_,
                                                       float dcBlockFreq1,
                                                       float dcBlockFreq2,
                                                       float ampScale_)
    : AudioConverter(inputSampleRate_, outputSampleRate_,
                     dcBlockFreq1, dcBlockFreq2, ampScale_)
  {
    for (int i = 0; i < bufSize; i++) {
      buf[i] = 0.0f;
    }
    bufPos = 0.0f;
    nxtPos = 1.0f;
    resampleRatio = outputSampleRate_ / inputSampleRate_;
  }

  AudioConverterHighQuality::~AudioConverterHighQuality()
  {
  }

  void AudioConverterHighQuality::setInputSampleRate(float sampleRate_)
  {
    inputSampleRate = sampleRate_;
    resampleRatio = outputSampleRate / inputSampleRate;
  }

  void AudioConverterHighQuality::setOutputSampleRate(float sampleRate_)
  {
    outputSampleRate = sampleRate_;
    resampleRatio = outputSampleRate / inputSampleRate;
  }

}       // namespace Plus4Emu

