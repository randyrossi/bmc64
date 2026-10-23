//
// dlist.h
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
// Modified for BMC64 from BMA800XL's src/kms/pi4/dlist.h: planes can be
// blended by per-pixel alpha.
//
// Builds HVS display list entries: RGB planes from memory, placed and
// scaled on the screen, opaque or blended by their pixels' alpha.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//

#ifndef BMC64_KMS_PI4_DLIST_H_
#define BMC64_KMS_PI4_DLIST_H_

#include <stdint.h>

namespace kms {

struct Plane {
  uint32_t address;       // first pixel as the HVS sees it (bus address), 32-bit 0xAARRGGBB words
  unsigned pitch;         // bytes per source line
  unsigned src_width, src_height;
  unsigned x, y;          // on the screen
  unsigned width, height; // on the screen
  bool nearest;           // sharp (nearest neighbour) rather than smooth
  bool alpha;             // blended by each pixel's alpha (else opaque)
};

// Words of line buffer memory the plane needs (0 without vertical
// scaling), aligned to 64 words.
unsigned LbmWords(const Plane &plane);

// Writes the plane's entry and the end word to OUT (at least
// MaxWords long). LBM is the plane's line buffer memory address (if it
// needs one); KERNEL the word offset of the filter kernel in display list
// memory. Returns the number of words written, 0 if the plane can't be
// shown (zero size, too big, or shrinking by more than the PPF can).
const unsigned MaxWords = 24;
unsigned BuildList(const Plane &plane, unsigned lbm, unsigned kernel,
                   uint32_t *out);

} // namespace kms

#endif
