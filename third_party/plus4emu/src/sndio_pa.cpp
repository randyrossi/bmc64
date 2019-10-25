
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
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
#include "system.hpp"
#include "soundio.hpp"
#include "sndio_pa.hpp"

#include <portaudio.h>
#include <vector>

#ifdef ENABLE_SOUND_DEBUG

static bool isPortAudioError(const char *msg, PaError paError)
{
  std::fprintf(stderr, " === %s\n", msg);
  if (paError == paNoError)
    return false;
  std::fprintf(stderr,
               " *** PortAudio error (error code = %d)\n", int(paError));
  std::fprintf(stderr, " ***   %s\n", Pa_GetErrorText(paError));
#  ifndef USING_OLD_PORTAUDIO_API
  if (paError == paUnanticipatedHostError) {
    const PaHostErrorInfo   *errInfo = Pa_GetLastHostErrorInfo();
    std::fprintf(stderr, " *** host API error:\n");
    std::fprintf(stderr, " ***   %s\n", errInfo->errorText);
  }
#  endif
  return true;
}

#else

static inline bool isPortAudioError(const char *msg, PaError paError)
{
  (void) msg;
  return (paError != paNoError);
}

#endif

namespace Plus4Emu {

  AudioOutput_PortAudio::AudioOutput_PortAudio()
    : AudioOutput(),
      paInitialized(false),
      disableRingBuffer(false),
      usingBlockingInterface(false),
      paLockTimeout(0U),
      writeBufIndex(0),
      readBufIndex(0),
      paStream((PaStream *) 0),
      latencyFramesHW(4096L),
      nextTime(0.0),
      closeDeviceLock(true)
  {
    // initialize PortAudio
    if (isPortAudioError("calling Pa_Initialize()", Pa_Initialize()))
      throw Exception("error initializing PortAudio");
    paInitialized = true;
  }

  AudioOutput_PortAudio::~AudioOutput_PortAudio()
  {
    paLockTimeout = 0U;
    if (paStream) {
      (void) isPortAudioError("calling Pa_StopStream()",
                              Pa_StopStream(paStream));
      closeDeviceLock.wait();
      (void) isPortAudioError("calling Pa_CloseStream()",
                              Pa_CloseStream(paStream));
      closeDeviceLock.notify();
      paStream = (PaStream *) 0;
    }
    disableRingBuffer = false;
    usingBlockingInterface = false;
    writeBufIndex = 0;
    readBufIndex = 0;
    buffers.clear();
    if (paInitialized) {
      (void) isPortAudioError("calling Pa_Terminate()", Pa_Terminate());
      paInitialized = false;
    }
  }

  void AudioOutput_PortAudio::sendAudioData(const int16_t *buf, size_t nFrames)
  {
    if (paStream) {
#ifndef USING_OLD_PORTAUDIO_API
      if (usingBlockingInterface) {
        // reduce timing jitter
        double  t = nextTime - timer_.getRealTime();
        double  periodTime = double(long(nFrames)) / double(sampleRate);
        long    framesToWrite = Pa_GetStreamWriteAvailable(paStream);
        switch (int((framesToWrite << 3) / latencyFramesHW)) {
        case 0:
          periodTime = periodTime * 2.0;
          break;
        case 1:
          periodTime = periodTime * 1.25;
          break;
        case 2:
          periodTime = periodTime * 1.1;
          break;
        case 3:
          periodTime = periodTime * 1.05;
          break;
        case 4:
          periodTime = periodTime * 0.95;
          break;
        case 5:
          periodTime = periodTime * 0.9;
          break;
        default:
          timer_.reset();
          nextTime = 0.0;
          periodTime = 0.0;
          break;
        }
        nextTime = nextTime + periodTime;
        if (t > 0.00075) {
          Timer::wait(t);
        }
        else if (t < -0.5) {
          timer_.reset();
          nextTime = 0.0;
        }
        // ring buffer is not used for blocking I/O, so assume nPeriodsSW == 1
        for (size_t i = 0; i < nFrames; i++) {
          Buffer& buf_ = buffers[0];
          buf_.audioData[buf_.writePos++] = buf[i];
          buf_.audioData[buf_.writePos++] = buf[i];
          if (buf_.writePos >= buf_.audioData.size()) {
            buf_.writePos = 0;
            Pa_WriteStream(paStream,
                           &(buf_.audioData[0]), buf_.audioData.size() >> 1);
          }
        }
      }
      else
#endif
      {
        for (size_t i = 0; i < nFrames; i++) {
          Buffer& buf_ = buffers[writeBufIndex];
          buf_.audioData[buf_.writePos++] = buf[i];
          buf_.audioData[buf_.writePos++] = buf[i];
          if (buf_.writePos >= buf_.audioData.size()) {
            buf_.writePos = 0;
            buf_.paLock.notify();
            if (buf_.epLock.wait(1000)) {
              if (++writeBufIndex >= buffers.size())
                writeBufIndex = 0;
            }
          }
        }
      }
    }
    else {
      // if there is no audio device, only synchronize to real time
      double  curTime = timer_.getRealTime();
      double  waitTime = nextTime - curTime;
      if (waitTime > 0.0)
        Timer::wait(waitTime);
      if (waitTime < double(-totalLatency)) {
        timer_.reset();
        nextTime = 0.0;
      }
      nextTime += (double(long(nFrames)) / sampleRate);
    }
    // call base class to write sound file
    AudioOutput::sendAudioData(buf, nFrames);
  }

