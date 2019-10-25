
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

#include "plus4emu.hpp"
#include "bplist.hpp"

#include <map>

namespace Plus4Emu {

  void BreakPointList::parseBreakPoint(std::map< uint32_t, uint8_t >& bpList,
                                       const std::string& s)
  {
    size_t    j;
    uint16_t  addr = 0, lastAddr;
    bool      isIgnore = false;
    bool      isVideo = false;
    int       rwxMode = 0;
    int       priority = -1;
    uint32_t  n = 0;

    for (j = 0; j < s.length(); j++) {
      char    c = s[j];
      if (c >= '0' && c <= '9')
        n = (n << 4) + uint32_t(c - '0');
      else if (c >= 'A' && c <= 'F')
        n = (n << 4) + uint32_t(c - 'A') + 10;
      else if (c >= 'a' && c <= 'f')
        n = (n << 4) + uint32_t(c - 'a') + 10;
      else
        break;
    }
    if (j != 4)
      throw Exception("syntax error in breakpoint list");
    if (j < s.length() && s[j] == ':') {
      if (n >= 0x0200)
        throw Exception("syntax error in breakpoint list");
      j++;
      isVideo = true; // break on video position
      addr = uint16_t(n << 7);
      n = 0;
      for ( ; j < s.length(); j++) {
        char    c = s[j];
        if (c >= '0' && c <= '9')
          n = (n << 4) + uint32_t(c - '0');
        else if (c >= 'A' && c <= 'F')
          n = (n << 4) + uint32_t(c - 'A') + 10;
        else if (c >= 'a' && c <= 'f')
          n = (n << 4) + uint32_t(c - 'a') + 10;
        else
          break;
      }
      if (j != 7)
        throw Exception("syntax error in breakpoint list");
      addr |= uint16_t((n & 0xFEU) >> 1);
    }
    else
      addr = uint16_t(n);
    lastAddr = addr;
    if (j < s.length() && s[j] == '-') {
      size_t  len = 0;
      n = 0;
      j++;
      for ( ; j < s.length(); j++, len++) {
        char    c = s[j];
        if (c >= '0' && c <= '9')
          n = (n << 4) + uint32_t(c - '0');
        else if (c >= 'A' && c <= 'F')
          n = (n << 4) + uint32_t(c - 'A') + 10;
        else if (c >= 'a' && c <= 'f')
          n = (n << 4) + uint32_t(c - 'a') + 10;
        else
          break;
      }
      if (isVideo) {
        if (len != 2 || n < ((uint32_t(addr) & 0x7FU) << 1))
          throw Exception("syntax error in breakpoint list");
        lastAddr = uint16_t((addr & 0xFF80) | ((n & 0xFE) >> 1));
      }
      else {
        if (len != 4 || n < addr)
          throw Exception("syntax error in breakpoint list");
        lastAddr = n;
      }
    }
    for ( ; j < s.length(); j++) {
      switch (s[j]) {
      case 'r':
        rwxMode |= 1;
        break;
      case 'w':
        rwxMode |= 2;
        break;
      case 'x':
        rwxMode |= 4;
        break;
      case 'i':
        if (isVideo)
          throw Exception("ignore flag is not allowed for video breakpoints");
        isIgnore = true;
        break;
      case 'p':
        if (++j >= s.length())
          throw Exception("syntax error in breakpoint list");
        if (s[j] < '0' || s[j] > '3')
          throw Exception("syntax error in breakpoint list");
        priority = int(s[j] - '0');
        break;
      default:
        throw Exception("syntax error in breakpoint list");
      }
    }
    if (isVideo && rwxMode != 0) {
      throw Exception("read/write/execute flags are not allowed "
                      "for video breakpoints");
    }
    if (isIgnore && (rwxMode != 0 || priority >= 0)) {
      throw Exception("read/write/execute flags and priority "
                      "are not allowed for ignore breakpoints");
    }
    while (true) {
      uint32_t    addr_ = uint32_t(addr);
      if (isVideo)    // use separate address space for video breakpoints
        addr_ |= uint32_t(0x80000000UL);
      uint8_t bpFlags = uint8_t(rwxMode);
      if (isIgnore)
        bpFlags = bpFlags | 0x08;
      else if (priority >= 0)
        bpFlags = bpFlags | uint8_t(priority << 4);
      else
        bpFlags = bpFlags | 0x20;     // default priority = 2
      if (!(int(isIgnore | isVideo) | rwxMode))
        bpFlags = bpFlags | 0x07;     // default to break on any memory access
      std::map< uint32_t, uint8_t >::iterator i_ = bpList.find(addr_);
      if (i_ == bpList.end()) {
        // add new breakpoint
        bpList.insert(std::pair< uint32_t, uint8_t >(addr_, bpFlags));
      }
      else {
        // update existing breakpoint
        uint8_t bp = (*i_).second;
        bp = bp | (bpFlags & 0x0F);
        bpFlags = bpFlags | (bp & 0x0F);
        bp = (bp > bpFlags ? bp : bpFlags);
        (*i_).second = bp;
      }
      if (addr == lastAddr)
        break;
      addr++;
    }
  }

