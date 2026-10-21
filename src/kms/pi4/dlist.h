//
// dlist.h
//
// Builds HVS display list entries: RGB planes from memory, placed and
// scaled on the screen, opaque or blended by their pixels' alpha.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
