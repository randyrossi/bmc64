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
// From BMA800XL's src/kms/pi5/phy.h, unchanged but for the include guard's
// name.
//
// The BCM2712 HDMI PHY's settings for a TMDS character rate: its PLL's
// VCO divider, the rate manager's offset, the lane driver words, and the
// fixed PLL set-up values, from the behaviour of the Linux vc4 display
// driver.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//

#ifndef BMC64_KMS_PI5_PHY_H_
#define BMC64_KMS_PI5_PHY_H_

#include <stdint.h>

namespace kms {

struct Pi5Phy {
  unsigned vco_divider;
  uint64_t vco_hz;
  uint32_t rm_offset;          // with "offset only" set
  uint32_t data_lane;          // TX0, TX1 and TX2
  uint32_t clock_lane;
  uint32_t word_select;        // TMDS_CLK_WORD_SEL
};

// False at 340 MHz and above (no scrambling), or for a rate of 0.
bool ComputePi5Phy(uint64_t tmds_hz, Pi5Phy *phy);

// The PLL's nine fixed set-up words (PLL_MISC_0 to PLL_MISC_8).
const unsigned PllMiscWords = 9;
extern const uint32_t PllMisc[PllMiscWords];

} // namespace kms

#endif
