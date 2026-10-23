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
// Parts adapted from Linux (the Raspberry Pi kernel, rpi-6.18.y),
// which remain under their license, the GNU General Public License
// version 2 or (at your option) any later version:
//
//     drivers/gpu/drm/v3d/v3d_regs.h        Copyright (C) 2017-2018 Broadcom
//     drivers/gpu/drm/v3d/v3d_drv.h         Copyright (C) 2015-2018 Broadcom
//     drivers/pmdomain/bcm/bcm2835-power.c  Copyright (C) 2018 Broadcom
//
// From BMA800XL's src/v3d/regs.h, unchanged but for the include guard's name.
//
// Addresses and registers of the 3D GPU, V3D 4.2 on the Pi 4 and V3D 7.1
// on the Pi 5, and of the power controls it needs.
//
// Register offsets and fields adapted from Linux (the Raspberry Pi kernel,
// rpi-6.18.y), drivers/gpu/drm/v3d/v3d_regs.h, v3d_drv.h and
// drivers/pmdomain/bcm/bcm2835-power.c, Copyright (C) 2015-2018 Broadcom,
// GPL 2.0 or later (sdcard/licenses/Linux-V3D.txt). Addresses from the
// BCM2711 device tree (bcm2711.dtsi, bcm2711-rpi-ds.dtsi) and the BCM2712
// device tree (bcm2712-ds.dtsi: v3d, pm), with Circle's
// include/circle/memorymap64.h for where the BCM2712's AXI and SoC
// peripherals are.
//

#ifndef BMC64_V3D_REGS_H_
#define BMC64_V3D_REGS_H_

#include <stdint.h>

