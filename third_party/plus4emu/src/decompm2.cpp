
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
#include "decompm2.hpp"

namespace Plus4Emu {

  unsigned int Decompressor::readBits(size_t nBits)
  {
    unsigned int  retval = 0U;
    for (size_t i = 0; i < nBits; i++) {
      if (PLUS4EMU_UNLIKELY(!(shiftRegister & 0x7F))) {
        if (inputBufferPosition >= inputBufferSize)
          throw Exception("unexpected end of compressed data");
        shiftRegister = inputBuffer[inputBufferPosition++];
        retval = (retval << 1) | ((shiftRegister >> 7) & 0x01);
        shiftRegister = (shiftRegister << 1) | 0x01;
        continue;
      }
      retval = (retval << 1) | ((shiftRegister >> 7) & 0x01);
      shiftRegister = shiftRegister << 1;
    }
    return retval;
  }

  unsigned char Decompressor::readLiteralByte()
  {
    if (PLUS4EMU_UNLIKELY(inputBufferPosition >= inputBufferSize))
      throw Exception("unexpected end of compressed data");
    return inputBuffer[inputBufferPosition++];
  }

  unsigned int Decompressor::readMatchLength()
  {
    if (!readBits(1))
      return 0x80000001U;                       // literal byte
    unsigned char slotNum = 0;
    while (readBits(1) != 0U) {
      if (++slotNum >= 8)
        return (readBits(8) + 0x80000011U);     // literal sequence
    }
    return readLZMatchParameter(slotNum, &(lengthDecodeTable[0]));
  }

  unsigned int Decompressor::readLZMatchParameter(
      unsigned char slotNum, const unsigned int *decodeTable)
  {
    return (decodeTable[int(slotNum) * 2]
            + readBits(size_t(decodeTable[int(slotNum) * 2 + 1])));
  }

  void Decompressor::readDecodeTables()
  {
    unsigned int  tmp = 1U;
    unsigned int  *tablePtr = &(lengthDecodeTable[0]);
    offs3PrefixSize = size_t(readBits(2)) + 2;
    size_t  offs3NumSlots = size_t(1) << offs3PrefixSize;
    for (size_t i = 0; i < (8 + 4 + 8 + offs3NumSlots); i++) {
      if (i == 8) {
        tmp = 1U;
        tablePtr = &(offs1DecodeTable[0]);
      }
      else if (i == (8 + 4)) {
        tmp = 1U;
        tablePtr = &(offs2DecodeTable[0]);
      }
      else if (i == (8 + 4 + 8)) {
        tmp = 1U;
        tablePtr = &(offs3DecodeTable[0]);
      }
      tablePtr[0] = tmp;
      tablePtr[1] = readBits(4);
      tmp = tmp + (1U << tablePtr[1]);
      tablePtr = tablePtr + 2;
    }
  }

  bool Decompressor::decompressDataBlock(std::vector< unsigned char >& buf)
  {
    if ((buf.size() + 65536) > buf.capacity())
      buf.reserve(((buf.size() + (buf.size() >> 2)) | 0xFFFF) + 1);
    unsigned int  nSymbols = readBits(16) + 1U;
    bool    isLastBlock = readBits(1);
    if (!readBits(1)) {
      // compression disabled: copy literal data
      do {
        buf.push_back(readLiteralByte());
      } while (--nSymbols);
    }
    else {
      readDecodeTables();
      size_t  blockSize = 0;
      do {
        unsigned int  matchLength = readMatchLength();
        blockSize += size_t(matchLength & 0x7FFFFFFFU);
        if (PLUS4EMU_UNLIKELY(blockSize > 65536))
          throw Exception("error in compressed data");
        if (matchLength >= 0x80000000U) {
          // literal sequence
          matchLength &= 0x7FFFFFFFU;
          do {
            buf.push_back(readLiteralByte());
          } while (--matchLength);
        }
        else {
          // get match offset:
          unsigned int  offs = 0U;
          if (matchLength == 1U) {
            offs = readLZMatchParameter((unsigned char) readBits(2),
                                        &(offs1DecodeTable[0]));
          }
          else if (matchLength == 2U) {
            offs = readLZMatchParameter((unsigned char) readBits(3),
                                        &(offs2DecodeTable[0]));
          }
          else {
            offs = readLZMatchParameter(
                       (unsigned char) readBits(offs3PrefixSize),
                       &(offs3DecodeTable[0]));
          }
          if (offs > buf.size())
            throw Exception("error in compressed data");
          size_t  lzMatchReadPos = buf.size() - offs;
          do {
            buf.push_back(buf[lzMatchReadPos]);
            lzMatchReadPos++;
          } while (--matchLength);
        }
      } while (--nSymbols);
    }
    if (buf.size() > 0x04000000)
      throw Exception("error in compressed data");
    return isLastBlock;
  }

  Decompressor::Decompressor()
    : offs3PrefixSize(2),
      shiftRegister(0x80),
      inputBuffer((unsigned char *) 0),
      inputBufferSize(0),
      inputBufferPosition(0)
  {
  }

  Decompressor::~Decompressor()
  {
  }

  void Decompressor::decompressData(
      std::vector< unsigned char >& outBuf,
      const unsigned char *inBuf, size_t inBufSize)
  {
    outBuf.clear();
    if (!inBuf || inBufSize < 1)
      return;
    unsigned char chkSum = 0xFF;
    // verify checksum
    for (size_t i = inBufSize; i-- > 0; ) {
      chkSum = chkSum ^ inBuf[i];
      chkSum = ((chkSum & 0x7F) << 1) | ((chkSum & 0x80) >> 7);
      chkSum = (chkSum + 0xAC) & 0xFF;
    }
    if (chkSum != 0x80)
      throw Exception("error in compressed data");
    // decompress all data blocks
    inputBuffer = inBuf;
    inputBufferSize = inBufSize;
    inputBufferPosition = 1;
    shiftRegister = 0x80;
    while (!decompressDataBlock(outBuf))
      ;
    // on successful decompression, all input data must be consumed
    if (!(inputBufferPosition >= inputBufferSize &&
          !(shiftRegister & (shiftRegister - 1)))) {
      throw Exception("error in compressed data");
    }
  }

  // --------------------------------------------------------------------------

  void decompressData(std::vector< unsigned char >& outBuf,
                      const unsigned char *inBuf, size_t inBufSize)
  {
    Decompressor  decompressor;
    decompressor.decompressData(outBuf, inBuf, inBufSize);
  }

}       // namespace Plus4Emu

