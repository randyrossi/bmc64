
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
#include "via6522.hpp"

namespace Plus4 {

  VIA6522::VIA6522()
  {
    portAInput = 0xFF;
    portAOutput = 0xFF;
    portBInput = 0xFF;
    portBOutput = 0xFF;
    timer1Counter = 0x0000;
    timer1Latch = 0x0000;
    timer2Counter = 0x0000;
    timer2LatchL = 0x00;
    ca1Input = true;
    ca2Input = true;
    cb1Input = true;
    cb2Input = true;
#if 0
    // FIXME: shift register is unimplemented
    shiftRegister = 0x00;
#endif
    irqState = false;
    this->reset();
  }

  VIA6522::~VIA6522()
  {
  }

  void VIA6522::reset()
  {
    for (int i = 0; i < 16; i++)
      viaRegisters[i] = 0x00;
    portADataDirection = 0x00;
    portARegister = 0x00;
    portALatch = 0x00;
    portBDataDirection = 0x00;
    portBRegister = 0x00;
    portBLatch = 0x00;
    portBTimerOutputMask = 0x00;
    portBTimerOutput = 0x80;
    timer1SingleShotMode = true;
    timer1SingleShotModeDone = true;
    timer2PulseCountingMode = false;
    timer2SingleShotModeDone = true;
    ca1Output = true;
    ca1IsOutput = false;
    ca1PositiveEdge = false;
    ca2Output = true;
    ca2IsOutput = false;
    cb1Output = true;
    cb1IsOutput = false;
    cb1PositiveEdge = false;
    cb2Output = true;
    cb2IsOutput = false;
#if 0
    // FIXME: shift register is unimplemented
    shiftCounter = 0;
#endif
    updatePortA();
    updatePortB();
    if (irqState) {
      irqState = false;
      irqStateChangeCallback(false);
    }
  }

  uint8_t VIA6522::readRegister(uint16_t addr)
  {
    addr = addr & 0x000F;
    uint8_t value = viaRegisters[addr];
    switch (addr) {
    case 0x00:                                  // port B
      if (!(viaRegisters[0x0B] & 0x02) || !(viaRegisters[0x0D] & 0x10))
        value = getPortB();
      else
        value = portBLatch;
      value = (value & (portBDataDirection ^ 0xFF))
              | (portBRegister & portBDataDirection);
      if ((viaRegisters[0x0C] & 0xA0) == 0x00) {
        viaRegisters[0x0D] = viaRegisters[0x0D] & 0xE7;
        updateInterruptFlags();
      }
      break;
    case 0x01:                                  // port A (controls handshake)
      if (!(viaRegisters[0x0B] & 0x01) || !(viaRegisters[0x0D] & 0x02))
        value = getPortA();
      else
        value = portALatch;
      if ((viaRegisters[0x0C] & 0x0A) == 0x00) {
        viaRegisters[0x0D] = viaRegisters[0x0D] & 0xFC;
        updateInterruptFlags();
      }
      break;
    case 0x02:                                  // port B data direction
      break;
    case 0x03:                                  // port A data direction
      break;
    case 0x04:                                  // timer 1 low byte
      value = uint8_t(timer1Counter & 0x00FF);
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xBF;
      updateInterruptFlags();
      break;
    case 0x05:                                  // timer 1 high byte
      value = uint8_t((timer1Counter & 0xFF00) >> 8);
      break;
    case 0x06:                                  // timer 1 latch low byte
      value = uint8_t(timer1Latch & 0x00FF);
      break;
    case 0x07:                                  // timer 1 latch high byte
      value = uint8_t(timer1Latch >> 8);
      break;
    case 0x08:                                  // timer 2 low byte
      value = uint8_t(timer2Counter & 0x00FF);
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xDF;
      updateInterruptFlags();
      break;
    case 0x09:                                  // timer 2 high byte
      value = uint8_t(timer2Counter >> 8);
      break;
    case 0x0A:                                  // shift register
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xFB;
      updateInterruptFlags();
      break;
    case 0x0B:                                  // auxiliary control register
      break;
    case 0x0C:                                  // peripheral control register
      break;
    case 0x0D:                                  // interrupt flag register
      break;
    case 0x0E:                                  // interrupt enable register
      break;
    case 0x0F:                                  // port A (no handshake)
      if (!(viaRegisters[0x0B] & 0x01) || !(viaRegisters[0x0D] & 0x02))
        value = getPortA();
      else
        value = portALatch;
      break;
    }
    return value;
  }

