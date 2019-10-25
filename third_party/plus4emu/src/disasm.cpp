
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
#include "vm.hpp"
#include "disasm.hpp"

namespace Plus4 {

  const char * M7501Disassembler::opcodeNames =
    " ??? ADC*ANC AND*ANE*ARR ASL*ASR BCC BCS"
    " BEQ BIT BMI BNE BPL BRK BVC BVS CLC CLD"
    " CLI CLV CMP CPX CPY*DCP DEC DEX DEY EOR"
    " INC INX INY*ISB JMP JSR*LAS*LAX LDA LDX"
    " LDY LSR*LXA NOP*NOP ORA PHA PHP PLA PLP"
    "*RLA ROL ROR*RRA RTI RTS*SAX SBC*SBC*SBX"
    " SEC SED SEI*SHA*SHS*SHX*SHY*SLO*SRE STA"
    " STX STY TAX TAY TSX TXA TXS TYA";

  const unsigned char M7501Disassembler::opcodeToName[256] = {
    15, 45,  0, 67,  44, 45,  6, 67,  47, 45,  6,  2,  44, 45,  6, 67,
    14, 45,  0, 67,  44, 45,  6, 67,  18, 45, 44, 67,  44, 45,  6, 67,
    35,  3,  0, 50,  11,  3, 51, 50,  49,  3, 51,  2,  11,  3, 51, 50,
    12,  3,  0, 50,  44,  3, 51, 50,  60,  3, 44, 50,  44,  3, 51, 50,
    54, 29,  0, 68,  44, 29, 41, 68,  46, 29, 41,  7,  34, 29, 41, 68,
    16, 29,  0, 68,  44, 29, 41, 68,  20, 29, 44, 68,  44, 29, 41, 68,
    55,  1,  0, 53,  44,  1, 52, 53,  48,  1, 52,  5,  34,  1, 52, 53,
    17,  1,  0, 53,  44,  1, 52, 53,  62,  1, 44, 53,  44,  1, 52, 53,
    44, 69, 44, 56,  71, 69, 70, 56,  28, 44, 75,  4,  71, 69, 70, 56,
     8, 69,  0, 63,  71, 69, 70, 56,  77, 69, 76, 64,  66, 69, 65, 63,
    40, 38, 39, 37,  40, 38, 39, 37,  73, 38, 72, 42,  40, 38, 39, 37,
     9, 38,  0, 37,  40, 38, 39, 37,  21, 38, 74, 36,  40, 38, 39, 37,
    24, 22,  0, 25,  24, 22, 26, 25,  32, 22, 27, 59,  24, 22, 26, 25,
    13, 22,  0, 25,  44, 22, 26, 25,  19, 22, 44, 25,  44, 22, 26, 25,
    23, 57,  0, 33,  23, 57, 30, 33,  31, 57, 43, 58,  23, 57, 30, 33,
    10, 57,  0, 33,  44, 57, 30, 33,  61, 57, 44, 33,  44, 57, 30, 33
  };

  //  0: implied
  //  1: #nn
  //  2: nn     (zeropage)
  //  3: nn     (relative)
  //  4: nn, X
  //  5: nn, Y
  //  6: (nn, X)
  //  7: (nn), Y
  //  8: nnnn
  //  9: (nnnn)
  // 10: nnnn, X
  // 11: nnnn, Y

  const unsigned char M7501Disassembler::opcodeToAddrMode[256] = {
     0,  6,  0,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10,
     8,  6,  0,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10,
     0,  6,  0,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10,
     0,  6,  0,  6,   2,  2,  2,  2,   0,  1,  0,  1,   9,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10,
     1,  6,  1,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  5,  5,   0, 11,  0, 11,  10, 10, 11, 11,
     1,  6,  1,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  5,  5,   0, 11,  0, 11,  10, 10, 11, 11,
     1,  6,  1,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10,
     1,  6,  1,  6,   2,  2,  2,  2,   0,  1,  0,  1,   8,  8,  8,  8,
     3,  7,  0,  7,   4,  4,  4,  4,   0, 11,  0, 11,  10, 10, 10, 10
  };

