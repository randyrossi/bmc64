//
// vicesound.cpp
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
// See vicesound.h.
//

#include "vicesound.h"

#include "sound.h"

#include <circle/sched/scheduler.h>
#include <circle/timer.h>

// The one sound device, started on core 0.
static Pi4Sound s_Sound;

// The VCHIQ volume (BMC64's src/kernel.cpp maps 0..100 % onto -2720..400)
// as a percentage.
static int VolumePercent(int volume) {
  int percent = (volume + 2720) * 100 / (VCHIQ_SOUND_VOLUME_MAX + 2720);
  return percent < 0 ? 0 : percent > 100 ? 100 : percent;
}

bool ViceSound::Start(CInterruptSystem *pInterrupt, TVCHIQSoundDestination Destination) {
  Pi4Sound::Output output = Pi4Sound::Output::Auto;
  if (Destination == VCHIQSoundDestinationHDMI) {
    output = Pi4Sound::Output::Hdmi;
  } else if (Destination == VCHIQSoundDestinationHeadphones) {
    output = Pi4Sound::Output::Headphones;
  }
  return s_Sound.Initialize(pInterrupt, 100, output);
}

ViceSound::ViceSound(CVCHIQDevice *pVCHIQDevice, TVCHIQSoundDestination Destination)
    : num_channels(1), playing(false) {
  (void)pVCHIQDevice;
  (void)Destination;
}

ViceSound::~ViceSound(void) {}

boolean ViceSound::Playback(int volume, int channels) {
  num_channels = channels == 2 ? 2 : 1;
  s_Sound.SetVolume(VolumePercent(volume));
  playing = s_Sound.Open((int)num_channels, FRAG_SIZE * NUM_FRAGS) != 0;
  return playing;
}

boolean ViceSound::PlaybackActive(void) const { return playing && s_Sound.Active(); }

void ViceSound::CancelPlayback(void) {
  // The device keeps running (it sends silence when nothing is written);
  // it can only be started on core 0.
  playing = false;
}

void ViceSound::SetControl(int nVolume, TVCHIQSoundDestination Destination) {
  (void)Destination;  // chosen at start up
  s_Sound.SetVolume(VolumePercent(nVolume));
}

unsigned ViceSound::AddChunk(s16 *pBuffer, unsigned nChunkSize) {
  if (!PlaybackActive()) {
    return 0;
  }
  unsigned frames = nChunkSize / num_channels;
  // VICE expects to wait if the buffer is full, but it shouldn't be.
  unsigned start = CTimer::GetClockTicks();
  while (s_Sound.FreeFrames() < frames &&
         CTimer::GetClockTicks() - start < CLOCKHZ / 10) {
    CScheduler::Get()->Yield();
  }
  s_Sound.Write(pBuffer, frames);
  return 0;
}

unsigned ViceSound::BufferSpaceSamples() {
  if (!PlaybackActive()) {
    return FRAG_SIZE * NUM_FRAGS;
  }
  return s_Sound.FreeFrames();
}
