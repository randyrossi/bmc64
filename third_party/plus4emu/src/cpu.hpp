
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_CPU_HPP
#define PLUS4EMU_CPU_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "bplist.hpp"

namespace Plus4 {

  class M7501Registers {
   public:
    uint16_t  reg_PC;
    uint8_t   reg_SR;
    uint8_t   reg_AC;
    uint8_t   reg_XR;
    uint8_t   reg_YR;
    uint8_t   reg_SP;
    M7501Registers()
      : reg_PC(0x0000),
        reg_SR(uint8_t(0x24)),
        reg_AC(0x00),
        reg_XR(0x00),
        reg_YR(0x00),
        reg_SP(uint8_t(0xFF))
    {
    }
  };

  typedef PLUS4EMU_REGPARM2 uint8_t (*M7501MemoryReadCallback)(void *userData,
                                                               uint16_t addr);

  typedef PLUS4EMU_REGPARM3 void (*M7501MemoryWriteCallback)(void *userData,
                                                             uint16_t addr,
                                                             uint8_t value);

  class M7501 : protected M7501Registers {
   private:
    static const unsigned char  CPU_OP_RD_OPCODE            =  0;
    static const unsigned char  CPU_OP_RD_TMP               =  1;
    static const unsigned char  CPU_OP_RD_TMP_NODEBUG       =  2;
    static const unsigned char  CPU_OP_RD_L                 =  3;
    static const unsigned char  CPU_OP_RD_H                 =  4;
    static const unsigned char  CPU_OP_LD_TMP_MEM           =  5;
    static const unsigned char  CPU_OP_LD_MEM_TMP           =  6;
    static const unsigned char  CPU_OP_LD_MEM_TMP_NODEBUG   =  7;
    static const unsigned char  CPU_OP_LD_H_MEMP1_L_TMP     =  8;
    static const unsigned char  CPU_OP_LD_DUMMY_MEM_PC      =  9;
    static const unsigned char  CPU_OP_LD_DUMMY_MEM_SP      = 10;
    static const unsigned char  CPU_OP_PUSH_TMP             = 11;
    static const unsigned char  CPU_OP_POP_TMP              = 12;
    static const unsigned char  CPU_OP_PUSH_PCL             = 13;
    static const unsigned char  CPU_OP_POP_PCL              = 14;
    static const unsigned char  CPU_OP_PUSH_PCH             = 15;
    static const unsigned char  CPU_OP_POP_PCH              = 16;
    static const unsigned char  CPU_OP_DEC_SP               = 17;
    static const unsigned char  CPU_OP_LD_TMP_SR            = 18;
    static const unsigned char  CPU_OP_LD_TMP_A             = 19;
    static const unsigned char  CPU_OP_LD_PC_HL             = 20;
    static const unsigned char  CPU_OP_LD_SR_TMP            = 21;
    static const unsigned char  CPU_OP_LD_A_TMP             = 22;
    static const unsigned char  CPU_OP_ADDR_X               = 23;
    static const unsigned char  CPU_OP_ADDR_X_SLOW          = 24;
    static const unsigned char  CPU_OP_ADDR_X_ZEROPAGE      = 25;
    static const unsigned char  CPU_OP_ADDR_X_SHY           = 26;
    static const unsigned char  CPU_OP_ADDR_Y               = 27;
    static const unsigned char  CPU_OP_ADDR_Y_SLOW          = 28;
    static const unsigned char  CPU_OP_ADDR_Y_ZEROPAGE      = 29;
    static const unsigned char  CPU_OP_ADDR_Y_SHA           = 30;
    static const unsigned char  CPU_OP_ADDR_Y_SHS           = 31;
    static const unsigned char  CPU_OP_ADDR_Y_SHX           = 32;
    static const unsigned char  CPU_OP_SET_NZ               = 33;
    static const unsigned char  CPU_OP_ADC                  = 34;
    static const unsigned char  CPU_OP_ANC                  = 35;
    static const unsigned char  CPU_OP_AND                  = 36;
    static const unsigned char  CPU_OP_ANE                  = 37;
    static const unsigned char  CPU_OP_ARR                  = 38;
    static const unsigned char  CPU_OP_ASL                  = 39;
    static const unsigned char  CPU_OP_BCC                  = 40;
    static const unsigned char  CPU_OP_BCS                  = 41;
    static const unsigned char  CPU_OP_BEQ                  = 42;
    static const unsigned char  CPU_OP_BIT                  = 43;
    static const unsigned char  CPU_OP_BMI                  = 44;
    static const unsigned char  CPU_OP_BNE                  = 45;
    static const unsigned char  CPU_OP_BPL                  = 46;
    static const unsigned char  CPU_OP_BRK                  = 47;
    static const unsigned char  CPU_OP_BVC                  = 48;
    static const unsigned char  CPU_OP_BVS                  = 49;
    static const unsigned char  CPU_OP_CLC                  = 50;
    static const unsigned char  CPU_OP_CLD                  = 51;
    static const unsigned char  CPU_OP_CLI                  = 52;
    static const unsigned char  CPU_OP_CLV                  = 53;
    static const unsigned char  CPU_OP_CMP                  = 54;
    static const unsigned char  CPU_OP_CPX                  = 55;
    static const unsigned char  CPU_OP_CPY                  = 56;
    static const unsigned char  CPU_OP_DEC                  = 57;
    static const unsigned char  CPU_OP_DEX                  = 58;
    static const unsigned char  CPU_OP_DEY                  = 59;
    static const unsigned char  CPU_OP_EOR                  = 60;
    static const unsigned char  CPU_OP_INC                  = 61;
    static const unsigned char  CPU_OP_INTERRUPT            = 62;
    static const unsigned char  CPU_OP_INX                  = 63;
    static const unsigned char  CPU_OP_INY                  = 64;
    static const unsigned char  CPU_OP_JMP_RELATIVE         = 65;
    static const unsigned char  CPU_OP_LAS                  = 66;
    static const unsigned char  CPU_OP_LAX                  = 67;
    static const unsigned char  CPU_OP_LDA                  = 68;
    static const unsigned char  CPU_OP_LDX                  = 69;
    static const unsigned char  CPU_OP_LDY                  = 70;
    static const unsigned char  CPU_OP_LSR                  = 71;
    static const unsigned char  CPU_OP_LXA                  = 72;
    static const unsigned char  CPU_OP_ORA                  = 73;
    static const unsigned char  CPU_OP_RESET                = 74;
    static const unsigned char  CPU_OP_ROL                  = 75;
    static const unsigned char  CPU_OP_ROR                  = 76;
    static const unsigned char  CPU_OP_SAX                  = 77;
    static const unsigned char  CPU_OP_SBC                  = 78;
    static const unsigned char  CPU_OP_SBX                  = 79;
    static const unsigned char  CPU_OP_SEC                  = 80;
    static const unsigned char  CPU_OP_SED                  = 81;
    static const unsigned char  CPU_OP_SEI                  = 82;
    static const unsigned char  CPU_OP_STA                  = 83;
    static const unsigned char  CPU_OP_STX                  = 84;
    static const unsigned char  CPU_OP_STY                  = 85;
    static const unsigned char  CPU_OP_TAX                  = 86;
    static const unsigned char  CPU_OP_TAY                  = 87;
    static const unsigned char  CPU_OP_TSX                  = 88;
    static const unsigned char  CPU_OP_TXA                  = 89;
    static const unsigned char  CPU_OP_TXS                  = 90;
    static const unsigned char  CPU_OP_TYA                  = 91;
    static const unsigned char  CPU_OP_INVALID_OPCODE       = 92;
    static const unsigned char  opcodeTable[4128];
    const unsigned char *currentOpcode;
    uint8_t     interruptDelayRegister;
    bool        interruptFlag;
    bool        resetFlag;
    bool        haltFlag;
    uint8_t     reg_TMP;
    uint8_t     reg_L;
    uint8_t     reg_H;
    bool        breakOnInvalidOpcode;
    M7501MemoryReadCallback   *memoryReadCallbacks;
    M7501MemoryWriteCallback  *memoryWriteCallbacks;
    void        *memoryCallbackUserData;
    uint8_t     *breakPointTable;
    unsigned int  breakPointCnt;
    // 0: normal mode, 1: single step, 2: step over, 3: trace
    uint8_t     singleStepMode;
    bool        haveBreakPoints;
    uint8_t     breakPointPriorityThreshold;
    int32_t     singleStepModeNextAddr;
    int32_t     newPCAddress;
    void checkOpcodeReadBreakPoint(uint16_t addr, uint8_t value);
    void checkReadBreakPoint(uint16_t addr, uint8_t value);
    void checkWriteBreakPoint(uint16_t addr, uint8_t value);
   protected:
    inline uint8_t readMemory(uint16_t addr)
    {
      return (memoryReadCallbacks[addr](memoryCallbackUserData, addr));
    }
    inline void writeMemory(uint16_t addr, uint8_t value)
    {
      memoryWriteCallbacks[addr](memoryCallbackUserData, addr, value);
    }
   public:
    M7501();
    virtual ~M7501();
    inline void setMemoryReadCallback(uint16_t addr_,
                                      M7501MemoryReadCallback func)
    {
      memoryReadCallbacks[addr_] = func;
    }
    inline void setMemoryWriteCallback(uint16_t addr_,
                                       M7501MemoryWriteCallback func)
    {
      memoryWriteCallbacks[addr_] = func;
    }
    inline void setMemoryCallbackUserData(void *userData)
    {
      memoryCallbackUserData = userData;
    }
    inline M7501MemoryReadCallback getMemoryReadCallback(uint16_t addr_) const
    {
      return memoryReadCallbacks[addr_];
    }
    inline M7501MemoryWriteCallback getMemoryWriteCallback(uint16_t addr_) const
    {
      return memoryWriteCallbacks[addr_];
    }
    void runOneCycle_RDYHigh();
    void runOneCycle_RDYLow();
    inline void runOneCycle()
    {
      if (PLUS4EMU_EXPECT(!haltFlag))
        runOneCycle_RDYHigh();
      else
        runOneCycle_RDYLow();
    }
    inline void run_RDYHigh(int nCycles = 1)
    {
      do {
        this->runOneCycle_RDYHigh();
      } while (--nCycles);
    }
    inline void run_RDYLow(int nCycles = 1)
    {
      do {
        this->runOneCycle_RDYLow();
      } while (--nCycles);
    }
    inline void run(int nCycles = 1)
    {
      if (PLUS4EMU_EXPECT(!haltFlag))
        run_RDYHigh(nCycles);
      else
        run_RDYLow(nCycles);
    }
    inline void interruptRequest()
    {
      // delay interrupt requests by 2 cycles
      interruptDelayRegister |= uint8_t(0x04);
    }
    inline void clearInterruptRequest()
    {
      interruptDelayRegister &= uint8_t(0x03);
    }
    inline void interruptRequest(bool isIRQ)
    {
      // delay interrupt requests by 2 cycles
      interruptDelayRegister &= uint8_t(0x03);
      interruptDelayRegister |= (uint8_t(isIRQ) << 2);
    }
    virtual void reset(bool isColdReset = false);
    inline void setIsCPURunning(bool n)
    {
      haltFlag = !n;
    }
    inline bool getIsCPURunning() const
    {
      return (!haltFlag);
    }
    inline void setOverflowFlag()
    {
      reg_SR |= uint8_t(0x40);
    }
    // 'type' can be the sum of any of:
    //   1: memory read
    //   2: memory write
    //   4: execute (opcode read)
    //   8: ignore
    // a negative priority value deletes a previously set breakpoint
    void setBreakPoint(int type, uint16_t addr, int priority);
    void clearBreakPoints();
    void setBreakPointPriorityThreshold(int n);
    inline int getBreakPointPriorityThreshold() const
    {
      return int(breakPointPriorityThreshold >> 4);
    }
    Plus4Emu::BreakPointList getBreakPointList();
    // 'mode_' can be one of the following values:
    //   0: normal mode
    //   1: single step (break on every instruction, disable breakpoints)
    //   2: step over (skip JSR or branch instruction)
    //   3: trace (break on every instruction, breakpoints are not disabled)
    //   4: step into (break after branch instruction only if branch is taken)
    void setSingleStepMode(int mode_);
    // Set the next address where single step mode will stop, ignoring any
    // other instructions. If 'addr' is negative, then a break is triggered
    // immediately at the next instruction.
    // Note: setSingleStepMode() must be called first with a mode parameter
    // of 2 or 4.
    void setSingleStepModeNextAddress(int32_t addr);
    inline void setBreakOnInvalidOpcode(bool isEnabled)
    {
      this->breakOnInvalidOpcode = isEnabled;
    }
    inline bool getIsBreakOnInvalidOpcode() const
    {
      return (this->breakOnInvalidOpcode);
    }
    void setRegisters(const M7501Registers& r);
    void getRegisters(M7501Registers& r) const;
    void saveState(Plus4Emu::File::Buffer&);
    void saveState(Plus4Emu::File&);
    void loadState(Plus4Emu::File::Buffer&);
    void registerChunkType(Plus4Emu::File&);
   protected:
    // type is 0 for opcode read at breakpoint, 1 for memory read,
    // 2 for memory write, and 3 for opcode read in single step mode
    virtual void breakPointCallback(int type, uint16_t addr, uint8_t value);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_CPU_HPP