  const char * M7501Disassembler::formatTable[2][12] = {
    { "  %04X  %02X           %.4s",
      "  %04X  %02X %02X        %.4s  #$%02X",
      "  %04X  %02X %02X        %.4s  $%02X",
      "  %04X  %02X %02X        %.4s  $%04X",
      "  %04X  %02X %02X        %.4s  $%02X, X",
      "  %04X  %02X %02X        %.4s  $%02X, Y",
      "  %04X  %02X %02X        %.4s  ($%02X, X)",
      "  %04X  %02X %02X        %.4s  ($%02X), Y",
      "  %04X  %02X %02X %02X     %.4s  $%04X",
      "  %04X  %02X %02X %02X     %.4s  ($%04X)",
      "  %04X  %02X %02X %02X     %.4s  $%04X, X",
      "  %04X  %02X %02X %02X     %.4s  $%04X, Y"   },
    { "%06X  %02X           %.4s",
      "%06X  %02X %02X        %.4s  #$%02X",
      "%06X  %02X %02X        %.4s  $%02X",
      "%06X  %02X %02X        %.4s  $%06X",
      "%06X  %02X %02X        %.4s  $%02X, X",
      "%06X  %02X %02X        %.4s  $%02X, Y",
      "%06X  %02X %02X        %.4s  ($%02X, X)",
      "%06X  %02X %02X        %.4s  ($%02X), Y",
      "%06X  %02X %02X %02X     %.4s  $%04X",
      "%06X  %02X %02X %02X     %.4s  ($%04X)",
      "%06X  %02X %02X %02X     %.4s  $%04X, X",
      "%06X  %02X %02X %02X     %.4s  $%04X, Y"     }
  };

  uint32_t M7501Disassembler::disassembleInstruction(
      std::string& buf, const Plus4Emu::VirtualMachine& vm,
      uint32_t addr, bool isCPUAddress, int32_t offs)
  {
    char      tmpBuf[40];
    uint32_t  addrMask = (isCPUAddress ? 0x0000FFFFU : 0x003FFFFFU);
    addr &= addrMask;
    uint32_t  baseAddr = (addr + uint32_t(offs)) & addrMask;
    uint8_t   opNum = vm.readMemory(addr, isCPUAddress) & 0xFF;
    uint8_t   d1 = 0;
    uint8_t   d2 = 0;
    addr = (addr + 1U) & addrMask;
    uint32_t  operand = 0U;
    unsigned char addrMode = opcodeToAddrMode[opNum];
    const char    *opName = &(opcodeNames[opcodeToName[opNum] << 2]);
    if (addrMode >= 1) {
      d1 = vm.readMemory(addr, isCPUAddress);
      operand = d1;
      addr = (addr + 1U) & addrMask;
      if (addrMode >= 8) {
        d2 = vm.readMemory(addr, isCPUAddress);
        operand |= (uint32_t(d2) << 8);
        addr = (addr + 1U) & addrMask;
      }
      else if (addrMode == 3) {
        // relative address for branch instructions
        if (operand & uint32_t(0x80))
          operand |= uint32_t(0xFFFFFF00UL);
        operand = (addr + operand + uint32_t(offs)) & addrMask;
      }
    }
    if (addrMode == 0) {
      std::sprintf(&(tmpBuf[0]), formatTable[(isCPUAddress ? 0 : 1)][addrMode],
                   (unsigned int) baseAddr,
                   (unsigned int) opNum,
                   opName);
    }
    else if (addrMode < 8) {
      std::sprintf(&(tmpBuf[0]), formatTable[(isCPUAddress ? 0 : 1)][addrMode],
                   (unsigned int) baseAddr,
                   (unsigned int) opNum, (unsigned int) d1,
                   opName, (unsigned int) operand);
    }
    else {
      std::sprintf(&(tmpBuf[0]), formatTable[(isCPUAddress ? 0 : 1)][addrMode],
                   (unsigned int) baseAddr,
                   (unsigned int) opNum, (unsigned int) d1, (unsigned int) d2,
                   opName, (unsigned int) operand);
    }
    buf = &(tmpBuf[0]);
    return addr;
  }

  uint32_t M7501Disassembler::getNextInstructionAddr(
      const Plus4Emu::VirtualMachine& vm, uint32_t addr, bool isCPUAddress)
  {
    uint32_t  addrMask = (isCPUAddress ? 0x0000FFFFU : 0x003FFFFFU);
    addr &= addrMask;
    uint8_t   opNum = vm.readMemory(addr, isCPUAddress) & 0xFF;
    unsigned char addrMode = opcodeToAddrMode[opNum];
    if (addrMode < 1)
      return ((addr + 1U) & addrMask);
    else if (addrMode >= 8)
      return ((addr + 3U) & addrMask);
    return ((addr + 2U) & addrMask);
  }

}       // namespace Plus4

