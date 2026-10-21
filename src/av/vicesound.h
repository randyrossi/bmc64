//
// vicesound.h
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
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
