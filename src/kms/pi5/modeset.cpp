//
// modeset.cpp
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
// Modified for BMC64 from BMA800XL's src/kms/pi5/modeset.cpp: "BMA800XL"
// changed to "BMC64" in a log message.
//
// See modeset.h.
//

#include "modeset.h"

#include "phy.h"
#include "regs.h"

#include <circle/bcmpropertytags.h>
#include <circle/logger.h>
#include <circle/memio.h>
#include <circle/synchronize.h>
#include <circle/timer.h>
#include <string.h>

namespace kms {

static const char FromModeSet[] = "kms";

namespace {

inline uint32_t Rd(uintptr_t a) { return read32(a); }
inline void Wr(uintptr_t a, uint32_t v) { write32(a, v); }
inline void SetBits(uintptr_t a, uint32_t bits) { write32(a, read32(a) | bits); }
inline void ClearBits(uintptr_t a, uint32_t bits) { write32(a, read32(a) & ~bits); }
// Replaces bits HIGH:LOW of the register at A with VALUE.
inline void Put(uintptr_t a, unsigned high, unsigned low, uint32_t value) {
  uint32_t mask = Field(0xFFFFFFFFu, high, low);
  write32(a, (read32(a) & ~mask) | Field(value, high, low));
}

uint32_t GetClockRate(uint32_t id) {
  CBcmPropertyTags tags;
  TPropertyTagClockRate tag;
  tag.nClockId = id;
  if (!tags.GetTag(PROPTAG_GET_CLOCK_RATE, &tag, sizeof tag, 4)) {
    return 0;
  }
  return tag.nRate;
}

bool SetClockRate(uint32_t id, uint32_t hz) {
  CBcmPropertyTags tags;
  TPropertyTagSetClockRate tag;
  tag.nClockId = id;
  tag.nRate = hz;
  tag.nSkipSettingTurbo = SKIP_SETTING_TURBO;
  return tags.GetTag(PROPTAG_SET_CLOCK_RATE, &tag, sizeof tag, 12);
}

// The firmware's "get EDID block for display" tag.
const uint32_t GetEdidBlockDisplay = 0x00030023;

} // namespace

ModeSetter::ModeSetter(void)
    : m_nSavedPackets(0), m_bEdidRead(false), m_nPixelHz(0), m_bPvRead(false),
      m_nPpc(pv::PixelsPerClock), m_nOddTiming(pv::OddTiming), m_nPipeInit(pv::PipeInit) {
  memset(&m_Edid, 0, sizeof m_Edid);
  SetPort(0);
}

void ModeSetter::SetPort(unsigned port) {
  const Port &p = Ports[port != 0];
  m_nPort = port != 0;
  m_Pv = FromBus(p.pv);
  m_Core = FromBus(p.core);
  m_Dvp = FromBus(p.dvp);
  m_Phy = FromBus(p.phy);
  m_Rm = FromBus(p.rm);
  m_Ram = FromBus(p.ram);
  m_Csc = FromBus(p.csc);
  m_VidCtl = FromBus(hdmi::HdBus) + p.vid_ctl;
}

bool ModeSetter::WaitFor(uintptr_t reg, uint32_t mask, bool set,
                         unsigned timeout_ms) {
  u64 start = CTimer::GetClockTicks64();
  u64 limit = (u64)timeout_ms * (CLOCKHZ / 1000);
  while (((Rd(reg) & mask) != 0) != set) {
    if (CTimer::GetClockTicks64() - start > limit) {
      return false;
    }
  }
  return true;
}

// The pixel valve's timings, read back.
bool ModeSetter::Current(Timing *timing) {
  if (!(Rd(m_Pv + pv::Control) & pv::Enable) ||
      !(Rd(m_Pv + pv::VControl) & pv::VideoEnable)) {
    return false;
  }
  uint32_t horza = Rd(m_Pv + pv::HorzA), horzb = Rd(m_Pv + pv::HorzB);
  uint32_t verta = Rd(m_Pv + pv::VertA), vertb = Rd(m_Pv + pv::VertB);
  // The firmware may run the pixel valve at one pixel a clock or two.
  unsigned v_active = GetField(vertb, 15, 0);
  unsigned v_total = GetField(vertb, 31, 16) + v_active + GetField(verta, 15, 0) +
                     GetField(verta, 31, 16);
  unsigned h_active = 0, h_total = 0, ppc = 1;
  const Timing *known = 0;
  for (ppc = 1; ppc <= 2; ppc++) {
    h_active = GetField(horzb, 15, 0) * ppc;
    h_total = (GetField(horzb, 31, 16) + GetField(horzb, 15, 0) +
               GetField(horza, 15, 0) + GetField(horza, 31, 16)) * ppc;
    known = FindMode(h_active, h_total, v_active, v_total);
    if (known != 0) {
      break;
    }
  }
  if (!m_bPvRead) {
    m_bPvRead = true;
    uint32_t vcontrol = Rd(m_Pv + pv::VControl), pipe = Rd(m_Pv + pv::PipeInitCtrl);
    if (known != 0) {
      m_nPpc = ppc;
      m_nOddTiming = vcontrol & pv::OddTiming;
      m_nPipeInit = pipe;
      CLogger::Get()->Write(FromModeSet, LogNotice, "Pixel valve at %u pixel(s) a clock", ppc);
    }
  }
  if (known == 0) {
    CLogger::Get()->Write(FromModeSet, LogNotice,
                          "Video mode %ux%u (totals %ux%u) isn't one BMC64 switches",
                          h_active, v_active, h_total, v_total);
    return false;
  }
  *timing = *known;
  if (m_nPixelHz != 0) {
    timing->pixel_hz = m_nPixelHz;
  }
  return true;
}

// Block B of the port's EDID, through the tag for a given display
// (BY_DISPLAY), or the older tag for the default display.
bool ModeSetter::ReadEdidBlock(unsigned b, bool by_display, uint8_t block[128]) {
  CBcmPropertyTags tags;
  if (by_display) {
    struct {
      TPropertyTag tag;
      uint32_t block_number;
      uint32_t display_id;
      uint8_t data[128];
    } PACKED request;
    request.block_number = b;
    request.display_id = Ports[m_nPort].display_id;
    if (!tags.GetTag(GetEdidBlockDisplay, &request, sizeof request, 8)) {
      return false;
    }
    memcpy(block, request.data, 128);
    return true;
  }
  TPropertyTagEDIDBlock tag;
  tag.nBlockNumber = b;
  if (!tags.GetTag(PROPTAG_GET_EDID_BLOCK, &tag, sizeof tag, 4) ||
      tag.nStatus != EDID_STATUS_SUCCESS) {
    return false;
  }
  memcpy(block, tag.Block, 128);
  return true;
}

const Edid &ModeSetter::GetEdid(void) {
  if (!m_bEdidRead) {
    m_bEdidRead = true;
    static uint8_t data[4 * 128];
    unsigned blocks = 0;
    // For HDMI0, the older tag if the newer one gives nothing usable.
    for (int by_display = 1; by_display >= (m_nPort == 0 ? 0 : 1); by_display--) {
      // The base block, then its extensions (up to 3).
      blocks = 0;
      for (unsigned b = 0; b < 4 && (b == 0 || b <= data[126]); b++) {
        if (!ReadEdidBlock(b, by_display != 0, data + 128 * b)) {
          break;
        }
        blocks++;
      }
      ParseEdid(data, blocks, &m_Edid);
      if (m_Edid.valid) {
        break;
      }
    }
    CLogger::Get()->Write(FromModeSet, LogNotice, "EDID (HDMI%u): %u block(s), %s, %u video modes",
                          m_nPort, blocks, !m_Edid.valid ? "not valid" : m_Edid.hdmi ? "HDMI" : "DVI",
                          m_Edid.vic_count);
  }
  return m_Edid;
}

bool ModeSetter::Set(Hvs &hvs, const Timing &timing, unsigned audio_fs,
                     void (*reinstall)(void *), void *param) {
  Timing old;
  if (!Current(&old)) {
    return false;
  }
  Pi5Phy phy;
  if (!ComputePi5Phy(timing.pixel_hz, &phy)) {
    return false;
  }
  bool hdmi_sink = GetEdid().hdmi;
  if (Apply(hvs, timing, hdmi_sink, audio_fs, reinstall, param)) {
    m_nPixelHz = timing.pixel_hz;
    CLogger::Get()->Write(FromModeSet, LogNotice, "Video mode VIC %u, %ux%u at %u Hz pixel clock (%.3f Hz)",
                          timing.vic, timing.h_active, timing.v_active,
                          timing.pixel_hz, timing.Refresh());
    return true;
  }
  CLogger::Get()->Write(FromModeSet, LogError, "Setting the video mode failed; going back");
  if (Apply(hvs, old, hdmi_sink, audio_fs, reinstall, param)) {
    m_nPixelHz = old.pixel_hz;
  }
  return false;
}

// Disables the pixel valve and blanks HDMI. On the BCM2712 HDMI's video
// enable stays set; the picture is blanked instead.
void ModeSetter::Disable(Hvs &hvs) {
  m_nSavedPackets = Rd(m_Core + hdmi::RamPacketConfig);

  ClearBits(m_Pv + pv::VControl, pv::VideoEnable);
  WaitFor(m_Pv + pv::VControl, pv::VideoEnable, false, 1);
  // Otherwise a pixel can stick in a FIFO between the PV and HDMI.
  CTimer::SimpleMsDelay(20);

  Wr(m_Core + hdmi::RamPacketConfig, 0);
  SetBits(m_VidCtl, hdmi::VidClearRgb | hdmi::VidBlankPixels);
  CTimer::SimpleMsDelay(1);

  ClearBits(m_Pv + pv::Control, pv::Enable);
  SetBits(m_Pv + pv::Control, pv::FifoClear);

  hvs.StopChannel();

  SetBits(m_VidCtl, hdmi::VidBlankPixels);
  Wr(m_Phy + hdmi::PhyResetCtl, 0);
  Wr(m_Phy + hdmi::PhyPowerupCtl, 0);
  Wr(m_Phy + hdmi::PhyPllPostKdiv, hdmi::PhyKdivBypass);
}

// Raises the HDMI state machine and BVB clocks if they are below what the
// TMDS rate needs.
void ModeSetter::SetClocks(uint64_t tmds_hz) {
  uint32_t hsm = (uint32_t)(tmds_hz / 100 * 101);
  if (hsm < hdmi::HsmMinHz) {
    hsm = hdmi::HsmMinHz;
  }
  uint32_t bvb = tmds_hz > 297000000 ? 300000000
                 : tmds_hz > 148500000 ? 150000000 : 75000000;
  if (GetClockRate(hdmi::ClockHsm) < hsm) {
    SetClockRate(hdmi::ClockHsm, hsm);
  }
  if (GetClockRate(hdmi::ClockBvb) < bvb) {
    SetClockRate(hdmi::ClockBvb, bvb);
  }
}

// Sets up the PHY and its PLL for the TMDS rate.
void ModeSetter::SetPhy(uint64_t tmds_hz) {
  Pi5Phy phy;
  ComputePi5Phy(tmds_hz, &phy);

  Wr(m_Phy + hdmi::PhyResetCtl, 0);
  Wr(m_Phy + hdmi::PhyPowerupCtl, 0);
  Wr(m_Phy + hdmi::PhyPllPostKdiv, hdmi::PhyKdivBypass);
  for (unsigned i = 0; i < PllMiscWords; i++) {
    Wr(m_Phy + hdmi::PhyPllMisc0 + 4 * i, PllMisc[i]);
  }
  Wr(m_Phy + hdmi::PhyPllRefclk, hdmi::PhyRefclkCmos | Field(54, 9, 0));
  Wr(m_Phy + hdmi::PhyResetCtl, hdmi::PhyResetRun);
  Wr(m_Rm + hdmi::RmOffset, phy.rm_offset);
  Wr(m_Phy + hdmi::PhyPllVcoclkDiv, hdmi::PhyVcoDivEnable | Field(phy.vco_divider, 9, 0));
  Wr(m_Phy + hdmi::PhyPllCfg, 0);
  Wr(m_Phy + hdmi::PhyPllPostKdiv, hdmi::PhyKdivRun);
  // TX0, TX1, TX2 carry the data lanes in order, then the clock.
  for (unsigned i = 0; i < 3; i++) {
    Wr(m_Phy + hdmi::PhyCtl0 + 4 * i, phy.data_lane);
  }
  Wr(m_Phy + hdmi::PhyCtlCk, phy.clock_lane);
  Wr(m_Phy + hdmi::PhyTmdsClkWordSel, phy.word_select);
  Wr(m_Phy + hdmi::PhyPowerupCtl, hdmi::PhyPowerupRun);
  Wr(m_Phy + hdmi::PhyPllPowerupCtl, 1);
  ClearBits(m_Phy + hdmi::PhyPllResetCtl, hdmi::PhyPllResetb);
  SetBits(m_Phy + hdmi::PhyPllResetCtl, hdmi::PhyPllResetb);
}

// HDMI's timings, set before the pixel valve.
void ModeSetter::SetHdmiTimings(const Timing &t) {
  Wr(m_Core + hdmi::HorzA, (t.v_positive ? Bit(15) : 0) | (t.h_positive ? Bit(14) : 0) |
                               Field(t.h_front, 28, 16) | Field(t.h_active, 13, 0));
  Wr(m_Core + hdmi::HorzB, Field(t.h_back, 26, 16) | Field(t.h_sync, 10, 0));
  uint32_t verta = Field(t.v_sync, 28, 24) | Field(t.v_front, 22, 16) |
                   Field(t.v_active, 12, 0);
  Wr(m_Core + hdmi::VertA0, verta);
  Wr(m_Core + hdmi::VertA1, verta);
  Wr(m_Core + hdmi::VertB0, Field(0, 29, 16) | Field(t.v_back, 8, 0));
  Wr(m_Core + hdmi::VertB1, Field(t.HTotal() / 2, 29, 16) | Field(t.v_back, 8, 0));

  Put(m_Core + hdmi::DeepColorConfig1, 10, 8, 2);   // pack phase
  Put(m_Core + hdmi::DeepColorConfig1, 3, 0, 0);    // 8 bits a colour
  Put(m_Core + hdmi::GcpWord1, 15, 8, 0);
  Put(m_Core + hdmi::GcpWord1, 7, 0, hdmi::GcpClearAvMute);
  SetBits(m_Core + hdmi::GcpConfig, hdmi::GcpEnable);
  Put(m_Core + hdmi::MiscControl, 3, 0, 0);         // no pixel repetition
  Wr(m_Dvp + hdmi::ClockStop, 0);
}

// Sets up and enables the pixel valve.
void ModeSetter::SetPixelValve(const Timing &t) {
  // As the firmware set it up: pixels a clock, odd timing, pipe init.
  unsigned ppc = m_nPpc;
  Wr(m_Pv + pv::HorzA, Field(t.h_back / ppc, 31, 16) | Field(t.h_sync / ppc, 15, 0));
  Wr(m_Pv + pv::HorzB, Field(t.h_front / ppc, 31, 16) | Field(t.h_active / ppc, 15, 0));
  Wr(m_Pv + pv::VControl, pv::Continuous | m_nOddTiming);
  Wr(m_Pv + pv::VsyncdEven, 0);
  Wr(m_Pv + pv::VertA, Field(t.v_back, 31, 16) | Field(t.v_sync, 15, 0));
  Wr(m_Pv + pv::VertB, Field(t.v_front, 31, 16) | Field(t.v_active, 15, 0));
  Wr(m_Pv + pv::MuxCfg, Field(pv::MuxNoSwap, 5, 2));
  Wr(m_Pv + pv::PipeInitCtrl, m_nPipeInit);

  unsigned level = pv::FifoLevel;
  Wr(m_Pv + pv::Control, pv::FifoClear |
                             Field(level >> 6, pv::FifoLevelHighHigh, pv::FifoLevelHighLow) |
                             Field(level & 0x3F, pv::FifoLevelHigh, pv::FifoLevelLow) |
                             Field(0, pv::FormatHigh, pv::FormatLow) |
                             pv::ClearAtStart | pv::TriggerUnderflow | pv::WaitHstart |
                             Field(0, pv::ClockSelectHigh, pv::ClockSelectLow));
  SetBits(m_Pv + pv::Control, pv::Enable);
}

// Colour conversion: RGB, limited (16-235) or full range.
void ModeSetter::SetColour(bool limited) {
  const uint32_t one = limited ? 0x1B7C : 0x2000;   // 219/255 or 1, 2.13
  const uint32_t offset = limited ? 0x400 : 0;      // 16 or 0, 9.6
  // Rows of (c1, c2, c3, offset), two 16-bit values a word, low first.
  for (unsigned row = 0; row < 3; row++) {
    uint32_t c[4] = {0, 0, 0, offset};
    c[row] = one;
    Wr(m_Csc + hdmi::Csc12_11 + 8 * row, c[1] << 16 | c[0]);
    Wr(m_Csc + hdmi::Csc12_11 + 8 * row + 4, c[3] << 16 | c[2]);
  }
  Wr(m_Dvp + hdmi::VecInterfaceCfg, 0);
  Wr(m_Dvp + hdmi::VecInterfaceXbar, hdmi::XbarRgb);
  Wr(m_Csc + hdmi::CscChannelCtl, 0);
  Wr(m_Csc + hdmi::CscCtl, hdmi::CscEnable | Field(hdmi::CscModeCustom, 1, 0));
}

// The AVI InfoFrame, in slot 2.
bool ModeSetter::WriteInfoFrame(const Timing &timing) {
  const unsigned slot = 0x82 - 0x80;
  uint8_t frame[AviLength];
  AviInfoFrame(timing, frame);
  uint32_t words[PacketWords];
  PacketRamWords(frame, AviLength, words);

  ClearBits(m_Core + hdmi::RamPacketConfig, Bit(slot));
  if (!WaitFor(m_Core + hdmi::RamPacketStatus, Bit(slot), false, 100)) {
    return false;
  }
  uintptr_t base = m_Ram + slot * hdmi::PacketStride;
  for (unsigned i = 0; i < PacketWords; i++) {
    Wr(base + 4 * i, words[i]);
  }
  SetBits(m_Core + hdmi::RamPacketConfig, Bit(slot));
  return WaitFor(m_Core + hdmi::RamPacketStatus, Bit(slot), true, 100);
}

bool ModeSetter::RecenterFifo(void) {
  uint32_t drift = Rd(m_Core + hdmi::FifoCtl) & hdmi::FifoWritable;
  for (int i = 0; i < 2; i++) {
    Wr(m_Core + hdmi::FifoCtl, drift & ~hdmi::FifoRecenter);
    Wr(m_Core + hdmi::FifoCtl, drift | hdmi::FifoRecenter);
    if (i == 0) {
      CTimer::SimpleMsDelay(1);
    }
  }
  return WaitFor(m_Core + hdmi::FifoCtl, hdmi::FifoRecenterDone, true, 10);
}

bool ModeSetter::Apply(Hvs &hvs, const Timing &t, bool hdmi_sink, unsigned audio_fs,
                       void (*reinstall)(void *), void *param) {
  Disable(hvs);

  // HVS channel: the picture's list, then the channel itself.
  if (reinstall != 0) {
    reinstall(param);
  }
  hvs.StartChannel(t.h_active, t.v_active);

  // HDMI before the PV.
  SetClocks(t.pixel_hz);
  SetPhy(t.pixel_hz);
  SetBits(m_Core + hdmi::SchedulerControl,
      hdmi::SchedManualFormat | hdmi::SchedIgnoreVsyncPredicts);
  SetHdmiTimings(t);

  SetPixelValve(t);

  // HDMI after the PV is enabled, before video.
  SetColour(hdmi_sink);
  Wr(m_Core + hdmi::FifoCtl, hdmi::FifoMasterSlave);

  SetBits(m_Pv + pv::VControl, pv::VideoEnable);

  // HDMI after video enable: un-blanked.
  uint32_t vid = Rd(m_VidCtl) & ~(hdmi::VidVsyncLow | hdmi::VidHsyncLow);
  Wr(m_VidCtl, vid | hdmi::VidEnable | hdmi::VidClearRgb |
                   hdmi::VidUnderflowEnable | hdmi::VidFrameCounterReset |
                   hdmi::VidBlankInsert |
                   (t.v_positive ? 0 : hdmi::VidVsyncLow) |
                   (t.h_positive ? 0 : hdmi::VidHsyncLow));
  ClearBits(m_VidCtl, hdmi::VidBlankPixels);

  bool ok = true;
  if (hdmi_sink) {
    SetBits(m_Core + hdmi::SchedulerControl, hdmi::SchedHdmiMode);
    ok = WaitFor(m_Core + hdmi::SchedulerControl, hdmi::SchedHdmiActive, true, 1000);
    // Packet RAM on, with the packets that were being sent (the audio
    // InfoFrame) still on; then the AVI InfoFrame.
    Wr(m_Core + hdmi::RamPacketConfig,
       hdmi::RamPacketEnable | (m_nSavedPackets & 0xFFFF & ~Bit(0x82 - 0x80)));
    ok = WriteInfoFrame(t) && ok;
  } else {
    ClearBits(m_Core + hdmi::RamPacketConfig, hdmi::RamPacketEnable);
    ClearBits(m_Core + hdmi::SchedulerControl, hdmi::SchedHdmiMode);
    ok = WaitFor(m_Core + hdmi::SchedulerControl, hdmi::SchedHdmiActive, false, 1000);
  }
  ok = RecenterFifo() && ok;

  // HDMI sound's clock regeneration for the new TMDS rate (external CTS).
  uint32_t crp = Rd(m_Core + hdmi::CrpCfg);
  if (audio_fs != 0 && (crp & Bit(24))) {
    uint32_t cts = AudioCts(t.pixel_hz, crp & 0xFFFFF, audio_fs);
    Wr(m_Core + hdmi::Cts0, cts);
    Wr(m_Core + hdmi::Cts1, cts);
  }
  return ok;
}

} // namespace kms
