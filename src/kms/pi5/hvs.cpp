//
// hvs.cpp
//
// See hvs.h.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#include "hvs.h"

#include "regs.h"

#include <circle/memio.h>
#include <circle/synchronize.h>
#include <circle/timer.h>

namespace kms {

namespace {

// BMC64's part of display list memory: two kernels, then two list
// slots, each with room for MaxPlanes planes and the end word, from one of
// these bases, whichever doesn't overlap the firmware's lists.
const unsigned SlotWords = 100;
const unsigned RegionWords = 2 * hvs::KernelWords + 2 + 2 * SlotWords;
const unsigned RegionBases[] = {3072, 2048, 1024};

// How long to watch a channel's frame count to see that it runs.
const unsigned RunCheckMs = 100;

} // namespace

Hvs::Hvs(void)
    : m_Base(FromBus(hvs::Bus)), m_bD(false), m_nPort(0), m_nChannel(0), m_nWidth(0),
      m_nHeight(0), m_nFirmwareList(0), m_nFirmwareAddress(0), m_nFirmwarePitch(0), m_nRegion(0),
      m_nKernelSmooth(0), m_nKernelSharp(0), m_nNextSlot(0), m_bAttached(false),
      m_nListAreas(0), m_nUpmHandlesUsed(0), m_nUpmAreas(0), m_nLbmAreas(0),
      m_nUpmHandles(0) {
  m_nSlot[0] = m_nSlot[1] = 0;
}

uint32_t Hvs::Read(uint32_t reg) const { return read32(m_Base + reg); }

void Hvs::Write(uint32_t reg, uint32_t value) const { write32(m_Base + reg, value); }

uint32_t Hvs::ReadChannel(unsigned chan, uint32_t reg) const {
  return Read(hvs::ChannelBase(m_bD, chan) + reg);
}

void Hvs::WriteChannel(unsigned chan, uint32_t reg, uint32_t value) const {
  Write(hvs::ChannelBase(m_bD, chan) + reg, value);
}

void Hvs::WriteList(unsigned word, uint32_t value) const {
  write32(m_Base + hvs::DlistOffset + 4 * word, value);
}

uint32_t Hvs::ReadList(unsigned word) const {
  return read32(m_Base + hvs::DlistOffset + 4 * word);
}

// Enabled and composing frames: the frame count moves.
bool Hvs::ChannelRunning(unsigned chan) const {
  if (!(ReadChannel(chan, hvs::Ctrl0) & hvs::CtrlEnable)) {
    return false;
  }
  uint32_t status = ReadChannel(chan, hvs::Status(m_bD));
  if (GetField(status, hvs::StatModeHigh, hvs::StatModeLow) == hvs::ModeDisabled) {
    return false;
  }
  unsigned from = GetField(status, hvs::StatFrameHigh, hvs::StatFrameLow);
  u64 start = CTimer::GetClockTicks64();
  u64 limit = (u64)RunCheckMs * (CLOCKHZ / 1000);
  while (GetField(ReadChannel(chan, hvs::Status(m_bD)), hvs::StatFrameHigh,
                  hvs::StatFrameLow) == from) {
    if (CTimer::GetClockTicks64() - start > limit) {
      return false;
    }
  }
  return true;
}

bool Hvs::FindChannel(unsigned port) {
  // The port's pixel valve must be running too.
  uintptr_t pv = FromBus(Ports[port].pv);
  if (!(read32(pv + pv::Control) & pv::Enable) ||
      !(read32(pv + pv::VControl) & pv::VideoEnable)) {
    return false;
  }
  unsigned chan = Ports[port].channel;
  if (chan == 1 &&
      GetField(Read(hvs::Control), hvs::ControlDsp1High, hvs::ControlDsp1Low) != 0) {
    return false;
  }
  if (!ChannelRunning(chan)) {
    return false;
  }
  uint32_t ctrl = ReadChannel(chan, hvs::Ctrl0);
  m_nPort = port;
  m_nChannel = chan;
  m_nWidth = GetField(ctrl, hvs::CtrlWidthHigh, hvs::CtrlWidthLow) + 1;
  m_nHeight = GetField(ctrl, hvs::CtrlHeightHigh, hvs::CtrlHeightLow) + 1;
  return true;
}

void Hvs::NoteFirmwareList(unsigned chan) {
  unsigned start = GetField(ReadChannel(chan, hvs::Dl(m_bD)), hvs::ListHigh, hvs::ListLow);
  unsigned word = start;
  bool first = true;
  while (word < hvs::DlistWords) {
    uint32_t entry[64];
    unsigned available = hvs::DlistWords - word < 64 ? hvs::DlistWords - word : 64;
    for (unsigned i = 0; i < available; i++) {
      entry[i] = ReadList(word + i);
    }
    EntryUse use;
    if (!ReadEntry(entry, available, &use)) {
      break;
    }
    if (use.has_upm) {
      m_nUpmHandlesUsed |= 1u << use.upm_handle;
      if (m_nUpmAreas < MaxAreas) {
        m_UpmAreas[m_nUpmAreas][0] = use.upm_base;
        m_UpmAreas[m_nUpmAreas][1] = use.upm_units;
        m_nUpmAreas++;
      }
    }
    if (use.has_lbm && m_nLbmAreas < MaxAreas) {
      m_LbmAreas[m_nLbmAreas][0] = use.lbm;
      m_LbmAreas[m_nLbmAreas][1] = use.lbm_blocks;
      m_nLbmAreas++;
    }
    if (first && chan == m_nChannel) {
      bool unity = (entry[0] & hvs::CtlUnity) != 0;
      unsigned ptr0 = unity ? 5 : 6;
      m_nFirmwareAddress =
          ((uint64_t)GetField(entry[ptr0], hvs::Ptr0AddressHigh, hvs::Ptr0AddressLow) << 32) |
          entry[ptr0 + 1];
      m_nFirmwarePitch = GetField(entry[ptr0 + 2], hvs::PitchHigh, hvs::PitchLow);
    }
    first = false;
    word += use.words;
  }
  // The list's words, with its end word.
  if (m_nListAreas < MaxAreas) {
    m_ListAreas[m_nListAreas][0] = start;
    m_ListAreas[m_nListAreas][1] = word + 1 - start;
    m_nListAreas++;
  }
}

bool Hvs::Overlaps(unsigned base, unsigned size, const unsigned (*areas)[2],
                   unsigned count) const {
  for (unsigned i = 0; i < count; i++) {
    if (base < areas[i][0] + areas[i][1] && areas[i][0] < base + size) {
      return true;
    }
  }
  return false;
}

// The highest free stretch, away from the firmware's allocations (which
// start at the bottom, and may not all be read): at the top, or just below
// one of the areas.
bool Hvs::FindFree(unsigned size, unsigned total, const unsigned (*areas)[2],
                   unsigned count, unsigned *base) const {
  bool found = false;
  unsigned best = 0;
  for (unsigned i = 0; i <= count; i++) {
    unsigned end = i == count ? total : areas[i][0];
    if (end < size || end > total) {
      continue;
    }
    unsigned at = end - size;
    if (!Overlaps(at, size, areas, count) && (!found || at > best)) {
      best = at;
      found = true;
    }
  }
  *base = best;
  return found;
}

bool Hvs::Attach(void) {
  uint32_t version = GetField(Read(hvs::Version), 7, 0);
  if (version != hvs::VersionC && version != hvs::VersionD) {
    return false;
  }
  m_bD = version == hvs::VersionD;
  if (!(Read(hvs::Control) & hvs::ControlEnable)) {
    return false;
  }
  // The port the firmware shows its picture on, with a display plugged in
  // (the firmware may run the other port without one); else the first
  // running port.
  bool found = false;
  for (unsigned port = 0; port < 2 && !found; port++) {
    found = (read32(FromBus(Ports[port].core) + hdmi::Hotplug) & hdmi::HotplugConnected) &&
            FindChannel(port);
  }
  if (!found && !FindChannel(0) && !FindChannel(1)) {
    return false;
  }
  m_nFirmwareList = GetField(ReadChannel(m_nChannel, hvs::Dl(m_bD)), hvs::ListHigh,
                             hvs::ListLow);

  // The memory used by the firmware's lists on every running channel.
  m_nListAreas = m_nUpmAreas = m_nLbmAreas = 0;
  m_nUpmHandlesUsed = 0;
  m_nFirmwareAddress = 0;
  m_nFirmwarePitch = 0;
  for (unsigned chan = 0; chan < 3; chan++) {
    if (ReadChannel(chan, hvs::Ctrl0) & hvs::CtrlEnable) {
      NoteFirmwareList(chan);
    }
  }
  m_nRegion = 0;
  for (unsigned base : RegionBases) {
    if (!Overlaps(base, RegionWords, m_ListAreas, m_nListAreas)) {
      m_nRegion = base;
      break;
    }
  }
  if (m_nRegion == 0) {
    return false;
  }
  // Prefetch handles the firmware isn't using, from the top, one per
  // plane.
  m_nUpmHandles = 0;
  for (unsigned handle = hvs::UpmHandles; handle > 0 && m_nUpmHandles < MaxPlanes;
       handle--) {
    if (!(m_nUpmHandlesUsed & (1u << (handle - 1)))) {
      m_UpmHandle[m_nUpmHandles++] = handle - 1;
    }
  }
  if (m_nUpmHandles == 0) {
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
  WriteChannel(m_nChannel, hvs::Ctrl1,
               ReadChannel(m_nChannel, hvs::Ctrl1) | hvs::Ctrl1BackgroundFill);
  m_bAttached = true;
  return true;
}

bool Hvs::Show(const Plane *plane) { return Show(plane, plane != 0 ? 1 : 0); }

bool Hvs::Show(const Plane *planes, unsigned count) {
  if (!m_bAttached || count > m_nUpmHandles) {
    return false;
  }
  static_assert(MaxPlanes * (MaxWords - 1) + 1 <= SlotWords, "list slot too small");
  // The firmware's prefetch and line buffer areas, and the planes' as
  // they are placed.
  unsigned upm[MaxAreas + MaxPlanes][2], lbm_areas[MaxAreas + MaxPlanes][2];
  unsigned upm_count = m_nUpmAreas, lbm_count = m_nLbmAreas;
  for (unsigned i = 0; i < m_nUpmAreas; i++) {
    upm[i][0] = m_UpmAreas[i][0];
    upm[i][1] = m_UpmAreas[i][1];
  }
  for (unsigned i = 0; i < m_nLbmAreas; i++) {
    lbm_areas[i][0] = m_LbmAreas[i][0];
    lbm_areas[i][1] = m_LbmAreas[i][1];
  }
  uint32_t words[MaxPlanes * MaxWords];
  unsigned n = 0;
  for (unsigned i = 0; i < count; i++) {
    const Plane &plane = planes[i];
    PlaneMemory memory;
    memory.upm_handle = m_UpmHandle[i];
    memory.kernel = plane.nearest ? m_nKernelSharp : m_nKernelSmooth;
    memory.lbm = 0;
    unsigned units = UpmUnitsFor(plane.pitch);
    if (!FindFree(units, hvs::UpmUnits, upm, upm_count, &memory.upm_base)) {
      return false;
    }
    upm[upm_count][0] = memory.upm_base;
    upm[upm_count][1] = units;
    upm_count++;
    unsigned blocks = LbmBlocksFor(plane);
    if (blocks != 0) {
      if (!FindFree(blocks, hvs::LbmBlocks, lbm_areas, lbm_count, &memory.lbm)) {
        return false;
      }
      lbm_areas[lbm_count][0] = memory.lbm;
      lbm_areas[lbm_count][1] = blocks;
      lbm_count++;
    }
    unsigned used = BuildList(plane, memory, m_bD, &words[n]);
    if (used == 0) {
      return false;
    }
    // The list's end word is overwritten by the next entry.
    n += used - 1;
  }
  words[n++] = hvs::CtlEnd;
  unsigned slot = m_nSlot[m_nNextSlot];
  for (unsigned i = 0; i < n; i++) {
    WriteList(slot + i, words[i]);
  }
  DataSyncBarrier();
  // Taken from the start of the next frame.
  WriteChannel(m_nChannel, hvs::Lptrs(m_bD), Field(slot, hvs::ListHigh, hvs::ListLow));
  m_nNextSlot ^= 1;
  return true;
}

unsigned Hvs::FrameCount(void) const {
  return GetField(ReadChannel(m_nChannel, hvs::Status(m_bD)), hvs::StatFrameHigh,
                  hvs::StatFrameLow);
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
    WriteChannel(m_nChannel, hvs::Lptrs(m_bD),
                 Field(m_nFirmwareList, hvs::ListHigh, hvs::ListLow));
  }
}

void Hvs::StopChannel(void) {
  uint32_t ctrl = ReadChannel(m_nChannel, hvs::Ctrl0);
  if (!(ctrl & hvs::CtrlEnable)) {
    return;
  }
  ctrl |= hvs::CtrlReset;
  WriteChannel(m_nChannel, hvs::Ctrl0, ctrl);
  WriteChannel(m_nChannel, hvs::Ctrl0, ctrl & ~hvs::CtrlEnable);
}

void Hvs::StartChannel(unsigned width, unsigned height) {
  WriteChannel(m_nChannel, hvs::Ctrl0, hvs::CtrlReset);
  WriteChannel(m_nChannel, hvs::Ctrl1,
               (ReadChannel(m_nChannel, hvs::Ctrl1) & ~hvs::Ctrl1Interlace) |
                   hvs::Ctrl1BackgroundFill);
  WriteChannel(m_nChannel, hvs::Ctrl0,
               hvs::CtrlEnable |
                   Field(width - 1, hvs::CtrlWidthHigh, hvs::CtrlWidthLow) |
                   Field(height - 1, hvs::CtrlHeightHigh, hvs::CtrlHeightLow));
  m_nWidth = width;
  m_nHeight = height;
}

} // namespace kms
