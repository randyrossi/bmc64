//
// phy.h
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
// From BMA800XL's src/kms/pi4/phy.h, unchanged but for the include guard's
// name.
//
// The BCM2711 HDMI PHY's settings for a TMDS character rate and the
// wiring of its output lanes.
//
// Source: the behaviour of the Linux vc4 display driver (register
// values and lane wiring).
//
// Pure computation, so tools/pi4kms can test it on a PC.
//

#ifndef BMC64_KMS_PI4_PHY_H_
#define BMC64_KMS_PI4_PHY_H_

#include <stdint.h>

namespace kms {

// The PHY's settings for a TMDS character rate.
struct PhySettings {
  uint64_t vco_hz;
  unsigned vco_divider;
  bool vco_high;               // VCO select: above 4.5 GHz
  uint32_t rm_offset;          // rate manager offset
  unsigned icp;                // charge pump current
  unsigned vco_gain;
  // Per lane (0, 1, 2, clock): pre-emphasis, main driver, res_sel,
  // termination.
  unsigned preemphasis[4], main_driver[4], res_sel[4], termination[4];
};
// False above 340 MHz (no scrambling).
bool ComputePhy(uint64_t tmds_hz, PhySettings *phy);

// How HDMI PORT's PHY is wired: for each output (TX0, TX1, TX2, clock), the
// lane it carries, 3 being the clock lane. Each output takes that lane's
// PhySettings.
void PhyLanes(unsigned port, unsigned lanes[4]);
// CHANNEL_SWAP for that wiring: 2 bits per output at 1:0, 5:4, 9:8, 13:12.
uint32_t PhyChannelSwap(unsigned port);

} // namespace kms

#endif
