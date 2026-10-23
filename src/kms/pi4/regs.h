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
// Modified for BMC64 from BMA800XL's src/kms/pi4/regs.h: adds the per-pixel
// alpha mode.
//
// Addresses and register fields of the BCM2711's display hardware used by
// the direct display driver: the HVS, pixel valves 2 and 4, HDMI0 and HDMI1.
//
// Sources: the BCM2711 device tree (addresses) and the behaviour of the
// Linux vc4 display driver (registers, fields and values).
//

#ifndef BMC64_KMS_PI4_REGS_H_
#define BMC64_KMS_PI4_REGS_H_

#include <stdint.h>

namespace kms {

// The ARM's view of the VideoCore's peripherals (bus 0x7Exxxxxx), in the
// Pi 4's low peripheral mode.
const uintptr_t Peripherals = 0xFE000000;
inline uintptr_t FromBus(uint32_t bus) { return Peripherals + (bus - 0x7E000000); }

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
// HVS (Hardware Video Scaler)
//
namespace hvs {
const uint32_t Bus = 0x7E400000;

const uint32_t DispCtrl = 0x00;      // bit 31: HVS enable
const uint32_t DispDither = 0x14;    // bits 31:30: channel feeding output 5 (HDMI1)
const uint32_t DispEoln = 0x18;      // bits 31:30: channel feeding output 4 (HDMI0)
const unsigned OutputMuxHigh = 31, OutputMuxLow = 30;
const uint32_t MuxNone = 3;

inline uint32_t DispList(unsigned chan) { return 0x20 + 4 * chan; }
inline uint32_t DispCtrlX(unsigned chan) { return 0x40 + 0x10 * chan; }
inline uint32_t DispBkgndX(unsigned chan) { return 0x44 + 0x10 * chan; }
inline uint32_t DispStatX(unsigned chan) { return 0x48 + 0x10 * chan; }

// DISPCTRLx
const uint32_t CtrlEnable = Bit(31);
const uint32_t CtrlReset = Bit(30);
const unsigned CtrlWidthHigh = 28, CtrlWidthLow = 16;
const unsigned CtrlHeightHigh = 12, CtrlHeightLow = 0;

// DISPBKGNDx
const uint32_t BkgndBackToBack = Bit(31);
const uint32_t BkgndInterlace = Bit(30);
const uint32_t BkgndGamma = Bit(29);
const uint32_t BkgndFill = Bit(24);

// DISPSTATx bits 31:30
const unsigned StatModeHigh = 31, StatModeLow = 30;
const uint32_t ModeDisabled = 0, ModeRun = 2;

// Frame counters: register and field per channel.
const uint32_t DispStat1 = 0x58, DispStat2 = 0x68;

// Display list memory: 4096 words at +0x4000. The firmware's boot list is
// in the first 32 words.
const uint32_t DlistOffset = 0x4000;
const unsigned DlistWords = 4096;
const unsigned DlistBootWords = 32;

// Line buffer memory, in words.
const unsigned LbmWords = 60 * 1024;

// Display list words (HVS5).
const uint32_t CtlEnd = Bit(31);
const uint32_t CtlValid = Bit(30);
const unsigned CtlSizeHigh = 29, CtlSizeLow = 24;
const unsigned CtlOrderLow = 13;          // bits 14:13
const uint32_t CtlUnity = Bit(15);
const uint32_t CtlAlphaExpand = Bit(12);
const uint32_t CtlRgbExpand = Bit(11);
const unsigned CtlScl1Low = 8;            // bits 10:8
const unsigned CtlScl0Low = 5;            // bits 7:5
const uint32_t FormatRgba8888 = 7;        // bits 4:0
const uint32_t OrderArgb = 2;             // 0xAARRGGBB words
// Scaling modes for SCL0/SCL1.
const uint32_t SclPpfBoth = 0;
const uint32_t SclPpfHorizontal = 4;
const uint32_t SclPpfVertical = 5;

const unsigned Pos0XHigh = 13, Pos0XLow = 0;
const unsigned Pos0YHigh = 27, Pos0YLow = 16;
const unsigned Ctl2AlphaModeLow = 30;     // bits 31:30
const uint32_t AlphaModePipeline = 0;    // each pixel's alpha
const uint32_t AlphaModeFixed = 1;
const unsigned Ctl2AlphaHigh = 15, Ctl2AlphaLow = 4;
const unsigned SizeWHigh = 12, SizeWLow = 0;    // position words 1 and 2
const unsigned SizeHHigh = 28, SizeHLow = 16;
const uint32_t Context = 0xC0C0C0C0;     // words the HVS writes itself
const unsigned PitchHigh = 15, PitchLow = 0;

// PPF word.
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
// Pixel valves 2 (HDMI0) and 4 (HDMI1); the same registers.
//
namespace pv {
const unsigned PixelsPerClock = 2;

const uint32_t Control = 0x00;
const uint32_t VControl = 0x04;
const uint32_t VsyncdEven = 0x08;
const uint32_t HorzA = 0x0C;   // 31:16 back porch, 15:0 sync
const uint32_t HorzB = 0x10;   // 31:16 front porch, 15:0 active
const uint32_t VertA = 0x14;   // 31:16 back porch, 15:0 sync
const uint32_t VertB = 0x18;   // 31:16 front porch, 15:0 active
const uint32_t IntStat = 0x28;
const uint32_t MuxCfg = 0x34;

// CONTROL
const unsigned FifoLevelHighHigh = 26, FifoLevelHighLow = 25;
const unsigned FormatHigh = 23, FormatLow = 21;   // 0: 24 bit
const unsigned FifoLevelHigh = 20, FifoLevelLow = 15;
const uint32_t ClearAtStart = Bit(14);
const uint32_t TriggerUnderflow = Bit(13);
const uint32_t WaitHstart = Bit(12);
const unsigned PixelRepHigh = 5, PixelRepLow = 4;
const unsigned ClockSelectHigh = 3, ClockSelectLow = 2;  // 0: HDMI on PV2 and PV4
const uint32_t FifoClear = Bit(1);
const uint32_t Enable = Bit(0);

// V_CONTROL
const uint32_t Continuous = Bit(1);
const uint32_t VideoEnable = Bit(0);

// MUX_CFG bits 5:2
const uint32_t MuxNoSwap = 8;

// INTSTAT
const uint32_t VfpStart = Bit(7);
} // namespace pv

//
// HDMI0 and HDMI1: the same registers in each port's own ranges, except
// where the port table below says otherwise.
//
namespace hdmi {
const uint32_t HdBus = 0x7EF20000;     // "hd", shared by both ports

// hd (VID_CTL's offset is per port)
const uint32_t VidEnable = Bit(31);
const uint32_t VidUnderflowEnable = Bit(30);
const uint32_t VidFrameCounterReset = Bit(29);
const uint32_t VidVsyncLow = Bit(28);
const uint32_t VidHsyncLow = Bit(27);
const uint32_t VidClearRgb = Bit(23);
const uint32_t VidBlankPixels = Bit(18);
const uint32_t VidBlankInsert = Bit(16);

// core
const uint32_t FifoCtl = 0x074;
const uint32_t FifoRecenterDone = Bit(14);
const uint32_t FifoRecenter = Bit(6);
const uint32_t FifoMasterSlave = Bit(0);
const uint32_t FifoWritable = 0xEFFF;
const uint32_t RamPacketConfig = 0x0BC;
const uint32_t RamPacketEnable = Bit(16);
const uint32_t RamPacketStatus = 0x0C4;
const uint32_t CrpCfg = 0x0C8;          // 19:0 N
const uint32_t Cts0 = 0x0CC, Cts1 = 0x0D0;
const uint32_t SchedulerControl = 0x0E0;
const uint32_t SchedManualFormat = Bit(15);
const uint32_t SchedIgnoreVsyncPredicts = Bit(5);
const uint32_t SchedHdmiActive = Bit(1);
const uint32_t SchedHdmiMode = Bit(0);
const uint32_t HorzA = 0x0E4;   // 15 vsync +, 14 hsync +, 28:16 front, 13:0 active
const uint32_t HorzB = 0x0E8;   // 26:16 back, 10:0 sync
const uint32_t VertA0 = 0x0EC, VertA1 = 0x0F4;  // 28:24 sync, 22:16 front, 12:0 active
const uint32_t VertB0 = 0x0F0, VertB1 = 0x0F8;  // 29:16 offset, 8:0 back
const uint32_t MiscControl = 0x100;             // 3:0 pixel repeat - 1
const uint32_t DeepColorConfig1 = 0x170;        // 10:8 pack phase, 3:0 depth
const uint32_t GcpConfig = 0x178;
const uint32_t GcpEnable = Bit(31);
const uint32_t GcpWord1 = 0x17C;                // 15:8 depth, 7:0 byte 0
const uint32_t GcpClearAvMute = Bit(4);
const uint32_t Hotplug = 0x1A8;
const uint32_t HotplugConnected = Bit(0);

// dvp
const uint32_t ClockStop = 0x0BC;
const uint32_t ClockStopPixel = Bit(1);
const uint32_t VecInterfaceCfg = 0x0EC;
const uint32_t VecInterfaceXbar = 0x0F0;
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
const uint32_t PhyResetCtl = 0x00;
const uint32_t PhyPllResetb = Bit(5);
const uint32_t PhyPllDivResetb = Bit(4);
const uint32_t PhyLaneResets = 0x0F;
const uint32_t PhyPowerdownCtl = 0x04;
const uint32_t PhyRndgenPowerdown = Bit(4);
const uint32_t PhyCtl0 = 0x08;
const uint32_t PhyCtl1 = 0x0C;
const uint32_t PhyCtl2 = 0x10;
const uint32_t PhyCtl3 = 0x14;
const uint32_t PhyPllCtl0 = 0x1C;
const uint32_t PllMash11 = Bit(13);
const unsigned PllVcoSelLow = 9;
const uint32_t PllVcoContinuous = Bit(6);
const uint32_t PllVcoClockEnable = Bit(5);
const uint32_t PhyPllCtl1 = 0x20;
const uint32_t PllDoublerEnable = Bit(13);
const uint32_t PhyClkDiv = 0x28;          // 15:8 VCO divider
const uint32_t PhyPllCfg = 0x34;          // 3:0 PDIV
const uint32_t PhyTmdsClkWordSel = 0x44;
const uint32_t PhyChannelSwap = 0x4C;
const uint32_t PhyCalibration1 = 0x50;    // 27:0 min limit
const uint32_t PhyCalibration2 = 0x54;    // 27:0 max limit
const uint32_t PhyCalibration4 = 0x5C;    // 31:16 stable, 15:0 hold

// rate manager
const uint32_t RmControl = 0x00;
const uint32_t RmFreezeCounters = Bit(19);
const uint32_t RmLoadIntegrator = Bit(17);
const uint32_t RmFreeRun = Bit(4);
const uint32_t RmOffset = 0x18;
const uint32_t RmOffsetOnly = Bit(31);
const uint32_t RmFormat = 0x1C;          // 25:24 shift

// Mailbox clocks [firmware wiki].
const uint32_t ClockHsm = 13;            // M2MC, the "hdmi" clock
const uint32_t ClockBvb = 14;            // PIXEL_BVB
const uint32_t HsmMinHz = 120000000;
} // namespace hdmi

//
// What differs between the two ports.
//
struct Port {
  uint32_t pv;                 // pixel valve (bus address)
  uint32_t core, dvp, phy, rm, ram, csc;   // HDMI ranges (bus addresses)
  uint32_t vid_ctl;            // VID_CTL's offset in "hd"
  uint32_t hvs_mux;            // HVS register holding the output's channel
  unsigned pv_fifo_level;      // the pixel valve's FIFO level
  uint32_t display_id;         // the firmware's display ID
};

const Port Ports[2] = {
    // HDMI0: pixel valve 2, HVS output 4. FIFO level: 256 bytes less the
    // HVS's latency of 6 three-byte pixels.
    {0x7E20A000, 0x7EF00700, 0x7EF00300, 0x7EF00F00, 0x7EF00F80, 0x7EF01B00,
     0x7EF00200, 0x44, hvs::DispEoln, 256 - 3 * 6, 2},
    // HDMI1: pixel valve 4, HVS output 5; PV4 needs a FIFO level of 32.
    {0x7E216000, 0x7EF05700, 0x7EF05300, 0x7EF05F00, 0x7EF05F80, 0x7EF06B00,
     0x7EF00280, 0x48, hvs::DispDither, 32, 7},
};

} // namespace kms

#endif
