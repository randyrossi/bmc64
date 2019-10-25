
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "system.hpp"
#include "cpu.hpp"
#include "ted.hpp"

namespace Plus4 {

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_0000_to_0FFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 4]];
    ted.dataBusState = p[addr];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_1000_to_3FFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap]];
    ted.dataBusState = p[addr];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_4000_to_7FFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 1]];
    ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_8000_to_BFFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 2]];
    if (p)
      ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_C000_to_FBFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 3]];
    if (p)
      ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_FC00_to_FCFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 5]];
    if (p)
      ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_FD00_to_FEFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 6]];
    if (p)
      ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_memory_FF00_to_FFFF(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 7]];
    if (p)
      ted.dataBusState = p[addr & 0x3FFF];
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FD16(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    if (ted.ramSegments < 16) {
      ted.dataBusState = ted.user_port_state & (ted.tape_button_state ?
                                                uint8_t(0xFB) : uint8_t(0xFF));
    }
    else
      ted.dataBusState = ted.hannesRegister;
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t TED7360::read_register_FF3E_FF3F(
      void *userData, uint16_t addr)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    if (ted.memoryReadMap & 0x0080U) {
      uint8_t *p = ted.segmentTable[ted.memoryMapTable[ted.memoryReadMap + 7]];
      if (p)
        ted.dataBusState = p[addr & 0x3FFF];
    }
    else
      ted.dataBusState = uint8_t(0xFF);
    return ted.dataBusState;
  }

  // --------------------------------------------------------------------------

  PLUS4EMU_REGPARM3 void TED7360::write_memory_0000_to_0FFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 4]];
    ted.dataBusState = value;
    p[addr] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_1000_to_3FFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap]];
    ted.dataBusState = value;
    p[addr] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_4000_to_7FFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 1]];
    ted.dataBusState = value;
    p[addr & 0x3FFF] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_8000_to_BFFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 2]];
    ted.dataBusState = value;
    p[addr & 0x3FFF] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_C000_to_FCFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 3]];
    ted.dataBusState = value;
    p[addr & 0x3FFF] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_FD00_to_FEFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 6]];
    ted.dataBusState = value;
    if (p)
      p[addr & 0x3FFF] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_memory_FF00_to_FFFF(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    uint8_t   *p = ted.segmentTable[ted.memoryMapTable[ted.memoryWriteMap + 7]];
    ted.dataBusState = value;
    p[addr & 0x3FFF] = value;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FD16(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    if (ted.ramSegments < 16) {
      ted.user_port_state = value;
      ted.hannesRegister = uint8_t(0xFF);
      ted.memoryReadMap |= 0x0678U;
      ted.memoryWriteMap |= 0x0678U;
      ted.cpuMemoryReadMap |= 0x0678U;
      ted.tedDMAReadMap |= 0x0678U;
      ted.tedBitmapReadMap |= 0x0678U;
      return;
    }
    ted.hannesRegister = value;
    unsigned int  tmp =
        (unsigned int) ((value | uint8_t(ted.ramSegments < 64 ? 0x3C : 0x30))
                        & uint8_t(0xCF)) << 3;
    ted.memoryReadMap = (ted.memoryReadMap & 0x7980U) | tmp;
    ted.memoryWriteMap = (ted.memoryWriteMap & 0x7980U) | tmp;
    ted.cpuMemoryReadMap = (ted.cpuMemoryReadMap & 0x7980U) | tmp;
    ted.tedDMAReadMap = (ted.tedDMAReadMap & 0x7980U) | tmp;
    ted.tedBitmapReadMap = (ted.tedBitmapReadMap & 0x7980U) | tmp;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FDDx(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    unsigned int  tmp = ((unsigned int) addr & 0x000FU) << 11;
    ted.memoryReadMap = (ted.memoryReadMap & 0x07F8U) | tmp;
    ted.cpuMemoryReadMap = (ted.cpuMemoryReadMap & 0x07F8U) | tmp;
    ted.tedDMAReadMap = (ted.tedDMAReadMap & 0x07F8U) | tmp;
    ted.tedBitmapReadMap = (ted.tedBitmapReadMap & 0x07F8U) | tmp;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF3E(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.memoryReadMap |= 0x0080U;
    ted.cpuMemoryReadMap |= 0x0080U;
    ted.tedDMAReadMap |= 0x0080U;
  }

  PLUS4EMU_REGPARM3 void TED7360::write_register_FF3F(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360&  ted = *(reinterpret_cast<TED7360 *>(userData));
    ted.dataBusState = value;
    ted.memoryReadMap &= 0x7F78U;
    ted.cpuMemoryReadMap &= 0x7F78U;
    ted.tedDMAReadMap &= 0x7F78U;
  }

  // --------------------------------------------------------------------------

  uint8_t TED7360::getMemoryPage(int n) const
  {
    return memoryMapTable[cpuMemoryReadMap + ((unsigned int) n & 3U)];
  }

  int TED7360::getSegmentType(uint8_t n) const
  {
    if (segmentTable[n] == (uint8_t *) 0)
      return 0;
    return (n < uint8_t(0x80) ? 1 : 2);
  }

  uint8_t TED7360::readMemoryRaw(uint32_t addr) const
  {
    uint8_t   segment = uint8_t((addr >> 14) & 0xFF);
    uint8_t   *p = segmentTable[segment];
    if (p)
      return p[addr & 0x3FFF];
    return uint8_t(0xFF);
  }

  uint8_t TED7360::readMemoryCPU(uint16_t addr, bool forceRAM_) const
  {
    // ugly hack to work around const declaration and avoid side-effects
    TED7360&      ted_ = *(const_cast<TED7360 *>(this));
    unsigned int  savedMemoryReadMap = memoryReadMap;
    uint8_t       savedDataBusState = dataBusState;
    ted_.memoryReadMap =
        (forceRAM_ ? (cpuMemoryReadMap & 0x7F78U) : cpuMemoryReadMap);
    ted_.dataBusState = uint8_t(0xFF);
    uint8_t retval = ted_.readMemory(addr);
    ted_.memoryReadMap = savedMemoryReadMap;
    ted_.dataBusState = savedDataBusState;
    return retval;
  }

  void TED7360::writeMemoryRaw(uint32_t addr, uint8_t value)
  {
    uint8_t   segment = uint8_t((addr >> 14) & 0xFF);
    if (segment >= uint8_t(0x08)) {
      uint8_t   *p = segmentTable[segment];
      if (p)
        p[addr & 0x3FFF] = value;
    }
  }

  void TED7360::writeMemoryCPU(uint16_t addr, uint8_t value)
  {
    writeMemory(addr, value);
  }

  void TED7360::loadROM(int bankNum, int offs, int cnt, const uint8_t *buf)
  {
    int   i = offs, j, k;

    if (buf) {
      for (j = 0, k = cnt; k > 0; i++, j++, k--) {
        uint8_t segment = uint8_t(((bankNum & 3) << 1) + ((i & 0x4000) >> 14));
        if (segmentTable[segment] == (uint8_t *) 0) {
          segmentTable[segment] = new uint8_t[16384];
          for (size_t tmp = 0; tmp < 16384; tmp++)
            segmentTable[segment][tmp] = uint8_t(0xFF);
        }
        segmentTable[segment][i & 0x3FFF] = buf[j];
      }
    }
    else {
      uint8_t segment = uint8_t(((bankNum & 3) << 1) + ((i & 0x4000) >> 14));
      if (segmentTable[segment] != (uint8_t *) 0) {
        delete[] segmentTable[segment];
        segmentTable[segment] = (uint8_t *) 0;
      }
    }
  }

  void TED7360::initializeRAMSegment(uint8_t *p)
  {
    uint8_t   ramPatternBase[256];
    uint8_t   bitMasks[8];
    uint8_t   xorValue = 0x00;
    uint64_t  tmp = ramPatternCode;
    for (unsigned int i = 0U; i < 8U; i++) {
      bitMasks[i] = uint8_t(1 << ((unsigned char) tmp & 7));
      xorValue |= uint8_t((((unsigned char) tmp & 8) >> 3) << i);
      tmp = tmp >> 4;
    }
    for (unsigned int i = 0U; i < 256U; i++) {
      uint8_t n = xorValue;
      for (unsigned int j = 0U; j < 8U; j++) {
        if (i & (unsigned int) bitMasks[j])
          n ^= uint8_t(1U << j);
      }
      ramPatternBase[i] = n;
    }
    ramPatternBase[0] ^= (uint8_t(tmp) & uint8_t(0xFF));
    unsigned int  tmp2 = (unsigned int) tmp & 0xFF00U;
    for (unsigned int i = 0U; i <= 0x3FFFU; i++) {
      unsigned int  rndVal =
          (unsigned int) Plus4Emu::getRandomNumber(randomSeed) & 0xFFFFU;
      if (tmp2 <= rndVal)
        p[i] = ramPatternBase[i & 0xFFU];
      else
        p[i] = uint8_t(rndVal & 0xFFU);
    }
  }

  void TED7360::setRAMSize(size_t n, uint64_t ramPattern)
  {
    if (n > 256)
      n = 1024;
    else if (n > 64)
      n = 256;
    else if (n > 32)
      n = 64;
    else if (n > 16)
      n = 32;
    else
      n = 16;
    ramSegments = uint8_t(n >> 4);
    ramPatternCode = ramPattern & ((uint64_t(1) << 48) - uint64_t(1));
    // free old segments if reducing memory size
    for (uint8_t i = uint8_t(0x08); i <= (uint8_t(0xFF) - ramSegments); i++) {
      if (segmentTable[i]) {
        delete[] segmentTable[i];
        segmentTable[i] = (uint8_t *) 0;
      }
    }
    for (uint8_t i = uint8_t(0xFF); i > (uint8_t(0xFF) - ramSegments); i--) {
      // allocate new segment if needed
      if (!segmentTable[i])
        segmentTable[i] = new uint8_t[16384];
      // clear memory
      initializeRAMSegment(segmentTable[i]);
    }
    // set up memory map table
    for (size_t i = 0; i < 4096; i++) {
      uint8_t *memoryMap_ = &(memoryMapTable[i << 3]);
      uint8_t ramBank_ = uint8_t(i) & 0x0F;
      bool    isROM_ = !!(i & 0x10);
      bool    isTED_ = !!(i & 0x20);
      bool    tedReadsHannes_ = !!(i & 0x40);
      bool    allowHannesBelow4000_ = !(i & 0x80);
      uint8_t romBankLow_ = uint8_t(i >> 8) & 0x03;
      uint8_t romBankHigh_ = uint8_t(i >> 10) & 0x03;
      for (uint8_t j = 0; j < 4; j++) {
        uint8_t segment_ = (ramBank_ << 2) + j;
        if (isROM_ && j == 2)
          segment_ = (romBankLow_ << 1) | (segment_ & 0x01);
        else if (isROM_ && j == 3)
          segment_ = (romBankHigh_ << 1) | (segment_ & 0x01);
        else if (ramSegments < 2)
          segment_ = 0xFF;
        else if (ramSegments < 4)
          segment_ = 0xFE | segment_;
        else if (ramSegments < 16 || (isTED_ && !tedReadsHannes_))
          segment_ = 0xFC | segment_;
        else if (ramSegments < 64)
          segment_ = 0xF0 | segment_;
        else
          segment_ = 0xC0 | segment_;
        memoryMap_[j] = segment_;
      }
      memoryMap_[4] = uint8_t(ramSegments >= 4 ?
                              0xFC : (ramSegments >= 2 ? 0xFE : 0xFF));
      if (!allowHannesBelow4000_)
        memoryMap_[0] = memoryMap_[4];
      memoryMap_[5] = (isROM_ ? uint8_t(0x01) : memoryMap_[3]);
      memoryMap_[6] = uint8_t(0x7F);
      memoryMap_[7] = memoryMap_[3];
    }
    initRegisters();
  }

}       // namespace Plus4

