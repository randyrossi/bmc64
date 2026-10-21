//
// hvs.h
//
// The direct display driver's use of the HVS: finds the channel that feeds
// HDMI0 or, if none does, HDMI1, and shows planes on it through display
// lists BMC64 writes itself, scaled by the HVS with its own filter
// kernels. Frames are paced by the channel's frame counter.
//
// Display list memory used (words): the firmware keeps the first 32 and
// its own lists; BMC64 uses 224 words from 3072 (or 2048, or 1024,
// whichever doesn't overlap the firmware's list for the channel): two
// kernels, then two list slots of up to MaxPlanes planes, used in turn, so
// the list being shown is never written to.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_KMS_PI4_HVS_H_
#define BMC64_KMS_PI4_HVS_H_

#include "dlist.h"
#include "../common/filter.h"

#include <stdint.h>

namespace kms {

class Hvs {
public:
  Hvs(void);

  // Finds the running channel feeding HDMI0 (or else HDMI1) and its size,
  // and uploads the filter kernels. False if neither port is being fed.
  bool Attach(void);

  unsigned Port(void) const { return m_nPort; }      // 0 HDMI0, 1 HDMI1
  unsigned Channel(void) const { return m_nChannel; }
  unsigned Width(void) const { return m_nWidth; }
  unsigned Height(void) const { return m_nHeight; }

  // Shows PLANE (or, with 0, just the black background) from the next
  // frame on. False if the plane can't be shown.
  bool Show(const Plane *plane);
  // Shows COUNT planes, each on top of the ones before it (none: just the
  // background), from the next frame on. False if they can't be shown.
  static const unsigned MaxPlanes = 4;
  bool Show(const Plane *planes, unsigned count);

  // The channel's frame count (6 bits).
  unsigned FrameCount(void) const;
  // Waits until the frame count is no longer FROM, at most TIMEOUT_MS.
  // False on a time out.
  bool WaitForChange(unsigned from, unsigned timeout_ms) const;

  // Hands the channel back to the firmware's list (at power off, or if
  // something goes wrong).
  void Restore(void);

  // Stage 2 (modeset.cpp): stop and restart the channel for a new mode.
  void StopChannel(void);
  void StartChannel(unsigned width, unsigned height);

private:
  uint32_t Read(uint32_t reg) const;
  void Write(uint32_t reg, uint32_t value) const;
  void WriteList(unsigned word, uint32_t value) const;

  // The running channel feeding PORT's output, through its running pixel
  // valve, or false.
  bool FindChannel(unsigned port);

  uintptr_t m_Base;
  unsigned m_nPort;
  unsigned m_nChannel;
  unsigned m_nWidth, m_nHeight;
  uint32_t m_nFirmwareList;    // the firmware's DISPLISTx, to restore
  unsigned m_nRegion;          // BMC64's display list memory (words)
  unsigned m_nKernelSmooth, m_nKernelSharp;
  unsigned m_nSlot[2];
  unsigned m_nNextSlot;        // list slot written next: 0 or 1
  bool m_bAttached;
};

} // namespace kms

#endif
