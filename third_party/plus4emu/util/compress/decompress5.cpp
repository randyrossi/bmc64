
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
#include "decompress5.hpp"
#include <vector>

namespace Plus4Compress {

  // 0:       unused symbol
  // 1 to 15: code length in bits
  // 16:      repeat the last code length 3 to 6 times (2 extra bits)
  // 17:      repeat unused symbol 3 to 10 times (3 extra bits)
  // 18:      repeat unused symbol 11 to 138 times (7 extra bits)

  static const unsigned char deflateCodeLengthCodeTable[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
  };

  // --------------------------------------------------------------------------

  unsigned int Decompressor_ZLib::readByte()
  {
    if (inputBufferPosition >= inputBufferSize)
      throw Plus4Emu::Exception("unexpected end of compressed data");
    shiftRegister = inputBuffer[inputBufferPosition];
    shiftRegisterCnt = 8;
    inputBufferPosition++;
    return (unsigned int) shiftRegister;
  }

  PLUS4EMU_INLINE unsigned int Decompressor_ZLib::readBit()
  {
    unsigned int  retval = (unsigned int) (shiftRegister & 0x01);
    if (PLUS4EMU_UNLIKELY(shiftRegisterCnt < 1))
      retval = readByte() & 1U;
    shiftRegister = shiftRegister >> 1;
    shiftRegisterCnt--;
    return retval;
  }

  unsigned int Decompressor_ZLib::readBits(size_t nBits)
  {
    unsigned int  retval = 0U;
    for (unsigned char i = 0; i < (unsigned char) nBits; ) {
      if (PLUS4EMU_UNLIKELY(shiftRegisterCnt < 1))
        (void) readByte();
      unsigned char n = (unsigned char) nBits - i;
      if (n > (unsigned char) shiftRegisterCnt)
        n = (unsigned char) shiftRegisterCnt;
      retval |= (((unsigned int) shiftRegister & ((1U << n) - 1U)) << i);
      shiftRegister = shiftRegister >> n;
      shiftRegisterCnt = shiftRegisterCnt - n;
      i = i + n;
    }
    return retval;
  }

  unsigned int Decompressor_ZLib::huffmanDecode(int huffTable)
  {
    int     tmp = 0;
    int     cnt = -1;
    const unsigned int  *symCntTable =
        (huffTable == 0 ? huffmanSymCntTable0 : huffmanSymCntTable1);
    const unsigned int  *offsetTable =
        (huffTable == 0 ? huffmanOffsetTable0 : huffmanOffsetTable1);
    const unsigned int  *decodeTable =
        (huffTable == 0 ? huffmanDecodeTable0 : huffmanDecodeTable1);
    do {
      if (++cnt >= 15)
        throw Plus4Emu::Exception("error in compressed data");
      tmp = ((tmp << 1) | int(readBit())) - int(symCntTable[cnt]);
    } while (tmp >= 0);
    tmp = tmp + int(offsetTable[cnt]);
    if (decodeTable[tmp] == 0xFFFFFFFFU)
      throw Plus4Emu::Exception("error in compressed data");
    return decodeTable[tmp];
  }

  void Decompressor_ZLib::buildDecodeTable(int huffTable,
                                           const unsigned char *lenBuf,
                                           size_t nSymbols)
  {
    unsigned int  *symCntTable =
        (huffTable == 0 ? huffmanSymCntTable0 : huffmanSymCntTable1);
    unsigned int  *offsetTable =
        (huffTable == 0 ? huffmanOffsetTable0 : huffmanOffsetTable1);
    unsigned int  *decodeTable =
        (huffTable == 0 ? huffmanDecodeTable0 : huffmanDecodeTable1);
    for (size_t i = 0; i < 15; i++)
      symCntTable[i] = 0U;
    for (size_t i = 0; i < nSymbols; i++) {
      decodeTable[i] = 0xFFFFFFFFU;
      if (lenBuf[i])
        symCntTable[lenBuf[i] - 1] = symCntTable[lenBuf[i] - 1] + 1U;
    }
    {
      unsigned int  offs = 0U;
      for (size_t i = 0; i < 15; i++) {
        offsetTable[i] = offs;
        offs = offs + symCntTable[i];
      }
    }
    for (size_t i = 0; i < nSymbols; i++) {
      unsigned char len = lenBuf[i];
      if (len) {
        len--;
        unsigned int  offs = offsetTable[len];
        decodeTable[offs] = (unsigned int) i;
        offsetTable[len] = offs + 1U;
      }
    }
  }

