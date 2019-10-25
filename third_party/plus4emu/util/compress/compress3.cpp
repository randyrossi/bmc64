
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
#include "comprlib.hpp"
#include "compress3.hpp"

namespace Plus4Compress {

  static size_t getSymbolSize(unsigned int n)
  {
    size_t  nBits = 0;
    while (n > 1U) {
      n = n >> 1;
      nBits++;
    }
    return nBits;
  }

  static unsigned int encodeSymbol(unsigned int n)
  {
    unsigned char nBits = 0;
    unsigned int  tmp = n;
    while (tmp > 1U) {
      tmp = tmp >> 1;
      nBits++;
    }
    n = ((unsigned int) nBits << 24) | (n & ((1U << nBits) - 1U));
    return n;
  }

  // --------------------------------------------------------------------------

  void Compressor_M3::writeRepeatCode(std::vector< unsigned int >& buf,
                                      size_t d, size_t n)
  {
    if (d > 0) {
      n--;
      if (n == 1) {
        if (d > 510)
          throw Plus4Emu::Exception("internal error: match offset overflow");
        d++;
      }
    }
    unsigned char nBits = (unsigned char) getSymbolSize((unsigned int) n);
    buf.push_back(((unsigned int) (nBits + 1) << 24)
                  | ((1U << (nBits + 1)) - 2U));
    if (n > 1)
      buf.push_back(encodeSymbol((unsigned int) n));
    if (d < 1)
      return;
    nBits = (unsigned char) getSymbolSize((unsigned int) d);
    buf.push_back((n == 1 ? 0x03000000U : 0x04000000U)
                  | (unsigned int) (nBits - (unsigned char) (n == 1)));
    if (d > 1)
      buf.push_back(encodeSymbol((unsigned int) d));
  }

  inline size_t Compressor_M3::getRepeatCodeLength(size_t d, size_t n) const
  {
    size_t  nBits = 0;
    if (d == 0) {
      nBits = (getSymbolSize((unsigned int) n) << 1) + 1;
    }
    else {
      nBits = (getSymbolSize((unsigned int) (n - 1)) << 1)
              + getSymbolSize(d + size_t(n == 2)) + (n == 2 ? 4 : 5);
    }
    return nBits;
  }

  void Compressor_M3::optimizeMatches(LZMatchParameters *matchTable,
                                      size_t *bitCountTable,
                                      unsigned char *bitIncMaxTable,
                                      size_t nBytes)
  {
    size_t  maxLen = maxRepeatLen;
    size_t  minLen = minRepeatLen;
    for (size_t i = nBytes; --i > 0; ) {
      size_t  bestSize = 0x7FFFFFFF;
      size_t  bestLen = 1;
      size_t  bestOffs = 0;
      const unsigned int  *matchPtr = searchTable->getMatches(i);
      size_t  len = *matchPtr;          // match length
      if (len >= minLen) {
        bestLen = len;
        bestOffs = *(++matchPtr) >> 10;
        bestSize = getRepeatCodeLength(bestOffs, len)
                   + size_t((i + len) < nBytes) + bitCountTable[i + len];
        if (len > maxLen) {
          // long LZ77 match
          if (bestOffs == 1) {
            // if a long RLE match is possible, use that
            matchTable[i].d = 1;
            matchTable[i].len = (unsigned short) len;
            bitCountTable[i] = bestSize;
            bitIncMaxTable[i] = bitIncMaxTable[i + len];
            continue;
          }
          len = maxLen;
        }
        // otherwise check all possible LZ77 match lengths,
        for ( ; len > 0; len = (*matchPtr & 0x03FFU)) {
          unsigned int  d = *matchPtr >> 10;
          size_t  nxtLen = *(++matchPtr) & 0x03FFU;
          nxtLen = (nxtLen >= minLen ? nxtLen : (minLen - 1));
          size_t  nBitsBase = getSymbolSize(d) + 6;
          while (len > nxtLen) {
            size_t  nBits = (getSymbolSize((unsigned int) (len - 1)) << 1)
                            + nBitsBase + bitCountTable[i + len];
            if (PLUS4EMU_UNLIKELY(len < 3)) {
              if (PLUS4EMU_UNLIKELY(d > 510U))
                nBits = 0x7FFFFFFF;
              else
                nBits = (nBits - nBitsBase) + (getSymbolSize(d + 1) + 5);
            }
            if (nBits < bestSize) {
              bestSize = nBits;
              bestOffs = d;
              bestLen = len;
            }
            len--;
          }
        }
      }
      {
        size_t  nBitsBase = 9;
        for (size_t k = 1; (i + k) <= nBytes; k++) {
          // and all possible literal sequence lengths
          size_t  nBits = bitCountTable[i + k] + nBitsBase;
          nBitsBase += size_t((k & (k + 1)) != 0 ? 8 : 10);
          if (nBits <= bestSize &&
              !((i + k) < nBytes && matchTable[i + k].d == 0)) {
            // a literal sequence can only be followed by an LZ77 match
            bestSize = nBits;
            bestOffs = 0;
            bestLen = k;
          }
          else if (nBits > (bestSize + 31)) {
            break;
          }
        }
      }
      matchTable[i].d = (unsigned short) bestOffs;
      matchTable[i].len = (unsigned short) bestLen;
      // store total compressed size in bits from this position
      bitCountTable[i] = bestSize;
      // store maximum size increase in bits from this position
      bitIncMaxTable[i] = bitIncMaxTable[i + bestLen];
      if (bestSize > ((nBytes - i) << 3)) {
        unsigned char tmp = (unsigned char) (bestSize - ((nBytes - i) << 3));
        if (tmp > bitIncMaxTable[i])
          bitIncMaxTable[i] = tmp;
      }
    }
    // at position 0: only a literal sequence is possible, with no flag bit
    {
      size_t  bestSize = 0x7FFFFFFF;
      size_t  bestLen = 1;
      size_t  nBitsBase = 9;
      for (size_t k = 1; k <= nBytes; k++) {
        // check all possible literal sequence lengths
        size_t  nBits = bitCountTable[k] + nBitsBase;
        nBitsBase += size_t((k & (k + 1)) != 0 ? 8 : 10);
        // check if the compressed data would
        // overflow a 2-byte decompressor buffer
        if (nBits <= bestSize &&
            !((k < nBytes && matchTable[k].d == 0) ||
              size_t(bitIncMaxTable[k]) >= (((nBits + 7) & 7) + 16))) {
          bestSize = nBits;
          bestLen = k;
        }
      }
      matchTable[0].d = 0;
      matchTable[0].len = (unsigned short) bestLen;
      bitCountTable[0] = bestSize;
    }
  }

