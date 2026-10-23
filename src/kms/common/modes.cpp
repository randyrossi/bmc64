//
// modes.cpp
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
// Modified for BMC64 from BMA800XL's src/kms/common/modes.cpp: "BMA800XL"
// changed to "BMC64" in a comment.
//
// See modes.h.
//

#include "modes.h"

#include <string.h>

namespace kms {

namespace {

// CTA-861 modes BMC64 switches between: 720p and 1080p, at 60 and
// 50 Hz.
const Timing Modes[] = {
    {4, 74250000, 1280, 110, 40, 220, 720, 5, 5, 20, true, true},
    {19, 74250000, 1280, 440, 40, 220, 720, 5, 5, 20, true, true},
    {16, 148500000, 1920, 88, 44, 148, 1080, 4, 5, 36, true, true},
    {31, 148500000, 1920, 528, 44, 148, 1080, 4, 5, 36, true, true},
};

} // namespace

const Timing *CtaMode(unsigned vic) {
  for (const Timing &t : Modes) {
    if (t.vic == vic) {
      return &t;
    }
  }
  return 0;
}

const Timing *FindMode(unsigned h_active, unsigned h_total, unsigned v_active,
                       unsigned v_total) {
  for (const Timing &t : Modes) {
    if (t.h_active == h_active && t.HTotal() == h_total &&
        t.v_active == v_active && t.VTotal() == v_total) {
      return &t;
    }
  }
  return 0;
}

bool Edid::Supports(unsigned vic) const {
  for (unsigned i = 0; i < vic_count; i++) {
    if (vics[i] == vic) {
      return true;
    }
  }
  return false;
}

void ParseEdid(const uint8_t *data, unsigned blocks, Edid *edid) {
  memset(edid, 0, sizeof *edid);
  static const uint8_t header[8] = {0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00};
  if (blocks == 0 || memcmp(data, header, 8) != 0) {
    return;
  }
  uint8_t sum = 0;
  for (unsigned i = 0; i < 128; i++) {
    sum = (uint8_t)(sum + data[i]);
  }
  if (sum != 0) {
    return;
  }
  edid->valid = true;

  unsigned extensions = data[126];
  for (unsigned b = 1; b <= extensions && b < blocks; b++) {
    const uint8_t *ext = data + 128 * b;
    uint8_t ext_sum = 0;
    for (unsigned i = 0; i < 128; i++) {
      ext_sum = (uint8_t)(ext_sum + ext[i]);
    }
    if (ext[0] != 0x02 || ext_sum != 0) {
      continue;   // not a (good) CTA extension
    }
    // Data blocks from byte 4 up to the offset in byte 2: a header byte
    // (tag in bits 7:5, length in 4:0), then the payload.
    unsigned end = ext[2];
    if (end < 4 || end > 127) {
      continue;
    }
    for (unsigned i = 4; i < end;) {
      unsigned tag = ext[i] >> 5, length = ext[i] & 0x1F;
      if (i + 1 + length > end) {
        break;
      }
      const uint8_t *payload = ext + i + 1;
      if (tag == 2) {
        // Video data block: VICs, low 7 bits (bit 7 marks "native").
        for (unsigned k = 0; k < length && edid->vic_count < 64; k++) {
          edid->vics[edid->vic_count++] = payload[k] & 0x7F;
        }
      } else if (tag == 3 && length >= 3 && payload[0] == 0x03 &&
                 payload[1] == 0x0C && payload[2] == 0x00) {
        // Vendor specific data block, IEEE OUI 00-0C-03 (stored least
        // significant byte first): HDMI.
        edid->hdmi = true;
      }
      i += 1 + length;
    }
  }
}

bool ChooseMode(const Timing &current, double fps, const Edid &edid,
                Timing *chosen) {
  if (fps < 45.0 || fps > 65.0) {
    return false;
  }
  bool fifty = fps < 55.0;
  const Timing *base = 0;
  for (const Timing &t : Modes) {
    bool t_fifty = t.Refresh() < 55.0;
    if (t.h_active == current.h_active && t.v_active == current.v_active &&
        t_fifty == fifty) {
      base = &t;
    }
  }
  if (base == 0 || !edid.valid || !edid.Supports(base->vic)) {
    return false;
  }
  *chosen = *base;
  double clock = (double)base->HTotal() * base->VTotal() * fps;
  // CTA-861 allows the pixel clock to be 0.5 % off.
  if (clock < base->pixel_hz * 0.995 || clock > base->pixel_hz * 1.005) {
    return false;
  }
  chosen->pixel_hz = (uint32_t)(clock + 0.5);
  return true;
}

void AviInfoFrame(const Timing &timing, uint8_t frame[AviLength]) {
  memset(frame, 0, AviLength);
  frame[0] = 0x82;   // AVI InfoFrame
  frame[1] = 2;      // version
  frame[2] = 13;     // payload length
  // PB1: Y = 0 (RGB), A0 = 1 (active format present), no bars, S = 0.
  frame[4] = 0x10;
  // PB2: C = 0 (no colorimetry data), M = 2 (16:9), R = 8 (as the picture).
  frame[5] = (2 << 4) | 8;
  // PB3: Q = 1 (limited range RGB).
  frame[6] = 1 << 2;
  // PB4: the VIC.
  frame[7] = timing.vic;
  // Checksum: all 17 bytes add up to 0.
  uint8_t sum = 0;
  for (unsigned i = 0; i < AviLength; i++) {
    sum = (uint8_t)(sum + frame[i]);
  }
  frame[3] = (uint8_t)(0x100 - sum);
}

void PacketRamWords(const uint8_t *frame, unsigned length, uint32_t words[PacketWords]) {
  for (unsigned i = 0; i < PacketWords; i++) {
    words[i] = 0;
  }
  uint8_t bytes[28] = {0};
  for (unsigned i = 0; i < length && i < sizeof bytes; i++) {
    bytes[i] = frame[i];
  }
  unsigned w = 0;
  for (unsigned i = 0; i + 7 <= sizeof bytes && w + 1 < PacketWords; i += 7) {
    words[w++] = bytes[i] | bytes[i + 1] << 8 | bytes[i + 2] << 16;
    words[w++] = bytes[i + 3] | bytes[i + 4] << 8 | bytes[i + 5] << 16 |
                 (uint32_t)bytes[i + 6] << 24;
  }
}

uint32_t AudioCts(uint64_t tmds_hz, uint32_t n, uint32_t fs) {
  if (fs == 0) {
    return 0;
  }
  return (uint32_t)((tmds_hz * n + 64ull * fs) / (128ull * fs));
}

} // namespace kms
