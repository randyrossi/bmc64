
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

#ifndef PLUS4EMU_DISASM_HPP
#define PLUS4EMU_DISASM_HPP

#include "plus4emu.hpp"
#include "vm.hpp"

namespace Plus4 {

  class M7501Disassembler {
   private:
    static const char *opcodeNames;
    static const unsigned char opcodeToName[256];
    static const unsigned char opcodeToAddrMode[256];
    static const char *formatTable[2][12];
   public:
    // Disassemble one M65xx/75xx/85xx instruction, reading from memory
    // of virtual machine 'vm', starting at address 'addr', and write the
    // result to 'buf' (not including a newline character). 'offs' is added
    // to the instruction address that is printed. The maximum line width
    // is 34 characters.
    // Returns the address of the next instruction. If 'isCPUAddress' is
    // true, 'addr' is interpreted as a 16-bit CPU address, otherwise it
    // is assumed to be a 22-bit physical address (8 bit segment + 14 bit
    // offset).
    static uint32_t disassembleInstruction(std::string& buf,
                                           const Plus4Emu::VirtualMachine& vm,
                                           uint32_t addr,
                                           bool isCPUAddress = false,
                                           int32_t offs = 0);
    // Same as disassembleInstruction() without actually writing to a string.
    static uint32_t getNextInstructionAddr(const Plus4Emu::VirtualMachine& vm,
                                           uint32_t addr,
                                           bool isCPUAddress = false);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_DISASM_HPP

