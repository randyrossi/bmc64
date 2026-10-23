//
// screen.h
//
// BMA800XL - A bare metal Atari 800XL emulator for Raspberry Pi
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
// Written for BMC64's Raspberry Pi 4 and 5 support.
//
// The Pi 4's and Pi 5's screen: BMC64 takes over the HVS channel feeding
// the HDMI port with a display, and shows its layers (src/av/fbl.h) on it
// as planes of its own display lists, each scaled to its place by the HVS,
// the menus and status bar blended over the emulator's picture by their
// pixels' alpha. The video mode can be switched to run at the emulated
// machine's frame rate.
//
// Pixel buffers are 32-bit 0xAARRGGBB words. On the Pi 4, and on a Pi 5
// whose HVS reads the firmware's framebuffer at its ARM address, they are
// in the ARM's memory below 1 GB; otherwise they are carved out of the
// firmware's framebuffer (no longer shown), at the same offsets from the
// address the firmware's plane gives the HVS.
//
// Sources:
//  - Circle, include/circle/bcmframebuffer.h and lib/bcmframebuffer.cpp
//    (R. Stange, GPL 3): the firmware's framebuffer, asked for through the
//    mailbox property interface.
//  - Circle, include/circle/memory.h: HEAP_DMA30, memory below 1 GB that
//    the VideoCore can read; include/circle/bcm2835.h: BUS_ADDRESS();
//    include/circle/synchronize64.h: CleanAndInvalidateDataCacheRange(),
//    so what the ARM wrote is in memory before the HVS reads it.
//  - Circle, lib/sched/scheduler.cpp: CScheduler::Yield() runs Circle's
//    tasks on the core that calls it.
//  - BMA800XL, src/kms/pi4/display.cpp and src/kms/pi5/display.cpp: the
//    single plane version of this.
//

#ifndef BMC64_AV_SCREEN_H_
#define BMC64_AV_SCREEN_H_

#if RASPPI >= 5
#include "../kms/pi5/hvs.h"
#include "../kms/pi5/modeset.h"
#else
#include "../kms/pi4/hvs.h"
#include "../kms/pi4/modeset.h"
#endif

#include <circle/bcmframebuffer.h>
#include <stddef.h>
#include <stdint.h>

class Pi45Screen {
public:
  Pi45Screen(void);
  ~Pi45Screen(void);

  // At start up, on core 0: takes the firmware's framebuffer and the HVS.
  // False if the HVS isn't feeding either HDMI port.
  bool Initialize(void);
  bool Ready(void) const { return m_bAttached; }

  // The video mode's size.
  unsigned Width(void) const { return m_Hvs.Width(); }
  unsigned Height(void) const { return m_Hvs.Height(); }

  // Memory for BYTES of pixels the HVS (and the 3D GPU) can read, 4 KB
  // aligned and cleared; 0 if there is none left.
  uint32_t *Allocate(size_t bytes);
  void Free(uint32_t *buffer);
  // What the HVS is given for a pixel at P.
  uint64_t HvsAddress(const void *p) const;

  // Shows COUNT planes, the last on top, from the next frame on (none:
  // black). With WAIT, returns once they are on the screen. The buffers
  // shown before are no longer read once WaitForFlip() returns.
  bool Present(const kms::Plane *planes, unsigned count, bool wait);
  // Waits, if a frame presented is not on the screen yet, until it is.
  void WaitForFlip(void);

  // Switches the video mode to run at FPS frames a second, if the display
  // can. AUDIO_FS is the HDMI sound's sample rate. LINES:
  // 1080 or 720 for that size, 0 for the firmware's. True if the mode
  // changed (the layers are then laid out again).
  bool MatchRate(double fps, unsigned audio_fs, unsigned lines = 0);

  // A black screen (before halting).
  void Blank(void);

  static Pi45Screen *Get(void) { return s_pThis; }

private:
  bool WaitForChange(unsigned from);
  static void Reinstall(void *param);

  CBcmFrameBuffer *m_pFrameBuffer;
  kms::Hvs m_Hvs;
  kms::ModeSetter m_ModeSetter;
  bool m_bAttached;

  // Buffers in the firmware's framebuffer, when the HVS can't read the
  // ARM's memory (the Pi 5): handed out from the bottom, all freed
  // together.
  bool m_bPool;
  uint8_t *m_pPool;
  size_t m_nPoolBytes;
  size_t m_nPoolUsed;
  unsigned m_nPoolBuffers;
  uint64_t m_nPoolAddress;

  // What is on the screen, to put back after a mode change.
  kms::Plane m_Planes[kms::Hvs::MaxPlanes];
  unsigned m_nPlanes;
  bool m_bFlipPending;
  unsigned m_nFlipCount;
  bool m_bTimedOut;
  unsigned m_nFirmwareWidth, m_nFirmwareHeight;

  static Pi45Screen *s_pThis;
};

#endif
