//
// regs.h
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
// Modified for BMC64 from BMA800XL's src/kms/pi5/regs.h: adds the per-pixel
// alpha field.
//
// Addresses and register fields of the BCM2712's display hardware used by
// the Pi 5's direct display driver: the HVS (sixth generation, in its C
// and D steppings), pixel valves 0 and 1, HDMI0 and HDMI1 and their PHYs.
//
// Sources: the BCM2712 device tree (addresses) and the behaviour of the
// Linux vc4 display driver (registers, fields and values).
//

#ifndef BMC64_KMS_PI5_REGS_H_
#define BMC64_KMS_PI5_REGS_H_

#include <stdint.h>

namespace kms {

// The ARM sees the display blocks' bus addresses (0x7Cxxxxxx) at
// 0x10_7Cxxxxxx.
const uint64_t Peripherals = 0x1000000000ull;
inline uintptr_t FromBus(uint32_t bus) { return (uintptr_t)(Peripherals + bus); }

// Bit field helper: VALUE into bits HIGH:LOW.
constexpr uint32_t Field(uint32_t value, unsigned high, unsigned low) {
  return (value & ((high - low == 31) ? 0xFFFFFFFFu
                                       : ((1u << (high - low + 1)) - 1)))
         << low;
}
constexpr uint32_t GetField(uint32_t word, unsigned high, unsigned low) {
  return (word >> low) &
         ((high - low == 31) ? 0xFFFFFFFFu : ((1u << (high - low + 1)) - 1));
}
constexpr uint32_t Bit(unsigned n) { return 1u << n; }

//
// HVS (sixth generation)
//
namespace hvs {
const uint32_t Bus = 0x7C580000;

// VERSION bits 7:0.
const uint32_t Version = 0x000;
const uint32_t VersionC = 0x53;
const uint32_t VersionD = 0x54;
const uint32_t CxmSize = 0x004;
const uint32_t LbmSize = 0x008;
const uint32_t UbmSize = 0x00C;

// CONTROL: bit 31 HVS enable; bits 13:12 where channel 1 goes (0: pixel
// valve 1).
const uint32_t Control = 0x020;
const uint32_t ControlEnable = Bit(31);
const unsigned ControlDsp1High = 13, ControlDsp1Low = 12;

// Per channel registers: CTRL0 at the channel's base, the others after it.
inline uint32_t ChannelBase(bool d, unsigned chan) {
  return d ? 0x100 + 0x40 * chan : 0x030 + 0x20 * chan;
}
const uint32_t Ctrl0 = 0x00;
const uint32_t Ctrl1 = 0x04;
inline uint32_t Lptrs(bool d) { return d ? 0x10 : 0x0C; }
inline uint32_t Status(bool d) { return d ? 0x18 : 0x14; }
inline uint32_t Dl(bool d) { return d ? 0x1C : 0x18; }

// CTRL0: width and height less one.
const uint32_t CtrlEnable = Bit(31);
const uint32_t CtrlReset = Bit(30);
const unsigned CtrlWidthHigh = 28, CtrlWidthLow = 16;
const unsigned CtrlHeightHigh = 12, CtrlHeightLow = 0;

// CTRL1
const uint32_t Ctrl1BackgroundFill = Bit(8);
const uint32_t Ctrl1Interlace = Bit(0);

// STATUS
const unsigned StatFrameHigh = 21, StatFrameLow = 16;
const unsigned StatModeHigh = 14, StatModeLow = 13;
const uint32_t ModeDisabled = 0, ModeRun = 2, ModeEof = 3;

// LPTRS and DL: a display list's start, in words, bits 11:0.
const unsigned ListHigh = 11, ListLow = 0;

// Display list memory: 4096 words at +0x4000; the firmware's boot lists
// are in the first 32.
const uint32_t DlistOffset = 0x4000;
const unsigned DlistWords = 4096;
const unsigned DlistBootWords = 32;

// Prefetch memory (UPM): 32 handles; 1024 units of 256 bytes.
const unsigned UpmHandles = 32;
const unsigned UpmUnits = 1024;
const unsigned UpmUnitBytes = 256;
// Line buffer memory: 1024 blocks.
const unsigned LbmBlocks = 1024;

// Display list words: control 0.
const uint32_t CtlEnd = Bit(31);
const uint32_t CtlValid = Bit(30);
const unsigned CtlSizeHigh = 29, CtlSizeLow = 24;   // words, with the entry's end word
const unsigned CtlAlphaMaskLow = 18;     // bits 19:18
const uint32_t AlphaMaskFixed = 3;       // D only
const uint32_t CtlUnity = Bit(15);
const unsigned CtlOrderLow = 13;         // bits 14:13
const unsigned CtlScl1Low = 8;           // bits 10:8
const unsigned CtlScl0Low = 5;           // bits 7:5
const uint32_t FormatRgba8888 = 7;       // bits 4:0
const uint32_t OrderArgb = 2;            // 0xAARRGGBB words
const uint32_t SclPpfBoth = 0;
const uint32_t SclPpfHorizontal = 4;
const uint32_t SclPpfVertical = 5;

// Position 0: x and y on the screen; positions 1 and 2: sizes less one.
const unsigned Pos0XHigh = 12, Pos0XLow = 0;
const unsigned Pos0YHigh = 28, Pos0YLow = 16;
const unsigned SizeWHigh = 12, SizeWLow = 0;
const unsigned SizeHHigh = 28, SizeHLow = 16;

// Control 2: fixed alpha. C: alpha mode 1 (fixed) in 31:30; D: fixed
// alpha chosen in control 0, premultiplied (bit 29) as Linux sets it.
const uint32_t Ctl2FixedC = Field(1, 31, 30) | Field(0xFFF, 15, 4);
const uint32_t Ctl2FixedD = Bit(29) | Field(0xFFF, 15, 4);
// Each pixel's alpha, not premultiplied: C alpha mode 0; D alpha mask 0
// in control 0.
const uint32_t Ctl2PixelAlpha = Field(0xFFF, 15, 4);

const uint32_t Context = 0xC0C0C0C0;     // words the HVS writes itself

// Pointer 0: UPM base (256-byte units), handle, buffer lines (0: 2), and
// address bits 39:32. Pointer 1: address bits 31:0. Pointer 2: pitch.
const unsigned Ptr0UpmBaseHigh = 28, Ptr0UpmBaseLow = 16;
const unsigned Ptr0UpmHandleHigh = 14, Ptr0UpmHandleLow = 10;
const unsigned Ptr0UpmLinesHigh = 9, Ptr0UpmLinesLow = 8;
const unsigned Ptr0AddressHigh = 7, Ptr0AddressLow = 0;
const unsigned PitchHigh = 16, PitchLow = 0;
const unsigned UpmLines = 2;

// PPF word, as on the HVS5.
const uint32_t PpfNoInterpolation = Bit(31);
const uint32_t PpfAgc = Bit(30);
const unsigned PpfScaleHigh = 24, PpfScaleLow = 8;
const unsigned PpfPhaseHigh = 6, PpfPhaseLow = 0;
const unsigned PhaseBits = 6;            // phase in 1/64ths of a pixel
const unsigned KernelOffsetHigh = 13, KernelOffsetLow = 0;

// Filter kernels: 11 words.
const unsigned KernelWords = 11;
} // namespace hvs

//
// Pixel valves 0 (HDMI0) and 1 (HDMI1), 64-byte FIFO. The firmware runs
// them at one or two pixels a clock; mode changes keep its choice, these
// are the defaults if its mode can't be read.
//
namespace pv {
const unsigned PixelsPerClock = 1;
// FIFO level: the FIFO's 64 bytes less the HVS's latency of 6 three-byte
// pixels.
const unsigned FifoLevel = 64 - 3 * 6;

const uint32_t Control = 0x00;
const uint32_t VControl = 0x04;
const uint32_t VsyncdEven = 0x08;
const uint32_t HorzA = 0x0C;   // 31:16 back porch, 15:0 sync
const uint32_t HorzB = 0x10;   // 31:16 front porch, 15:0 active
const uint32_t VertA = 0x14;   // 31:16 back porch, 15:0 sync
const uint32_t VertB = 0x18;   // 31:16 front porch, 15:0 active
const uint32_t IntEn = 0x24;
const uint32_t MuxCfg = 0x34;
const uint32_t PipeInitCtrl = 0x94;

// CONTROL
const unsigned FifoLevelHighHigh = 26, FifoLevelHighLow = 25;
const unsigned FormatHigh = 23, FormatLow = 21;   // 0: 24 bit
const unsigned FifoLevelHigh = 20, FifoLevelLow = 15;
const uint32_t ClearAtStart = Bit(14);
const uint32_t TriggerUnderflow = Bit(13);
const uint32_t WaitHstart = Bit(12);
const unsigned ClockSelectHigh = 3, ClockSelectLow = 2;  // 0: HDMI
const uint32_t FifoClear = Bit(1);
const uint32_t Enable = Bit(0);

// V_CONTROL
const uint32_t OddTiming = Bit(29);      // with one pixel a clock
const uint32_t Continuous = Bit(1);
const uint32_t VideoEnable = Bit(0);

// MUX_CFG bits 5:2
const uint32_t MuxNoSwap = 8;

// PIPE_INIT_CTRL: 11:8 width, 7:4 idle, 0 enable.
const uint32_t PipeInit = Field(1, 11, 8) | Field(1, 7, 4) | Bit(0);
} // namespace pv

//
// HDMI0 and HDMI1: the same registers in each port's own ranges, except
// VID_CTL in the shared "hd" range.
//
namespace hdmi {
const uint32_t HdBus = 0x7C720000;

// hd: VID_CTL (offset per port). Its enable bit is never cleared.
const uint32_t VidEnable = Bit(31);
const uint32_t VidUnderflowEnable = Bit(30);
const uint32_t VidFrameCounterReset = Bit(29);
const uint32_t VidVsyncLow = Bit(28);
const uint32_t VidHsyncLow = Bit(27);
const uint32_t VidClearRgb = Bit(23);
const uint32_t VidBlankPixels = Bit(18);
const uint32_t VidBlankInsert = Bit(16);

// core
const uint32_t FifoCtl = 0x07C;
const uint32_t FifoRecenterDone = Bit(14);
const uint32_t FifoRecenter = Bit(6);
const uint32_t FifoMasterSlave = Bit(0);
const uint32_t FifoWritable = 0xEFFF;
const uint32_t RamPacketConfig = 0x0C4;
const uint32_t RamPacketEnable = Bit(16);
const uint32_t RamPacketStatus = 0x0CC;
const uint32_t CrpCfg = 0x0D0;          // 24 external CTS, 19:0 N
const uint32_t Cts0 = 0x0D4, Cts1 = 0x0D8;
const uint32_t SchedulerControl = 0x0E8;
const uint32_t SchedManualFormat = Bit(15);
const uint32_t SchedIgnoreVsyncPredicts = Bit(5);
const uint32_t SchedHdmiActive = Bit(1);
const uint32_t SchedHdmiMode = Bit(0);
const uint32_t HorzA = 0x0EC;   // 15 vsync +, 14 hsync +, 28:16 front, 13:0 active
const uint32_t HorzB = 0x0F0;   // 26:16 back, 10:0 sync
const uint32_t VertA0 = 0x0F4, VertA1 = 0x100;  // 28:24 sync, 22:16 front, 12:0 active
const uint32_t VertB0 = 0x0F8, VertB1 = 0x104;  // 29:16 offset, 8:0 back
const uint32_t MiscControl = 0x114;             // 3:0 pixel repeat - 1
const uint32_t DeepColorConfig1 = 0x18C;        // 10:8 pack phase, 3:0 depth
const uint32_t GcpConfig = 0x194;
const uint32_t GcpEnable = Bit(31);
const uint32_t GcpWord1 = 0x198;                // 15:8 depth, 7:0 byte 0
const uint32_t GcpClearAvMute = Bit(4);
const uint32_t Hotplug = 0x1C8;
const uint32_t HotplugConnected = Bit(0);

// dvp
const uint32_t ClockStop = 0x0BC;
const uint32_t VecInterfaceCfg = 0x0F0;
const uint32_t VecInterfaceXbar = 0x0F4;
const uint32_t XbarRgb = 0x354021;

// csc
const uint32_t CscCtl = 0x00;
const uint32_t CscEnable = Bit(2);
const uint32_t CscModeCustom = 3;     // bits 1:0
const uint32_t Csc12_11 = 0x04;       // then 14_13, 22_21, 24_23, 32_31, 34_33
const uint32_t CscChannelCtl = 0x2C;

// packet RAM
const uint32_t PacketStride = 0x24;

// phy
const uint32_t PhyResetCtl = 0x000;
const uint32_t PhyResetRun = 0x7F;
const uint32_t PhyPowerupCtl = 0x004;
const uint32_t PhyPowerupRun = Bit(8) | Bit(7) | Bit(6) | Bit(3) | Bit(2) | Bit(1) | Bit(0);
const uint32_t PhyCtl0 = 0x008;           // TX0, then TX1, TX2, clock
const uint32_t PhyCtlCk = 0x014;
const uint32_t PhyPllRefclk = 0x01C;
const uint32_t PhyRefclkCmos = Bit(13);
const uint32_t PhyPllPostKdiv = 0x028;
const uint32_t PhyKdivBypass = Bit(4);
const uint32_t PhyKdivRun = Field(2, 3, 2) | Field(1, 1, 0);
const uint32_t PhyPllVcoclkDiv = 0x02C;
const uint32_t PhyVcoDivEnable = Bit(10);
const uint32_t PhyPllCfg = 0x044;
const uint32_t PhyTmdsClkWordSel = 0x054;
const uint32_t PhyPllMisc0 = 0x060;       // to MISC_8 at 0x080
const uint32_t PhyPllResetCtl = 0x190;
const uint32_t PhyPllResetb = Bit(0);
const uint32_t PhyPllPowerupCtl = 0x194;

// rate manager
const uint32_t RmOffset = 0x018;
const uint32_t RmOffsetOnly = Bit(31);

// Mailbox clocks.
const uint32_t ClockHsm = 13;            // the HDMI state machine's
const uint32_t ClockBvb = 14;
const uint32_t HsmMinHz = 120000000;
} // namespace hdmi

//
// The two ports: HDMI0 is HVS channel 0 through pixel valve 0, HDMI1
// channel 1 through pixel valve 1.
//
struct Port {
  uint32_t pv;                 // pixel valve (bus address)
  unsigned channel;            // HVS channel
  uint32_t display_id;         // the firmware's display ID
  uint32_t core, dvp, phy, rm, ram, csc;   // HDMI ranges (bus addresses)
  uint32_t vid_ctl;            // VID_CTL's offset in "hd"
};

const Port Ports[2] = {
    {0x7C410000, 0, 2, 0x7C701400, 0x7C701000, 0x7C701D00, 0x7C702000, 0x7C703800,
     0x7C700100, 0x44},
    {0x7C411000, 1, 7, 0x7C706400, 0x7C706000, 0x7C706D00, 0x7C707000, 0x7C708800,
     0x7C700180, 0x48},
};

} // namespace kms

#endif
