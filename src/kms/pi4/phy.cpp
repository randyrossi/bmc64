//
// phy.cpp
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
// From BMA800XL's src/kms/pi4/phy.cpp, unchanged.
//
// See phy.h for the BCM2711 HDMI PHY calculations and their source.
//

#include "phy.h"

namespace kms {

bool ComputePhy(uint64_t tmds_hz, PhySettings *phy) {
  if (tmds_hz == 0 || tmds_hz > 340000000ull) {
    return false;
  }
  // VCO: f x 10 x d, the smallest d reaching 3 GHz.
  unsigned d = 1;
  while (tmds_hz * 10 * d < 3000000000ull) {
    d++;
  }
  phy->vco_divider = d;
  phy->vco_hz = tmds_hz * 10 * d;
  phy->vco_high = phy->vco_hz > 4500000000ull;
  // Rate manager offset: VCO x 2 x 2^22 / 54 MHz, then / 4.
  phy->rm_offset = (uint32_t)(((phy->vco_hz * 2) << 22) / 54000000ull >> 2);
  phy->icp = phy->vco_hz < 3700000000ull ? 0x1C : 0x18;
  if (phy->vco_hz < 3350000000ull) {
    phy->vco_gain = 0xF;
  } else if (phy->vco_hz < 3700000000ull) {
    phy->vco_gain = 0xC;
  } else if (phy->vco_hz < 4050000000ull) {
    phy->vco_gain = 0x6;
  } else if (phy->vco_hz < 4800000000ull) {
    phy->vco_gain = 0x5;
  } else if (phy->vco_hz < 5200000000ull) {
    phy->vco_gain = 0x7;
  } else {
    phy->vco_gain = 0x2;
  }

  // Lane drive settings by rate band.
  struct Band {
    uint64_t max_hz;
    unsigned data[4];   // pre-emphasis, main driver, res_sel, termination
    unsigned clock[4];
  };
  static const Band bands[] = {
      {50000000, {0, 0x0A, 0x12, 0}, {0, 0x0A, 0x18, 0}},
      {75000000, {0, 0x09, 0x12, 0}, {0, 0x0C, 0x18, 3}},
      {165000000, {0, 0x09, 0x12, 0}, {0, 0x0C, 0x18, 3}},
      {250000000, {0, 0x0F, 0x12, 1}, {0, 0x0C, 0x18, 3}},
      {340000000, {2, 0x0D, 0x12, 1}, {0, 0x0C, 0x18, 0xF}},
  };
  const Band *band = &bands[0];
  for (const Band &b : bands) {
    band = &b;
    if (tmds_hz <= b.max_hz) {
      break;
    }
  }
  for (unsigned lane = 0; lane < 4; lane++) {
    const unsigned *s = lane < 3 ? band->data : band->clock;
    phy->preemphasis[lane] = s[0];
    phy->main_driver[lane] = s[1];
    phy->res_sel[lane] = s[2];
    phy->termination[lane] = s[3];
  }
  return true;
}

void PhyLanes(unsigned port, unsigned lanes[4]) {
  static const unsigned wiring[2][4] = {
      {0, 1, 2, 3},   // HDMI0: in order
      {1, 0, 3, 2},   // HDMI1: lane 1, lane 0, clock, lane 2
  };
  for (unsigned out = 0; out < 4; out++) {
    lanes[out] = wiring[port != 0][out];
  }
}

uint32_t PhyChannelSwap(unsigned port) {
  unsigned lanes[4];
  PhyLanes(port, lanes);
  return lanes[0] | lanes[1] << 4 | lanes[2] << 8 | lanes[3] << 12;
}

} // namespace kms