  void Compressor_M3::compressData_(std::vector< unsigned int >& tmpOutBuf,
                                    const std::vector< unsigned char >& inBuf)
  {
    size_t  nBytes = inBuf.size();
    tmpOutBuf.clear();
    // compress data by searching for repeated byte sequences,
    // and replacing them with length/distance codes
    std::vector< LZMatchParameters >  matchTable(nBytes);
    std::vector< size_t >         bitCountTable(nBytes + 1, 0);
    std::vector< unsigned char >  bitIncMaxTable(nBytes + 1, 0x00);
    optimizeMatches(&(matchTable.front()), &(bitCountTable.front()),
                    &(bitIncMaxTable.front()), nBytes);
    if (size_t(matchTable[0].len) >= nBytes &&
        (bitCountTable[1] + 1) < ((nBytes - 1) << 3)) {
      std::fprintf(stderr, "WARNING: disabled compression to avoid "
                           "Z80 decompressor buffer overflow\n");
    }
    // write compressed data
    bool    prvLiteralSeqFlag = false;
    for (size_t i = 0; i < nBytes; ) {
      LZMatchParameters&  tmp = matchTable[i];
      if (tmp.d > 0) {
        // write LZ77 match
        if (!prvLiteralSeqFlag)
          tmpOutBuf.push_back(0x01000001U);
        writeRepeatCode(tmpOutBuf, tmp.d, tmp.len);
        i = i + tmp.len;
        prvLiteralSeqFlag = false;
      }
      else {
        // write literal sequence
        if (i != 0)
          tmpOutBuf.push_back(0x01000000U);
        writeRepeatCode(tmpOutBuf, 0, tmp.len);
        for (size_t j = 0; j < size_t(tmp.len); j++) {
          tmpOutBuf.push_back(0x88000000U | (unsigned int) inBuf[i]);
          i++;
        }
        prvLiteralSeqFlag = true;
      }
    }
  }

  Compressor_M3::Compressor_M3(std::vector< unsigned char >& outBuf_)
    : Compressor(outBuf_),
      searchTable((LZSearchTable *) 0)
  {
  }

  Compressor_M3::~Compressor_M3()
  {
    if (searchTable)
      delete searchTable;
  }

