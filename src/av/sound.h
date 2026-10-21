//
// sound.h
//
// The Pi 4's and 5's sound, without the VideoCore firmware: Circle's HDMI
// sound driver, adapted for either HDMI port (hdmisound.h), which sends the
// samples to the port the display is on by DMA, or Circle's PWM sound
// driver (CPWMSoundBaseDevice) for the Pi 4 B's headphone jack.
// The emulator's samples go into the driver's queue (Write()).
//
// Sources:
//  - Circle, include/circle/sound/hdmisoundbasedevice.h and
//    lib/sound/hdmisoundbasedevice.cpp (R. Stange, GPL 3): the HDMI driver;
//    HDMI0 only on the Pi 4, chunk sizes in multiples of 384 samples;
//    Start() fails when the display takes no audio ("Requires HDMI display
//    with audio support").
//  - Circle, include/circle/sound/pwmsoundbasedevice.h and
//    lib/sound/pwmsoundbasedevice.cpp: the PWM driver, on the Pi 4 PWM1
//    clocked at 125 MHz (so 125 MHz / 48 kHz = 2604 steps, about 11 bits);
//    the chunk size counts words, two per stereo frame;
//    lib/machineinfo.cpp: its pins on the Pi 4 are GPIO 40 and 41, the
//    headphone jack's; README.md: "PWM sound output (on headphone jack)".
//  - Raspberry Pi 400, 5, 500 and 500+ product briefs: no headphone jack
//    (Circle's README.md: PWM sound on the Pi 5 only "with adapter").
//  - Circle, include/circle/sound/soundbasedevice.h: the queue
//    (AllocateQueueFrames, SetWriteFormat, Write, GetQueueFramesAvail) and
//    the rule that only Write(), GetQueueFramesAvail() and IsActive() may
//    be called on other cores than core 0.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_AV_SOUND_H_
#define BMC64_AV_SOUND_H_

#include <circle/interrupt.h>
#include <circle/sound/soundbasedevice.h>
#include <stdint.h>

class Pi4Sound {
public:
  enum { kSampleRate = 48000 };

  Pi4Sound(void);
  ~Pi4Sound(void);

  // Where the sound goes (the audio_out option): HDMI, falling back to the
  // headphone jack if the display takes no sound (Auto); HDMI; or the
  // headphone jack (the Pi 400 and 5 have none: HDMI there).
  enum class Output { Auto, Hdmi, Headphones };

  // Core 0, at start up. VOLUME is 0..100 percent.
  bool Initialize(CInterruptSystem *pInterrupt, int volume, Output output);

  // From the emulator. LATENCY_FRAMES is how much sound to keep queued,
  // in frames at kSampleRate.
  unsigned Open(int channels, unsigned latency_frames);
  // VOLUME is 0..100 percent.
  void SetVolume(int volume);
  bool Active(void) const { return m_pDevice != 0 && m_pDevice->IsActive(); }
  unsigned FreeFrames(void);
  void Write(const int16_t *samples, unsigned frames);

private:
  bool Start(CSoundBaseDevice *device, const char *name);

  CSoundBaseDevice *m_pDevice;
  int m_nChannels;        // what the emulator writes: 1 or 2
  unsigned m_nLatency;    // frames at kSampleRate
  int m_nGain;            // 0..256
};

#endif
