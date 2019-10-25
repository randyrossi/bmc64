
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

#ifndef PLUS4EMU_RIOT6532_HPP
#define PLUS4EMU_RIOT6532_HPP

#include "plus4emu.hpp"

namespace Plus4 {

  class RIOT6532 {
   private:
    uint8_t     memory_ram[128];
    uint8_t     portAInput;
    uint8_t     portADataDirection;
    uint8_t     portADataDirectionInv;
    uint8_t     portARegister;
    uint8_t     portAPinState;
    uint8_t     portAPrvPinState;
    uint8_t     portBInput;
    uint8_t     portBDataDirection;
    uint8_t     portBDataDirectionInv;
    uint8_t     portBRegister;
    uint8_t     portBPinState;
    uint8_t     timerState;
    uint16_t    timerDivideCnt;
    uint16_t    timerDivideReload;
    uint8_t     interruptFlags;
    uint8_t     interruptMask;
    bool        prvInterruptState;
    uint8_t     pa7EdgeDetectMode;      // 0x00: negative, 0x80: positive
    // --------
   protected:
    // called when the state of the IRQ line changes
    // 'newState' is true if the IRQ line is low, and false if it is high
    virtual void irqStateChangeCallback(bool newState);
   private:
    inline void updateInterruptRequestFlag()
    {
      bool    newInterruptState = bool(interruptFlags & interruptMask);
      if (newInterruptState != prvInterruptState) {
        prvInterruptState = newInterruptState;
        irqStateChangeCallback(newInterruptState);
      }
    }
    inline void updatePortA()
    {
      portAPinState = (portARegister | portADataDirectionInv) & portAInput;
      if ((portAPinState ^ portAPrvPinState) & 0x80) {
        if ((portAPinState & 0x80) == pa7EdgeDetectMode) {
          interruptFlags = interruptFlags | 0x40;
          updateInterruptRequestFlag();
        }
      }
      portAPrvPinState = portAPinState;
    }
    inline void updatePortB()
    {
      portBPinState = (portBRegister & portBDataDirection)
                      | (portBInput & portBDataDirectionInv);
    }
   public:
    RIOT6532();
    virtual ~RIOT6532();
    void reset();
    inline uint8_t readMemory(uint16_t addr) const
    {
      return memory_ram[addr & 0x007F];
    }
    inline void writeMemory(uint16_t addr, uint8_t value)
    {
      memory_ram[addr & 0x007F] = value;
    }
    uint8_t readRegister(uint16_t addr);
    void writeRegister(uint16_t addr, uint8_t value);
    uint8_t readRegisterDebug(uint16_t addr) const;
    inline uint8_t getPortA() const
    {
      return portAPinState;
    }
    inline uint8_t getPortAInput() const
    {
      return portAInput;
    }
    inline uint8_t getPortARegister() const
    {
      return portARegister;
    }
    inline void setPortA(uint8_t value)
    {
      if (value != portAInput) {
        portAInput = value;
        updatePortA();
      }
    }
    inline uint8_t getPortB() const
    {
      return portBPinState;
    }
    inline uint8_t getPortBInput() const
    {
      return portBInput;
    }
    inline uint8_t getPortBRegister() const
    {
      return portBRegister;
    }
    inline void setPortB(uint8_t value)
    {
      if (value != portBInput) {
        portBInput = value;
        updatePortB();
      }
    }
    inline bool isInterruptRequest() const
    {
      return prvInterruptState;
    }
    inline void runOneCycle()
    {
      if (!timerDivideCnt) {
        if (!timerState) {
          interruptFlags = interruptFlags | 0x80;
          updateInterruptRequestFlag();
          timerDivideReload = 1;
        }
        timerDivideCnt = timerDivideReload;
        timerState = (timerState - 1) & 0xFF;
      }
      timerDivideCnt--;
    }
  };

}       // namespace Plus4

#endif  // PLUS4EMU_RIOT6532_HPP