  void Decompressor_ZLib::huffmanInit(unsigned char blockType)
  {
    unsigned char lenBuf[320];
    if (blockType == 1) {
      // fixed Huffman codes
      for (int i = 0; i < 320; i++) {
        lenBuf[i] =
            (i < 144 ? 8 : (i < 256 ? 9 : (i < 280 ? 7 : (i < 288 ? 8 : 5))));
      }
    }
    else {
      for (int i = 0; i < 320; i++)
        lenBuf[i] = 0;
      size_t  litCnt = size_t(readBits(5)) + 257;
      size_t  distCnt = size_t(readBits(5)) + 1;
      size_t  codeCnt = size_t(readBits(4)) + 4;
      for (size_t i = 0; i < 19; i++)
        lenBuf[i] = 0;
      for (size_t i = 0; i < codeCnt; i++)
        lenBuf[deflateCodeLengthCodeTable[i]] = (unsigned char) readBits(3);
      buildDecodeTable(1, lenBuf, 19);
      size_t  totalCnt = distCnt + 288;
      size_t  rleCnt = 0;
      unsigned char rleCode = 0x00;
      for (size_t i = 0; i < totalCnt; i++) {
        if (i == litCnt)
          i = 288;
        if (rleCnt > 0) {
          lenBuf[i] = rleCode;
          rleCnt--;
          continue;
        }
        unsigned char c = (unsigned char) huffmanDecode(1);
        if (c < 16) {
          lenBuf[i] = c;
          rleCode = c;
          continue;
        }
        else if (c == 16) {
          if (!i)
            throw Plus4Emu::Exception("error in compressed data");
          rleCnt = size_t(readBits(2)) + 2;
        }
        else {
          if (c == 17)
            rleCnt = size_t(readBits(3)) + 2;
          else
            rleCnt = size_t(readBits(7)) + 10;
          rleCode = 0x00;
        }
        lenBuf[i] = rleCode;
      }
      if (rleCnt > 0 || lenBuf[256] == 0)
        throw Plus4Emu::Exception("error in compressed data");
    }
    buildDecodeTable(0, lenBuf, 288);
    buildDecodeTable(1, &(lenBuf[288]), 32);
  }

  bool Decompressor_ZLib::decompressDataBlock(std::vector< unsigned char >& buf)
  {
    static const size_t maxDataSize = 0x04000000;
    bool    isLastBlock = bool(readBit());
    unsigned char blockType = (unsigned char) readBits(2);
    if (blockType == 3)
      throw Plus4Emu::Exception("error in compressed data");
    if ((buf.size() + 65536) > buf.capacity())
      buf.reserve(((buf.size() + (buf.size() >> 2)) | 0xFFFF) + 1);
    if (!blockType) {
      // uncompressed data
      shiftRegisterCnt = 0;
      unsigned int  blockSize = readBits(32);
      blockSize = blockSize ^ ((~blockSize & 0xFFFFU) << 16);
      if (!(blockSize >= 1U && blockSize <= 0xFFFFU))
        throw Plus4Emu::Exception("error in compressed data");
      do {
        if (PLUS4EMU_UNLIKELY(buf.size() >= maxDataSize))
          throw Plus4Emu::Exception("error in compressed data");
        buf.push_back((unsigned char) readByte());
      } while (--blockSize);
      shiftRegisterCnt = 0;
      return isLastBlock;
    }
    huffmanInit(blockType);
    unsigned int  prvDistance = 0U;
    while (true) {
      unsigned int  c = huffmanDecode(0);
      if (c == 0x0100U)
        break;
      if (c < 0x0100U) {
        // literal character
        if (PLUS4EMU_UNLIKELY(buf.size() >= maxDataSize))
          throw Plus4Emu::Exception("error in compressed data");
        buf.push_back((unsigned char) c);
        continue;
      }
      // decode length:
      //   0x0101..0x0108: 3 to 10
      //   0x0109..0x010C: 11 to 18 (1 extra bit)
      //   0x010D..0x0110: 19 to 34 (2 extra bits)
      //   0x0111..0x0114: 35 to 66 (3 extra bits)
      //   0x0115..0x0118: 67 to 130 (4 extra bits)
      //   0x0119..0x011C: 131 to 258 (5 extra bits)
      //   0x011D:         258, no extra bits
      //   0x011E:         2 bytes (non-standard extension)
      //   0x011F:         2 bytes, repeat prv. offset (non-standard extension)
      unsigned int  len = c - 0x0100U;
      unsigned int  d = prvDistance;
      if (len > 8U) {
        if (len > 28U) {
          len = (len == 29U ? 256U : 0U);
        }
        else {
          unsigned char nBits = (unsigned char) ((len - 5U) >> 2);
          len = (((((len - 1U) & 3U) | 4U) << nBits) | readBits(nBits)) + 1U;
        }
      }
      len = len + 2U;
      // decode offset:
      //     0..3: 1 to 4
      //     4..5: 5 to 8 (1 extra bit)
      //     ...
      //   28..29: 16385 to 32768 (13 extra bits)
      //   30..31: 32769 to 65536 (14 extra bits, non-standard extension)
      if (c != 0x011FU) {
        c = huffmanDecode(1);
        if (c < 4U) {
          d = c + 1U;
        }
        else {
          unsigned char nBits = (unsigned char) ((c - 2U) >> 1);
          d = ((((c & 1U) | 2U) << nBits) | readBits(nBits)) + 1U;
        }
      }
      if (!(d > 0U && d <= (unsigned int) buf.size()))
        throw Plus4Emu::Exception("error in compressed data");
      prvDistance = d;
      if (PLUS4EMU_UNLIKELY((buf.size() + size_t(len)) > maxDataSize))
        throw Plus4Emu::Exception("error in compressed data");
      do {
        buf.push_back(buf[buf.size() - size_t(d)]);
      } while (--len);
    }
    return isLastBlock;
  }

