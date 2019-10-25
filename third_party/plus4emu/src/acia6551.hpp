
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

#ifndef PLUS4EMU_ACIA6551_HPP
#define PLUS4EMU_ACIA6551_HPP

#include "plus4emu.hpp"

namespace Plus4 {

  class ACIA6551 {
   private:
    uint8_t     transmitDataRegister;
    uint8_t     receiveDataRegister;
    uint8_t     statusRegister;
    uint8_t     commandRegister;
    uint8_t     controlRegister;
    uint32_t    cyclesRemaining;
    uint32_t    cyclesPerBit;
    //   0..1: start bit
    //  2..17: data bits (2..7 may be skipped depending on the number of bits)
    // 18..19: parity bit
    // 20..23: stop bit(s)
    // 24..25: restart with delay
    int         transmitState;
    int         receiveState;
    bool        transmitContinuousMark;
    bool        halfBitFlag;
    // --------
    void runHalfBit();
   public:
    ACIA6551();
    virtual ~ACIA6551();
    inline bool isEnabled() const
    {
      return bool(commandRegister & 0x01);
    }
    inline void runOneCycle()
    {
      if (this->isEnabled()) {
        cyclesRemaining--;
        if (!cyclesRemaining)
          this->runHalfBit();
      }
    }
    inline bool getInterruptFlag() const
    {
      return bool(statusRegister & 0x80);
    }
    uint8_t readRegister(uint16_t addr);
    uint8_t readRegisterDebug(uint16_t addr) const;
    void writeRegister(uint16_t addr, uint8_t value);
    void reset();
    static inline size_t getSnapshotSize()
    {
      return 11;
    }
    void saveSnapshot(uint8_t *buf);
    void loadSnapshot(const uint8_t *buf);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_ACIA6551_HPP

