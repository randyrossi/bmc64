
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

#ifndef PLUS4EMU_CIA8520_HPP
#define PLUS4EMU_CIA8520_HPP

#include "plus4emu.hpp"

namespace Plus4 {

  class CIA8520 {
   private:
    uint8_t     ciaRegisters[16];
    uint16_t    timerAState;
    uint16_t    timerALatch;
    uint16_t    timerBState;
    uint16_t    timerBLatch;
    bool        timerAOutput;
    bool        timerBOutput;
    bool        cntState;
    bool        prvIRQState;
    bool        flagState;
    bool        todReadLatchFlag;
    bool        todWriteLatchFlag;
    bool        serialPortState;
    bool        serialPortOutputClockState;
    bool        serialPortRegisterWriteFlag;
    uint8_t     irqMask;
    uint8_t     shiftRegister;
    uint8_t     shiftCnt;
    uint8_t     todClkDiv;
    uint32_t    todCounter;
    uint32_t    todReadLatch;
    uint32_t    todAlarm;
   public:
    CIA8520();
    virtual ~CIA8520();
    uint8_t readRegister(uint16_t addr);
    void    writeRegister(uint16_t addr, uint8_t value);
    uint8_t readRegisterDebug(uint16_t addr) const;
    // set the FLAG input to 'newState' (true = high, false = low)
    inline void setFlagState(bool newState)
    {
      // interrupt on negative edge
      if (flagState && !newState)
        ciaRegisters[0x0D] |= uint8_t(0x10);
      flagState = newState;
    }
    // set the CNT input to 'newState' (true = high, false = low)
    inline void setCntState(bool newState)
    {
      if (newState && !cntState) {
        // may need to update timers on positive edge
        if ((ciaRegisters[0x0E] & uint8_t(0x21)) == uint8_t(0x21))
          timerAState = (timerAState - 1) & 0xFFFF;
        if ((ciaRegisters[0x0F] & uint8_t(0x61)) == uint8_t(0x21))
          timerBState = (timerBState - 1) & 0xFFFF;
        // serial port input
        if (!(ciaRegisters[0x0E] & uint8_t(0x40))) {
          shiftRegister = (shiftRegister << 1) & uint8_t(0xFF);
          if (serialPortState)
            shiftRegister |= uint8_t(0x01);
          shiftCnt++;
          if (shiftCnt >= 8) {
            shiftCnt = 0;
            ciaRegisters[0x0C] = shiftRegister;
            ciaRegisters[0x0D] |= uint8_t(0x08);
          }
        }
      }
      cntState = newState;
    }
    // returns the current state of CNT (true = high, false = low)
    inline bool getCntState() const
    {
      if (ciaRegisters[0x0E] & uint8_t(0x40))
        return serialPortOutputClockState;
      return cntState;
    }
    inline void setSerialPortState(bool newState)
    {
      serialPortState = newState;
    }
    inline bool getSerialPortState() const
    {
      return serialPortState;
    }
    inline uint8_t getPortA() const
    {
      return (ciaRegisters[0x00] | (ciaRegisters[0x02] ^ uint8_t(0xFF)));
    }
    inline void setPortA(uint8_t value)
    {
      ciaRegisters[0x00] &= ciaRegisters[0x02];
      ciaRegisters[0x00] |= (value & (ciaRegisters[0x02] ^ uint8_t(0xFF)));
    }
    inline uint8_t getPortB() const
    {
      uint8_t n = ciaRegisters[0x01] | (ciaRegisters[0x03] ^ uint8_t(0xFF));
      if (ciaRegisters[0x0E] & uint8_t(0x02)) {
        // timer A output on port B bit 6
        n &= uint8_t(0xBF);
        n |= uint8_t(timerAOutput ? 0x40 : 0x00);
      }
      if (ciaRegisters[0x0F] & uint8_t(0x02)) {
        // timer B output on port B bit 7
        n &= uint8_t(0x7F);
        n |= uint8_t(timerBOutput ? 0x80 : 0x00);
      }
      return n;
    }
    inline void setPortB(uint8_t value)
    {
      uint8_t outputMask = ciaRegisters[0x03];
      // timer A output on port B bit 6
      outputMask |= ((ciaRegisters[0x0E] & uint8_t(0x02)) << 5);
      // timer B output on port B bit 7
      outputMask |= ((ciaRegisters[0x0F] & uint8_t(0x02)) << 6);
      ciaRegisters[0x01] &= outputMask;
      ciaRegisters[0x01] |= (value & (outputMask ^ uint8_t(0xFF)));
    }
    // update time of day counter
    inline void todClock()
    {
      if (!todWriteLatchFlag) {
        todClkDiv--;
        if (!todClkDiv) {
          todClkDiv = 5;        // 50 Hz input, 10 Hz counter
          if (!(ciaRegisters[0x0E] & uint8_t(0x80)))
            todClkDiv++;        // 60 Hz input, 10 Hz counter
          todCounter = (todCounter + 1U) & 0x00FFFFFFU;
          if (todCounter == todAlarm)
            ciaRegisters[0x0D] |= uint8_t(0x04);
        }
      }
    }
    void    run(size_t nCycles = 1);
    void    reset();
   protected:
    // called when the IRQ output changes; 'irqState' is true when IRQ is low
    virtual void interruptCallback(bool irqState);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_CIA8520_HPP

