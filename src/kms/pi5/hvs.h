//
// hvs.h
//
// The Pi 5's direct display driver's use of the BCM2712's HVS: finds the
// channel that feeds the HDMI port in use, and shows planes on it
// through display lists BMC64 writes itself, scaled by the HVS with its
// own filter kernels. Frames are paced by the channel's frame counter.
//
// The firmware's live lists (on both channels) are read first, so that
// BMC64's display list words, prefetch buffer and line buffer stay
// clear of the memory they use. Display list memory used: 88 words from
// 3072 (or 2048, or 1024): two kernels, then two list slots used in turn,
// so the list being shown is never written to.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_KMS_PI5_HVS_H_
#define BMC64_KMS_PI5_HVS_H_

#include "dlist.h"
#include "../common/filter.h"

#include <stdint.h>

namespace kms {

class Hvs {
public:
  Hvs(void);

  // Finds the running channel feeding the HDMI port with a display plugged
  // in (else HDMI0, else HDMI1) and its size, and uploads the filter
  // kernels. False if neither port is being fed.
  bool Attach(void);

  unsigned Port(void) const { return m_nPort; }      // 0 HDMI0, 1 HDMI1
  unsigned Channel(void) const { return m_nChannel; }
  unsigned Width(void) const { return m_nWidth; }
  unsigned Height(void) const { return m_nHeight; }
  bool SteppingD(void) const { return m_bD; }

  // The address of the first pixel of the firmware's plane on the
  // channel, as the HVS sees it (0 if there was none).
  uint64_t FirmwareAddress(void) const { return m_nFirmwareAddress; }
  // Its pitch in bytes.
  unsigned FirmwarePitch(void) const { return m_nFirmwarePitch; }

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

  // Mode setting (modeset.cpp): stop and restart the channel for a new
  // mode.
  void StopChannel(void);
  void StartChannel(unsigned width, unsigned height);

private:
  uint32_t Read(uint32_t reg) const;
  void Write(uint32_t reg, uint32_t value) const;
  uint32_t ReadChannel(unsigned chan, uint32_t reg) const;
  void WriteChannel(unsigned chan, uint32_t reg, uint32_t value) const;
  void WriteList(unsigned word, uint32_t value) const;
  uint32_t ReadList(unsigned word) const;

  // The running channel feeding PORT, through its running pixel valve,
  // or false.
  bool FindChannel(unsigned port);
  bool ChannelRunning(unsigned chan) const;
  // Notes the memory used by the live list on channel CHAN.
  void NoteFirmwareList(unsigned chan);
  bool Overlaps(unsigned base, unsigned size, const unsigned (*areas)[2],
                unsigned count) const;
  // A free stretch of SIZE units in a memory of TOTAL, clear of AREAS.
  bool FindFree(unsigned size, unsigned total, const unsigned (*areas)[2], unsigned count,
                unsigned *base) const;

  uintptr_t m_Base;
  bool m_bD;
  unsigned m_nPort;
  unsigned m_nChannel;
  unsigned m_nWidth, m_nHeight;
  uint32_t m_nFirmwareList;    // the firmware's list start, to restore
  uint64_t m_nFirmwareAddress;
  unsigned m_nFirmwarePitch;
  unsigned m_nRegion;          // BMC64's display list memory (words)
  unsigned m_nKernelSmooth, m_nKernelSharp;
  unsigned m_nSlot[2];
  unsigned m_nNextSlot;        // list slot written next: 0 or 1
  bool m_bAttached;

  // What the firmware's live lists use: display list words, prefetch
  // (handle bit mask, areas in units) and line buffer (areas in blocks).
  static const unsigned MaxAreas = 8;
  unsigned m_ListAreas[MaxAreas][2];
  unsigned m_nListAreas;
  uint32_t m_nUpmHandlesUsed;
  unsigned m_UpmAreas[MaxAreas][2];
  unsigned m_nUpmAreas;
  unsigned m_LbmAreas[MaxAreas][2];
  unsigned m_nLbmAreas;
  unsigned m_UpmHandle[MaxPlanes];  // BMC64's prefetch handles
  unsigned m_nUpmHandles;
};

} // namespace kms

#endif