  void AudioOutput_PortAudio::closeDevice()
  {
    paLockTimeout = 0U;
    if (paStream) {
      (void) isPortAudioError("calling Pa_StopStream()",
                              Pa_StopStream(paStream));
      closeDeviceLock.wait();
      (void) isPortAudioError("calling Pa_CloseStream()",
                              Pa_CloseStream(paStream));
      closeDeviceLock.notify();
      paStream = (PaStream *) 0;
    }
    disableRingBuffer = false;
    usingBlockingInterface = false;
    writeBufIndex = 0;
    readBufIndex = 0;
    buffers.clear();
    // call base class to reset internal state
    AudioOutput::closeDevice();
  }

  std::vector< std::string > AudioOutput_PortAudio::getDeviceList()
  {
    std::vector< std::string >  tmp;
    if (paInitialized) {
#ifndef USING_OLD_PORTAUDIO_API
      PaDeviceIndex devCnt = Pa_GetDeviceCount();
      PaDeviceIndex i;
#else
      PaDeviceID    devCnt = PaDeviceID(Pa_CountDevices());
      PaDeviceID    i;
#endif
      for (i = 0; i < devCnt; i++) {
        const PaDeviceInfo  *devInfo = Pa_GetDeviceInfo(i);
        if (devInfo) {
          if (devInfo->maxOutputChannels >= 2) {
            std::string s("");
            if (devInfo->name)
              s = devInfo->name;
#ifndef USING_OLD_PORTAUDIO_API
            const PaHostApiInfo *apiInfo = Pa_GetHostApiInfo(devInfo->hostApi);
            if (apiInfo) {
              s += " (";
              if (apiInfo->name)
                s += apiInfo->name;
              s += ")";
            }
#endif
            tmp.push_back(s);
          }
        }
      }
    }
    return tmp;
  }

#ifndef USING_OLD_PORTAUDIO_API
  int AudioOutput_PortAudio::portAudioCallback(const void *input, void *output,
                                               unsigned long frameCount,
                                               const PaStreamCallbackTimeInfo
                                                   *timeInfo,
                                               PaStreamCallbackFlags
                                                   statusFlags,
                                               void *userData)
#else
  int AudioOutput_PortAudio::portAudioCallback(void *input, void *output,
                                               unsigned long frameCount,
                                               PaTimestamp outTime,
                                               void *userData)
#endif
  {
    AudioOutput_PortAudio *p =
        reinterpret_cast<AudioOutput_PortAudio *>(userData);
    if (!p->closeDeviceLock.wait(0)) {
#ifndef USING_OLD_PORTAUDIO_API
      return int(paAbort);
#else
      return 1;
#endif
    }
    int16_t *buf = reinterpret_cast<int16_t *>(output);
    size_t  i = 0, nFrames = frameCount;
    (void) input;
#ifndef USING_OLD_PORTAUDIO_API
    (void) timeInfo;
    (void) statusFlags;
#else
    (void) outTime;
#endif
    if (nFrames > (p->buffers[p->readBufIndex].audioData.size() >> 1))
      nFrames = p->buffers[p->readBufIndex].audioData.size() >> 1;
    nFrames <<= 1;
    if (p->buffers[p->readBufIndex].paLock.wait(p->paLockTimeout)) {
      for ( ; i < nFrames; i++)
        buf[i] = p->buffers[p->readBufIndex].audioData[i];
    }
    p->buffers[p->readBufIndex].epLock.notify();
    if (++(p->readBufIndex) >= p->buffers.size())
      p->readBufIndex = 0;
    for ( ; i < (frameCount << 1); i++)
      buf[i] = 0;
    p->closeDeviceLock.notify();
#ifndef USING_OLD_PORTAUDIO_API
    return int(paContinue);
#else
    return 0;
#endif
  }