  void VIA6522::writeRegister(uint16_t addr, uint8_t value)
  {
    addr = addr & 0x000F;
    value = value & 0xFF;
    switch (addr) {
    case 0x00:                                  // port B
      viaRegisters[addr] = value;
      portBRegister = value;
      updatePortB();
      if ((viaRegisters[0x0C] & 0xA0) == 0x00) {
        viaRegisters[0x0D] = viaRegisters[0x0D] & 0xE7;
        updateInterruptFlags();
      }
      else if ((viaRegisters[0x0C] & 0xC0) == 0x80)
        cb2Output = false;
      break;
    case 0x01:                                  // port A (controls handshake)
      viaRegisters[addr] = value;
      portARegister = value;
      updatePortA();
      if ((viaRegisters[0x0C] & 0x0A) == 0x00) {
        viaRegisters[0x0D] = viaRegisters[0x0D] & 0xFC;
        updateInterruptFlags();
      }
      else if ((viaRegisters[0x0C] & 0x0C) == 0x08)
        ca2Output = false;
      break;
    case 0x02:                                  // port B data direction
      viaRegisters[addr] = value;
      portBDataDirection = value;
      updatePortB();
      break;
    case 0x03:                                  // port A data direction
      viaRegisters[addr] = value;
      portADataDirection = value;
      updatePortA();
      break;
    case 0x04:                                  // timer 1 low byte
      timer1Latch = (timer1Latch & 0xFF00) | uint16_t(value);
      break;
    case 0x05:                                  // timer 1 high byte
      timer1Latch = (timer1Latch & 0x00FF) | (uint16_t(value) << 8);
      timer1Counter = timer1Latch;
      timer1SingleShotModeDone = false;
      if (timer1SingleShotMode) {
        portBTimerOutput = 0x00;
        updatePortB();
      }
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xBF;
      updateInterruptFlags();
      break;
    case 0x06:                                  // timer 1 latch low byte
      timer1Latch = (timer1Latch & 0xFF00) | uint16_t(value);
      break;
    case 0x07:                                  // timer 1 latch high byte
      timer1Latch = (timer1Latch & 0x00FF) | (uint16_t(value) << 8);
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xBF;
      updateInterruptFlags();
      break;
    case 0x08:                                  // timer 2 low byte
      timer2LatchL = value;
      break;
    case 0x09:                                  // timer 2 high byte
      timer2Counter = uint16_t(timer2LatchL) | (uint16_t(value) << 8);
      timer2SingleShotModeDone = false;
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xDF;
      updateInterruptFlags();
      break;
    case 0x0A:                                  // shift register
      viaRegisters[0x0D] = viaRegisters[0x0D] & 0xFB;
      updateInterruptFlags();
      break;
    case 0x0B:                                  // auxiliary control register
      viaRegisters[addr] = value;
      timer2PulseCountingMode = bool(value & 0x20);
      timer1SingleShotMode = !(value & 0x40);
      portBTimerOutputMask = value & 0x80;
      updatePortB();
      break;
    case 0x0C:                                  // peripheral control register
      viaRegisters[addr] = value;
      ca1PositiveEdge = !!(value & 0x01);
      ca2IsOutput = !!(value & 0x08);
      switch (value & 0x0E) {
      case 0x0A:                        // pulse output
        ca2Output = true;
        break;
      case 0x0C:                        // manual output / low
        ca2Output = false;
        break;
      case 0x0E:                        // manual output / high
        ca2Output = true;
        break;
      }
      cb1PositiveEdge = !!(value & 0x10);
      cb2IsOutput = !!(value & 0x80);
      switch (value & 0xE0) {
      case 0xA0:                        // pulse output
        cb2Output = true;
        break;
      case 0xC0:                        // manual output / low
        cb2Output = false;
        break;
      case 0xE0:                        // manual output / high
        cb2Output = true;
        break;
      }
      break;
    case 0x0D:                                  // interrupt flag register
      viaRegisters[addr] = viaRegisters[addr] & (value ^ 0xFF);
      updateInterruptFlags();
      break;
    case 0x0E:                                  // interrupt enable register
      if (value & 0x80)
        viaRegisters[addr] = (viaRegisters[addr] | value) & 0x7F;
      else
        viaRegisters[addr] = (viaRegisters[addr] & (value ^ 0xFF)) & 0x7F;
      updateInterruptFlags();
      break;
    case 0x0F:                                  // port A (no handshake)
      viaRegisters[addr] = value;
      portARegister = value;
      updatePortA();
      break;
    }
  }

