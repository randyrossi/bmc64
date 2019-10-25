
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2018 Istvan Varga <istvanv@users.sourceforge.net>
// https://github.com/istvan-v/plus4emu/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "plus4emu.hpp"
#include "charconv.hpp"

namespace Plus4Emu {

  static const uint16_t petsciiToUnicode[64] = {
    0x2500, 0x2660, 0x2502, 0x2500, 0x003F, 0x003F, 0x003F, 0x003F,     // 0x60
    0x003F, 0x256E, 0x2570, 0x256F, 0x003F, 0x2572, 0x2571, 0x003F,     // 0x68
    0x003F, 0x25CF, 0x003F, 0x2665, 0x003F, 0x256D, 0x2573, 0x25CB,     // 0x70
    0x2663, 0x003F, 0x2666, 0x253C, 0x003F, 0x2502, 0x03C0, 0x25E5,     // 0x78
    0x00A0, 0x258C, 0x2584, 0x2594, 0x2581, 0x258F, 0x2592, 0x2595,     // 0x80
    0x003F, 0x25E4, 0x003F, 0x251C, 0x2597, 0x2514, 0x2510, 0x2582,     // 0x88
    0x250C, 0x2534, 0x252C, 0x2524, 0x258E, 0x258D, 0x003F, 0x003F,     // 0x90
    0x003F, 0x2583, 0x2713, 0x2596, 0x259D, 0x2518, 0x2598, 0x259A      // 0x98
  };

  static const uint16_t unicodeToPETSCII[94] = {
    0x00A0, 0xA0,   0x00A3, 0x5C,   0x03C0, 0x7E,   0x2190, 0x5F,
    0x2500, 0x63,   0x2502, 0x62,   0x250C, 0xB0,   0x2510, 0xAE,
    0x2514, 0xAD,   0x2518, 0xBD,   0x251C, 0xAB,   0x2524, 0xB3,
    0x252C, 0xB2,   0x2534, 0xB1,   0x253C, 0x7B,   0x256D, 0x75,
    0x256E, 0x69,   0x256F, 0x6B,   0x2570, 0x6A,   0x2571, 0x6E,
    0x2572, 0x6D,   0x2573, 0x76,   0x2581, 0xA4,   0x2582, 0xAF,
    0x2583, 0xB9,   0x2584, 0xA2,   0x258C, 0xA1,   0x258D, 0xB5,
    0x258E, 0xB4,   0x258F, 0xA5,   0x2592, 0xA6,   0x2594, 0xA3,
    0x2595, 0xA7,   0x2596, 0xBB,   0x2597, 0xAC,   0x2598, 0xBE,
    0x259A, 0xBF,   0x259D, 0xBC,   0x25CB, 0x77,   0x25CF, 0x71,
    0x25E4, 0xA9,   0x25E5, 0x7F,   0x2660, 0x61,   0x2663, 0x78,
    0x2665, 0x73,   0x2666, 0x7A,   0x2713, 0xBA
  };

  uint8_t utf8ToPETSCII(const char *buf, size_t& bufPos, bool lowerCaseMode)
  {
    if (!buf)
      return 0x00;
    uint32_t  w = 0U;
    uint8_t   c = uint8_t(buf[bufPos]);
    if (c) {
      bufPos++;
      int     n = 0;
      if (c < 0xC0) {
        w = c;
      }
      else if (c < 0xE0) {
        w = c & 0x1F;
        n = 1;
      }
      else if (c < 0xF0) {
        w = c & 0x0F;
        n = 2;
      }
      else {
        w = c & 0x07;
        n = 3;
      }
      for ( ; n > 0 && buf[bufPos] != '\0'; bufPos++, n--) {
        c = uint8_t(buf[bufPos]);
        if ((c & 0xC0) != 0x80)
          break;
        w = (w << 6) | (c & 0x3F);
      }
    }
    c = 0xFF;
    if (w >= 0x0080U) {
      size_t  i = 0;
      size_t  j = (sizeof(unicodeToPETSCII) / sizeof(uint16_t)) >> 1;
      while (j > i) {
        size_t  k = (i + j) >> 1;
        uint32_t  tmp = unicodeToPETSCII[k << 1];
        if (w < tmp) {
          j = k;
        }
        else if (w > tmp) {
          i = k + 1;
        }
        else {
          c = uint8_t(unicodeToPETSCII[(k << 1) + 1]);
          break;
        }
      }
    }
    else {
      c = uint8_t(w);
      switch (c & 0xE0) {
      case 0x00:
        if (c == '\n')
          c = '\r';
        else if (c != '\0' && c != '\t')
          c = 0xFF;
        break;
      case 0x40:
        if (c >= 0x41 && c <= 0x5A && lowerCaseMode)
          c = c | 0x20;
        break;
      case 0x60:
        if (c >= 0x61 && c <= 0x7A)
          c = c & 0xDF;
        break;
      }
    }
    return c;
  }

  void petsciiToUTF8(std::string& buf, uint8_t c, bool lowerCaseMode)
  {
    uint32_t  w = c;
    switch (c & 0xE0) {
    case 0x40:
      if (lowerCaseMode && c >= 0x41 && c <= 0x5A)
        w = w | 0x20;
      if (c == 0x5C)
        w = 0x00A3;
      if (c == 0x5F)
        w = 0x2190;
      break;
    case 0x60:
      if (lowerCaseMode && c >= 0x61 && c <= 0x7A)
        w = w & 0xDF;
      else
        w = petsciiToUnicode[c & 0x1F];
      break;
    case 0xA0:
      w = petsciiToUnicode[c & 0x3F];
      break;
    }
    if (w < 0x0080U) {
      if (w)
        buf += char(w);
    }
    else if (w < 0x0800U) {
      buf += char((w >> 6) | 0xC0);
      buf += char((w & 0x3F) | 0x80);
    }
    else {
      buf += char((w >> 12) | 0xE0);
      buf += char(((w >> 6) & 0x3F) | 0x80);
      buf += char((w & 0x3F) | 0x80);
    }
  }

  void screenCharToUTF8(std::string& buf, uint8_t c, bool lowerCaseMode)
  {
    c = c & 0x7F;
    switch (c & 0xE0) {
    case 0x00:
      c = c | 0x40;
      break;
    case 0x40:
      c = c | 0x20;
      break;
    case 0x60:
      c = c ^ 0xC0;
      break;
    }
    petsciiToUTF8(buf, c, lowerCaseMode);
  }

}       // namespace Plus4Emu

