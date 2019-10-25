
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2016 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "cpu.hpp"
#include "ted.hpp"

namespace Plus4 {

  PLUS4EMU_REGPARM3 void TED7360::write_register_0000(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.ioRegister_0000 = value & uint8_t(0xDF);
    ted.writeMemory(0x0001, ted.ioRegister_0001);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_0001(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.ioRegister_0001 = value;
    uint8_t tmp = value | (ted.ioRegister_0000 ^ uint8_t(0xFF));
    uint8_t tmp2 = tmp ^ uint8_t(0xFF);
    tmp |= uint8_t(((tmp2 & 0x80) >> 7) | ((tmp2 & 0x40) >> 5));
    ted.ioPortWrite(tmp);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FD1x(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    if (ted.ramSegments < 16 || !(addr & 0x000F))
      ted.user_port_state = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FD3x(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.keyboard_row_select_mask = int(value) | 0xFF00;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FFxx(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    uint8_t   n = uint8_t(addr) & uint8_t(0xFF);
    ted.tedRegisters[n] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF00(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.timer1_run = false;
    ted.timer1_state = (ted.timer1_state & 0xFF00) | int(value);
    ted.timer1_reload_value = (ted.timer1_reload_value & 0xFF00) | int(value);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF01(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.timer1_run = true;
    int   tmp = int(value) << 8;
    ted.timer1_state = (ted.timer1_state & 0x00FF) | tmp;
    ted.timer1_reload_value = (ted.timer1_reload_value & 0x00FF) | tmp;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF02(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.timer2_run = false;
    ted.delayedEvents0.stopTimer2Start();
    ted.timer2_state = (ted.timer2_state & 0xFF00) | int(value);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF03(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    if (!ted.timer2_run) {
      if (!(ted.videoColumn & 0x01))
        ted.timer2_run = true;
      else
        ted.delayedEvents0.timer2Start();
    }
    ted.timer2_state = (ted.timer2_state & 0x00FF) | (int(value) << 8);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF04(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.timer3_run = false;
    ted.timer3_state = (ted.timer3_state & 0xFF00) | int(value);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF05(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.timer3_run = true;
    ted.timer3_state = (ted.timer3_state & 0x00FF) | (int(value) << 8);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF06(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    uint8_t   bitsChanged = value ^ ted.tedRegisters[0x06];
    ted.tedRegisters[0x06] = value;
    ted.videoMode = uint8_t((ted.videoMode & 0x09) | ((value >> 4) & 0x06));
    if (bitsChanged & uint8_t(0x07)) {
      // delay vertical scroll changes by one cycle
      ted.delayedEvents0.setVerticalScroll();
    }
    if (bitsChanged & uint8_t(0x18)) {
      // display enabled or number of rows has changed
      switch (ted.savedVideoLine) {
      case 0:
        if (value & uint8_t(0x10)) {
          // turned on display enable during line 0:
          // initialize internal registers
          ted.delayedEvents0.initializeDisplay();
          ted.delayedEvents0.setVerticalScroll();
        }
        break;
      case 4:
        if ((value & uint8_t(0x18)) == uint8_t(0x18))
          ted.displayWindow = true;
        break;
      case 8:
        if ((value & uint8_t(0x18)) == uint8_t(0x10))
          ted.displayWindow = true;
        break;
      case 200:
        if (!(value & uint8_t(0x08)))
          ted.displayWindow = false;
        break;
      case 204:
        if (value & uint8_t(0x08))
          ted.displayWindow = false;
        break;
      }
    }
    if (bitsChanged & uint8_t(0x60)) {
      ted.updateVideoMode();
      // delay video mode changes by one cycle
      if (!(ted.videoColumn & 0x01))
        ted.delayedEvents1.selectRenderer();
      else
        ted.delayedEvents0.selectRenderer();
    }
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF07(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    uint8_t   bitsChanged = value ^ ted.tedRegisters[0x07];
    ted.tedRegisters[0x07] = value;
    ted.ted_disabled = bool(value & uint8_t(0x20));
    ted.videoMode = uint8_t((ted.videoMode & 0x06) | ((value >> 4) & 0x09));
    if (bitsChanged & uint8_t(0x07)) {
      // delay horizontal scroll changes by one character
      if (!(ted.videoColumn & 0x01))
        ted.delayedEvents0.setHorizontalScroll();
      else
        ted.delayedEvents1.setHorizontalScroll();
    }
    if (PLUS4EMU_UNLIKELY(bitsChanged & uint8_t(0x60))) {
      if (bitsChanged & uint8_t(0x20)) {
        if (ted.ted_disabled) {
          if (!(ted.videoColumn & uint8_t(0x01))) {
            ted.delayedEvents0.dramRefreshOn();
            ted.videoOutputFlags |= uint8_t(0x20);
            ted.current_render_func = &TED7360::render_blank;
          }
        }
      }
      if (bitsChanged & uint8_t(0x40)) {
        if (value & uint8_t(0x40)) {
          ted.videoOutputFlags |= uint8_t(0x01);
          ted.singleClockCycleLength = 16;
          ted.ntscModeChangeCallback(true);
        }
        else {
          ted.videoOutputFlags &= uint8_t(0xFC);
          ted.singleClockCycleLength = 20;
          ted.ntscModeChangeCallback(false);
        }
        if (ted.savedVideoLine >= 226 && ted.savedVideoLine <= 269) {
          ted.checkVerticalEvents();
        }
      }
    }
    if (bitsChanged & uint8_t(0x90)) {
      ted.updateVideoMode();
      // delay video mode changes by one cycle
      if (!(ted.videoColumn & 0x01))
        ted.delayedEvents1.selectRenderer();
      else
        ted.delayedEvents0.selectRenderer();
    }
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF08(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    int     mask = ted.keyboard_row_select_mask & ((int(value) << 8) | 0xFF);
    uint8_t key_state = uint8_t(0xFF);
    for (int i = 0; i < 11; i++, mask >>= 1) {
      if (!(mask & 1))
        key_state &= ted.keyboard_matrix[i];
    }
    ted.tedRegisters[0x08] = key_state;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF09(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    // bit 2 (light pen interrupt) is always set
    ted.tedRegisters[0x09] = (ted.tedRegisters[0x09] & (value ^ uint8_t(0xFF)))
                             | uint8_t(0x04);
    ted.updateInterruptFlag();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0A(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0A] = value;
    ted.updateInterruptFlag();
    ted.videoInterruptLine = (ted.videoInterruptLine & 0x00FF)
                             | (int(value & uint8_t(0x01)) << 8);
    ted.checkVideoInterrupt();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0B(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0B] = value;
    ted.videoInterruptLine = (ted.videoInterruptLine & 0x0100)
                             | int(value & uint8_t(0xFF));
    ted.checkVideoInterrupt();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0C(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0C] = value;
    ted.cursor_position &= 0x00FF;
    ted.cursor_position |= (int(value & uint8_t(0x03)) << 8);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF0D(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x0D] = value;
    ted.cursor_position &= 0x0300;
    ted.cursor_position |= int(value);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF12(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x12] = value;
    ted.tedBitmapReadMap = (ted.tedBitmapReadMap & 0x7F78U)
                           | ((unsigned int) (value & uint8_t(0x04)) << 5);
    ted.bitmap_base_addr = int(value & uint8_t(0x38)) << 10;
    int     tmp = int(ted.tedRegisters[0x0E]) | (int(value & 0x03) << 8);
    ted.soundChannel1Reload = uint16_t((((tmp + 1) ^ 0x03FF) & 0x03FF) + 1);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF13(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    if ((value ^ ted.tedRegisters[0x13]) & 0x02)
      ted.delayedEvents0.setForceSingleClockFlag();
    ted.tedRegisters[0x13] = value;
    ted.updateVideoMode();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF14(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x14] = value;
    ted.dmaBaseAddr = (ted.dmaBaseAddr & 0x0400) | (int(value & 0xF8) << 8);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF15_to_FF19(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    uint8_t   n = uint8_t(addr) & uint8_t(0xFF);
    ted.tedRegisters[n] = value | uint8_t(0x80);
    n = n - 0x15;
    ted.colorRegisters[n] = uint8_t(0xFF);
    if (!(ted.videoColumn & 0x01))
      ted.delayedEvents1.setColorRegister(n);
    else
      ted.delayedEvents0.setColorRegister(n);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1A(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x1A] = value;
    ted.characterPositionReload =
        (int(value & 0x03) << 8) | int(ted.tedRegisters[0x1B]);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1B(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x1B] = value;
    ted.characterPositionReload =
        (int(ted.tedRegisters[0x1A] & 0x03) << 8) | int(value);
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1C(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x1C] = value;
    ted.videoLine = int(ted.tedRegisters[0x1D]) | (int(value & 0x01) << 8);
    ted.checkDMAPositionReset();
    ted.checkVideoInterrupt();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1D(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.tedRegisters[0x1D] = value;
    ted.videoLine = int(value) | (int(ted.tedRegisters[0x1C] & 0x01) << 8);
    ted.checkDMAPositionReset();
    ted.checkVideoInterrupt();
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1E(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    // NOTE: values written to this register are inverted
    uint8_t   newVal = ((value ^ uint8_t(0xFF)) & uint8_t(0xFC)) >> 1;
    ted.videoColumn = (ted.videoColumn & uint8_t(0x01)) | newVal;
    ted.tedRegisters[0x1E] = ted.videoColumn;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF1F(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    if ((ted.tedRegisters[0x1F] ^ value) & 0x78) {
      // flash state is inverted when the counter changes from 15
      // (all bits 1) to any other value
      // FIXME: writing anything other than 15 to the counter may "randomly"
      // invert the flash state sometimes
      if ((ted.tedRegisters[0x1F] & 0x78) == 0x78)
        ted.flashState = uint8_t(ted.flashState == 0x00 ? 0xFF : 0x00);
    }
    ted.tedRegisters[0x1F] = value;
    ted.characterLine = int(value & uint8_t(0x07));
  }

}       // namespace Plus4