  void AudioOutput_PortAudio::openDevice()
  {
    writeBufIndex = 0;
    readBufIndex = 0;
    paStream = (PaStream *) 0;
    // find audio device
#ifndef USING_OLD_PORTAUDIO_API
    int     devCnt = int(Pa_GetDeviceCount());
#else
    int     devCnt = int(Pa_CountDevices());
#endif
    if (devCnt < 1)
      throw Exception("no audio device is available");
    int     devIndex;
    int     devNum = deviceNumber;
    for (devIndex = 0; devIndex < devCnt; devIndex++) {
      const PaDeviceInfo  *devInfo;
#ifndef USING_OLD_PORTAUDIO_API
      devInfo = Pa_GetDeviceInfo(PaDeviceIndex(devIndex));
#else
      devInfo = Pa_GetDeviceInfo(PaDeviceID(devIndex));
#endif
      if (!devInfo)
        throw Exception("error querying audio device information");
      if (devInfo->maxOutputChannels >= 2) {
        if (--devNum == -1) {
          devNum = devIndex;
          break;
        }
      }
    }
    if (devIndex >= devCnt)
      throw Exception("device number is out of range");
    usingBlockingInterface = false;
    int     nPeriodsHW_ = nPeriodsHW;
#ifndef USING_OLD_PORTAUDIO_API
    int     nPeriodsSW_ = nPeriodsSW;
    const PaDeviceInfo  *devInfo = Pa_GetDeviceInfo(PaDeviceIndex(devIndex));
    if (devInfo) {
      const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo(devInfo->hostApi);
      if (hostApiInfo) {
        if ((1UL << uint8_t(hostApiInfo->type))
            & ((1UL << uint8_t(paMME)) | (1UL << uint8_t(paOSS))
               | (1UL << uint8_t(paALSA)) | (1UL << uint8_t(paWASAPI)))) {
          nPeriodsHW_++;
          nPeriodsSW_ = 1;
          usingBlockingInterface = true;
        }
#  ifdef WIN32
        else if (hostApiInfo->type == paDirectSound) {
          nPeriodsSW_ = 1;
        }
        else {
          // ASIO or WDM-KS: force double buffering
          nPeriodsHW_ = 2;
          nPeriodsSW_ = (nPeriodsSW_ >= 2 ? nPeriodsSW_ : 2);
        }
#  else
        else {
          nPeriodsSW_ = (nPeriodsSW_ >= 2 ? nPeriodsSW_ : 2);
        }
#  endif
      }
    }
#else
    int     nPeriodsSW_ = 1;
#endif
    // calculate buffer size
    disableRingBuffer = (nPeriodsSW_ < 2);
    int     periodSize =
        int(totalLatency * (usingBlockingInterface ? 1.4142f : 0.7071f)
            * sampleRate + 0.5f)
        / (nPeriodsHW_ + nPeriodsSW_ - 2);
    for (int i = 16; i < 16384; i <<= 1) {
      if (i >= periodSize) {
        periodSize = i;
        break;
      }
    }
    if (periodSize > 16384)
      periodSize = 16384;
    latencyFramesHW = long(nPeriodsHW_ - 1) * long(periodSize);
    if (disableRingBuffer) {
      paLockTimeout = (unsigned int) (double(latencyFramesHW) * 1000.0
                                      / double(sampleRate) + 0.999);
    }
    else {
      paLockTimeout = 0U;
    }
    // initialize buffers
    buffers.resize(size_t(nPeriodsSW_));
    for (int i = 0; i < nPeriodsSW_; i++) {
      buffers[i].audioData.resize(size_t(periodSize) << 1);
      for (int j = 0; j < (periodSize << 1); j++)
        buffers[i].audioData[j] = 0;
    }
    // open audio stream
#ifndef USING_OLD_PORTAUDIO_API
    PaStreamParameters  streamParams;
    std::memset(&streamParams, 0, sizeof(PaStreamParameters));
    streamParams.device = PaDeviceIndex(devNum);
    streamParams.channelCount = 2;
    streamParams.sampleFormat = paInt16;
    streamParams.suggestedLatency =
        PaTime(double(periodSize) * (nPeriodsHW_ - 1) / sampleRate);
    streamParams.hostApiSpecificStreamInfo = (void *) 0;
    PaError err = paNoError;
    if (usingBlockingInterface)
      err = Pa_OpenStream(&paStream, (PaStreamParameters *) 0, &streamParams,
                          sampleRate, unsigned(periodSize),
                          paNoFlag, (PaStreamCallback *) 0, (void *) this);
    else
      err = Pa_OpenStream(&paStream, (PaStreamParameters *) 0, &streamParams,
                          sampleRate, unsigned(periodSize),
                          paNoFlag, &portAudioCallback, (void *) this);
    if (err == paNoError) {
      const PaStreamInfo  *streamInfo = Pa_GetStreamInfo(paStream);
      if (streamInfo) {
        // find out the actual buffer size
        long    tmp = long(double(streamInfo->outputLatency)
                           * streamInfo->sampleRate + 0.5);
        if (tmp >= 16L && tmp <= 262144L)
          latencyFramesHW = tmp;
      }
    }
#else
    PaError err =
        Pa_OpenStream(&paStream,
                      paNoDevice, 0, paInt16, (void *) 0,
                      PaDeviceID(devNum), 2, paInt16, (void *) 0,
                      sampleRate, unsigned(periodSize), unsigned(nPeriodsHW_),
                      paNoFlag, &portAudioCallback, (void *) this);
#endif
    if (isPortAudioError("calling Pa_OpenStream()", err)) {
      paStream = (PaStream *) 0;
      throw Exception("error opening audio device");
    }
    Pa_StartStream(paStream);
  }

}       // namespace Plus4Emu

