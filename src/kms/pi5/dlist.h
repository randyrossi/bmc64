//
// dlist.h
//
// Builds the BCM2712 HVS's display list entries: one RGB plane from
// memory, placed and scaled on the screen; and reads the entries of a
// list already in display list memory (the firmware's), to find the
// memory they use.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
//

#ifndef BMC64_KMS_PI5_DLIST_H_
#define BMC64_KMS_PI5_DLIST_H_

#include <stdint.h>

namespace kms {

struct Plane {
  uint64_t address;       // first pixel as the HVS sees it (40 bits)
  unsigned pitch;         // bytes per source line
  unsigned src_width, src_height;
  unsigned x, y;          // on the screen
  unsigned width, height; // on the screen
  bool nearest;           // sharp (nearest neighbour) rather than smooth
  bool alpha;             // blended by each pixel's alpha (else opaque)
};

// Where the plane's share of the HVS's memories is.
struct PlaneMemory {
  unsigned upm_handle;    // prefetch buffer handle, 0-31
  unsigned upm_base;      // its start, in 256-byte units
  unsigned lbm;           // line buffer start, in blocks (vertical scaling)
  unsigned kernel;        // filter kernel's word in display list memory
};

// Units of prefetch memory (256 bytes) a plane with PITCH bytes per line
// needs.
unsigned UpmUnitsFor(unsigned pitch);

// Blocks of line buffer memory the plane needs (0 without vertical
// scaling).
unsigned LbmBlocksFor(const Plane &plane);

// Writes the plane's entry and the list's end word to OUT (at least
// MaxWords long), for the HVS's D stepping if STEPPING_D, else C. Returns
// the number of words written, 0 if the plane can't be shown (zero size,
// too big, or shrinking by more than the PPF can).
const unsigned MaxWords = 24;
unsigned BuildList(const Plane &plane, const PlaneMemory &memory, bool stepping_d,
                   uint32_t *out);

// What one entry of an existing list uses.
struct EntryUse {
  unsigned words;         // its length
  bool has_upm;
  unsigned upm_handle, upm_base, upm_units;
  bool has_lbm;
  unsigned lbm, lbm_blocks;
};

// Reads the entry starting at WORDS[0] (AVAILABLE words readable). False
// at the end of the list or if the entry doesn't make sense.
bool ReadEntry(const uint32_t *words, unsigned available, EntryUse *use);

} // namespace kms

#endif
