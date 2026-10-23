//
// modeset.h
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
// Modified for BMC64 from BMA800XL's src/kms/pi5/modeset.h: "BMA800XL" changed
// to "BMC64" in a comment and the include guard.
//
// The Pi 5's direct display driver's mode setting: re-times the HDMI port
// in use to another mode of the same size, through its pixel valve (0 or
// 1), the HDMI encoder and its PHY, with the HVS channel restarted to
// match. Only modes BMC64 knows (src/kms/common/modes.h) are switched
// between, starting from a mode it recognises; if a step doesn't complete,
// the old mode is put back.
//
// The steps and their order follow the behaviour of the Linux vc4 display
// driver.
//

#ifndef BMC64_KMS_PI5_MODESET_H_
#define BMC64_KMS_PI5_MODESET_H_

#include "hvs.h"
#include "../common/modes.h"

#include <stdint.h>

namespace kms {

class ModeSetter {
public:
  ModeSetter(void);

  // Uses HDMI PORT (0 or 1), its pixel valve and its registers. Called
  // before anything else.
  void SetPort(unsigned port);

  // The mode the port is running, if it is one BMC64 knows (with its
  // nominal pixel clock, or the last one set here).
  bool Current(Timing *timing);

  // The display's EDID on the port, through the firmware (read once).
  const Edid &GetEdid(void);

  // Switches to TIMING (same size as now). AUDIO_FS is the sample rate of
  // the HDMI sound, for the audio clock (0: none). REINSTALL is called
  // between stopping and restarting the HVS channel, to put the picture's
  // display list back. False if it failed (the old mode is then restored
  // if possible).
  bool Set(Hvs &hvs, const Timing &timing, unsigned audio_fs,
           void (*reinstall)(void *), void *param);

private:
  bool ReadEdidBlock(unsigned b, bool by_display, uint8_t block[128]);
  bool Apply(Hvs &hvs, const Timing &timing, bool hdmi_sink, unsigned audio_fs,
             void (*reinstall)(void *), void *param);
  void Disable(Hvs &hvs);
  void SetClocks(uint64_t tmds_hz);
  void SetPhy(uint64_t tmds_hz);
  void SetHdmiTimings(const Timing &timing);
  void SetPixelValve(const Timing &timing);
  void SetColour(bool limited);
  bool WriteInfoFrame(const Timing &timing);
  bool RecenterFifo(void);
  bool WaitFor(uintptr_t reg, uint32_t mask, bool set, unsigned timeout_ms);

  unsigned m_nPort;
  uintptr_t m_Pv, m_Core, m_Dvp, m_Phy, m_Rm, m_Ram, m_Csc, m_VidCtl;
  uint32_t m_nSavedPackets;   // RAM_PACKET_CONFIG before disabling
  bool m_bEdidRead;
  Edid m_Edid;
  uint32_t m_nPixelHz;        // the pixel clock last set here (0: firmware's)
  // The firmware's pixel valve set-up, kept when re-timing: pixels a
  // clock, V_CONTROL's odd timing bit and PIPE_INIT_CTRL (read once).
  bool m_bPvRead;
  unsigned m_nPpc;
  uint32_t m_nOddTiming;
  uint32_t m_nPipeInit;
};

} // namespace kms

#endif
