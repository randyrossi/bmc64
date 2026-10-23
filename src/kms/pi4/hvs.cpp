//
// hvs.cpp
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
// Modified for BMC64 from BMA800XL's src/kms/pi4/hvs.cpp: shows up to four
// planes.
//
// See hvs.h.
//

#include "hvs.h"

#include "regs.h"

#include <circle/memio.h>
#include <circle/synchronize.h>
#include <circle/timer.h>

namespace kms {

namespace {

// BMC64's part of display list memory: two kernels, then two list
// slots, each with room for MaxPlanes planes and the end word; from one of
// these bases, whichever doesn't overlap the firmware's list.
const unsigned SlotWords = 100;
const unsigned RegionWords = 2 * hvs::KernelWords + 2 + 2 * SlotWords;
const unsigned RegionBases[] = {3072, 2048, 1024};
// Line buffer memory for the planes (words), after the firmware's.
const unsigned LbmBase = 32768;

} // namespace

Hvs::Hvs(void)
    : m_Base(FromBus(hvs::Bus)), m_nPort(0), m_nChannel(0), m_nWidth(0), m_nHeight(0),
      m_nFirmwareList(0), m_nRegion(0), m_nKernelSmooth(0), m_nKernelSharp(0),
      m_nNextSlot(0), m_bAttached(false) {
  m_nSlot[0] = m_nSlot[1] = 0;
}

uint32_t Hvs::Read(uint32_t reg) const { return read32(m_Base + reg); }

void Hvs::Write(uint32_t reg, uint32_t value) const {
  write32(m_Base + reg, value);
}

void Hvs::WriteList(unsigned word, uint32_t value) const {
  write32(m_Base + hvs::DlistOffset + 4 * word, value);
}

bool Hvs::FindChannel(unsigned port) {
  // The port's pixel valve must be running too: a mux field the firmware
  // never set says nothing.
  uintptr_t pv = FromBus(Ports[port].pv);
  if (!(read32(pv + pv::Control) & pv::Enable) ||
      !(read32(pv + pv::VControl) & pv::VideoEnable)) {
    return false;
  }
  uint32_t mux = GetField(Read(Ports[port].hvs_mux), hvs::OutputMuxHigh,
                          hvs::OutputMuxLow);
  if (mux == hvs::MuxNone) {
    return false;
  }
  uint32_t ctrl = Read(hvs::DispCtrlX(mux));
  uint32_t mode = GetField(Read(hvs::DispStatX(mux)), hvs::StatModeHigh,
                           hvs::StatModeLow);
  if (!(ctrl & hvs::CtrlEnable) || mode != hvs::ModeRun) {
    return false;
  }
  unsigned width = GetField(ctrl, hvs::CtrlWidthHigh, hvs::CtrlWidthLow);
  unsigned height = GetField(ctrl, hvs::CtrlHeightHigh, hvs::CtrlHeightLow);
  if (width == 0 || height == 0) {
    return false;
  }
  m_nPort = port;
  m_nChannel = mux;
  m_nWidth = width;
  m_nHeight = height;
  return true;
}

bool Hvs::Attach(void) {
  if (!(Read(hvs::DispCtrl) & Bit(31))) {
    return false;
  }
  // The port the firmware shows its picture on; HDMI0 if both.
  if (!FindChannel(0) && !FindChannel(1)) {
    return false;
  }
  m_nFirmwareList = Read(hvs::DispList(m_nChannel));
  if (m_nFirmwareList >= hvs::DlistWords) {
    return false;
  }

  // The firmware's list: entries until the end word.
  unsigned fw_end = m_nFirmwareList;
  while (fw_end < hvs::DlistWords) {
    uint32_t word = read32(m_Base + hvs::DlistOffset + 4 * fw_end);
    if (word & hvs::CtlEnd) {
      break;
    }
    unsigned size = GetField(word, hvs::CtlSizeHigh, hvs::CtlSizeLow);
    if (size == 0) {
      break;
    }
    fw_end += size;
  }
  m_nRegion = 0;
  for (unsigned base : RegionBases) {
    if (base + RegionWords <= m_nFirmwareList || base > fw_end) {
      m_nRegion = base;
      break;
    }
  }
  if (m_nRegion == 0) {
    return false;
  }
  m_nKernelSmooth = m_nRegion;
  m_nKernelSharp = m_nRegion + hvs::KernelWords;
  m_nSlot[0] = m_nRegion + 2 * hvs::KernelWords + 2;
  m_nSlot[1] = m_nSlot[0] + SlotWords;

  uint32_t words[hvs::KernelWords];
  KernelWords(Filter::Smooth, words);
  for (unsigned i = 0; i < hvs::KernelWords; i++) {
    WriteList(m_nKernelSmooth + i, words[i]);
  }
  KernelWords(Filter::Sharp, words);
  for (unsigned i = 0; i < hvs::KernelWords; i++) {
    WriteList(m_nKernelSharp + i, words[i]);
  }

  // Black wherever no plane is (the picture is letterboxed).
  Write(hvs::DispBkgndX(m_nChannel),
        Read(hvs::DispBkgndX(m_nChannel)) | hvs::BkgndFill);
  m_bAttached = true;
  return true;
}

bool Hvs::Show(const Plane *plane) { return Show(plane, plane != 0 ? 1 : 0); }

bool Hvs::Show(const Plane *planes, unsigned count) {
  if (!m_bAttached || count > MaxPlanes) {
    return false;
  }
  static_assert(MaxPlanes * (MaxWords - 1) + 1 <= SlotWords, "list slot too small");
  uint32_t words[MaxPlanes * MaxWords];
  unsigned n = 0;
  unsigned lbm = LbmBase;
  for (unsigned i = 0; i < count; i++) {
    const Plane &plane = planes[i];
    // Each entry's end word is overwritten by the next entry.
    unsigned used = BuildList(plane, lbm, plane.nearest ? m_nKernelSharp : m_nKernelSmooth,
                              &words[n]);
    if (used == 0 || lbm + LbmWords(plane) > hvs::LbmWords) {
      return false;
    }
    lbm += LbmWords(plane);
    n += used - 1;
  }
  words[n++] = hvs::CtlEnd;
  unsigned slot = m_nSlot[m_nNextSlot];
  for (unsigned i = 0; i < n; i++) {
    WriteList(slot + i, words[i]);
  }
  DataSyncBarrier();
  // Taken from the start of the next frame.
  Write(hvs::DispList(m_nChannel), slot);
  m_nNextSlot ^= 1;
  return true;
}

unsigned Hvs::FrameCount(void) const {
  switch (m_nChannel) {
  case 0:
    return GetField(Read(hvs::DispStat1), 25, 20);
  case 1:
    return GetField(Read(hvs::DispStat1), 19, 14);
  default:
    return GetField(Read(hvs::DispStat2), 19, 14);
  }
}

bool Hvs::WaitForChange(unsigned from, unsigned timeout_ms) const {
  u64 start = CTimer::GetClockTicks64();
  u64 limit = (u64)timeout_ms * (CLOCKHZ / 1000);
  while (FrameCount() == from) {
    if (CTimer::GetClockTicks64() - start > limit) {
      return false;
    }
  }
  return true;
}

void Hvs::Restore(void) {
  if (m_bAttached) {
    Write(hvs::DispList(m_nChannel), m_nFirmwareList);
  }
}

void Hvs::StopChannel(void) {
  uint32_t reg = hvs::DispCtrlX(m_nChannel);
  if (!(Read(reg) & hvs::CtrlEnable)) {
    return;
  }
  Write(reg, hvs::CtrlReset);
  Write(reg, 0);
}

void Hvs::StartChannel(unsigned width, unsigned height) {
  uint32_t reg = hvs::DispCtrlX(m_nChannel);
  Write(reg, 0);
  Write(reg, hvs::CtrlReset);
  Write(reg, 0);
  Write(reg, hvs::CtrlEnable |
                 Field(width, hvs::CtrlWidthHigh, hvs::CtrlWidthLow) |
                 Field(height, hvs::CtrlHeightHigh, hvs::CtrlHeightLow));
  uint32_t bkgnd = Read(hvs::DispBkgndX(m_nChannel));
  bkgnd &= ~(hvs::BkgndBackToBack | hvs::BkgndGamma | hvs::BkgndInterlace);
  Write(hvs::DispBkgndX(m_nChannel), bkgnd | hvs::BkgndFill);
  m_nWidth = width;
  m_nHeight = height;
}

} // namespace kms
