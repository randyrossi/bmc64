
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

#ifndef PLUS4EMU_VIA6522_HPP
#define PLUS4EMU_VIA6522_HPP

#include "plus4emu.hpp"

namespace Plus4 {

  class VIA6522 {
   private:
    uint8_t   viaRegisters[16];
    uint8_t   portADataDirection;
    uint8_t   portARegister;
    uint8_t   portAInput;
    uint8_t   portALatch;
    uint8_t   portAOutput;
    uint8_t   portBDataDirection;
    uint8_t   portBRegister;
    uint8_t   portBInput;
    uint8_t   portBLatch;
    uint8_t   portBTimerOutputMask;
    uint8_t   portBTimerOutput;
    uint8_t   portBOutput;
    int32_t   timer1Counter;
    uint16_t  timer1Latch;
    bool      timer1SingleShotMode;
    bool      timer1SingleShotModeDone;
    uint16_t  timer2Counter;
    uint8_t   timer2LatchL;
    bool      timer2PulseCountingMode;
    bool      timer2SingleShotModeDone;
    bool      ca1Input;
    bool      ca1Output;
    bool      ca1IsOutput;
    bool      ca1PositiveEdge;
    bool      ca2Input;
    bool      ca2Output;
    bool      ca2IsOutput;
    bool      cb1Input;
    bool      cb1Output;
    bool      cb1IsOutput;
    bool      cb1PositiveEdge;
    bool      cb2Input;
    bool      cb2Output;
    bool      cb2IsOutput;
#if 0
    // FIXME: shift register is unimplemented
    uint8_t   shiftRegister;
    uint8_t   shiftCounter;
#endif
    bool      irqState;
    inline void updateInterruptFlags()
    {
      bool    newIRQState =
          bool((viaRegisters[0x0D] & viaRegisters[0x0E]) & 0x7F);
      if (newIRQState != irqState) {
        irqState = newIRQState;
        viaRegisters[0x0D] &= uint8_t(0x7F);
        viaRegisters[0x0D] |= uint8_t(int(newIRQState) << 7);
        irqStateChangeCallback(newIRQState);
      }
    }
    void timer1Underflow();
    inline void updateTimer1()
    {
      if (PLUS4EMU_UNLIKELY(--timer1Counter < 0)) {
        if (timer1Counter < -1)
          timer1Counter = timer1Latch;
        else
          timer1Underflow();
      }
    }
    inline void updateTimer2()
    {
      if (PLUS4EMU_UNLIKELY(!timer2Counter)) {
        if (!timer2SingleShotModeDone) {
          timer2SingleShotModeDone = true;
          viaRegisters[0x0D] = viaRegisters[0x0D] | 0x20;
          updateInterruptFlags();
        }
      }
      timer2Counter--;
    }
   public:
    VIA6522();
    virtual ~VIA6522();
    void reset();
    inline void runOneCycle()
    {
      updateTimer1();
      if (PLUS4EMU_EXPECT(!timer2PulseCountingMode))
        updateTimer2();
    }
    uint8_t readRegister(uint16_t addr);
    void writeRegister(uint16_t addr, uint8_t value);
    uint8_t readRegisterDebug(uint16_t addr) const;
   private:
    void updatePortA();
    void updatePortB();
    void setCA1_(bool value);
    void setCA2_(bool value);
    void setCB1_(bool value);
    void setCB2_(bool value);
   public:
    inline uint8_t getPortA() const
    {
      return portAOutput;
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
      return portBOutput;
    }
    inline void setPortB(uint8_t value)
    {
      if (value != portBInput) {
        portBInput = value;
        updatePortB();
      }
    }
    inline bool getCA1() const
    {
      return (ca1Input && (ca1Output || !ca1IsOutput));
    }
    inline void setCA1(bool value)
    {
      if (value != ca1Input)
        setCA1_(value);
    }
    inline bool getCA2() const
    {
      return (ca2Input && (ca2Output || !ca2IsOutput));
    }
    inline void setCA2(bool value)
    {
      if (value != ca2Input)
        setCA2_(value);
    }
    inline bool getCB1() const
    {
      return (cb1Input && (cb1Output || !cb1IsOutput));
    }
    inline void setCB1(bool value)
    {
      if (value != cb1Input)
        setCB1_(value);
    }
    inline bool getCB2() const
    {
      return (cb2Input && (cb2Output || !cb2IsOutput));
    }
    inline void setCB2(bool value)
    {
      if (value != cb2Input)
        setCB2_(value);
    }
   protected:
    // called when the state of the IRQ line changes
    // 'newState' is true if the IRQ line is low, and false if it is high
    virtual void irqStateChangeCallback(bool newState);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_VIA6522_HPP

