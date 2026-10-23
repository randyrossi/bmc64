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
// From BMA800XL's src/kms/pi5/phy.cpp, unchanged.
//
// See phy.h.
//

#include "phy.h"

namespace kms {

namespace {

// The PLL's VCO runs between 8 and 12 GHz, from the 54 MHz crystal.
const uint64_t VcoMinHz = 8000000000ull;
const uint64_t VcoMaxHz = 12000000000ull;
const uint64_t ReferenceHz = 54000000;
const uint64_t ScramblingHz = 340000000;

// Lane driver words by TMDS rate (up to and including MAX_HZ): the data
// lanes', then the clock lane's.
struct LaneRow {
  uint64_t max_hz;
  uint32_t data, clock;
};
const LaneRow LaneRows[] = {
    {222000000, 0x80828700, 0x80828700},
    {297000000, 0xC0870000, 0xC0870800},
    {597000044, 0x848F8700, 0x849F8F00},
};

} // namespace

const uint32_t PllMisc[PllMiscWords] = {
    0x810C6000, 0x00B8C451, 0x46402E31, 0x00B8C005, 0x42410261,
    0xCC021001, 0xC8301C80, 0xB0804444, 0xF80F8000,
};

bool ComputePi5Phy(uint64_t tmds_hz, Pi5Phy *phy) {
  if (tmds_hz == 0 || tmds_hz >= ScramblingHz) {
    return false;
  }
  // The VCO is ten times the TMDS rate (ten bits a character), times a
  // divider: the middle one of those keeping it between 8 and 12 GHz.
  uint64_t bit_hz = 10 * tmds_hz;
  unsigned low = (unsigned)((VcoMinHz + bit_hz - 1) / bit_hz);
  unsigned high = low;
  while (bit_hz * (high + 1) < VcoMaxHz) {
    high++;
  }
  phy->vco_divider = low + (high - low) / 2;
  phy->vco_hz = bit_hz * phy->vco_divider;
  // The VCO in units of the reference, with 21 fraction bits.
  uint64_t offset = phy->vco_hz * 2 * (1ull << 22) / ReferenceHz / 4;
  phy->rm_offset = (uint32_t)(offset & 0x7FFFFFFF) | 0x80000000u;
  const LaneRow *row = &LaneRows[sizeof LaneRows / sizeof LaneRows[0] - 1];
  for (const LaneRow &r : LaneRows) {
    if (tmds_hz <= r.max_hz) {
      row = &r;
      break;
    }
  }
  phy->data_lane = row->data;
  phy->clock_lane = row->clock;
  phy->word_select = 0;
  return true;
}

} // namespace kms
