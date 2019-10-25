
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2018 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
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
#include "cpu.hpp"
#include "ted.hpp"

namespace Plus4 {

  PLUS4EMU_REGPARM2 void TED7360::render_BMM_hires(TED7360& ted,
                                                   int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 5;
    bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
    if (nextCharCnt == 0) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      ted.shiftRegisterCharacter.bitmap_() = b << 4;
      uint8_t c = ted.shiftRegisterCharacter.char_();
      uint8_t c0 = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
      uint8_t c1 = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
      bufp[1] = ((b & uint8_t(0x80)) ? c1 : c0);
      bufp[2] = ((b & uint8_t(0x40)) ? c1 : c0);
      bufp[3] = ((b & uint8_t(0x20)) ? c1 : c0);
      bufp[4] = ((b & uint8_t(0x10)) ? c1 : c0);
    }
    else {
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t c = ted.shiftRegisterCharacter.char_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      uint8_t c0 = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
      uint8_t c1 = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
      switch (nextCharCnt) {
      case 1:
        bufp[1] = ((b & uint8_t(0x80)) ? c1 : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 3;
        c = ted.shiftRegisterCharacter.char_();
        c0 = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        c1 = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        bufp[2] = ((b & uint8_t(0x80)) ? c1 : c0);
        bufp[3] = ((b & uint8_t(0x40)) ? c1 : c0);
        bufp[4] = ((b & uint8_t(0x20)) ? c1 : c0);
        break;
      case 2:
        bufp[1] = ((b & uint8_t(0x80)) ? c1 : c0);
        bufp[2] = ((b & uint8_t(0x40)) ? c1 : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        c = ted.shiftRegisterCharacter.char_();
        c0 = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        c1 = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        bufp[3] = ((b & uint8_t(0x80)) ? c1 : c0);
        bufp[4] = ((b & uint8_t(0x40)) ? c1 : c0);
        break;
      case 3:
        bufp[1] = ((b & uint8_t(0x80)) ? c1 : c0);
        bufp[2] = ((b & uint8_t(0x40)) ? c1 : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? c1 : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 1;
        c = ted.shiftRegisterCharacter.char_();
        c0 = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        c1 = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        bufp[4] = ((b & uint8_t(0x80)) ? c1 : c0);
        break;
      default:
        bufp[1] = ((b & uint8_t(0x80)) ? c1 : c0);
        bufp[2] = ((b & uint8_t(0x40)) ? c1 : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? c1 : c0);
        bufp[4] = ((b & uint8_t(0x10)) ? c1 : c0);
        ted.shiftRegisterCharacter.bitmap_() =
            ted.shiftRegisterCharacter.bitmap_() << 4;
        break;
      }
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_BMM_multicolor(TED7360& ted,
                                                        int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 5;
    bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
    uint8_t c_[4];
    c_[0] = ted.colorRegisters[0];
    c_[3] = ted.colorRegisters[1];
    if (nextCharCnt == 0) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      ted.shiftRegisterCharacter.bitmap_() = b << 4;
      uint8_t c = ted.shiftRegisterCharacter.char_();
      c_[1] = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
      c_[2] = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
      bufp[1] = c_[(b >> 6) & 3];
      c_[0] = ted.tedRegisters[0x15];
      c_[3] = ted.tedRegisters[0x16];
      bufp[2] = c_[(b >> 6) & 3];
      bufp[4] = bufp[3] = c_[(b >> 4) & 3];
    }
    else {
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t c = ted.shiftRegisterCharacter.char_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      c_[1] = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
      c_[2] = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
      switch (nextCharCnt) {
      case 1:
        bufp[1] = c_[(b >> 6) & 3];
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        c = ted.shiftRegisterCharacter.char_();
        c_[0] = ted.tedRegisters[0x15];
        c_[1] = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        c_[2] = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        c_[3] = ted.tedRegisters[0x16];
        bufp[3] = bufp[2] = c_[(b >> 6) & 3];
        bufp[4] = c_[(b >> 4) & 3];
        break;
      case 2:
        bufp[1] = c_[(b >> 6) & 3];
        c_[0] = ted.tedRegisters[0x15];
        c_[3] = ted.tedRegisters[0x16];
        bufp[2] = c_[(b >> 6) & 3];
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        c = ted.shiftRegisterCharacter.char_();
        c_[1] = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        c_[2] = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        bufp[4] = bufp[3] = c_[(b >> 6) & 3];
        break;
      case 3:
        bufp[1] = c_[(b >> 6) & 3];
        c_[0] = ted.tedRegisters[0x15];
        c_[3] = ted.tedRegisters[0x16];
        bufp[3] = bufp[2] = c_[(b >> 4) & 3];
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        c = ted.shiftRegisterCharacter.char_();
        b = ted.shiftRegisterCharacter.bitmap_();
        c_[1] = ((a & uint8_t(0x07)) << 4) | ((c & uint8_t(0xF0)) >> 4);
        c_[2] = (a & uint8_t(0x70)) | (c & uint8_t(0x0F));
        bufp[4] = c_[(b >> 6) & 3];
        break;
      default:
        bufp[1] = c_[(b >> 6) & 3];
        c_[0] = ted.tedRegisters[0x15];
        c_[3] = ted.tedRegisters[0x16];
        if (!(nextCharCnt & 1)) {
          bufp[2] = c_[(b >> 6) & 3];
          bufp[4] = bufp[3] = c_[(b >> 4) & 3];
        }
        else {
          bufp[3] = bufp[2] = c_[(b >> 4) & 3];
          bufp[4] = c_[(b >> 2) & 3];
        }
        ted.shiftRegisterCharacter.bitmap_() =
            ted.shiftRegisterCharacter.bitmap_() << 4;
        break;
      }
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_char_std(TED7360& ted, int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 5;
    bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
    uint8_t c0 = ted.colorRegisters[0];
    if (nextCharCnt == 0) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      uint8_t f = ted.shiftRegisterCharacter.flags_();
      if (a & 0x80)
        b = b & ((f & 0xF0) | (f >> 4) | ted.flashState);
      ted.shiftRegisterCharacter.bitmap_() = b << 4;
      if ((ted.shiftRegisterCharacter.char_() & (f << 4)) & 0x80)
        b = b ^ 0xFF;
      b = b ^ (f & ted.flashState);
      bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
      c0 = ted.tedRegisters[0x15];
      bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
      bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
      bufp[4] = ((b & uint8_t(0x10)) ? a : c0);
    }
    else {
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      uint8_t f = ted.shiftRegisterCharacter.flags_();
      if ((ted.shiftRegisterCharacter.char_() & (f << 4)) & 0x80)
        b = b ^ 0xFF;
      b = b ^ (f & ted.flashState);
      switch (nextCharCnt) {
      case 1:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        f = ted.shiftRegisterCharacter.flags_();
        if (a & 0x80)
          b = b & ((f & 0xF0) | (f >> 4) | ted.flashState);
        ted.shiftRegisterCharacter.bitmap_() = b << 3;
        if ((ted.shiftRegisterCharacter.char_() & (f << 4)) & 0x80)
          b = b ^ 0xFF;
        b = b ^ (f & ted.flashState);
        c0 = ted.tedRegisters[0x15];
        bufp[2] = ((b & uint8_t(0x80)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x20)) ? a : c0);
        break;
      case 2:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[0x15];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        f = ted.shiftRegisterCharacter.flags_();
        if (a & 0x80)
          b = b & ((f & 0xF0) | (f >> 4) | ted.flashState);
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        if ((ted.shiftRegisterCharacter.char_() & (f << 4)) & 0x80)
          b = b ^ 0xFF;
        b = b ^ (f & ted.flashState);
        bufp[3] = ((b & uint8_t(0x80)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x40)) ? a : c0);
        break;
      case 3:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[0x15];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        f = ted.shiftRegisterCharacter.flags_();
        if (a & 0x80)
          b = b & ((f & 0xF0) | (f >> 4) | ted.flashState);
        ted.shiftRegisterCharacter.bitmap_() = b << 1;
        if ((ted.shiftRegisterCharacter.char_() & (f << 4)) & 0x80)
          b = b ^ 0xFF;
        b = b ^ (f & ted.flashState);
        bufp[4] = ((b & uint8_t(0x80)) ? a : c0);
        break;
      default:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[0x15];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x10)) ? a : c0);
        ted.shiftRegisterCharacter.bitmap_() =
            ted.shiftRegisterCharacter.bitmap_() << 4;
        break;
      }
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_char_ECM(TED7360& ted, int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 5;
    bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
    if (nextCharCnt == 0) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      ted.shiftRegisterCharacter.bitmap_() = b << 4;
      uint8_t c = ted.shiftRegisterCharacter.char_() >> 6;
      uint8_t c0 = ted.colorRegisters[c];
      c += uint8_t(0x15);
      bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
      c0 = ted.tedRegisters[c];
      bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
      bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
      bufp[4] = ((b & uint8_t(0x10)) ? a : c0);
    }
    else {
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      uint8_t c = ted.shiftRegisterCharacter.char_() >> 6;
      uint8_t c0 = ted.colorRegisters[c];
      c += uint8_t(0x15);
      switch (nextCharCnt) {
      case 1:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 3;
        c = ted.shiftRegisterCharacter.char_();
        c0 = ted.tedRegisters[(c >> 6) + uint8_t(0x15)];
        bufp[2] = ((b & uint8_t(0x80)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x20)) ? a : c0);
        break;
      case 2:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[c];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        c = ted.shiftRegisterCharacter.char_();
        c0 = ted.tedRegisters[(c >> 6) + uint8_t(0x15)];
        bufp[3] = ((b & uint8_t(0x80)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x40)) ? a : c0);
        break;
      case 3:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[c];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 1;
        c = ted.shiftRegisterCharacter.char_();
        c0 = ted.tedRegisters[(c >> 6) + uint8_t(0x15)];
        bufp[4] = ((b & uint8_t(0x80)) ? a : c0);
        break;
      default:
        bufp[1] = ((b & uint8_t(0x80)) ? a : c0);
        c0 = ted.tedRegisters[c];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c0);
        bufp[3] = ((b & uint8_t(0x20)) ? a : c0);
        bufp[4] = ((b & uint8_t(0x10)) ? a : c0);
        ted.shiftRegisterCharacter.bitmap_() =
            ted.shiftRegisterCharacter.bitmap_() << 4;
        break;
      }
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_char_MCM(TED7360& ted, int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 5;
    bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
    uint8_t c_[4];
    c_[0] = ted.colorRegisters[0];
    c_[1] = ted.colorRegisters[1];
    c_[2] = ted.colorRegisters[2];
    if (nextCharCnt == 0) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      ted.shiftRegisterCharacter.bitmap_() = b << 4;
      if (a & uint8_t(0x08)) {
        c_[3] = a & uint8_t(0x77);
        int     tmp = (b >> 6) & 3;
        bufp[1] = c_[tmp];
        c_[0] = ted.tedRegisters[0x15];
        c_[1] = ted.tedRegisters[0x16];
        c_[2] = ted.tedRegisters[0x17];
        bufp[2] = c_[tmp];
        bufp[4] = bufp[3] = c_[(b >> 4) & 3];
      }
      else {
        bufp[1] = ((b & uint8_t(0x80)) ? a : c_[0]);
        c_[0] = ted.tedRegisters[0x15];
        bufp[2] = ((b & uint8_t(0x40)) ? a : c_[0]);
        bufp[3] = ((b & uint8_t(0x20)) ? a : c_[0]);
        bufp[4] = ((b & uint8_t(0x10)) ? a : c_[0]);
      }
    }
    else {
      uint8_t a = ted.shiftRegisterCharacter.attr_();
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      c_[3] = a & uint8_t(0x77);
      switch (nextCharCnt) {
      case 1:
        if (a & uint8_t(0x08)) {
          int     tmp = (b >> 6) & 3;
          bufp[1] = c_[tmp];
        }
        else {
          bufp[1] = ((b & uint8_t(0x80)) ? a : c_[0]);
        }
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        if (a & uint8_t(0x08)) {
          ted.shiftRegisterCharacter.bitmap_() = b << 2;
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          c_[3] = a & uint8_t(0x77);
          bufp[3] = bufp[2] = c_[(b >> 6) & 3];
          bufp[4] = c_[(b >> 4) & 3];
        }
        else {
          ted.shiftRegisterCharacter.bitmap_() = b << 3;
          c_[0] = ted.tedRegisters[0x15];
          bufp[2] = ((b & uint8_t(0x80)) ? a : c_[0]);
          bufp[3] = ((b & uint8_t(0x40)) ? a : c_[0]);
          bufp[4] = ((b & uint8_t(0x20)) ? a : c_[0]);
        }
        break;
      case 2:
        if (a & uint8_t(0x08)) {
          int     tmp = (b >> 6) & 3;
          bufp[1] = c_[tmp];
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          bufp[2] = c_[tmp];
        }
        else {
          bufp[1] = ((b & uint8_t(0x80)) ? a : c_[0]);
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          bufp[2] = ((b & uint8_t(0x40)) ? a : c_[0]);
        }
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        ted.shiftRegisterCharacter.bitmap_() = b << 2;
        if (a & uint8_t(0x08)) {
          c_[3] = a & uint8_t(0x77);
          bufp[4] = bufp[3] = c_[(b >> 6) & 3];
        }
        else {
          bufp[3] = ((b & uint8_t(0x80)) ? a : c_[0]);
          bufp[4] = ((b & uint8_t(0x40)) ? a : c_[0]);
        }
        break;
      case 3:
        if (a & uint8_t(0x08)) {
          int     tmp = (b >> 6) & 3;
          bufp[1] = c_[tmp];
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          bufp[3] = bufp[2] = c_[(b >> 4) & 3];
        }
        else {
          bufp[1] = ((b & uint8_t(0x80)) ? a : c_[0]);
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          bufp[2] = ((b & uint8_t(0x40)) ? a : c_[0]);
          bufp[3] = ((b & uint8_t(0x20)) ? a : c_[0]);
        }
        ted.shiftRegisterCharacter = ted.currentCharacter;
        a = ted.shiftRegisterCharacter.attr_();
        b = ted.shiftRegisterCharacter.bitmap_();
        if (a & uint8_t(0x08)) {
          c_[3] = a & uint8_t(0x77);
          bufp[4] = c_[(b >> 6) & 3];
        }
        else {
          ted.shiftRegisterCharacter.bitmap_() = b << 1;
          bufp[4] = ((b & uint8_t(0x80)) ? a : c_[0]);
        }
        break;
      default:
        if (a & uint8_t(0x08)) {
          int     tmp = (b >> 6) & 3;
          bufp[1] = c_[tmp];
          c_[0] = ted.tedRegisters[0x15];
          c_[1] = ted.tedRegisters[0x16];
          c_[2] = ted.tedRegisters[0x17];
          if (!(nextCharCnt & 1)) {
            bufp[2] = c_[tmp];
            bufp[4] = bufp[3] = c_[(b >> 4) & 3];
          }
          else {
            bufp[3] = bufp[2] = c_[(b >> 4) & 3];
            bufp[4] = c_[(b >> 2) & 3];
          }
        }
        else {
          bufp[1] = ((b & uint8_t(0x80)) ? a : c_[0]);
          c_[0] = ted.tedRegisters[0x15];
          bufp[2] = ((b & uint8_t(0x40)) ? a : c_[0]);
          bufp[3] = ((b & uint8_t(0x20)) ? a : c_[0]);
          bufp[4] = ((b & uint8_t(0x10)) ? a : c_[0]);
        }
        ted.shiftRegisterCharacter.bitmap_() =
            ted.shiftRegisterCharacter.bitmap_() << 4;
        break;
      }
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_blank(TED7360& ted, int nextCharCnt)
  {
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    ted.video_buf_pos = ted.video_buf_pos + 2;
    bufp[0] = ted.videoOutputFlags;
    bufp[1] = uint8_t(0x00);
    if ((unsigned int) nextCharCnt < 4U) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      if (PLUS4EMU_UNLIKELY(b != 0x00)) {
        if (!(ted.videoMode & 0x01) ||
            ((ted.videoMode & 0x03) == 0x01 &&
             !(ted.shiftRegisterCharacter.attr_() & 0x08))) {
          ted.shiftRegisterCharacter.bitmap_() = b << (4 - nextCharCnt);
        }
        else {
          ted.shiftRegisterCharacter.bitmap_() = b << ((4 - nextCharCnt) & 6);
        }
      }
    }
    else {
      ted.shiftRegisterCharacter.bitmap_() =
          ted.shiftRegisterCharacter.bitmap_() << 4;
    }
  }

  PLUS4EMU_REGPARM2 void TED7360::render_border(TED7360& ted, int nextCharCnt)
  {
    if ((unsigned int) nextCharCnt < 4U) {
      ted.shiftRegisterCharacter = ted.currentCharacter;
      uint8_t b = ted.shiftRegisterCharacter.bitmap_();
      if (PLUS4EMU_UNLIKELY(b != 0x00)) {
        if (!(ted.videoMode & 0x01) ||
            ((ted.videoMode & 0x03) == 0x01 &&
             !(ted.shiftRegisterCharacter.attr_() & 0x08))) {
          ted.shiftRegisterCharacter.bitmap_() = b << (4 - nextCharCnt);
        }
        else {
          ted.shiftRegisterCharacter.bitmap_() = b << ((4 - nextCharCnt) & 6);
        }
      }
    }
    else {
      ted.shiftRegisterCharacter.bitmap_() =
          ted.shiftRegisterCharacter.bitmap_() << 4;
    }
    ted.prv_video_buf_pos = ted.video_buf_pos;
    uint8_t *bufp = &(ted.video_buf[ted.video_buf_pos]);
    uint8_t borderColor = ted.colorRegisters[4];
    if (PLUS4EMU_UNLIKELY(borderColor != ted.tedRegisters[0x19])) {
      ted.video_buf_pos = ted.video_buf_pos + 5;
      bufp[0] = ted.videoOutputFlags | uint8_t(0x02);
      bufp[1] = borderColor;
      borderColor = ted.tedRegisters[0x19];
      bufp[2] = borderColor;
      bufp[3] = borderColor;
      bufp[4] = borderColor;
    }
    else {
      ted.video_buf_pos = ted.video_buf_pos + 2;
      bufp[0] = ted.videoOutputFlags;
      bufp[1] = borderColor;
    }
  }

  void TED7360::updateVideoMode()
  {
    switch (videoMode) {
    case 0x00:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xFC)) << 8;
      characterMask = uint8_t(0x7F);
      break;
    case 0x01:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xFC)) << 8;
      characterMask = uint8_t(0x7F);
      break;
    case 0x02:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x03:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x04:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0x3F);
      break;
    case 0x05:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0x3F);
      break;
    case 0x06:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x07:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x08:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0xFF);
      break;
    case 0x09:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0xFF);
      break;
    case 0x0A:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x0B:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x0C:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0x3F);
      break;
    case 0x0D:
      charset_base_addr = int(tedRegisters[0x13] & uint8_t(0xF8)) << 8;
      characterMask = uint8_t(0x3F);
      break;
    case 0x0E:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    case 0x0F:
      charset_base_addr = 0;
      characterMask = uint8_t(0);
      break;
    }
  }

}       // namespace Plus4