  // --------------------------------------------------------------------------

  Decompressor_ZLib::Decompressor_ZLib()
    : Decompressor(),
      huffmanSymCntTable0((unsigned int *) 0),
      huffmanOffsetTable0((unsigned int *) 0),
      huffmanDecodeTable0((unsigned int *) 0),
      huffmanSymCntTable1((unsigned int *) 0),
      huffmanOffsetTable1((unsigned int *) 0),
      huffmanDecodeTable1((unsigned int *) 0),
      shiftRegister(0x00),
      shiftRegisterCnt(0),
      inputBuffer((unsigned char *) 0),
      inputBufferSize(0),
      inputBufferPosition(0)
  {
    size_t  totalTableSize = 15 + 15 + 288 + 15 + 15 + 32;
    huffmanSymCntTable0 = new unsigned int[totalTableSize];
    for (size_t i = 0; i < totalTableSize; i++)
      huffmanSymCntTable0[i] = 0U;
    huffmanOffsetTable0 = &(huffmanSymCntTable0[15]);
    huffmanDecodeTable0 = &(huffmanOffsetTable0[15]);
    huffmanSymCntTable1 = &(huffmanDecodeTable0[288]);
    huffmanOffsetTable1 = &(huffmanSymCntTable1[15]);
    huffmanDecodeTable1 = &(huffmanOffsetTable1[15]);
  }

  Decompressor_ZLib::~Decompressor_ZLib()
  {
    delete[] huffmanSymCntTable0;
  }

  void Decompressor_ZLib::decompressData(
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

  void Decompressor_ZLib::decompressData(
      std::vector< unsigned char >& outBuf,
      const std::vector< unsigned char >& inBuf)
  {
    decompressData(outBuf, inBuf, (size_t *) 0);
  }

  void Decompressor_ZLib::decompressData(
      std::vector< unsigned char >& outBuf,
      const std::vector< unsigned char >& inBuf, size_t *inBufPos)
  {
    outBuf.clear();
    if (inBuf.size() < 1)
      return;
    inputBuffer = &(inBuf.front());
    inputBufferSize = inBuf.size();
    inputBufferPosition = 0;
    if (inBufPos)
      inputBufferPosition = *inBufPos;
    // check ZLib header
    {
      unsigned int  tmp = (readByte() << 8) | readByte();
      if (tmp >= 0x9000U ||             // b12..b15: log2(dictionary size) - 8
          (tmp & 0x0F00U) != 0x0800U || //  b8..b12: comp. method (8: Deflate)
          (tmp % 31U) != 0U) {
        throw Plus4Emu::Exception("error in compressed data");
      }
    }
    shiftRegister = 0x00;
    shiftRegisterCnt = 0;
    // decompress all data blocks
    while (!decompressDataBlock(outBuf))
      ;
    // verify Adler-32 checksum
    unsigned int  adler32Sum = (readByte() << 24) | (readByte() << 16)
                               | (readByte() << 8) | readByte();
    unsigned int  tmp1 = 1U;
    unsigned int  tmp2 = 0U;
    for (size_t i = 0; i < outBuf.size(); i++) {
      tmp1 += (unsigned int) outBuf[i];
      tmp1 = (tmp1 < 65521U ? tmp1 : (tmp1 - 65521U));
      tmp2 += tmp1;
      tmp2 = (tmp2 < 65521U ? tmp2 : (tmp2 - 65521U));
    }
    if ((tmp1 | (tmp2 << 16)) != adler32Sum)
      throw Plus4Emu::Exception("error in compressed data");
    // on successful decompression, all input data must be consumed
    if (inBufPos)
      *inBufPos = inputBufferPosition;
    else if (inputBufferPosition < inputBufferSize)
      throw Plus4Emu::Exception("error in compressed data");
  }

}       // namespace Plus4Compress

