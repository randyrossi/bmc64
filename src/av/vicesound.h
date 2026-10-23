//
// vicesound.h
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// The Pi 4's and Pi 5's ViceSound: the same interface as BMC64's VCHIQ one
// (src/vicesound.h, which includes this on those boards), on the HDMI and
// headphone jack drivers of src/av/sound.h. The device is started on core
// 0 at start up (Start()); VICE then opens, writes and checks the queue
// from its core.
//
// Sources:
//  - BMC64, src/vicesound.h, src/vicesoundbasedevice.h and src/kernel.cpp:
//    the interface, its constants, the volume scale and what VICE expects
//    of the buffer (FRAG_SIZE * NUM_FRAGS frames).
//

#ifndef BMC64_AV_VICESOUND_H_
#define BMC64_AV_VICESOUND_H_

#include "../defs.h"
#include <circle/interrupt.h>
#include <circle/types.h>
#include <vc4/vchiq/vchiqdevice.h>

#define FRAG_SIZE 256
#define NUM_FRAGS 16
#define CHUNK_SIZE 1024
#define BYTES_PER_SAMPLE 2

#define VCHIQ_SOUND_VOLUME_MIN -10000
#define VCHIQ_SOUND_VOLUME_DEFAULT 0
#define VCHIQ_SOUND_VOLUME_MAX 400

enum TVCHIQSoundDestination {
  VCHIQSoundDestinationAuto,
  VCHIQSoundDestinationHeadphones,
  VCHIQSoundDestinationHDMI,
  VCHIQSoundDestinationUnknown
};

class ViceSound {
public:
  ViceSound(CVCHIQDevice *pVCHIQDevice,
            TVCHIQSoundDestination Destination = VCHIQSoundDestinationAuto);
  ~ViceSound(void);

  // Core 0, at start up: starts the sound device for DESTINATION.
  static bool Start(CInterruptSystem *pInterrupt, TVCHIQSoundDestination Destination);

  // VOLUME on the VCHIQ scale (see src/kernel.cpp).
  boolean Playback(int volume, int channels);
  boolean PlaybackActive(void) const;
  void CancelPlayback(void);
  void SetControl(int nVolume,
                  TVCHIQSoundDestination Destination = VCHIQSoundDestinationUnknown);
  // NCHUNKSIZE samples (frames times channels).
  unsigned AddChunk(s16 *pBuffer, unsigned nChunkSize);
  // Frames VICE may write before the queue is full.
  unsigned BufferSpaceSamples();

private:
  unsigned num_channels;
  bool playing;
};

#endif
