
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
#include "riot6532.hpp"

namespace Plus4 {

  RIOT6532::RIOT6532()
    : portAInput(0xFF),
      portADataDirection(0x00),
      portADataDirectionInv(0xFF),
      portARegister(0x00),
      portAPinState(0xFF),
      portAPrvPinState(0xFF),
      portBInput(0xFF),
      portBDataDirection(0x00),
      portBDataDirectionInv(0xFF),
      portBRegister(0x00),
      portBPinState(0xFF),
      timerState(0x00),
      timerDivideCnt(0x0000),
      timerDivideReload(0x0001),
      interruptFlags(0x00),
      interruptMask(0x00),
      prvInterruptState(false),
      pa7EdgeDetectMode(0x00)
  {
    for (size_t i = 0; i < 128; i++)
      memory_ram[i] = 0x00;
  }

  RIOT6532::~RIOT6532()
  {
  }

  void RIOT6532::reset()
  {
    portADataDirection = 0x00;
    portADataDirectionInv = 0xFF;
    portARegister = 0x00;
    portAPinState = portAInput;
    portAPrvPinState = portAInput;
    portBDataDirection = 0x00;
    portBDataDirectionInv = 0xFF;
    portBRegister = 0x00;
    portBPinState = portBInput;
    interruptFlags = 0x00;
    interruptMask = 0x00;
    pa7EdgeDetectMode = 0x00;
    updateInterruptRequestFlag();
  }

  uint8_t RIOT6532::readRegister(uint16_t addr)
  {
    switch (addr & 0x001F) {
    case 0x00:                          // output register A
    case 0x08:
    case 0x10:
    case 0x18:
      return portAPinState;
    case 0x01:                          // port A data direction
    case 0x09:
    case 0x11:
    case 0x19:
      return portADataDirection;
    case 0x02:                          // output register B
    case 0x0A:
    case 0x12:
    case 0x1A:
      return ((portBRegister & portBDataDirection)
              | (portBInput & portBDataDirectionInv));
    case 0x03:                          // port B data direction
    case 0x0B:
    case 0x13:
    case 0x1B:
      return portBDataDirection;
    case 0x04:                          // read timer
    case 0x06:
    case 0x0C:
    case 0x0E:
    case 0x14:
    case 0x16:
    case 0x1C:
    case 0x1E:
      interruptFlags = interruptFlags & 0x40;
      interruptMask = (interruptMask & 0x40) | uint8_t((addr & 0x0008) << 4);
      updateInterruptRequestFlag();
      return timerState;
    case 0x05:                          // read interrupt flags
    case 0x07:
    case 0x0D:
    case 0x0F:
    case 0x15:
    case 0x17:
    case 0x1D:
    case 0x1F:
      {
        uint8_t retval = interruptFlags;
        interruptFlags = interruptFlags & 0x80;
        updateInterruptRequestFlag();
        return retval;
      }
    }
    return 0x00;        // not reached
  }

  void RIOT6532::writeRegister(uint16_t addr, uint8_t value)
  {
    switch (addr & 0x001F) {
    case 0x00:                          // output register A
    case 0x08:
    case 0x10:
    case 0x18:
      portARegister = value;
      updatePortA();
      break;
    case 0x01:                          // port A data direction
    case 0x09:
    case 0x11:
    case 0x19:
      portADataDirection = value;
      portADataDirectionInv = value ^ 0xFF;
      updatePortA();
      break;
    case 0x02:                          // output register B
    case 0x0A:
    case 0x12:
    case 0x1A:
      portBRegister = value;
      updatePortB();
      break;
    case 0x03:                          // port B data direction
    case 0x0B:
    case 0x13:
    case 0x1B:
      portBDataDirection = value;
      portBDataDirectionInv = value ^ 0xFF;
      updatePortB();
      break;
    case 0x14:                          // write timer
    case 0x15:
    case 0x16:
    case 0x17:
    case 0x1C:
    case 0x1D:
    case 0x1E:
    case 0x1F:
      timerState = (value - 1) & 0xFF;
      switch (addr & 0x0003) {
      case 0x00:
        timerDivideReload = 0x0001;
        break;
      case 0x01:
        timerDivideReload = 0x0008;
        break;
      case 0x02:
        timerDivideReload = 0x0040;
        break;
      case 0x03:
        timerDivideReload = 0x0400;
        break;
      }
      timerDivideCnt = timerDivideReload - 1;
      interruptFlags = interruptFlags & 0x40;
      interruptMask = (interruptMask & 0x40) | uint8_t((addr & 0x0008) << 4);
      updateInterruptRequestFlag();
      break;
    case 0x04:                          // write edge detect control
    case 0x05:
    case 0x06:
    case 0x07:
    case 0x0C:
    case 0x0D:
    case 0x0E:
    case 0x0F:
      pa7EdgeDetectMode = uint8_t((addr & 0x0001) << 7);
      interruptMask = (interruptMask & 0x80) | uint8_t((addr & 0x0002) << 5);
      updateInterruptRequestFlag();
      break;
    }
  }

  uint8_t RIOT6532::readRegisterDebug(uint16_t addr) const
  {
    switch (addr & 0x001F) {
    case 0x00:                          // output register A
    case 0x08:
    case 0x10:
    case 0x18:
      return portAPinState;
    case 0x01:                          // port A data direction
    case 0x09:
    case 0x11:
    case 0x19:
      return portADataDirection;
    case 0x02:                          // output register B
    case 0x0A:
    case 0x12:
    case 0x1A:
      return ((portBRegister & portBDataDirection)
              | (portBInput & portBDataDirectionInv));
    case 0x03:                          // port B data direction
    case 0x0B:
    case 0x13:
    case 0x1B:
      return portBDataDirection;
    case 0x04:                          // read timer
    case 0x06:
    case 0x0C:
    case 0x0E:
    case 0x14:
    case 0x16:
    case 0x1C:
    case 0x1E:
      return timerState;
    case 0x05:                          // read interrupt flags
    case 0x07:
    case 0x0D:
    case 0x0F:
    case 0x15:
    case 0x17:
    case 0x1D:
    case 0x1F:
      return interruptFlags;
    }
    return 0x00;        // not reached
  }

  void RIOT6532::irqStateChangeCallback(bool newState)
  {
    (void) newState;
  }

}       // namespace Plus4