  bool Compressor_M3::compressData(const std::vector< unsigned char >& inBuf,
                                   unsigned int startAddr, bool isLastBlock,
                                   bool enableProgressDisplay)
  {
    (void) enableProgressDisplay;
    // allow start address 0100H (program with EXOS 5 header) for compatibility
    if ((startAddr != 0x0000U && startAddr != 0xFFFFFFFFU) || !isLastBlock) {
      throw Plus4Emu::Exception("Compressor_M3::compressData(): "
                                "internal error: "
                                "unsupported output format parameters");
    }
    if (searchTable) {
      delete searchTable;
      searchTable = (LZSearchTable *) 0;
    }
    size_t        savedOutBufPos = 0x7FFFFFFF;
    unsigned char outputShiftReg = 0xFF;
    int           outputBitCnt = 0;
    size_t        nBytes = inBuf.size();
    if (nBytes < 1)
      return true;
    if (nBytes > 65535)
      throw Plus4Emu::Exception("input data size is too large");
    std::vector< unsigned int > outBufTmp;
    try {
      std::vector< unsigned char >  inBufRev(nBytes);
      for (size_t i = 0; i < nBytes; i++)
        inBufRev[(nBytes - 1) - i] = inBuf[i];          // reverse input data
      searchTable =
          new LZSearchTable(minRepeatLen, maxRepeatLen, lengthMaxValue,
                            0, 510, maxRepeatDist);
      searchTable->findMatches(&(inBufRev.front()), 0, nBytes);
      std::vector< unsigned int >   tmpBuf;
      compressData_(tmpBuf, inBufRev);
      // calculate compressed size
      size_t  compressedSize = 0;
      for (size_t i = 0; i < tmpBuf.size(); i++)
        compressedSize += size_t((tmpBuf[i] & 0x7F000000U) >> 24);
      compressedSize = (compressedSize + 7) & (~(size_t(7)));
      size_t  uncompressedSize = nBytes * 8;
      if (compressedSize >= uncompressedSize) {
        // if cannot reduce the data size, store without compression
        outBufTmp.push_back(0x88000000U);
        outBufTmp.push_back(0x88000000U);
        for (size_t i = 0; i < nBytes; i++)
          outBufTmp.push_back(0x88000000U | (unsigned int) inBufRev[i]);
        outBufTmp.push_back(0x88000000U | (unsigned int) (nBytes >> 8));
        outBufTmp.push_back(0x88000000U | (unsigned int) (nBytes & 0xFF));
      }
      else {
        size_t  tmp = compressedSize >> 3;
        outBufTmp.push_back(0x88000000U
                            | (unsigned int) ((nBytes - tmp) >> 8));
        outBufTmp.push_back(0x88000000U
                            | (unsigned int) ((nBytes - tmp) & 0xFF));
        // append compressed data to output buffer
        for (size_t i = 0; i < tmpBuf.size(); i++)
          outBufTmp.push_back(tmpBuf[i]);
        outBufTmp.push_back(0x88000000U | (unsigned int) (tmp >> 8));
        outBufTmp.push_back(0x88000000U | (unsigned int) (tmp & 0xFF));
      }
      delete searchTable;
      searchTable = (LZSearchTable *) 0;
    }
    catch (...) {
      if (searchTable) {
        delete searchTable;
        searchTable = (LZSearchTable *) 0;
      }
      throw;
    }
    // pack output data
    std::vector< unsigned char >  tmpOutBuf;
    for (size_t i = 0; i < outBufTmp.size(); i++) {
      unsigned int  c = outBufTmp[i];
      if (c >= 0x80000000U) {
        // special case for literal bytes, which are stored byte-aligned
        if (outputBitCnt > 0 && savedOutBufPos >= tmpOutBuf.size()) {
          // reserve space for the shift register to be stored later when
          // it is full, and save the write position
          savedOutBufPos = tmpOutBuf.size();
          tmpOutBuf.push_back((unsigned char) 0x00);
        }
        unsigned int  n = ((c & 0x7F000000U) + 0x07000000U) >> 27;
        while (n > 0U) {
          n--;
          tmpOutBuf.push_back((unsigned char) ((c >> (n * 8U)) & 0xFFU));
        }
      }
      else {
        unsigned int  nBits = c >> 24;
        c = c & 0x00FFFFFFU;
        for (unsigned int j = nBits; j > 0U; ) {
          j--;
          unsigned int  b = (unsigned int) (bool(c & (1U << j)));
          outputShiftReg = ((outputShiftReg & 0xFE) >> 1)
                           | (unsigned char) (b << 7);
          if (++outputBitCnt >= 8) {
            if (savedOutBufPos >= tmpOutBuf.size()) {
              tmpOutBuf.push_back(outputShiftReg);
            }
            else {
              // store at saved position if any literal bytes were inserted
              tmpOutBuf[savedOutBufPos] = outputShiftReg;
              savedOutBufPos = 0x7FFFFFFF;
            }
            outputShiftReg = 0xFF;
            outputBitCnt = 0;
          }
        }
      }
    }
    while (outputBitCnt != 0) {
      outputShiftReg = ((outputShiftReg & 0xFE) >> 1) | 0x80;
      if (++outputBitCnt >= 8) {
        if (savedOutBufPos >= tmpOutBuf.size()) {
          tmpOutBuf.push_back(outputShiftReg);
        }
        else {
          // store at saved position if any literal bytes were inserted
          tmpOutBuf[savedOutBufPos] = outputShiftReg;
          savedOutBufPos = 0x7FFFFFFF;
        }
        outputShiftReg = 0xFF;
        outputBitCnt = 0;
      }
    }
    // reverse output data
    for (size_t i = tmpOutBuf.size(); i-- > 0; )
      outBuf.push_back(tmpOutBuf[i]);
    return true;
  }

}       // namespace Plus4Compress

