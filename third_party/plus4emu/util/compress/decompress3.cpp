
// compressor utility for Commodore Plus/4 programs
// Copyright (C) 2007-2016 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "compress.hpp"
#include "decompress3.hpp"
#include <vector>

namespace Plus4Compress {

  unsigned char Decompressor_M3::readByte()
  {
    if (inBufPos < 1) {
      throw Plus4Emu::Exception("Decompressor_M3::readByte(): "
                                "unexpected end of input data");
    }
    inBufPos--;
    return inBufPtr[inBufPos];
  }

  unsigned int Decompressor_M3::readBits(size_t n)
  {
    unsigned int  retval = 0U;
    for (size_t i = 0; i < n; i++) {
      if (inputShiftReg < 0x02) {
        unsigned char tmp = readByte();
        retval = (retval << 1) | (unsigned int) (tmp & 0x01);
        inputShiftReg = ((tmp & 0xFE) >> 1) | 0x80;
      }
      else {
        retval = (retval << 1) | (unsigned int) (inputShiftReg & 0x01);
        inputShiftReg = (inputShiftReg & 0xFE) >> 1;
      }
    }
    return retval;
  }

  unsigned int Decompressor_M3::readLengthValue()
  {
    size_t  lenBits = 0;
    while (readBits(1) != 0U)
      lenBits++;
    if (lenBits > 15) {
      throw Plus4Emu::Exception("Decompressor_M3::readLengthValue(): "
                                "error in compressed data");
    }
    unsigned int  retval = 1U;
    while (lenBits-- > 0)
      retval = (retval << 1) | readBits(1);
    return retval;
  }

  Decompressor_M3::Decompressor_M3()
    : Decompressor(),
      inBufPtr((unsigned char *) 0),
      inBufPos(0),
      inputShiftReg(0x01)
  {
  }

  Decompressor_M3::~Decompressor_M3()
  {
  }

  void Decompressor_M3::decompressData(
      std::vector< std::vector< unsigned char > >& outBuf,
      const std::vector< unsigned char >& inBuf)
  {
    outBuf.clear();
    std::vector< unsigned char >  tmpOutBuf;
    decompressData(tmpOutBuf, inBuf);
    // NOTE: this format does not support start addresses,
    // so use a fixed value of $1001
    tmpOutBuf.insert(tmpOutBuf.begin(), 2, (unsigned char) 0x00);
    tmpOutBuf[0] = 0x01;
    tmpOutBuf[1] = 0x10;
    outBuf.push_back(tmpOutBuf);
  }

  void Decompressor_M3::decompressData(
      std::vector< unsigned char >& outBuf,
      const std::vector< unsigned char >& inBuf)
  {
    outBuf.clear();
    if (inBuf.size() < 4) {
      throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                "insufficient input data size");
    }
    size_t  compressedSize = size_t(inBuf[0]) | (size_t(inBuf[1]) << 8);
    size_t  uncompressedSize = size_t(inBuf[inBuf.size() - 2])
                               | (size_t(inBuf[inBuf.size() - 1]) << 8);
    uncompressedSize += compressedSize;
    if ((compressedSize == 0 && uncompressedSize > 0) ||
        compressedSize != (inBuf.size() - 4)) {
      throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                "invalid compressed data size");
    }
    if (uncompressedSize > 65535) {
      throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                "invalid uncompressed data size");
    }
    if (uncompressedSize == compressedSize) {
      // uncompressed data
      for (size_t i = 0; i < compressedSize; i++)
        outBuf.push_back(inBuf[i + 2]);
      return;
    }
    std::vector< unsigned char >  tmpOutBuf(uncompressedSize);
    inBufPtr = &(inBuf.front()) + 2;
    inBufPos = compressedSize;
    size_t  outBufPos = uncompressedSize;
    inputShiftReg = 0x01;
    while (outBufPos > 0) {
      // literal sequence:
      unsigned int  n = readLengthValue();              // get length
      if (size_t(n) > outBufPos) {
        throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                  "error in compressed data");
      }
      for (unsigned int i = 0U; i < n; i++) {
        outBufPos--;
        tmpOutBuf[outBufPos] = readByte();
      }
      if (outBufPos < 1)
        break;
      do {
        // read match parameters
        n = readLengthValue() + 1U;                     // length
        if (size_t(n) > outBufPos) {
          throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                    "error in compressed data");
        }
        unsigned int  d =                               // offset
            readBits(size_t(n == 2U ? 3 : 4)) + (unsigned int) (n == 2U);
        d = ((1U << d) | readBits(size_t(d))) - (unsigned int) (n == 2U);
        if ((outBufPos + size_t(d)) > uncompressedSize) {
          throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                    "error in compressed data");
        }
        // copy LZ77 match
        for (unsigned int i = 0U; i < n; i++) {
          outBufPos--;
          tmpOutBuf[outBufPos] = tmpOutBuf[outBufPos + size_t(d)];
        }
        if (outBufPos < 1)
          break;
        // read flag bit: if '1', then another match is following
      } while (readBits(1) != 0U);
    }
    if (inBufPos > 0) {
      throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                "more input data than expected");
    }
    if (((int(inputShiftReg) + 1) & int(inputShiftReg)) != 0) {
      // the last byte is expected to be padded with '1' bits
      throw Plus4Emu::Exception("Decompressor_M3::decompressData(): "
                                "error in compressed data");
    }
    for (size_t i = 0; i < uncompressedSize; i++)
      outBuf.push_back(tmpOutBuf[i]);
  }

}       // namespace Plus4Compress

