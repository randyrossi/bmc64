//
// vicesound.h
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _vice_sound_h
#define _vice_sound_h

#include "defs.h"
#include "vicesoundbasedevice.h"
#include <circle/types.h>
#include <vc4/vchiq/vchiqdevice.h>

// This is the fragment size we give to vice.
#define FRAG_SIZE 256

// This is the number of fragments we want for our buffer.
#define NUM_FRAGS 16

// This is the number of bytes we send to VC4 at a time.  It doesn't
// have to be equal to FRAG_SIZE but keep in mind this more or less
// determines the 'resolution' of our amount buffered tracking since
// that value goes up/down by this amount each time we queue data or
// get notified a previous chunk has completed. Vice seems to be
// sensitive to that value and should not jump around too much.
#define CHUNK_SIZE 1024

// 16 bit sound means this many bytes per sample.
#define BYTES_PER_SAMPLE 2

class ViceSound : private ViceSoundBaseDevice {
public:
  /// \param pVCHIQDevice	pointer to the VCHIQ interface device
  /// \param Destination	the target device, the sound data is sent to\n
  ///			(detected automatically, if equal to
  ///VCHIQSoundDestinationAuto)
  ViceSound(CVCHIQDevice *pVCHIQDevice,
            TVCHIQSoundDestination Destination = VCHIQSoundDestinationAuto);

  ~ViceSound(void);

  /// \brief Starts playback
  /// \return Operation successful?
  boolean Playback(int volume, int channels);

  /// \return Is playback running?
  boolean PlaybackActive(void) const;

  /// \brief Stops playback
  /// \note Cancel takes effect after a short delay
  void CancelPlayback(void);

  /// \param nVolume	Output volume to be set (-10000..400)
  /// \param Destination	the target device, the sound data is sent to\n
  ///			(not modified, if equal to VCHIQSoundDestinationUnknown)
  /// \note This method can be called, while playback is running.
  void
  SetControl(int nVolume,
             TVCHIQSoundDestination Destination = VCHIQSoundDestinationUnknown);
  unsigned AddChunk(s16 *pBuffer, unsigned nChunkSize);
  unsigned BufferSpaceSamples();

private:
  unsigned GetChunk(s16 *pBuffer, unsigned nChunkSize);
  void AmountBufferedBytes(unsigned);

  // Keep track of how many bytes we've sent to VC
  unsigned int bytes_buffered;

  // Pointer to sample source while we draw from vice's buffer
  s16 *src_buffer;
  // Position within src_buffer we are drawing next chunk from
  unsigned int src_pos;
  unsigned int src_size;
  unsigned int num_channels;
};

#endif // VICE_SOUND_H
