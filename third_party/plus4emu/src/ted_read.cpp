
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
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

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_0000(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    return (ted.ioRegister_0000 & uint8_t(0xDF));
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_0001(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t tmp = ted.ioPortRead();
    uint8_t mask_ = ted.ioRegister_0000;
    uint8_t nmask_ = mask_ ^ uint8_t(0xFF);
    uint8_t tmp2 = ted.ioRegister_0001 | nmask_;
    tmp2 = ((tmp2 & uint8_t(0x01)) << 7) | ((tmp2 & uint8_t(0x02)) << 5);
    tmp &= ((tmp2 ^ uint8_t(0xFF)) & nmask_);
    tmp |= (ted.ioRegister_0001 & mask_);
    return tmp;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FD0x(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    // RS232 registers (FIXME: unimplemented)
    ted.dataBusState = uint8_t(0x00);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FD1x(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    if (ted.ramSegments < 16 || !(addr & 0x000F)) {
      ted.dataBusState = ted.user_port_state & (ted.tape_button_state ?
                                                uint8_t(0xFB) : uint8_t(0xFF));
    }
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FD3x(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.keyboard_row_select_mask & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FFxx(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[uint8_t(addr) & uint8_t(0xFF)];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF00(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer1_state & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF01(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer1_state >> 8);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF02(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer2_state & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF03(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer2_state >> 8);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF04(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer3_state & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF05(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t(ted.timer3_state >> 8);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF06(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x06];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF09(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    // bit 2 (light pen interrupt) is always set
    uint8_t irq_state =
        (ted.tedRegisters[0x09] & uint8_t(0x7F)) | uint8_t(0x25);
    if (((irq_state & ted.tedRegisters[0x0A]) & uint8_t(0x5E)) != uint8_t(0))
      irq_state |= uint8_t(0x80);
    ted.dataBusState = irq_state;
    return irq_state;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF0A(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x0A] | uint8_t(0xA0);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF0C(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x0C] | uint8_t(0xFC);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF10(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = (ted.tedRegisters[0x10] | uint8_t(0x7C)) & uint8_t(0x7F);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF12(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x12] | uint8_t(0xC0);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF13(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = (ted.tedRegisters[0x13] & uint8_t(0xFE))
                       | ((uint8_t(ted.cpuMemoryReadMap) & uint8_t(0xFF)) >> 7);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF14(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x14] | uint8_t(0x07);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF1A(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x1A];
    // if the counter has just been changed in this cycle,
    // return the bitwise AND of the previous and the current value
    ted.dataBusState &= uint8_t(ted.characterPositionReload >> 8);
    ted.dataBusState |= uint8_t(0xFC);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF1B(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x1B];
    // if the counter has just been changed in this cycle,
    // return the bitwise AND of the previous and the current value
    ted.dataBusState &= uint8_t(ted.characterPositionReload & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF1C(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x1C] | uint8_t(0xFE);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF1E(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = uint8_t((ted.tedRegisters[0x1E] << 1) & 0xFF);
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF1F(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = ted.tedRegisters[0x1F] | uint8_t(0x80);
    // if the counter has just been incremented in this cycle,
    // return the bitwise AND of the previous and the current value
    ted.dataBusState &= uint8_t(ted.characterLine | 0xF8);
    return ted.dataBusState;
  }

}       // namespace Plus4

