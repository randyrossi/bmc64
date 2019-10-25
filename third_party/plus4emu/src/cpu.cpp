
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

static PLUS4EMU_REGPARM2
    uint8_t dummyMemoryReadCallback(void *userData, uint16_t addr)
{
  (void) userData;
  (void) addr;
  return uint8_t(0xFF);
}

static PLUS4EMU_REGPARM3
    void dummyMemoryWriteCallback(void *userData, uint16_t addr, uint8_t value)
{
  (void) userData;
  (void) addr;
  (void) value;
}

#define setFlagsNZ(reg__)                       \
  reg_SR = uint8_t((reg_SR & 0x7D)              \
                   | ((reg__) & 0x80)           \
                   | (uint8_t((reg__) == 0) + uint8_t((reg__) == 0)))

namespace Plus4 {

  M7501::M7501()
    : M7501Registers(),
      currentOpcode(&(opcodeTable[0x0FFF])),
      interruptDelayRegister(0x00),
      interruptFlag(false),
      resetFlag(true),
      haltFlag(false),
      reg_TMP(0),
      reg_L(0),
      reg_H(0),
      breakOnInvalidOpcode(false),
      memoryReadCallbacks((M7501MemoryReadCallback *) 0),
      memoryWriteCallbacks((M7501MemoryWriteCallback *) 0),
      memoryCallbackUserData((void *) 0),
      breakPointTable((uint8_t *) 0),
      breakPointCnt(0U),
      singleStepMode(0),
      haveBreakPoints(false),
      breakPointPriorityThreshold(0),
      singleStepModeNextAddr(int32_t(-1)),
      newPCAddress(int32_t(-1))
  {
    try {
      memoryReadCallbacks = new M7501MemoryReadCallback[65536];
      for (size_t i = 0; i < 65536; i++)
        memoryReadCallbacks[i] = &dummyMemoryReadCallback;
      memoryWriteCallbacks = new M7501MemoryWriteCallback[65536];
      for (size_t i = 0; i < 65536; i++)
        memoryWriteCallbacks[i] = &dummyMemoryWriteCallback;
    }
    catch (...) {
      if (memoryWriteCallbacks)
        delete[] memoryWriteCallbacks;
      if (memoryReadCallbacks)
        delete[] memoryReadCallbacks;
      throw;
    }
  }

  M7501::~M7501()
  {
    if (breakPointTable)
      delete[] breakPointTable;
    if (memoryWriteCallbacks)
      delete[] memoryWriteCallbacks;
    if (memoryReadCallbacks)
      delete[] memoryReadCallbacks;
  }

