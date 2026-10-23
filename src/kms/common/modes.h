//
// modes.h
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
// Modified for BMC64 from BMA800XL's src/kms/common/modes.h: "BMA800XL"
// changed to "BMC64" in a comment and the include guard.
//
// Shared video modes, EDID parsing and HDMI packet and audio clock
// calculations for the Pi 4 and Pi 5 direct display drivers.
//
// Sources:
//  - CTA-861: the timings of the standard modes used (VIC 4, 16, 19, 31),
//    the pixel clock tolerance of +/-0.5 %, the AVI InfoFrame, the EDID
//    extension's video and HDMI vendor data blocks.
//  - VESA Enhanced EDID 1.4: the base block's header, checksum and
//    extension count.
//
// Pure computation, so tools/pi4kms can test it on a PC.
//

#ifndef BMC64_KMS_COMMON_MODES_H_
#define BMC64_KMS_COMMON_MODES_H_

#include <stdint.h>

namespace kms {

struct Timing {
  uint8_t vic;                 // CTA-861 video identification code
  uint32_t pixel_hz;           // pixel (TMDS character) clock
  unsigned h_active, h_front, h_sync, h_back;
  unsigned v_active, v_front, v_sync, v_back;
  bool h_positive, v_positive; // sync polarities

  unsigned HTotal(void) const { return h_active + h_front + h_sync + h_back; }
  unsigned VTotal(void) const { return v_active + v_front + v_sync + v_back; }
  double Refresh(void) const { return (double)pixel_hz / HTotal() / VTotal(); }
};

// The CTA-861 timing for VIC, or 0 if it isn't one BMC64 knows.
const Timing *CtaMode(unsigned vic);

// The known mode with these active sizes and totals (as read back from the
// pixel valve), or 0.
const Timing *FindMode(unsigned h_active, unsigned h_total, unsigned v_active,
                       unsigned v_total);

// What the display says it takes.
struct Edid {
  bool valid;                  // a base block with a good checksum
  bool hdmi;                   // an HDMI sink (vendor block 00-0C-03)
  unsigned vic_count;
  uint8_t vics[64];

  bool Supports(unsigned vic) const;
};

// Parses BLOCKS 128-byte blocks of EDID.
void ParseEdid(const uint8_t *data, unsigned blocks, Edid *edid);

// The mode to run at FPS frames a second, with the resolution of CURRENT:
// its 50 Hz or 60 Hz sibling (whichever is nearer FPS), with the pixel
// clock moved (within CTA-861's 0.5 %) to make the refresh exactly FPS.
// False if there is none, or the display doesn't list it.
bool ChooseMode(const Timing &current, double fps, const Edid &edid,
                Timing *chosen);

// The AVI InfoFrame for TIMING: RGB, limited range, 16:9; 17 bytes (3 of
// header, then the checksum and 13 of payload).
const unsigned AviLength = 17;
void AviInfoFrame(const Timing &timing, uint8_t frame[AviLength]);

// LENGTH bytes of an InfoFrame laid out as the HDMI packet RAM takes
// them: 7 bytes in each pair of words (3 then 4). Writes WORDS (9) words,
// the unused ones 0.
const unsigned PacketWords = 9;
void PacketRamWords(const uint8_t *frame, unsigned length, uint32_t words[PacketWords]);

// HDMI audio clock regeneration: CTS for N at fs samples a second
// (128 fs = f_TMDS N / CTS).
uint32_t AudioCts(uint64_t tmds_hz, uint32_t n, uint32_t fs);

} // namespace kms

#endif