  BreakPointList::BreakPointList(const std::string& lst)
  {
    // b0 = R
    // b1 = W
    // b2 = X
    // b3 = I
    // b4..b5 = priority
    std::map< uint32_t, uint8_t > bpList;
    std::string curToken = "";
    char        ch = '\0';
    bool        wasSpace = true;

    for (size_t i = 0; i < lst.length(); i++) {
      ch = lst[i];
      if (ch == '#' || ch == ';') {
        ch = '\n';
        while (++i < lst.length() && lst[i] != '\r' && lst[i] != '\n')
          ;
      }
      bool  isSpace = (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
      if (isSpace && wasSpace)
        continue;
      if (isSpace) {
        parseBreakPoint(bpList, curToken);
        curToken = "";
      }
      else
        curToken += ch;
      wasSpace = isSpace;
    }
    if (!wasSpace)
      parseBreakPoint(bpList, curToken);

    std::map< uint32_t, uint8_t >::iterator i_;
    for (i_ = bpList.begin(); i_ != bpList.end(); i_++) {
      uint32_t  addr = (*i_).first;
      bool      isVideo = bool(addr & 0x80000000U);
      uint8_t   bpFlags = (*i_).second;
      addr = addr & 0xFFFFU;
      if (isVideo)
        lst_.push_back(BreakPoint(4, uint16_t(addr), int(bpFlags >> 4)));
      else
        addBreakPoint(bpFlags, uint16_t(addr));
    }
  }

  void BreakPointList::addBreakPoint(int type, uint16_t addr, int priority)
  {
    lst_.push_back(BreakPoint(type, addr, priority));
  }

  void BreakPointList::addBreakPoint(uint8_t bpFlags, uint16_t addr)
  {
    int     priority = int((bpFlags >> 4) & 3);
    if (bpFlags & 0x08)                 // i
      addBreakPoint(5, addr, priority);
    if ((bpFlags & 0x07) == 7) {        // rwx
      addBreakPoint(3, addr, priority);
    }
    else {
      if (bpFlags & 0x01)               // r
        addBreakPoint(1, addr, priority);
      if (bpFlags & 0x02)               // w
        addBreakPoint(2, addr, priority);
      if (bpFlags & 0x04)               // x
        addBreakPoint(6, addr, priority);
    }
  }

  // --------------------------------------------------------------------------

  class ChunkType_BPList : public File::ChunkTypeHandler {
   private:
    BreakPointList& ref;
   public:
    ChunkType_BPList(BreakPointList& ref_)
      : File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_BPList()
    {
    }
    virtual File::ChunkType getChunkType() const
    {
      return File::PLUS4EMU_CHUNKTYPE_BREAKPOINTS;
    }
    virtual void processChunk(File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  void BreakPointList::saveState(File::Buffer& buf)
  {
    buf.setPosition(0);
    buf.writeUInt32(0x01000002);        // version number
    for (size_t i = 0; i < lst_.size(); i++) {
      buf.writeByte(uint8_t(lst_[i].type()));
      buf.writeUInt32(lst_[i].addr());
      buf.writeByte(uint8_t(lst_[i].priority()));
    }
  }

  void BreakPointList::saveState(File& f)
  {
    File::Buffer  buf;
    this->saveState(buf);
    f.addChunk(File::PLUS4EMU_CHUNKTYPE_BREAKPOINTS, buf);
  }

  void BreakPointList::loadState(File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (version != 0x01000002) {
      buf.setPosition(buf.getDataSize());
      throw Exception("incompatible breakpoint list format");
    }
    // reset breakpoint list
    lst_.clear();
    // load saved state
    while (buf.getPosition() < buf.getDataSize()) {
      int       type = buf.readByte();
      uint16_t  addr = uint16_t(buf.readUInt32() & 0xFFFFU);
      int       priority = buf.readByte();
      BreakPoint  bp(type, addr, priority);
      lst_.push_back(bp);
    }
  }

  void BreakPointList::registerChunkType(File& f)
  {
    ChunkType_BPList  *p;
    p = new ChunkType_BPList(*this);
    try {
      f.registerChunkType(p);
    }
    catch (...) {
      delete p;
      throw;
    }
  }

}       // namespace Plus4Emu

