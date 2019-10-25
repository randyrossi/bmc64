
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "cia8520.hpp"

namespace Plus4 {

  void CIA8520::run(size_t nCycles)
  {
    while (nCycles--) {
      if (!(ciaRegisters[0x0E] & uint8_t(0x04)))
        timerAOutput = false;   // timer A pulse mode
      if (!(ciaRegisters[0x0F] & uint8_t(0x04)))
        timerBOutput = false;   // timer B pulse mode
      // update timer A
      if ((ciaRegisters[0x0E] & uint8_t(0x21)) == uint8_t(0x01))
        timerAState = (timerAState - 1) & 0xFFFF;
      // update timer B
      if ((ciaRegisters[0x0F] & uint8_t(0x61)) == uint8_t(0x01))
        timerBState = (timerBState - 1) & 0xFFFF;
      // check for timer A underflow
      if (!timerAState) {
        timerAState = timerALatch;
        timerAOutput = !timerAOutput;
        if (ciaRegisters[0x0E] & uint8_t(0x08))
          ciaRegisters[0x0E] &= uint8_t(0xFE);  // one shot mode
        // update timer B, depending on control register
        if ((ciaRegisters[0x0F] & uint8_t(0x41)) == uint8_t(0x41)) {
          if (cntState || !(ciaRegisters[0x0F] & uint8_t(0x20)))
            timerBState = (timerBState - 1) & 0xFFFF;
        }
        // set interrupt flag
        ciaRegisters[0x0D] |= uint8_t(0x01);
        // serial port output
        if (ciaRegisters[0x0E] & uint8_t(0x40)) {
          serialPortOutputClockState = !serialPortOutputClockState;
          if (shiftCnt < 8 && !serialPortOutputClockState) {
            serialPortState = !!(shiftRegister & uint8_t(0x80));
            shiftRegister = (shiftRegister << 1) & uint8_t(0xFF);
            shiftCnt++;
            if (shiftCnt >= 8) {
              ciaRegisters[0x0D] |= uint8_t(0x08);
              if (serialPortRegisterWriteFlag) {
                // continue with next byte if already available
                shiftCnt = 0;
                shiftRegister = ciaRegisters[0x0C];
                serialPortRegisterWriteFlag = false;
              }
            }
          }
        }
      }
      // check for timer B underflow
      if (!timerBState) {
        timerBState = timerBLatch;
        timerBOutput = !timerBOutput;
        if (ciaRegisters[0x0F] & uint8_t(0x08))
          ciaRegisters[0x0F] &= uint8_t(0xFE);  // one shot mode
        // set interrupt flag
        ciaRegisters[0x0D] |= uint8_t(0x02);
      }
      // check interrupts
      ciaRegisters[0x0D] &= uint8_t(0x1F);
      bool    newIRQState = !!(ciaRegisters[0x0D] & irqMask);
      if (newIRQState)
        ciaRegisters[0x0D] |= uint8_t(0x80);
      if (newIRQState != prvIRQState) {
        prvIRQState = newIRQState;
        interruptCallback(newIRQState);
      }
    }
  }

  uint8_t CIA8520::readRegister(uint16_t addr)
  {
    uint8_t n = uint8_t(addr & 0x000F);
    uint8_t value = ciaRegisters[n];
    switch (n) {
    case 0x04:                                  // timer A low
      value = uint8_t(timerAState & 0xFF);
      break;
    case 0x05:                                  // timer A high
      value = uint8_t((timerAState >> 8) & 0xFF);
      break;
    case 0x06:                                  // timer B low
      value = uint8_t(timerBState & 0xFF);
      break;
    case 0x07:                                  // timer B high
      value = uint8_t((timerBState >> 8) & 0xFF);
      break;
    case 0x08:                                  // TOD bits 0 to 7
      if (todReadLatchFlag)
        value = uint8_t(todReadLatch & 0xFFU);
      else
        value = uint8_t(todCounter & 0xFFU);
      todReadLatchFlag = false;
      break;
    case 0x09:                                  // TOD bits 8 to 15
      if (todReadLatchFlag)
        value = uint8_t((todReadLatch >> 8) & 0xFFU);
      else
        value = uint8_t((todCounter >> 8) & 0xFFU);
      break;
    case 0x0A:                                  // TOD bits 16 to 23
      todReadLatchFlag = true;
      todReadLatch = todCounter;
      value = uint8_t((todCounter >> 16) & 0xFFU);
      break;
    case 0x0B:                                  // unused
      value = uint8_t(0x00);
      break;
    case 0x0D:                                  // interrupt control register
      value &= uint8_t(0x9F);
      ciaRegisters[n] = uint8_t(0x00);  // clear interrupt flags on read
      if (prvIRQState) {
        prvIRQState = false;
        interruptCallback(false);
      }
      break;
    case 0x0E:                                  // control register A
      value &= uint8_t(0xEF);
      break;
    case 0x0F:                                  // control register B
      value &= uint8_t(0xEF);
      break;
    }
    return value;
  }

