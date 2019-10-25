
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

#ifndef PLUS4EMU_SOUNDIO_HPP
#define PLUS4EMU_SOUNDIO_HPP

#include "plus4emu.hpp"

#include <sndfile.h>
#include <vector>

namespace Plus4Emu {

  class AudioOutput {
   private:
    std::string outputFileName;
    SNDFILE *soundFile;
   protected:
    int     deviceNumber;
    float   sampleRate;
    float   totalLatency;
    int     nPeriodsHW;
    int     nPeriodsSW;
   public:
    AudioOutput();
    virtual ~AudioOutput();
    /*!
     * Set audio output parameters (changing these settings implies
     * restarting the audio output stream if it is already open).
     */
    void setParameters(int deviceNumber_, float sampleRate_,
                       float totalLatency_ = 0.1f,
                       int nPeriodsHW_ = 4, int nPeriodsSW_ = 4);
    inline float getSampleRate() const
    {
      return this->sampleRate;
    }
    /*!
     * Write sound output to the specified file name, closing any
     * previously opened file with a different name.
     * If the name is an empty string, no file is written.
     */
    void setOutputFile(const std::string& fileName);
    /*!
     * Write 'nFrames' mono samples from 'buf' (in 16 bit signed PCM format)
     * to the audio output device and file.
     */
    virtual void sendAudioData(const int16_t *buf, size_t nFrames);
    /*!
     * Close the audio device.
     */
    virtual void closeDevice();
    /*!
     * Returns an array of the available audio device names,
     * indexed by the device number (starting from zero).
     */
    virtual std::vector< std::string > getDeviceList();
   protected:
    virtual void openDevice();
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_SOUNDIO_HPP