  void M7501::runOneCycle_RDYHigh()
  {
    if (interruptDelayRegister != 0x00) {
      interruptFlag = interruptFlag | bool(interruptDelayRegister & 0x01);
      interruptDelayRegister &= (~(uint8_t((reg_SR >> 1) & 0x02)));
      interruptDelayRegister = (interruptDelayRegister & uint8_t(0x04))
                               | (interruptDelayRegister >> 1);
    }
    while (true) {
      unsigned char n = *(currentOpcode++);
      switch (n) {
      case CPU_OP_RD_OPCODE:
        if (PLUS4EMU_EXPECT(!(interruptFlag | resetFlag))) {
          uint8_t opNum = readMemory(reg_PC);
          if (uint8_t(haveBreakPoints) | singleStepMode)
            checkOpcodeReadBreakPoint(reg_PC, opNum);
          reg_PC = (reg_PC + 1) & 0xFFFF;
          currentOpcode = &(opcodeTable[size_t(opNum) << 4]);
        }
        else {
          if (PLUS4EMU_UNLIKELY(resetFlag)) {
            if (newPCAddress >= 0) {
              // set new PC if requested
              reg_PC = uint16_t(newPCAddress & 0xFFFF);
              newPCAddress = int32_t(-1);
              resetFlag = false;
              // continue with opcode fetch
              currentOpcode--;
              continue;
            }
            currentOpcode = &(opcodeTable[size_t(0x101) << 4]);
          }
          else
            currentOpcode = &(opcodeTable[size_t(0x100) << 4]);
          continue;
        }
        break;
      case CPU_OP_RD_TMP:
        reg_TMP = readMemory(reg_PC);
        if (haveBreakPoints)
          checkReadBreakPoint(reg_PC, reg_TMP);
        reg_PC = (reg_PC + 1) & 0xFFFF;
        break;
      case CPU_OP_RD_TMP_NODEBUG:
        reg_TMP = readMemory(reg_PC);
        reg_PC = (reg_PC + 1) & 0xFFFF;
        break;
      case CPU_OP_RD_L:
        reg_L = readMemory(reg_PC);
        reg_H = uint8_t(0x00);
        if (haveBreakPoints)
          checkReadBreakPoint(reg_PC, reg_L);
        reg_PC = (reg_PC + 1) & 0xFFFF;
        break;
      case CPU_OP_RD_H:
        reg_H = readMemory(reg_PC);
        if (haveBreakPoints)
          checkReadBreakPoint(reg_PC, reg_H);
        reg_PC = (reg_PC + 1) & 0xFFFF;
        break;
      case CPU_OP_LD_TMP_MEM:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          reg_TMP = readMemory(addr);
          if (haveBreakPoints)
            checkReadBreakPoint(addr, reg_TMP);
        }
        break;
      case CPU_OP_LD_MEM_TMP:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_TMP);
          writeMemory(addr, reg_TMP);
        }
        break;
      case CPU_OP_LD_MEM_TMP_NODEBUG:
        writeMemory(uint16_t(reg_L) | (uint16_t(reg_H) << 8), reg_TMP);
        break;
      case CPU_OP_LD_H_MEMP1_L_TMP:
        {
          uint16_t  addr = uint16_t((reg_L + uint8_t(1)) & uint8_t(0xFF))
                           | (uint16_t(reg_H) << 8);
          reg_L = reg_TMP;
          reg_H = readMemory(addr);
          if (haveBreakPoints)
            checkReadBreakPoint(addr, reg_H);
        }
        break;
      case CPU_OP_LD_DUMMY_MEM_PC:
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_LD_DUMMY_MEM_SP:
        (void) readMemory(uint16_t(0x0100) | uint16_t(reg_SP));
        break;
      case CPU_OP_PUSH_TMP:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP), reg_TMP);
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_TMP:
        // FIXME: should check breakpoints ?
        reg_SP = (reg_SP + uint8_t(1)) & uint8_t(0xFF);
        reg_TMP = readMemory(uint16_t(0x0100) | uint16_t(reg_SP));
        break;
      case CPU_OP_PUSH_PCL:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP),
                    uint8_t(reg_PC) & uint8_t(0xFF));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_PCL:
        // FIXME: should check breakpoints ?
        reg_SP = (reg_SP + uint8_t(1)) & uint8_t(0xFF);
        reg_PC = (reg_PC & uint16_t(0xFF00))
                 | uint16_t(readMemory(uint16_t(0x0100)
                                       | uint16_t(reg_SP)));
        break;
      case CPU_OP_PUSH_PCH:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP),
                    uint8_t(reg_PC >> 8) & uint8_t(0xFF));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_PCH:
        // FIXME: should check breakpoints ?
        reg_SP = (reg_SP + uint8_t(1)) & uint8_t(0xFF);
        reg_PC = (reg_PC & uint16_t(0x00FF))
                 | (uint16_t(readMemory(uint16_t(0x0100)
                                        | uint16_t(reg_SP))) << 8);
        break;
      case CPU_OP_DEC_SP:
        // dummy push operation for reset
        (void) readMemory(uint16_t(0x0100) | uint16_t(reg_SP));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_LD_TMP_SR:
        {
          reg_TMP = reg_SR | uint8_t(0x20);
          continue;
        }
        break;
      case CPU_OP_LD_TMP_A:
        {
          reg_TMP = reg_AC;
          continue;
        }
        break;
      case CPU_OP_LD_PC_HL:
        {
          reg_PC = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          continue;
        }
        break;
      case CPU_OP_LD_SR_TMP:
        {
          reg_SR = reg_TMP | uint8_t(0x30);
          continue;
        }
        break;
      case CPU_OP_LD_A_TMP:
        {
          reg_AC = reg_TMP;
          continue;
        }
        break;
      case CPU_OP_ADDR_X:
        {
          uint8_t tmp = (reg_L + reg_XR) & uint8_t(0xFF);
          if (tmp < reg_L) {
            (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
            reg_L = tmp;
            reg_H = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          }
          else {
            reg_L = tmp;
            continue;
          }
        }
        break;
      case CPU_OP_ADDR_X_SLOW:
        {
          uint8_t tmp = (reg_L + reg_XR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          if (tmp < reg_L)
            reg_H = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          reg_L = tmp;
        }
        break;
      case CPU_OP_ADDR_X_ZEROPAGE:
        (void) readMemory(uint16_t(reg_L));
        reg_L = (reg_L + reg_XR) & uint8_t(0xFF);
        reg_H = uint8_t(0x00);
        break;
      case CPU_OP_ADDR_X_SHY:
        {
          uint8_t tmp = (reg_L + reg_XR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          uint8_t addrHp1 = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          addrHp1 = addrHp1 & reg_YR;
          reg_TMP = addrHp1;
          if (tmp < reg_L)
            reg_H = addrHp1;
          reg_L = tmp;
        }
        break;
      case CPU_OP_ADDR_Y:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          if (tmp < reg_L) {
            (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
            reg_L = tmp;
            reg_H = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          }
          else {
            reg_L = tmp;
            continue;
          }
        }
        break;
      case CPU_OP_ADDR_Y_SLOW:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          if (tmp < reg_L)
            reg_H = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          reg_L = tmp;
        }
        break;
      case CPU_OP_ADDR_Y_ZEROPAGE:
        (void) readMemory(uint16_t(reg_L));
        reg_L = (reg_L + reg_YR) & uint8_t(0xFF);
        reg_H = uint8_t(0x00);
        break;
      case CPU_OP_ADDR_Y_SHA:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          uint8_t addrHp1 = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          addrHp1 = addrHp1 & (reg_AC & reg_XR);
          reg_TMP = addrHp1;
          if (tmp < reg_L)
            reg_H = addrHp1;
          reg_L = tmp;
        }
        break;
      case CPU_OP_ADDR_Y_SHS:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          uint8_t addrHp1 = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          reg_SP = reg_AC & reg_XR;
          addrHp1 = addrHp1 & reg_SP;
          reg_TMP = addrHp1;
          if (tmp < reg_L)
            reg_H = addrHp1;
          reg_L = tmp;
        }
        break;
      case CPU_OP_ADDR_Y_SHX:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
          uint8_t addrHp1 = (reg_H + uint8_t(1)) & uint8_t(0xFF);
          addrHp1 = addrHp1 & reg_XR;
          reg_TMP = addrHp1;
          if (tmp < reg_L)
            reg_H = addrHp1;
          reg_L = tmp;
        }
        break;
      case CPU_OP_SET_NZ:
        {
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_ADC:
        {
          if (!(reg_SR & uint8_t(0x08))) {
            // add in binary mode
            unsigned int  result = (unsigned int) (reg_AC & uint8_t(0xFF))
                                   + (unsigned int) (reg_TMP & uint8_t(0xFF))
                                   + (unsigned int) (reg_SR & uint8_t(0x01));
            reg_SR &= uint8_t(0x3C);
            reg_SR |= (uint8_t(result >> 8) & uint8_t(0x01));           // C
            reg_SR |= ((((reg_AC ^ ~reg_TMP) & (reg_AC ^ uint8_t(result)))
                        & uint8_t(0x80)) >> 1);                         // V
            reg_AC = uint8_t(result) & uint8_t(0xFF);
            reg_SR |= (reg_AC & uint8_t(0x80));                         // N
            reg_SR |= uint8_t(reg_AC == uint8_t(0x00) ? 0x02 : 0x00);   // Z
          }
          else {
            // add in BCD mode
            unsigned int  l = (unsigned int) (reg_SR & uint8_t(0x01))
                              + (unsigned int) (reg_AC & uint8_t(0x0F))
                              + (unsigned int) (reg_TMP & uint8_t(0x0F));
            unsigned int  h = (unsigned int) (reg_AC & uint8_t(0xF0))
                              + (unsigned int) (reg_TMP & uint8_t(0xF0));
            reg_SR &= uint8_t(0x3C);
            if (!(uint8_t(h + l) & uint8_t(0xFF)))
              reg_SR |= uint8_t(0x02);                                  // Z
            l += (l < 0x0AU ? 0x00U : 0x06U);
            h += (l >= 0x10U ? 0x10U : 0x00U);
            l &= 0x0FU;
            reg_SR = reg_SR | (uint8_t(h) & uint8_t(0x80));             // N
            reg_SR |= ((((reg_AC ^ ~reg_TMP) & (reg_AC ^ uint8_t(h)))
                        & uint8_t(0x80)) >> 1);                         // V
            h += (h < 0xA0U ? 0x00U : 0x60U);
            reg_SR |= (h >= 0x0100U ? uint8_t(0x01) : uint8_t(0x00));   // C
            reg_AC = uint8_t(h + l) & uint8_t(0xFF);
          }
          continue;
        }
        break;
      case CPU_OP_ANC:
        {
          reg_AC = reg_AC & reg_TMP;
          reg_SR &= uint8_t(0x7C);
          if (reg_AC & uint8_t(0x80))
            reg_SR |= uint8_t(0x81);
          else if (!reg_AC)
            reg_SR |= uint8_t(0x02);
          continue;
        }
        break;
      case CPU_OP_AND:
        {
          reg_AC = reg_AC & reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ANE:
        {
          reg_AC = (reg_AC | uint8_t(0xEE)) & (reg_XR & reg_TMP);
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ARR:
        {
          uint8_t tmp = reg_AC & reg_TMP;
          reg_AC = tmp >> 1;
          if (reg_SR & uint8_t(0x01))
            reg_AC |= uint8_t(0x80);
          reg_SR &= uint8_t(0x3C);
          reg_SR |= ((reg_AC ^ tmp) & uint8_t(0x40));
          reg_SR |= (reg_AC & uint8_t(0x80));
          if (!reg_AC)
            reg_SR |= uint8_t(0x02);
          if (!(reg_SR & uint8_t(0x08))) {
            reg_SR |= ((tmp & uint8_t(0x80)) >> 7);
          }
          else {
            if ((tmp & uint8_t(0x0F)) >= uint8_t(0x05))
              reg_AC = uint8_t((reg_AC & 0xF0) | ((reg_AC + 0x06) & 0x0F));
            if (tmp >= uint8_t(0x50)) {
              reg_AC = uint8_t((reg_AC + 0x60) & 0xFF);
              reg_SR |= uint8_t(0x01);
            }
          }
          continue;
        }
        break;
      case CPU_OP_ASL:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = tmp << 1;
          reg_SR = (reg_SR & uint8_t(0x7C)) | uint8_t(tmp >> 8);
          reg_TMP = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_BCC:
        if (reg_SR & uint8_t(0x01)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BCS:
        if (!(reg_SR & uint8_t(0x01))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BEQ:
        if (!(reg_SR & uint8_t(0x02))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BIT:
        {
          reg_SR = (reg_SR & uint8_t(0x3D)) | (reg_TMP & uint8_t(0xC0));
          reg_SR = reg_SR | ((reg_AC & reg_TMP) == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_BMI:
        if (!(reg_SR & uint8_t(0x80))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BNE:
        if (reg_SR & uint8_t(0x02)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BPL:
        if (reg_SR & uint8_t(0x80)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BRK:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          reg_TMP = reg_SR | uint8_t(0x10);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFE);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_BVC:
        if (reg_SR & uint8_t(0x40)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_BVS:
        if (!(reg_SR & uint8_t(0x40))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        break;
      case CPU_OP_CLC:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR & uint8_t(0xFE);
        break;
      case CPU_OP_CLD:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR & uint8_t(0xF7);
        break;
      case CPU_OP_CLI:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR & uint8_t(0xFB);
        break;
      case CPU_OP_CLV:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR & uint8_t(0xBF);
        break;
      case CPU_OP_CMP:
        {
          uint16_t  tmp = uint16_t(reg_AC)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_CPX:
        {
          uint16_t  tmp = uint16_t(reg_XR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_CPY:
        {
          uint16_t  tmp = uint16_t(reg_YR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_DEC:
        {
          reg_TMP = (reg_TMP - uint8_t(1)) & uint8_t(0xFF);
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_DEX:
        (void) readMemory(reg_PC);
        reg_XR = (reg_XR - uint8_t(1)) & uint8_t(0xFF);
        setFlagsNZ(reg_XR);
        break;
      case CPU_OP_DEY:
        (void) readMemory(reg_PC);
        reg_YR = (reg_YR - uint8_t(1)) & uint8_t(0xFF);
        setFlagsNZ(reg_YR);
        break;
      case CPU_OP_EOR:
        {
          reg_AC = reg_AC ^ reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_INC:
        {
          reg_TMP = (reg_TMP + uint8_t(1)) & uint8_t(0xFF);
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_INTERRUPT:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          reg_TMP = reg_SR & uint8_t(0xEF);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFE);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_INX:
        (void) readMemory(reg_PC);
        reg_XR = (reg_XR + uint8_t(1)) & uint8_t(0xFF);
        setFlagsNZ(reg_XR);
        break;
      case CPU_OP_INY:
        (void) readMemory(reg_PC);
        reg_YR = (reg_YR + uint8_t(1)) & uint8_t(0xFF);
        setFlagsNZ(reg_YR);
        break;
      case CPU_OP_JMP_RELATIVE:
        {
          unsigned int  tmp = reg_PC + int(int8_t(reg_L));
          if ((tmp ^ reg_PC) & 0xFF00U) {
            (void) readMemory((tmp & 0x00FFU) | (reg_PC & 0xFF00U));
            reg_PC = uint16_t(tmp & 0xFFFFU);
          }
          else {
            reg_PC = uint16_t(tmp & 0xFFFFU);
            continue;
          }
        }
        break;
      case CPU_OP_LAS:
        {
          reg_AC = reg_SP & reg_TMP;
          reg_XR = reg_AC;
          reg_SP = reg_AC;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_LAX:
        {
          reg_AC = reg_TMP;
          reg_XR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDA:
        {
          reg_AC = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDX:
        {
          reg_XR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDY:
        {
          reg_YR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LSR:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          reg_SR = (reg_SR & uint8_t(0x7C)) | (uint8_t(tmp) & uint8_t(0x01));
          reg_TMP = uint8_t(tmp >> 1);
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_LXA:
        {
          reg_AC = (reg_AC | uint8_t(0xEE)) & reg_TMP;
          reg_XR = reg_AC;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ORA:
        {
          reg_AC = reg_AC | reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_RESET:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          resetFlag = false;
          reg_TMP = reg_SR & uint8_t(0xEF);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFC);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_ROL:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = (tmp << 1) | uint16_t(reg_SR & uint8_t(0x01));
          reg_SR = (reg_SR & uint8_t(0x7C)) | uint8_t(tmp >> 8);
          reg_TMP = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_ROR:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = tmp | (uint16_t(reg_SR & uint8_t(0x01)) << 8);
          reg_SR = (reg_SR & uint8_t(0x7C)) | (uint8_t(tmp) & uint8_t(0x01));
          reg_TMP = uint8_t(tmp >> 1);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_SAX:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          uint8_t   tmp = reg_AC & reg_XR;
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, tmp);
          writeMemory(addr, tmp);
        }
        break;
      case CPU_OP_SBC:
        {
          // subtract in binary mode
          uint8_t       tmp = (reg_TMP ^ uint8_t(0xFF)) & uint8_t(0xFF);
          uint8_t       c = reg_SR & uint8_t(0x01);
          unsigned int  result = (unsigned int) (reg_AC & uint8_t(0xFF))
                                 + (unsigned int) tmp
                                 + (unsigned int) c;
          reg_SR &= uint8_t(0x3C);
          reg_SR |= (uint8_t(result >> 8) & uint8_t(0x01));             // C
          reg_SR |= ((((reg_AC ^ ~tmp) & (reg_AC ^ uint8_t(result)))
                      & uint8_t(0x80)) >> 1);                           // V
          result &= 0xFFU;
          reg_SR |= (uint8_t(result) & uint8_t(0x80));                  // N
          reg_SR |= uint8_t(result == 0x00U ? 0x02 : 0x00);             // Z
          if (reg_SR & uint8_t(0x08)) {
            // subtract in BCD mode
            unsigned int  l = (unsigned int) (reg_AC & uint8_t(0x0F))
                              + (unsigned int) (tmp & uint8_t(0x0F))
                              + (unsigned int) c;
            unsigned int  h = (unsigned int) (reg_AC & uint8_t(0xF0))
                              + (unsigned int) (tmp & uint8_t(0xF0));
            h += (l & 0x10U);
            l += (l < 0x10U ? 0x0AU : 0x00U);
            h += (h < 0x100U ? 0xA0U : 0x00U);
            result = (h + (l & 0x0FU)) & 0xFFU;
          }
          reg_AC = uint8_t(result);
          continue;
        }
        break;
      case CPU_OP_SBX:
        {
          uint16_t  tmp = uint16_t(reg_AC & reg_XR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_XR = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (reg_XR & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          reg_SR = reg_SR | (reg_XR == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_SEC:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR | uint8_t(0x01);
        break;
      case CPU_OP_SED:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR | uint8_t(0x08);
        break;
      case CPU_OP_SEI:
        (void) readMemory(reg_PC);
        reg_SR = reg_SR | uint8_t(0x04);
        break;
      case CPU_OP_STA:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_AC);
          writeMemory(addr, reg_AC);
        }
        break;
      case CPU_OP_STX:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_XR);
          writeMemory(addr, reg_XR);
        }
        break;
      case CPU_OP_STY:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_YR);
          writeMemory(addr, reg_YR);
        }
        break;
      case CPU_OP_TAX:
        (void) readMemory(reg_PC);
        reg_XR = reg_AC;
        setFlagsNZ(reg_AC);
        break;
      case CPU_OP_TAY:
        (void) readMemory(reg_PC);
        reg_YR = reg_AC;
        setFlagsNZ(reg_AC);
        break;
      case CPU_OP_TSX:
        (void) readMemory(reg_PC);
        reg_XR = reg_SP;
        setFlagsNZ(reg_SP);
        break;
      case CPU_OP_TXA:
        (void) readMemory(reg_PC);
        reg_AC = reg_XR;
        setFlagsNZ(reg_AC);
        break;
      case CPU_OP_TXS:
        (void) readMemory(reg_PC);
        reg_SP = reg_XR;
        break;
      case CPU_OP_TYA:
        (void) readMemory(reg_PC);
        reg_AC = reg_YR;
        setFlagsNZ(reg_AC);
        break;
      case CPU_OP_INVALID_OPCODE:
        if (breakOnInvalidOpcode) {
          uint16_t  tmp = (reg_PC - 1) & 0xFFFF;
          uint8_t   tmp2 = readMemory(tmp);
          if (!((singleStepMode == 1 || singleStepMode == 2) ||
                breakPointPriorityThreshold >= 16)) {
            reg_PC = tmp;
            breakPointCallback(0, tmp, tmp2);
            reg_PC = (reg_PC + 1) & 0xFFFF;
          }
          currentOpcode = &(opcodeTable[0x0FFF]);
        }
        else {
          (void) readMemory(0xFFFF);
          if (!resetFlag)
            currentOpcode--;
          else
            currentOpcode = &(opcodeTable[0x0FFF]);
        }
        break;
      }
      break;
    }
  }

  void M7501::runOneCycle_RDYLow()
  {
    if (interruptDelayRegister != 0x00) {
      interruptFlag = interruptFlag | bool(interruptDelayRegister & 0x01);
      interruptDelayRegister &= (~(uint8_t((reg_SR >> 1) & 0x02)));
      interruptDelayRegister = (interruptDelayRegister & uint8_t(0x04))
                               | (interruptDelayRegister >> 1);
    }
    while (true) {
      unsigned char n = *(currentOpcode++);
      switch (n) {
      case CPU_OP_RD_OPCODE:
        if (PLUS4EMU_EXPECT(!(interruptFlag | resetFlag))) {
          (void) readMemory(reg_PC);
          currentOpcode--;
        }
        else {
          if (PLUS4EMU_UNLIKELY(resetFlag)) {
            if (newPCAddress >= 0) {
              // set new PC if requested
              reg_PC = uint16_t(newPCAddress & 0xFFFF);
              newPCAddress = int32_t(-1);
              resetFlag = false;
              // continue with opcode fetch
              currentOpcode--;
              continue;
            }
            currentOpcode = &(opcodeTable[size_t(0x101) << 4]);
          }
          else
            currentOpcode = &(opcodeTable[size_t(0x100) << 4]);
          continue;
        }
        break;
      case CPU_OP_RD_TMP:
      case CPU_OP_RD_TMP_NODEBUG:
      case CPU_OP_RD_L:
      case CPU_OP_RD_H:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_LD_TMP_MEM:
        (void) readMemory(uint16_t(reg_L) | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_LD_MEM_TMP:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_TMP);
          writeMemory(addr, reg_TMP);
        }
        break;
      case CPU_OP_LD_MEM_TMP_NODEBUG:
        writeMemory(uint16_t(reg_L) | (uint16_t(reg_H) << 8), reg_TMP);
        break;
      case CPU_OP_LD_H_MEMP1_L_TMP:
        (void) readMemory(uint16_t((reg_L + uint8_t(1)) & uint8_t(0xFF))
                          | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_LD_DUMMY_MEM_PC:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_LD_DUMMY_MEM_SP:
        (void) readMemory(uint16_t(0x0100) | uint16_t(reg_SP));
        currentOpcode--;
        break;
      case CPU_OP_PUSH_TMP:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP), reg_TMP);
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_TMP:
        (void) readMemory(uint16_t(0x0100) | uint16_t((reg_SP + 1) & 0xFF));
        currentOpcode--;
        break;
      case CPU_OP_PUSH_PCL:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP),
                    uint8_t(reg_PC) & uint8_t(0xFF));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_PCL:
        (void) readMemory(uint16_t(0x0100) | uint16_t((reg_SP + 1) & 0xFF));
        currentOpcode--;
        break;
      case CPU_OP_PUSH_PCH:
        // FIXME: should check breakpoints ?
        writeMemory(uint16_t(0x0100) | uint16_t(reg_SP),
                    uint8_t(reg_PC >> 8) & uint8_t(0xFF));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_POP_PCH:
        (void) readMemory(uint16_t(0x0100) | uint16_t((reg_SP + 1) & 0xFF));
        currentOpcode--;
        break;
      case CPU_OP_DEC_SP:
        // dummy push operation for reset
        (void) readMemory(uint16_t(0x0100) | uint16_t(reg_SP));
        reg_SP = (reg_SP - uint8_t(1)) & uint8_t(0xFF);
        break;
      case CPU_OP_LD_TMP_SR:
        {
          reg_TMP = reg_SR | uint8_t(0x20);
          continue;
        }
        break;
      case CPU_OP_LD_TMP_A:
        {
          reg_TMP = reg_AC;
          continue;
        }
        break;
      case CPU_OP_LD_PC_HL:
        {
          reg_PC = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          continue;
        }
        break;
      case CPU_OP_LD_SR_TMP:
        {
          reg_SR = reg_TMP | uint8_t(0x30);
          continue;
        }
        break;
      case CPU_OP_LD_A_TMP:
        {
          reg_AC = reg_TMP;
          continue;
        }
        break;
      case CPU_OP_ADDR_X:
        {
          uint8_t tmp = (reg_L + reg_XR) & uint8_t(0xFF);
          if (tmp < reg_L) {
            (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
            currentOpcode--;
          }
          else {
            reg_L = tmp;
            continue;
          }
        }
        break;
      case CPU_OP_ADDR_X_SLOW:
        (void) readMemory(uint16_t((reg_L + reg_XR) & 0xFF)
                          | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_ADDR_X_ZEROPAGE:
        (void) readMemory(uint16_t(reg_L));
        currentOpcode--;
        break;
      case CPU_OP_ADDR_X_SHY:
        (void) readMemory(uint16_t((reg_L + reg_XR) & 0xFF)
                          | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_ADDR_Y:
        {
          uint8_t tmp = (reg_L + reg_YR) & uint8_t(0xFF);
          if (tmp < reg_L) {
            (void) readMemory(uint16_t(tmp) | (uint16_t(reg_H) << 8));
            currentOpcode--;
          }
          else {
            reg_L = tmp;
            continue;
          }
        }
        break;
      case CPU_OP_ADDR_Y_SLOW:
        (void) readMemory(uint16_t((reg_L + reg_YR) & 0xFF)
                          | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_ADDR_Y_ZEROPAGE:
        (void) readMemory(uint16_t(reg_L));
        currentOpcode--;
        break;
      case CPU_OP_ADDR_Y_SHA:
      case CPU_OP_ADDR_Y_SHS:
      case CPU_OP_ADDR_Y_SHX:
        (void) readMemory(uint16_t((reg_L + reg_YR) & 0xFF)
                          | (uint16_t(reg_H) << 8));
        currentOpcode--;
        break;
      case CPU_OP_SET_NZ:
        {
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_ADC:
        {
          if (!(reg_SR & uint8_t(0x08))) {
            // add in binary mode
            unsigned int  result = (unsigned int) (reg_AC & uint8_t(0xFF))
                                   + (unsigned int) (reg_TMP & uint8_t(0xFF))
                                   + (unsigned int) (reg_SR & uint8_t(0x01));
            reg_SR &= uint8_t(0x3C);
            reg_SR |= (uint8_t(result >> 8) & uint8_t(0x01));           // C
            reg_SR |= ((((reg_AC ^ ~reg_TMP) & (reg_AC ^ uint8_t(result)))
                        & uint8_t(0x80)) >> 1);                         // V
            reg_AC = uint8_t(result) & uint8_t(0xFF);
            reg_SR |= (reg_AC & uint8_t(0x80));                         // N
            reg_SR |= uint8_t(reg_AC == uint8_t(0x00) ? 0x02 : 0x00);   // Z
          }
          else {
            // add in BCD mode
            unsigned int  l = (unsigned int) (reg_SR & uint8_t(0x01))
                              + (unsigned int) (reg_AC & uint8_t(0x0F))
                              + (unsigned int) (reg_TMP & uint8_t(0x0F));
            unsigned int  h = (unsigned int) (reg_AC & uint8_t(0xF0))
                              + (unsigned int) (reg_TMP & uint8_t(0xF0));
            reg_SR &= uint8_t(0x3C);
            if (!(uint8_t(h + l) & uint8_t(0xFF)))
              reg_SR |= uint8_t(0x02);                                  // Z
            l += (l < 0x0AU ? 0x00U : 0x06U);
            h += (l >= 0x10U ? 0x10U : 0x00U);
            l &= 0x0FU;
            reg_SR = reg_SR | (uint8_t(h) & uint8_t(0x80));             // N
            reg_SR |= ((((reg_AC ^ ~reg_TMP) & (reg_AC ^ uint8_t(h)))
                        & uint8_t(0x80)) >> 1);                         // V
            h += (h < 0xA0U ? 0x00U : 0x60U);
            reg_SR |= (h >= 0x0100U ? uint8_t(0x01) : uint8_t(0x00));   // C
            reg_AC = uint8_t(h + l) & uint8_t(0xFF);
          }
          continue;
        }
        break;
      case CPU_OP_ANC:
        {
          reg_AC = reg_AC & reg_TMP;
          reg_SR &= uint8_t(0x7C);
          if (reg_AC & uint8_t(0x80))
            reg_SR |= uint8_t(0x81);
          else if (!reg_AC)
            reg_SR |= uint8_t(0x02);
          continue;
        }
        break;
      case CPU_OP_AND:
        {
          reg_AC = reg_AC & reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ANE:
        {
          reg_AC = (reg_AC | uint8_t(0xEE)) & (reg_XR & reg_TMP);
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ARR:
        {
          uint8_t tmp = reg_AC & reg_TMP;
          reg_AC = tmp >> 1;
          if (reg_SR & uint8_t(0x01))
            reg_AC |= uint8_t(0x80);
          reg_SR &= uint8_t(0x3C);
          reg_SR |= ((reg_AC ^ tmp) & uint8_t(0x40));
          reg_SR |= (reg_AC & uint8_t(0x80));
          if (!reg_AC)
            reg_SR |= uint8_t(0x02);
          if (!(reg_SR & uint8_t(0x08))) {
            reg_SR |= ((tmp & uint8_t(0x80)) >> 7);
          }
          else {
            if ((tmp & uint8_t(0x0F)) >= uint8_t(0x05))
              reg_AC = uint8_t((reg_AC & 0xF0) | ((reg_AC + 0x06) & 0x0F));
            if (tmp >= uint8_t(0x50)) {
              reg_AC = uint8_t((reg_AC + 0x60) & 0xFF);
              reg_SR |= uint8_t(0x01);
            }
          }
          continue;
        }
        break;
      case CPU_OP_ASL:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = tmp << 1;
          reg_SR = (reg_SR & uint8_t(0x7C)) | uint8_t(tmp >> 8);
          reg_TMP = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_BCC:
        if (reg_SR & uint8_t(0x01)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BCS:
        if (!(reg_SR & uint8_t(0x01))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BEQ:
        if (!(reg_SR & uint8_t(0x02))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BIT:
        {
          reg_SR = (reg_SR & uint8_t(0x3D)) | (reg_TMP & uint8_t(0xC0));
          reg_SR = reg_SR | ((reg_AC & reg_TMP) == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_BMI:
        if (!(reg_SR & uint8_t(0x80))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BNE:
        if (reg_SR & uint8_t(0x02)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BPL:
        if (reg_SR & uint8_t(0x80)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BRK:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          reg_TMP = reg_SR | uint8_t(0x10);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFE);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_BVC:
        if (reg_SR & uint8_t(0x40)) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_BVS:
        if (!(reg_SR & uint8_t(0x40))) {
          currentOpcode++;
          continue;
        }
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_CLC:
      case CPU_OP_CLD:
      case CPU_OP_CLI:
      case CPU_OP_CLV:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_CMP:
        {
          uint16_t  tmp = uint16_t(reg_AC)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_CPX:
        {
          uint16_t  tmp = uint16_t(reg_XR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_CPY:
        {
          uint16_t  tmp = uint16_t(reg_YR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (uint8_t(tmp) & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          tmp = tmp & uint16_t(0xFF);
          reg_SR = reg_SR | (tmp == uint16_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_DEC:
        {
          reg_TMP = (reg_TMP - uint8_t(1)) & uint8_t(0xFF);
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_DEX:
      case CPU_OP_DEY:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_EOR:
        {
          reg_AC = reg_AC ^ reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_INC:
        {
          reg_TMP = (reg_TMP + uint8_t(1)) & uint8_t(0xFF);
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_INTERRUPT:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          reg_TMP = reg_SR & uint8_t(0xEF);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFE);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_INX:
      case CPU_OP_INY:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_JMP_RELATIVE:
        {
          unsigned int  tmp = reg_PC + int(int8_t(reg_L));
          if ((tmp ^ reg_PC) & 0xFF00U) {
            (void) readMemory((tmp & 0x00FFU) | (reg_PC & 0xFF00U));
            currentOpcode--;
          }
          else {
            reg_PC = uint16_t(tmp & 0xFFFFU);
            continue;
          }
        }
        break;
      case CPU_OP_LAS:
        {
          reg_AC = reg_SP & reg_TMP;
          reg_XR = reg_AC;
          reg_SP = reg_AC;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_LAX:
        {
          reg_AC = reg_TMP;
          reg_XR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDA:
        {
          reg_AC = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDX:
        {
          reg_XR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LDY:
        {
          reg_YR = reg_TMP;
          setFlagsNZ(reg_TMP);
          continue;
        }
        break;
      case CPU_OP_LSR:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          reg_SR = (reg_SR & uint8_t(0x7C)) | (uint8_t(tmp) & uint8_t(0x01));
          reg_TMP = uint8_t(tmp >> 1);
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_LXA:
        {
          reg_AC = (reg_AC | uint8_t(0xEE)) & reg_TMP;
          reg_XR = reg_AC;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_ORA:
        {
          reg_AC = reg_AC | reg_TMP;
          setFlagsNZ(reg_AC);
          continue;
        }
        break;
      case CPU_OP_RESET:
        {
          interruptDelayRegister &= uint8_t(0xFC);
          interruptFlag = false;
          resetFlag = false;
          reg_TMP = reg_SR & uint8_t(0xEF);
          reg_SR = reg_SR | uint8_t(0x34);
          reg_L = uint8_t(0xFC);
          reg_H = uint8_t(0xFF);
          continue;
        }
        break;
      case CPU_OP_ROL:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = (tmp << 1) | uint16_t(reg_SR & uint8_t(0x01));
          reg_SR = (reg_SR & uint8_t(0x7C)) | uint8_t(tmp >> 8);
          reg_TMP = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_ROR:
        {
          uint16_t  tmp = uint16_t(reg_TMP);
          tmp = tmp | (uint16_t(reg_SR & uint8_t(0x01)) << 8);
          reg_SR = (reg_SR & uint8_t(0x7C)) | (uint8_t(tmp) & uint8_t(0x01));
          reg_TMP = uint8_t(tmp >> 1);
          reg_SR = reg_SR | (reg_TMP & uint8_t(0x80));
          reg_SR = reg_SR | (reg_TMP == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_SAX:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          uint8_t   tmp = reg_AC & reg_XR;
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, tmp);
          writeMemory(addr, tmp);
        }
        break;
      case CPU_OP_SBC:
        {
          // subtract in binary mode
          uint8_t       tmp = (reg_TMP ^ uint8_t(0xFF)) & uint8_t(0xFF);
          uint8_t       c = reg_SR & uint8_t(0x01);
          unsigned int  result = (unsigned int) (reg_AC & uint8_t(0xFF))
                                 + (unsigned int) tmp
                                 + (unsigned int) c;
          reg_SR &= uint8_t(0x3C);
          reg_SR |= (uint8_t(result >> 8) & uint8_t(0x01));             // C
          reg_SR |= ((((reg_AC ^ ~tmp) & (reg_AC ^ uint8_t(result)))
                      & uint8_t(0x80)) >> 1);                           // V
          result &= 0xFFU;
          reg_SR |= (uint8_t(result) & uint8_t(0x80));                  // N
          reg_SR |= uint8_t(result == 0x00U ? 0x02 : 0x00);             // Z
          if (reg_SR & uint8_t(0x08)) {
            // subtract in BCD mode
            unsigned int  l = (unsigned int) (reg_AC & uint8_t(0x0F))
                              + (unsigned int) (tmp & uint8_t(0x0F))
                              + (unsigned int) c;
            unsigned int  h = (unsigned int) (reg_AC & uint8_t(0xF0))
                              + (unsigned int) (tmp & uint8_t(0xF0));
            h += (l & 0x10U);
            l += (l < 0x10U ? 0x0AU : 0x00U);
            h += (h < 0x100U ? 0xA0U : 0x00U);
            result = (h + (l & 0x0FU)) & 0xFFU;
          }
          reg_AC = uint8_t(result);
          continue;
        }
        break;
      case CPU_OP_SBX:
        {
          uint16_t  tmp = uint16_t(reg_AC & reg_XR)
                          + uint16_t(reg_TMP ^ uint8_t(0xFF));
          tmp++;
          reg_XR = uint8_t(tmp) & uint8_t(0xFF);
          reg_SR = reg_SR & uint8_t(0x7C);
          reg_SR = reg_SR | (reg_XR & uint8_t(0x80));
          reg_SR = reg_SR | (uint8_t(tmp >> 8) & uint8_t(0x01));
          reg_SR = reg_SR | (reg_XR == uint8_t(0) ?
                             uint8_t(0x02) : uint8_t(0x00));
          continue;
        }
        break;
      case CPU_OP_SEC:
      case CPU_OP_SED:
      case CPU_OP_SEI:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_STA:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_AC);
          writeMemory(addr, reg_AC);
        }
        break;
      case CPU_OP_STX:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_XR);
          writeMemory(addr, reg_XR);
        }
        break;
      case CPU_OP_STY:
        {
          uint16_t  addr = uint16_t(reg_L) | (uint16_t(reg_H) << 8);
          if (haveBreakPoints)
            checkWriteBreakPoint(addr, reg_YR);
          writeMemory(addr, reg_YR);
        }
        break;
      case CPU_OP_TAX:
      case CPU_OP_TAY:
      case CPU_OP_TSX:
      case CPU_OP_TXA:
      case CPU_OP_TXS:
      case CPU_OP_TYA:
        (void) readMemory(reg_PC);
        currentOpcode--;
        break;
      case CPU_OP_INVALID_OPCODE:
        if (breakOnInvalidOpcode) {
          (void) readMemory((reg_PC - 1) & 0xFFFF);
          currentOpcode--;
        }
        else {
          (void) readMemory(0xFFFF);
          if (!resetFlag)
            currentOpcode--;
          else
            currentOpcode = &(opcodeTable[0x0FFF]);
        }
        break;
      }
      break;
    }
  }

  void M7501::reset(bool isColdReset)
  {
    resetFlag = true;
    newPCAddress = int32_t(-1);
    if (isColdReset) {
      reg_SR = uint8_t(0x24);
      reg_SP = uint8_t(0xFF);
      currentOpcode = &(opcodeTable[0x0FFF]);
      interruptDelayRegister = 0x00;
      interruptFlag = false;
      haltFlag = false;
    }
  }

  // --------------------------------------------------------------------------

  void M7501::checkOpcodeReadBreakPoint(uint16_t addr, uint8_t value)
  {
    if (haveBreakPoints && (singleStepMode == 0 || singleStepMode == 3)) {
      uint8_t *tbl = breakPointTable;
      if (tbl[addr] >= breakPointPriorityThreshold && (tbl[addr] & 12) == 4) {
        breakPointCallback(0, addr, value);
        return;
      }
      if (!singleStepMode)
        return;
    }
    if (singleStepMode == 2) {                  // step over mode
      if (singleStepModeNextAddr >= int32_t(0) &&
          singleStepModeNextAddr != int32_t(addr)) {
        return;
      }
      if (value == 0x20)
        singleStepModeNextAddr = (addr + 3) & 0xFFFF;
      else if ((value & 0x1F) == 0x10)
        singleStepModeNextAddr = (addr + 2) & 0xFFFF;
      else
        singleStepModeNextAddr = int32_t(-1);
    }
    else if (singleStepMode == 4) {             // step into mode
      if (singleStepModeNextAddr >= int32_t(0) &&
          singleStepModeNextAddr != int32_t(addr)) {
        return;
      }
      if ((value & 0x1F) == 0x10) {
        uint16_t  tmp = uint16_t(readMemory((addr + 1) & 0xFFFF));
        if (tmp >= 0x80)
          tmp = tmp | 0xFF00;
        singleStepModeNextAddr = int32_t((addr + 2 + tmp) & 0xFFFF);
      }
      else
        singleStepModeNextAddr = int32_t(-1);
    }
    if (breakPointTable != (uint8_t *) 0) {
      if (breakPointTable[addr] & 8)
        return;
    }
    breakPointCallback(3, addr, value);
  }

  void M7501::checkReadBreakPoint(uint16_t addr, uint8_t value)
  {
    if (!(singleStepMode == 1 || singleStepMode == 2)) {
      uint8_t *tbl = breakPointTable;
      if (tbl[addr] >= breakPointPriorityThreshold && (tbl[addr] & 1) != 0) {
        if (!(tbl[reg_PC] & 8))
          breakPointCallback(1, addr, value);
      }
    }
  }

  void M7501::checkWriteBreakPoint(uint16_t addr, uint8_t value)
  {
    if (!(singleStepMode == 1 || singleStepMode == 2)) {
      uint8_t *tbl = breakPointTable;
      if (tbl[addr] >= breakPointPriorityThreshold && (tbl[addr] & 2) != 0) {
        if (!(tbl[reg_PC] & 8))
          breakPointCallback(2, addr, value);
      }
    }
  }

  void M7501::setBreakPoint(int type, uint16_t addr, int priority)
  {
    if (priority >= 0) {
      // create new breakpoint, or change existing one
      uint8_t mode = uint8_t(type & 15);
      if (PLUS4EMU_UNLIKELY(!mode))
        return;
      if (PLUS4EMU_UNLIKELY(!breakPointTable)) {
        breakPointTable = new uint8_t[65536];
        haveBreakPoints = true;
        clearBreakPoints();
      }
      haveBreakPoints = true;
      uint8_t&  bp = breakPointTable[addr];
      if (!bp)
        breakPointCnt++;
      bp = bp | mode;
      mode = mode | uint8_t((priority < 3 ? priority : 3) << 4);
      if (mode > bp)
        bp = mode;
    }
    else if (breakPointTable) {
      if (breakPointTable[addr]) {
        // remove a previously existing breakpoint
        breakPointTable[addr] = 0;
        breakPointCnt--;
        haveBreakPoints = bool(breakPointCnt);
      }
    }
  }

  void M7501::clearBreakPoints()
  {
    if (haveBreakPoints) {
      breakPointCnt = 0U;
      haveBreakPoints = false;
      std::memset(breakPointTable, 0x00, sizeof(uint8_t) * 65536);
    }
  }

  void M7501::setBreakPointPriorityThreshold(int n)
  {
    breakPointPriorityThreshold = uint8_t((n > 0 ? (n < 4 ? n : 4) : 0) << 4);
  }

  Plus4Emu::BreakPointList M7501::getBreakPointList()
  {
    Plus4Emu::BreakPointList  bplst;
    if (breakPointTable) {
      for (size_t i = 0; i < 65536; i++) {
        uint8_t bp = breakPointTable[i];
        if (bp)
          bplst.addBreakPoint(bp, uint16_t(i));
      }
    }
    return bplst;
  }

  void M7501::setSingleStepMode(int mode_)
  {
    mode_ = (mode_ >= 0 && mode_ <= 4 ? mode_ : 0);
    if (mode_ == int(singleStepMode))
      return;
    singleStepMode = uint8_t(mode_);
    singleStepModeNextAddr = int32_t(-1);
    if (mode_ != 2 && mode_ != 4)
      return;
    // "step over" or "step into" mode:
    size_t    tablePos = size_t(currentOpcode - &(opcodeTable[0]));
    uint16_t  nxtOpcodeAddr = reg_PC;
    if (opcodeTable[tablePos] == CPU_OP_RD_OPCODE) {
      // read next opcode if needed
      tablePos = size_t(readMemory(nxtOpcodeAddr)) << 4;
      nxtOpcodeAddr = (nxtOpcodeAddr + 1) & 0xFFFF;
    }
    int     opNum = int(tablePos >> 4);
    if ((mode_ == 2 && opNum == 0x20) || (opNum & 0x1F) == 0x10) {
      // if JSR or branch opcode, find the beginning of the next instruction
      while (opcodeTable[tablePos] != CPU_OP_RD_OPCODE) {
        if (opcodeTable[tablePos] == CPU_OP_RD_TMP ||
            opcodeTable[tablePos] == CPU_OP_RD_TMP_NODEBUG ||
            opcodeTable[tablePos] == CPU_OP_RD_L ||
            opcodeTable[tablePos] == CPU_OP_RD_H) {
          nxtOpcodeAddr = (nxtOpcodeAddr + 1) & 0xFFFF;
        }
        tablePos++;
      }
      if (mode_ != 2) {
        // "step into" mode: calculate branch target address
        uint16_t  tmp = uint16_t(readMemory((nxtOpcodeAddr - 1) & 0xFFFF));
        if (tmp >= 0x80)
          tmp = tmp | 0xFF00;
        nxtOpcodeAddr = (nxtOpcodeAddr + tmp) & 0xFFFF;
      }
      singleStepModeNextAddr = int32_t(nxtOpcodeAddr);
    }
  }

  void M7501::setSingleStepModeNextAddress(int32_t addr)
  {
    if ((singleStepMode != 2 && singleStepMode != 4) || addr < 0)
      addr = int32_t(-1);
    else
      addr &= int32_t(0xFFFF);
    singleStepModeNextAddr = addr;
  }

  void M7501::setRegisters(const M7501Registers& r)
  {
    if (r.reg_PC != reg_PC) {
      resetFlag = true;
      newPCAddress = int32_t(r.reg_PC);
    }
    reg_SR = r.reg_SR | uint8_t(0x30);
    reg_AC = r.reg_AC;
    reg_XR = r.reg_XR;
    reg_YR = r.reg_YR;
    reg_SP = r.reg_SP;
  }

  void M7501::getRegisters(M7501Registers& r) const
  {
    if (newPCAddress < 0)
      r.reg_PC = reg_PC;
    else
      r.reg_PC = uint16_t(newPCAddress & 0xFFFF);
    r.reg_SR = reg_SR;
    r.reg_AC = reg_AC;
    r.reg_XR = reg_XR;
    r.reg_YR = reg_YR;
    r.reg_SP = reg_SP;
  }

  void M7501::breakPointCallback(int type, uint16_t addr, uint8_t value)
  {
    (void) type;
    (void) addr;
    (void) value;
  }

  // --------------------------------------------------------------------------

  class ChunkType_M7501Snapshot : public Plus4Emu::File::ChunkTypeHandler {
   private:
    M7501&  ref;
   public:
    ChunkType_M7501Snapshot(M7501& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_M7501Snapshot()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_M7501_STATE;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  void M7501::saveState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    buf.writeUInt32(0x01000002);        // version number
    buf.writeByte(uint8_t(reg_PC) & 0xFF);
    buf.writeByte(uint8_t(reg_PC >> 8));
    buf.writeByte(reg_SR);
    buf.writeByte(reg_AC);
    buf.writeByte(reg_XR);
    buf.writeByte(reg_YR);
    buf.writeByte(reg_SP);
    buf.writeByte(reg_TMP);
    buf.writeByte(reg_L);
    buf.writeByte(reg_H);
    buf.writeUInt32(uint32_t(currentOpcode - &(opcodeTable[0])));
    buf.writeUInt32(uint32_t(interruptDelayRegister));
    buf.writeBoolean(interruptFlag);
    buf.writeBoolean(resetFlag);
    buf.writeBoolean(haltFlag);
    buf.writeInt32(newPCAddress);
  }

  void M7501::saveState(Plus4Emu::File& f)
  {
    Plus4Emu::File::Buffer  buf;
    this->saveState(buf);
    f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_M7501_STATE, buf);
  }

  void M7501::loadState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (!(version >= 0x01000000 && version <= 0x01000002)) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible M7501 snapshot format");
    }
    try {
      // load saved state
      reg_PC = (reg_PC & uint16_t(0xFF00)) | uint16_t(buf.readByte());
      reg_PC = (reg_PC & uint16_t(0x00FF)) | (uint16_t(buf.readByte()) << 8);
      reg_SR = buf.readByte();
      reg_AC = buf.readByte();
      reg_XR = buf.readByte();
      reg_YR = buf.readByte();
      reg_SP = buf.readByte();
      reg_TMP = buf.readByte();
      reg_L = buf.readByte();
      reg_H = buf.readByte();
      uint32_t  currentOpcodeIndex = buf.readUInt32();
      if (version >= 0x01000002) {
        interruptDelayRegister = uint8_t(buf.readUInt32() & 0x07U);
      }
      else {
        interruptDelayRegister &= uint8_t(0x04);
        interruptDelayRegister |= uint8_t(buf.readUInt32() & 0x03U);
      }
      interruptFlag = buf.readBoolean();
      resetFlag = buf.readBoolean();
      haltFlag = buf.readBoolean();
      if (version != 0x01000000)
        newPCAddress = buf.readInt32();
      else
        newPCAddress = int32_t(-1);
      if (currentOpcodeIndex < 4128U)
        currentOpcode = &(opcodeTable[currentOpcodeIndex]);
      else
        this->reset(true);
      if (buf.getPosition() != buf.getDataSize())
        throw Plus4Emu::Exception("trailing garbage at end of "
                                  "M7501 snapshot data");
    }
    catch (...) {
      try {
        this->reset(true);
      }
      catch (...) {
      }
      throw;
    }
  }

  void M7501::registerChunkType(Plus4Emu::File& f)
  {
    ChunkType_M7501Snapshot *p;
    p = new ChunkType_M7501Snapshot(*this);
    try {
      f.registerChunkType(p);
    }
    catch (...) {
      delete p;
      throw;
    }
  }

}       // namespace Plus4