  void CIA8520::writeRegister(uint16_t addr, uint8_t value)
  {
    uint8_t n = uint8_t(addr & 0x000F);
    value &= uint8_t(0xFF);
    switch (n) {
    case 0x04:                                  // timer A low
      timerALatch = (timerALatch & uint16_t(0xFF00)) | uint16_t(value);
      break;
    case 0x05:                                  // timer A high
      timerALatch = (timerALatch & uint16_t(0x00FF)) | (uint16_t(value) << 8);
      if (!(ciaRegisters[0x0E] & uint8_t(0x01)))
        timerAState = timerALatch;
      break;
    case 0x06:                                  // timer B low
      timerBLatch = (timerBLatch & uint16_t(0xFF00)) | uint16_t(value);
      break;
    case 0x07:                                  // timer B high
      timerBLatch = (timerBLatch & uint16_t(0x00FF)) | (uint16_t(value) << 8);
      if (!(ciaRegisters[0x0F] & uint8_t(0x01)))
        timerBState = timerBLatch;
      break;
    case 0x08:                                  // TOD/alarm bits 0 to 7
      if (!(ciaRegisters[0x0F] & uint8_t(0x80))) {
        todCounter = (todCounter & 0x00FFFF00U) | uint32_t(value);
        todWriteLatchFlag = false;
      }
      else {
        todAlarm = (todAlarm & 0x00FFFF00U) | uint32_t(value);
      }
      break;
    case 0x09:                                  // TOD/alarm bits 8 to 15
      if (!(ciaRegisters[0x0F] & uint8_t(0x80)))
        todCounter = (todCounter & 0x00FF00FFU) | (uint32_t(value) << 8);
      else
        todAlarm = (todAlarm & 0x00FF00FFU) | (uint32_t(value) << 8);
      break;
    case 0x0A:                                  // TOD/alarm bits 16 to 23
      if (!(ciaRegisters[0x0F] & uint8_t(0x80))) {
        todWriteLatchFlag = true;
        todCounter = (todCounter & 0x0000FFFFU) | (uint32_t(value) << 16);
      }
      else
        todAlarm = (todAlarm & 0x0000FFFFU) | (uint32_t(value) << 16);
      break;
    case 0x0B:                                  // unused
      break;
    case 0x0C:                                  // serial port
      if (ciaRegisters[0x0E] & uint8_t(0x40)) {
        if (shiftCnt < 8) {
          // still sending a byte
          serialPortRegisterWriteFlag = true;
        }
        else {
          // start new transfer
          shiftRegister = value;
          shiftCnt = 0;
          serialPortOutputClockState = true;
          serialPortRegisterWriteFlag = false;
        }
      }
      break;
    case 0x0D:                                  // interrupt control register
      if (value & uint8_t(0x80)) {      // set bits
        irqMask = (irqMask | value) & uint8_t(0x1F);
        return;
      }
      else {                            // clear bits
        irqMask = (irqMask & (value ^ uint8_t(0xFF))) & uint8_t(0x1F);
        return;
      }
      break;
    case 0x0E:                                  // control register A
      if (value & uint8_t(0x10))
        timerAState = timerALatch;      // reload timer A from latch value
      if ((value ^ ciaRegisters[n]) & uint8_t(0x40)) {
        if (value & uint8_t(0x40))
          shiftCnt = 8;
        else
          shiftCnt = 0;
      }
      value &= uint8_t(0xEF);
      break;
    case 0x0F:                                  // control register B
      if (value & uint8_t(0x10))
        timerBState = timerBLatch;      // reload timer B from latch value
      value &= uint8_t(0xEF);
      break;
    }
    ciaRegisters[n] = value;
  }

  uint8_t CIA8520::readRegisterDebug(uint16_t addr) const
  {
    uint8_t n = uint8_t(addr & 0x000F);
    uint8_t value = ciaRegisters[n];
    switch (n) {
    case 0x04:                                  // timer A low
      value = uint8_t(timerAState & 0xFF);
      break;
    case 0x05:                                  // timer A high
      value = uint8_t((timerAState >> 8) & 0xFF);
      break;
    case 0x06:                                  // timer B low
      value = uint8_t(timerBState & 0xFF);
      break;
    case 0x07:                                  // timer B high
      value = uint8_t((timerBState >> 8) & 0xFF);
      break;
    case 0x08:                                  // TOD bits 0 to 7
      if (todReadLatchFlag)
        value = uint8_t(todReadLatch & 0xFFU);
      else
        value = uint8_t(todCounter & 0xFFU);
      break;
    case 0x09:                                  // TOD bits 8 to 15
      if (todReadLatchFlag)
        value = uint8_t((todReadLatch >> 8) & 0xFFU);
      else
        value = uint8_t((todCounter >> 8) & 0xFFU);
      break;
    case 0x0A:                                  // TOD bits 16 to 23
      value = uint8_t((todCounter >> 16) & 0xFFU);
      break;
    case 0x0B:                                  // unused
      value = uint8_t(0x00);
      break;
    case 0x0D:                                  // interrupt control register
      value &= uint8_t(0x9F);
      break;
    case 0x0E:                                  // control register A
      value &= uint8_t(0xEF);
      break;
    case 0x0F:                                  // control register B
      value &= uint8_t(0xEF);
      break;
    }
    return value;
  }

  void CIA8520::reset()
  {
    for (int i = 0; i < 16; i++)
      ciaRegisters[i] = 0x00;
    timerAState = 0x0000;
    timerALatch = 0xFFFF;
    timerBState = 0x0000;
    timerBLatch = 0xFFFF;
    timerAOutput = false;
    timerBOutput = false;
    if (prvIRQState) {
      prvIRQState = false;
      interruptCallback(false);
    }
    todReadLatchFlag = false;
    todWriteLatchFlag = false;
    serialPortState = false;
    serialPortOutputClockState = false;
    serialPortRegisterWriteFlag = false;
    irqMask = 0x00;
    shiftRegister = 0x00;
    shiftCnt = 0;
    todClkDiv = 6;
    todCounter = 0x00000000U;
    todReadLatch = 0x00000000U;
    todAlarm = 0x00000000U;
  }

  CIA8520::CIA8520()
    : cntState(false),
      prvIRQState(false),
      flagState(false)
  {
    this->reset();
  }

  CIA8520::~CIA8520()
  {
  }

  void CIA8520::interruptCallback(bool irqState)
  {
    (void) irqState;
  }

}       // namespace Plus4

