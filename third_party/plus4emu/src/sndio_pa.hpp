
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

#ifndef PLUS4EMU_SNDIO_PA_HPP
#define PLUS4EMU_SNDIO_PA_HPP

#include "plus4emu.hpp"
#include "system.hpp"
#include "soundio.hpp"

#include <portaudio.h>
#include <vector>

namespace Plus4Emu {

  class AudioOutput_PortAudio : public AudioOutput {
   private:
    struct Buffer {
      ThreadLock  paLock;
      ThreadLock  epLock;
      std::vector< int16_t >    audioData;
      size_t      writePos;
      Buffer()
        : paLock(true), epLock(false), writePos(0)
      {
      }
      ~Buffer()
      {
      }
    };
    bool          paInitialized;
    bool          disableRingBuffer;
    bool          usingBlockingInterface;
    unsigned int  paLockTimeout;
    std::vector< Buffer >   buffers;
    size_t        writeBufIndex;
    size_t        readBufIndex;
    PaStream      *paStream;
    long          latencyFramesHW;
    Timer         timer_;
    double        nextTime;
    ThreadLock    closeDeviceLock;
#ifndef USING_OLD_PORTAUDIO_API
    static int portAudioCallback(const void *input, void *output,
                                 unsigned long frameCount,
                                 const PaStreamCallbackTimeInfo *timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData);
#else
    static int portAudioCallback(void *input, void *output,
                                 unsigned long frameCount,
                                 PaTimestamp outTime, void *userData);
#endif
   public:
    AudioOutput_PortAudio();
    virtual ~AudioOutput_PortAudio();
    virtual void sendAudioData(const int16_t *buf, size_t nFrames);
    virtual void closeDevice();
    virtual std::vector< std::string > getDeviceList();
   protected:
    virtual void openDevice();
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_SNDIO_PA_HPP

