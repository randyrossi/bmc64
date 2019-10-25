
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

#ifndef PLUS4EMU_BPLIST_HPP
#define PLUS4EMU_BPLIST_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include <vector>

namespace Plus4Emu {

  class BreakPoint {
   private:
    // 0x00000000: no break (disabled)
    // 0x01000000: memory read (16 bit address)
    // 0x02000000: memory write (16 bit address)
    // 0x03000000: any memory access (read, write or CPU opcode read)
    // 0x04000000: video (16 bit address: bits 7..15 for Y, bits 0..6 for X)
    // 0x05000000: ignore other breakpoints if PC is at this address
    // 0x06000000: CPU opcode read
    // + priority (0 to 3) * 0x00400000
    // + address
    uint32_t  n_;
   public:
    // allowed values for 'type_':
    //   0: memory read/write/execute
    //   1: memory read
    //   2: memory write
    //   3: memory read/write/execute
    //   4: video
    //   5: ignore
    //   6: opcode read
    BreakPoint(int type_, uint16_t addr_, int priority_)
    {
      if (type_ >= 1 && type_ <= 6)
        this->n_ = uint32_t(type_) << 24;
      else
        this->n_ = 0x03000000U;
      if (priority_ > 0) {
        if (priority_ < 3)
          this->n_ |= (uint32_t(priority_) << 22);
        else
          this->n_ |= uint32_t(0x00C00000);
      }
      this->n_ |= uint32_t(addr_ & 0xFFFF);
      if (type_ == 5)
        this->n_ |= uint32_t(0x00C00000);
    }
    int type() const
    {
      return int(this->n_ >> 24);
    }
    int priority() const
    {
      return int((this->n_ & 0x00C00000) >> 22);
    }
    uint16_t addr() const
    {
      return uint16_t(this->n_ & 0xFFFF);
    }
    bool operator<(const BreakPoint& bp) const
    {
      return (this->n_ < bp.n_);
    }
  };

  class BreakPointList {
   private:
    std::vector<BreakPoint> lst_;
    // --------
    void parseBreakPoint(std::map< uint32_t, uint8_t >& bpList,
                         const std::string& s);
   public:
    BreakPointList()
    {
    }
    // Create breakpoint list from ASCII format breakpoint definitions,
    // separated by any whitespace characters (space, tab, or newline).
    // A breakpoint definition consists of an address or address range in
    // one of the following formats (each 'n' is a hexadecimal digit):
    //   nnnn           a single CPU memory address
    //   nnnn-nnnn      all CPU memory addresses in the specified range
    //   nnnn:nn        a single video position, as Y:X
    //   nnnn:nn-nn     range of video positions (Y:first_X-last_X)
    // and these optional modifiers:
    //   r              the breakpoint is triggered on data reads
    //   w              the breakpoint is triggered on writes
    //   x              the breakpoint is triggered on opcode reads
    //   p0             the breakpoint has a priority of 0
    //   p1             the breakpoint has a priority of 1
    //   p2             the breakpoint has a priority of 2
    //   p3             the breakpoint has a priority of 3
    //   i              ignore other breakpoints if the program counter
    //                  is at an address for which this breakpoint is set
    //                  (read/write flags and priority are not used in
    //                  this case)
    // by default, the breakpoint is triggered on both reads and writes if
    // 'r', 'w' or 'x' is not used, and has a priority of 2.
    // Example: 8000-8003rp1 means break on reading CPU addresses 0x8000,
    // 0x8001, 0x8002, and 0x8003, if the breakpoint priority threshold is
    // less than or equal to 1.
    // If there are any syntax errors in the list, Plus4Emu::Exception is
    // thrown, and no breakpoints are added.
    BreakPointList(const std::string& lst);
    // allowed values for type:
    //   0: memory read/write/execute
    //   1: memory read
    //   2: memory write
    //   3: memory read/write/execute
    //   4: video (address bits 7..15 for Y, bits 0..6 for X)
    //   5: ignore other breakpoints if PC is at this address
    //   6: CPU opcode read
    void addBreakPoint(int type, uint16_t addr, int priority);
    // Add memory breakpoint(s) at 'addr', bpFlags is the sum of
    // at least one of:
    //   1: read
    //   2: write
    //   4: execute
    //   8: ignore
    // and the priority (0 to 3) multiplied by 16.
    void addBreakPoint(uint8_t bpFlags, uint16_t addr);
    size_t getBreakPointCnt() const
    {
      return this->lst_.size();
    }
    const BreakPoint& getBreakPoint(size_t ndx) const
    {
      return ((const BreakPointList *) this)->lst_.at(ndx);
    }
    void saveState(File::Buffer&);
    void saveState(File&);
    void loadState(File::Buffer&);
    void registerChunkType(File&);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_BPLIST_HPP

