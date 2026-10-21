//
// hdmisound.h
//
// HDMI sound on either HDMI port of the Pi 4 or 5. Circle's
// CHDMISoundBaseDevice drives HDMI0 only ("This driver does not support
// HDMI1 on Raspberry Pi 4"); this is that driver, adapted to take the
// port, for the Pi 4 and 5 only and with DMA only.
//
// Derived from Circle, lib/sound/hdmisoundbasedevice.cpp and
// include/circle/sound/hdmisoundbasedevice.h,
// Copyright (C) 2021-2024 R. Stange <rsta2@o2online.de>, GPL 3 or later:
// the register set-up, the MAI, the audio InfoFrame, N and CTS, the DMA;
// for the Pi 5, its core register offsets, the D0 stepping's DMA request
// and MAI threshold fields, and no PHY power control. Changed here: the
// registers are those of the port asked for; the MAI sample rate ratio is
// worked out by this file's own continued fraction code; HDMI1's 108 MHz
// audio clock is ungated.
//
// HDMI1's addresses:
//  - BCM2711 device tree (arch/arm/boot/dts/broadcom/bcm2711.dtsi, node
//    hdmi1): core at 0x7EF05700, PHY at 0x7EF05F00, packet RAM at
//    0x7EF06B00, the shared "hd" block at 0x7EF20000; DMA request 17
//    (HDMI0's is 10).
//  - BCM2712 device trees (Raspberry Pi Linux, arch/arm64/boot/dts/
//    broadcom/bcm2712.dtsi, bcm2712-rpi.dtsi, bcm2712d0-rpi-5-b.dts):
//    HDMI0's core at 0x7C701400, PHY 0x7C701D00, packet RAM 0x7C703800;
//    HDMI1's at 0x7C706400, 0x7C706D00, 0x7C708800; "hd" at 0x7C720000;
//    the DVP at 0x7C700000; DMA requests 10 and 17, 12 and 13 on D0.
//  - Linux, drivers/gpu/drm/vc4/vc4_hdmi_regs.h (vc5_hdmi_hdmi1_fields,
//    vc6_hdmi_hdmi0_fields, vc6_hdmi_hdmi1_fields): HDMI1's MAI
//    registers at 0x30 to 0x40 in "hd" (HDMI0's at 0x10 to 0x20); its
//    core registers at the same offsets as HDMI0's; on the Pi 5 the hot
//    plug register at 0x1C8.
//  - Linux, drivers/clk/bcm/clk-bcm2711-dvp.c: bit 4 of the DVP's
//    MISC_CONFIG (DVP + 8) gates HDMI1's 108 MHz audio clock (set =
//    off); the Pi 5's DVP is the same block ("brcm,brcm2711-dvp").
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_AV_HDMISOUND_H_
#define BMC64_AV_HDMISOUND_H_

#include <circle/dmachannel.h>
#include <circle/interrupt.h>
#include <circle/sound/soundbasedevice.h>
#include <circle/spinlock.h>
#include <circle/types.h>

class Pi4HdmiSound : public CSoundBaseDevice {
public:
  // PORT 0 or 1. CHUNK_SIZE as for CHDMISoundBaseDevice: twice the frames
  // per DMA buffer, a multiple of 384.
  Pi4HdmiSound(CInterruptSystem *pInterrupt, unsigned port,
               unsigned nSampleRate = 48000, unsigned nChunkSize = 384 * 10);
  ~Pi4HdmiSound(void);

  boolean Start(void) override;
  void Cancel(void) override;
  boolean IsActive(void) const override;

  // The port a display is plugged into (HDMI0 first), or -1.
  static int ConnectedPort(void);

private:
  enum State { Created, Idle, Running, Cancelled, Error };

  void RunHdmi(void);
  void StopHdmi(void);
  void ResetHdmi(void);
  boolean SetAudioInfoFrame(void);
  void DmaCompletion(unsigned nBuffer, boolean bStatus);
  static void DmaCompletionStub(unsigned nChannel, unsigned nBuffer,
                                boolean bStatus, void *pParam);
  static boolean WaitForBit(uintptr nAddress, u32 nMask, boolean bSet,
                            unsigned nMsTimeout);

  unsigned m_nPort;
  unsigned m_nSampleRate;
  unsigned m_nChunkSize;
  unsigned long m_ulPixelClockRate;

  // The port's registers.
  uintptr m_Core, m_Phy, m_Ram, m_Mai;

  volatile State m_State;
  CDMAChannel *m_pDMAChannel;
  u32 *m_pDMABuffer[2];
  CSpinLock m_SpinLock;
};

#endif