  uint8_t VIA6522::readRegisterDebug(uint16_t addr) const
  {
    addr = addr & 0x000F;
    uint8_t value = viaRegisters[addr];
    switch (addr) {
    case 0x00:                                  // port B
      if (!(viaRegisters[0x0B] & 0x02) || !(viaRegisters[0x0D] & 0x10))
        value = getPortB();
      else
        value = portBLatch;
      value = (value & (portBDataDirection ^ 0xFF))
              | (portBRegister & portBDataDirection);
      break;
    case 0x01:                                  // port A (controls handshake)
      if (!(viaRegisters[0x0B] & 0x01) || !(viaRegisters[0x0D] & 0x02))
        value = getPortA();
      else
        value = portALatch;
      break;
    case 0x02:                                  // port B data direction
      break;
    case 0x03:                                  // port A data direction
      break;
    case 0x04:                                  // timer 1 low byte
      value = uint8_t(timer1Counter & 0x00FF);
      break;
    case 0x05:                                  // timer 1 high byte
      value = uint8_t((timer1Counter & 0xFF00) >> 8);
      break;
    case 0x06:                                  // timer 1 latch low byte
      value = uint8_t(timer1Latch & 0x00FF);
      break;
    case 0x07:                                  // timer 1 latch high byte
      value = uint8_t(timer1Latch >> 8);
      break;
    case 0x08:                                  // timer 2 low byte
      value = uint8_t(timer2Counter & 0x00FF);
      break;
    case 0x09:                                  // timer 2 high byte
      value = uint8_t(timer2Counter >> 8);
      break;
    case 0x0A:                                  // shift register
      break;
    case 0x0B:                                  // auxiliary control register
      break;
    case 0x0C:                                  // peripheral control register
      break;
    case 0x0D:                                  // interrupt flag register
      break;
    case 0x0E:                                  // interrupt enable register
      break;
    case 0x0F:                                  // port A (no handshake)
      if (!(viaRegisters[0x0B] & 0x01) || !(viaRegisters[0x0D] & 0x02))
        value = getPortA();
      else
        value = portALatch;
      break;
    }
    return value;
  }

  void VIA6522::timer1Underflow()
  {
    timer1Counter = 0xFFFF;
    if (!timer1SingleShotMode || !timer1SingleShotModeDone) {
      if (timer1SingleShotMode) {
        portBTimerOutput = 0x80;
      }
      else {
        timer1Counter = -1;             // reload counter after 1 cycle delay
        portBTimerOutput = portBTimerOutput ^ 0x80;
      }
      updatePortB();
      viaRegisters[0x0D] = viaRegisters[0x0D] | 0x40;
      updateInterruptFlags();
    }
    timer1SingleShotModeDone = true;
  }

  void VIA6522::updatePortA()
  {
    portAOutput =
        uint8_t(portAInput & (portARegister | (portADataDirection ^ 0xFF)));
  }

  void VIA6522::updatePortB()
  {
    uint8_t prvState = portBOutput;
    portBOutput =
        uint8_t(portBInput
                & ((portBRegister | (portBDataDirection ^ 0xFF))
                   & (portBTimerOutput | (portBTimerOutputMask ^ 0xFF))));
    if (timer2PulseCountingMode) {
      if ((portBOutput & 0x40) < (prvState & 0x40))
        updateTimer2();
    }
  }

  void VIA6522::setCA1_(bool value)
  {
    bool    prvState = getCA1();
    ca1Input = value;
    bool    newState = getCA1();
    if (newState != prvState) {
      if (newState == ca1PositiveEdge) {
        viaRegisters[0x0D] = viaRegisters[0x0D] | 0x02;
        updateInterruptFlags();
        portALatch = getPortA();
      }
    }
  }

  void VIA6522::setCA2_(bool value)
  {
    bool    prvState = getCA2();
    ca2Input = value;
    bool    newState = getCA2();
    if (newState != prvState) {
      if (!ca2IsOutput) {
        if (newState == !!(viaRegisters[0x0C] & 0x40)) {
          viaRegisters[0x0D] = viaRegisters[0x0D] | 0x01;
          updateInterruptFlags();
        }
      }
    }
  }

  void VIA6522::setCB1_(bool value)
  {
    bool    prvState = getCB1();
    cb1Input = value;
    bool    newState = getCB1();
    if (newState != prvState) {
      if (newState == cb1PositiveEdge) {
        viaRegisters[0x0D] = viaRegisters[0x0D] | 0x10;
        updateInterruptFlags();
        portBLatch = getPortB();
      }
    }
  }

  void VIA6522::setCB2_(bool value)
  {
    bool    prvState = getCB2();
    cb2Input = value;
    bool    newState = getCB2();
    if (newState != prvState) {
      if (!cb2IsOutput) {
        if (newState == !!(viaRegisters[0x0C] & 0x04)) {
          viaRegisters[0x0D] = viaRegisters[0x0D] | 0x08;
          updateInterruptFlags();
        }
      }
    }
  }

  void VIA6522::irqStateChangeCallback(bool newState)
  {
    (void) newState;
  }

}       // namespace Plus4