namespace v3d {

#if RASPPI >= 5
// ARM addresses.
const uintptr_t HubBase = 0x1002000000;
const uintptr_t CoreBase = 0x1002008000; // core 0
const uintptr_t SmsBase = 0x1002030800;  // the V3D's power state machine
const uintptr_t PmBase = 0x107D200000;
#else
// ARM addresses (low peripheral mode).
const uintptr_t HubBase = 0xFEC00000;
const uintptr_t CoreBase = 0xFEC04000;   // core 0
const uintptr_t PmBase = 0xFE100000;
const uintptr_t AsbBase = 0xFEC11000;    // the V3D's async bus bridges
#endif

// Firmware mailbox.
const uint32_t TagSetDomainState = 0x00038030;
const uint32_t TagSetClockState = 0x00038001; // clock id, state (bit 0: on)
const uint32_t DomainV3d = 10;
const uint32_t ClockV3d = 5;

// Hub registers.
const uint32_t HubAxiCfg = 0x00000;
const uint32_t HubIdent0 = 0x00008;
const uint32_t HubIdent1 = 0x0000C;      // 3:0 tech version, 7:4 revision, 11:8 cores
const uint32_t HubIdent2 = 0x00010;
const uint32_t HubIdent3 = 0x00014;      // 15:8 revision (V3D 7.1)
const uint32_t MmucControl = 0x01000;
const uint32_t MmucEnable = 1u << 0;
const uint32_t MmucFlush = 1u << 1;
const uint32_t MmucFlushing = 1u << 2;
const uint32_t MmuCtl = 0x01200;
const uint32_t MmuEnable = 1u << 0;
const uint32_t MmuTlbClear = 1u << 2;
const uint32_t MmuTlbClearing = 1u << 7;
const uint32_t MmuWriteViolationInt = 1u << 10;
const uint32_t MmuWriteViolationAbort = 1u << 11;
const uint32_t MmuPtInvalidEnable = 1u << 16;
const uint32_t MmuPtInvalidInt = 1u << 18;
const uint32_t MmuPtInvalidAbort = 1u << 19;
const uint32_t MmuCapExceededInt = 1u << 25;
const uint32_t MmuCapExceededAbort = 1u << 26;
const uint32_t MmuPtPaBase = 0x01204;    // page table address >> 12
const uint32_t MmuVioId = 0x0122C;
const uint32_t MmuIllegalAddr = 0x01230;
const uint32_t MmuIllegalAddrEnable = 1u << 31;
const uint32_t MmuVioAddr = 0x01234;
const uint32_t MmuDebugInfo = 0x01238;

// MMU page table entries: 4 KB pages.
const unsigned PageShift = 12;
const uint32_t PteValid = 1u << 28;
const uint32_t PteWriteable = 1u << 29;

// Core registers.
const uint32_t CtlIdent0 = 0x00000;
const uint32_t CtlIdent1 = 0x00004;      // 31:28 VPM size in 8 KB
const uint32_t CtlSlcActl = 0x00024;     // slice caches: write 0xF to each field to invalidate
const uint32_t CtlL2tcActl = 0x00030;
const uint32_t L2tFlush = 1u << 0;       // with mode 0 (flush) in bits 2:1
const uint32_t CtlL2tFlushStart = 0x00034;
const uint32_t CtlL2tFlushEnd = 0x00038;
const uint32_t CtlIntSts = 0x00050;
const uint32_t CleCt0Cs = 0x00100;
const uint32_t CleCt1Cs = 0x00104;
const uint32_t CleCt0Ca = 0x00110;
const uint32_t CleCt1Ca = 0x00114;
const uint32_t CleBfc = 0x00134;         // binning frames completed
const uint32_t CleRfc = 0x00138;         // rendering frames completed
const uint32_t CleCt0Qts = 0x0015C;      // binning tile state address
const uint32_t CleCt0QtsEnable = 1u << 1;
const uint32_t CleCt0Qba = 0x00160;      // binning control list start
const uint32_t CleCt1Qba = 0x00164;      // render control list start
const uint32_t CleCt0Qea = 0x00168;      // binning control list end; writing it starts the job
const uint32_t CleCt1Qea = 0x0016C;      // render control list end; writing it starts the job
const uint32_t CleCt0Qma = 0x00170;      // tile allocation memory
const uint32_t CleCt0Qms = 0x00174;      // and its size
const uint32_t PtbBpos = 0x0030C;        // binner overflow memory size
const uint32_t ErrStat = 0x00F20;

// Power manager: the GRAFX domain register; writes need the password.
const uint32_t PmGrafx = 0x10C;
const uint32_t PmPassword = 0x5A000000;
const uint32_t PmV3dResetN = 1u << 6;

#if RASPPI >= 5
// The BCM2712's power manager: the GRAFX domain's power-up steps, and the
// V3D's reset in its own register.
const uint32_t PmGrafx2712 = 0x304;
const uint32_t PmPowUp = 1u << 0;
const uint32_t PmPowOk = 1u << 1;
const uint32_t PmIsPow = 1u << 2;
const uint32_t PmMemRep = 1u << 3;
const uint32_t PmMrDone = 1u << 4;
const uint32_t PmIsFunc = 1u << 5;
const unsigned PmInrushShift = 13;       // 2 bits: 3.5, 5, 10, 20 mA
const uint32_t PmInrushMask = 3u << PmInrushShift;

// SMS: the TEE and REE control registers; the state in bits 3:0.
const uint32_t SmsReeCs = 0x000;
const uint32_t SmsTeeCs = 0x400;
const uint32_t SmsClearPowerOff = 1u << 29;
const uint32_t SmsStateMask = 0xF;
const uint32_t SmsIdle = 0x0;
const uint32_t SmsReset = 0x4;           // written to the state field
const uint32_t SmsIsolatingForReset = 0xA;
const uint32_t SmsResetting = 0xB;
#endif

#if RASPPI == 4
// Bus bridges: slave and master control; bit 0 requests stop, bit 1
// acknowledges it.
const uint32_t AsbSlaveCtrl = 0x08;
const uint32_t AsbMasterCtrl = 0x0C;
const uint32_t AsbRequestStop = 1u << 0;
const uint32_t AsbAck = 1u << 1;
#endif

} // namespace v3d

#endif
