
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
#include "system.hpp"
#include "comprlib.hpp"
#include "decompm2.hpp"

#define COMPRESS_MAX_THREADS    4
#define COMPRESS_BLOCK_SIZE     65536

namespace Plus4Emu {

  class Compressor_M2 {
   public:
    static const size_t minRepeatDist = 1;
    static const size_t maxRepeatDist = 131072; // was 524288 in epcompress
    static const size_t minRepeatLen = 1;
    static const size_t maxRepeatLen = 128;     // was 512 in epcompress
    static const unsigned int lengthMaxValue = 65535U;
   private:
    static const size_t lengthNumSlots = 8;
    static const size_t lengthPrefixSizeTable[lengthNumSlots];
    static const unsigned int offs1MaxValue = 4096U;
    static const size_t offs1NumSlots = 4;
    static const size_t offs1PrefixSize = 2;
    static const unsigned int offs2MaxValue = 16384U;
    static const size_t offs2NumSlots = 8;
    static const size_t offs2PrefixSize = 3;
    static const unsigned int offs3MaxValue = (unsigned int) maxRepeatDist;
    static const size_t offs3SlotCntTable[4];
    static const size_t literalSequenceMinLength = lengthNumSlots + 9;
    // --------
    struct LZMatchParameters {
      unsigned int  d;
      unsigned int  len;
      LZMatchParameters()
        : d(0),
          len(1)
      {
      }
      LZMatchParameters(const LZMatchParameters& r)
        : d(r.d),
          len(r.len)
      {
      }
      ~LZMatchParameters()
      {
      }
      inline LZMatchParameters& operator=(const LZMatchParameters& r)
      {
        d = r.d;
        len = r.len;
        return (*this);
      }
      inline void clear()
      {
        d = 0;
        len = 1;
      }
    };
    // --------
    Plus4Compress::EncodeTable  lengthEncodeTable;
    Plus4Compress::EncodeTable  offs1EncodeTable;
    Plus4Compress::EncodeTable  offs2EncodeTable;
    Plus4Compress::EncodeTable  offs3EncodeTable;
    size_t      offs3NumSlots;
    size_t      offs3PrefixSize;
    Plus4Compress::LZSearchTable  *searchTable;
    // --------
    void writeRepeatCode(std::vector< unsigned int >& buf, size_t d, size_t n);
    inline size_t getRepeatCodeLength(size_t d, size_t n) const;
    void optimizeMatches_noStats(LZMatchParameters *matchTable,
                                 size_t *bitCountTable,
                                 size_t offs, size_t nBytes);
    // NOTE: unsigned int is only guaranteed to work for offsSumTable
    // with maxOffset <= 192 KiB
    void optimizeMatches(LZMatchParameters *matchTable,
                         size_t *bitCountTable, unsigned int *offsSumTable,
                         size_t offs, size_t nBytes);
    size_t compressData(std::vector< unsigned int >& outBuf,
                        const unsigned char *inBuf, size_t offs, size_t nBytes,
                        bool firstPass, bool fastMode = false);
   public:
    Compressor_M2();
    virtual ~Compressor_M2();
    void compressDataBlock(std::vector< unsigned int >& outBuf,
                           const unsigned char *inBuf, size_t offs,
                           size_t nBytes, size_t bufSize,
                           bool isLastBlock, bool fastMode = false);
  };

  // --------------------------------------------------------------------------

  const size_t Compressor_M2::lengthPrefixSizeTable[lengthNumSlots] = {
    1, 2, 3, 4, 5, 6, 7, 8
  };

  const size_t Compressor_M2::offs3SlotCntTable[4] = {
    4, 8, 16, 32
  };

  // --------------------------------------------------------------------------

  void Compressor_M2::writeRepeatCode(std::vector< unsigned int >& buf,
                                      size_t d, size_t n)
  {
    Plus4Compress::EncodeTable& offsEncodeTable =
        (n > 2 ? offs3EncodeTable :
         (n > 1 ? offs2EncodeTable : offs1EncodeTable));
    unsigned int  offsPrefixSize =
        (unsigned int) (n > 2 ? offs3PrefixSize :
                        (n > 1 ? offs2PrefixSize : offs1PrefixSize));
    n = n - minRepeatLen;
    d = d - minRepeatDist;
    unsigned int  slotNum =
        (unsigned int) lengthEncodeTable.getSymbolSlotIndex((unsigned int) n);
    unsigned int  slotSize =
        (unsigned int) lengthEncodeTable.getSlotSize(slotNum);
    slotNum = slotNum + 2U;
    if (!slotSize) {
      buf.push_back((slotNum << 24) | ((1U << slotNum) - 2U));
    }
    else {
      buf.push_back(((slotNum << 24) | (((1U << slotNum) - 2U) << slotSize))
                    + lengthEncodeTable.encodeSymbol((unsigned int) n));
    }
    slotNum =
        (unsigned int) offsEncodeTable.getSymbolSlotIndex((unsigned int) d);
    slotSize = (unsigned int) offsEncodeTable.getSlotSize(slotNum);
    if (!slotSize) {
      buf.push_back((offsPrefixSize << 24) | slotNum);
    }
    else {
      buf.push_back(((offsPrefixSize << 24) | (slotNum << slotSize))
                    + offsEncodeTable.encodeSymbol((unsigned int) d));
    }
  }

  inline size_t Compressor_M2::getRepeatCodeLength(size_t d, size_t n) const
  {
    n = n - minRepeatLen;
    size_t  nBits = lengthEncodeTable.getSymbolSize(n) + 1;
    d = d - minRepeatDist;
    if ((n + minRepeatLen) > 2)
      nBits += offs3EncodeTable.getSymbolSize(d);
    else if ((n + minRepeatLen) > 1)
      nBits += offs2EncodeTable.getSymbolSize(d);
    else
      nBits += offs1EncodeTable.getSymbolSize(d);
    return nBits;
  }

  void Compressor_M2::optimizeMatches_noStats(LZMatchParameters *matchTable,
                                              size_t *bitCountTable,
                                              size_t offs, size_t nBytes)
  {
    // simplified optimal parsing code for the first optimization pass
    // when no symbol size information is available from the statistical
    // compression
    static const size_t matchSize1 = 4; // 6 if offset > 16, 8 if offset > 64
    static const size_t matchSize2 = 6; // 8 if offset > 1024
    static const size_t matchSize3 = 8;
    for (size_t i = nBytes; i-- > 0; ) {
      size_t  bestSize = 0x7FFFFFFF;
      size_t  bestLen = 1;
      size_t  bestOffs = 0;
      const unsigned int  *matchPtr = searchTable->getMatches(offs + i);
      size_t  len = matchPtr[0];        // match length
      if (len > (nBytes - i))
        len = nBytes - i;
      if (len > maxRepeatLen) {
        if (matchPtr[1] > 1024U) {
          // long LZ77 match
          bestSize = bitCountTable[i + len] + matchSize3;
          bestOffs = matchPtr[1] >> 10;
          bestLen = len;
          len = maxRepeatLen;
        }
        else {
          // if a long RLE match is possible, use that
          matchTable[i].d = 1;
          matchTable[i].len = (unsigned int) len;
          bitCountTable[i] = bitCountTable[i + len] + matchSize3;
          continue;
        }
      }
      // otherwise check all possible LZ77 match lengths,
      matchPtr++;
      for ( ; len > 0; len = (*matchPtr & 0x03FFU)) {
        if (len > (nBytes - i))
          len = nBytes - i;
        unsigned int  d = *matchPtr >> 10;
        size_t        nxtLen = *(++matchPtr) & 0x03FFU;
        nxtLen = (nxtLen >= minRepeatLen ? nxtLen : (minRepeatLen - 1));
        if (len <= nxtLen)
          continue;                     // ignore match
        if (len >= 3) {
          size_t  minLenM1 = (nxtLen > 2 ? nxtLen : 2);
          do {
            size_t  nBits = bitCountTable[i + len] + matchSize3;
            if (nBits <= bestSize) {
              bestSize = nBits;
              bestOffs = d;
              bestLen = len;
            }
          } while (--len > minLenM1);
          if (nxtLen >= 2)
            continue;
          len = 2;
        }
        // check short match lengths:
        if (len == 2) {                 // 2 bytes
          if (d <= offs2MaxValue) {
            size_t  nBits = bitCountTable[i + 2] + matchSize2
                            + (size_t(d > 1024U) << 1);
            if (nBits <= bestSize) {
              bestSize = nBits;
              bestOffs = d;
              bestLen = 2;
            }
          }
          if (nxtLen >= 1)
            continue;
        }
        if (d <= offs1MaxValue) {       // 1 byte
          size_t  nBits = bitCountTable[i + 1] + matchSize1
                          + ((size_t(d > 16U) + size_t(d > 64U)) << 1);
          if (nBits <= bestSize) {
            bestSize = nBits;
            bestOffs = d;
            bestLen = 1;
          }
        }
      }
      if (bestSize >= (bitCountTable[i + 1] + 8)) {
        // literal byte,
        size_t  nBits = bitCountTable[i + 1] + 9;
        if (nBits <= bestSize) {
          bestSize = nBits;
          bestOffs = 0;
          bestLen = 1;
        }
        for (size_t k = literalSequenceMinLength;
             k <= (literalSequenceMinLength + 255) && (i + k) <= nBytes;
             k++) {
          // and all possible literal sequence lengths
          nBits = bitCountTable[i + k] + (k * 8 + literalSequenceMinLength);
          if (nBits > (bestSize + literalSequenceMinLength))
            break;      // quit the loop earlier if the data can be compressed
          if (nBits <= bestSize) {
            bestSize = nBits;
            bestOffs = 0;
            bestLen = k;
          }
        }
      }
      matchTable[i].d = (unsigned int) bestOffs;
      matchTable[i].len = (unsigned int) bestLen;
      bitCountTable[i] = bestSize;
    }
  }

  void Compressor_M2::optimizeMatches(LZMatchParameters *matchTable,
                                      size_t *bitCountTable,
                                      unsigned int *offsSumTable,
                                      size_t offs, size_t nBytes)
  {
    size_t  len1BitsP1 = lengthEncodeTable.getSymbolSize(1U - minRepeatLen) + 1;
    size_t  len2BitsP1 = lengthEncodeTable.getSymbolSize(2U - minRepeatLen) + 1;
    for (size_t i = nBytes; i-- > 0; ) {
      size_t  bestSize = 0x7FFFFFFF;
      size_t  bestLen = 1;
      size_t  bestOffs = 0;
      unsigned int  bestOffsSum = 0xFFFFFFFFU;
      const unsigned int  *matchPtr = searchTable->getMatches(offs + i);
      size_t  len = matchPtr[0];        // match length
      if (len > (nBytes - i))
        len = nBytes - i;
      if (len > maxRepeatLen) {
        if (matchPtr[1] > 1024U) {
          // long LZ77 match
          bestOffs = matchPtr[1] >> 10;
          bestLen = len;
          bestSize = getRepeatCodeLength(bestOffs, len)
                     + bitCountTable[i + len];
          bestOffsSum = offsSumTable[i + len] + (unsigned int) bestOffs;
          len = maxRepeatLen;
        }
        else {
          // if a long RLE match is possible, use that
          matchTable[i].d = 1;
          matchTable[i].len = (unsigned int) len;
          bitCountTable[i] = bitCountTable[i + len]
                             + getRepeatCodeLength(1, len);
          offsSumTable[i] = offsSumTable[i + len] + 1U;
          continue;
        }
      }
      // otherwise check all possible LZ77 match lengths,
      matchPtr++;
      for ( ; len > 0; len = (*(++matchPtr) & 0x03FFU)) {
        if (len > (nBytes - i))
          len = nBytes - i;
        unsigned int  d = *matchPtr >> 10;
        if (len >= 3) {
          // flag bit + offset bits
          size_t  nBitsBase = offs3EncodeTable.getSymbolSize(
                                  d - (unsigned int) minRepeatDist) + 1;
          do {
            size_t  nBits = lengthEncodeTable.getSymbolSize(
                                (unsigned int) (len - minRepeatLen))
                            + nBitsBase + bitCountTable[i + len];
            if (nBits < bestSize ||
                (nBits == bestSize &&
                 (offsSumTable[i + len] + d) <= bestOffsSum)) {
              bestSize = nBits;
              bestOffs = d;
              bestLen = len;
              bestOffsSum = offsSumTable[i + len] + d;
            }
          } while (--len >= 3);
        }
        // check short match lengths:
        if (len == 2) {                                         // 2 bytes
          if (d <= offs2EncodeTable.getSymbolsEncoded()) {
            size_t  nBits = len2BitsP1 + offs2EncodeTable.getSymbolSize(
                                             d - (unsigned int) minRepeatDist)
                            + bitCountTable[i + 2];
            if (nBits < bestSize ||
                (nBits == bestSize &&
                 (offsSumTable[i + 2] + d) <= bestOffsSum)) {
              bestSize = nBits;
              bestOffs = d;
              bestLen = 2;
              bestOffsSum = offsSumTable[i + 2] + d;
            }
          }
        }
        if (d <= offs1EncodeTable.getSymbolsEncoded()) {        // 1 byte
          size_t  nBits = len1BitsP1 + offs1EncodeTable.getSymbolSize(
                                           d - (unsigned int) minRepeatDist)
                          + bitCountTable[i + 1];
          if (nBits < bestSize ||
              (nBits == bestSize && (offsSumTable[i + 1] + d) <= bestOffsSum)) {
            bestSize = nBits;
            bestOffs = d;
            bestLen = 1;
            bestOffsSum = offsSumTable[i + 1] + d;
          }
        }
      }
      if (bestSize >= (bitCountTable[i + 1] + 8)) {
        // literal byte,
        size_t  nBits = bitCountTable[i + 1] + 9;
        if (nBits < bestSize ||
            (nBits == bestSize && offsSumTable[i + 1] <= bestOffsSum)) {
          bestSize = nBits;
          bestOffs = 0;
          bestLen = 1;
          bestOffsSum = offsSumTable[i + 1];
        }
        if ((i + literalSequenceMinLength) <= nBytes &&
            (bitCountTable[i + literalSequenceMinLength]
             + (literalSequenceMinLength * 8)) <= bestSize) {
          for (size_t k = literalSequenceMinLength;
               k <= (literalSequenceMinLength + 255) && (i + k) <= nBytes;
               k++) {
            // and all possible literal sequence lengths
            nBits = bitCountTable[i + k] + (k * 8 + literalSequenceMinLength);
            if (nBits > bestSize) {
              if (nBits > (bestSize + literalSequenceMinLength))
                break;  // quit the loop earlier if the data can be compressed
              continue;
            }
            if (nBits == bestSize) {
              if (offsSumTable[i + k] > (offsSumTable[i + bestLen] + bestOffs))
                continue;
            }
            bestSize = nBits;
            bestOffs = 0;
            bestLen = k;
          }
          bestOffsSum = offsSumTable[i + bestLen] + bestOffs;
        }
      }
      matchTable[i].d = (unsigned int) bestOffs;
      matchTable[i].len = (unsigned int) bestLen;
      bitCountTable[i] = bestSize;
      offsSumTable[i] = bestOffsSum;
    }
  }

  size_t Compressor_M2::compressData(std::vector< unsigned int >& tmpOutBuf,
                                     const unsigned char *inBuf,
                                     size_t offs, size_t nBytes,
                                     bool firstPass, bool fastMode)
  {
    size_t  endPos = offs + nBytes;
    size_t  nSymbols = 0;
    tmpOutBuf.clear();
    if (!firstPass) {
      // generate optimal encode tables for offset values
      offs1EncodeTable.updateTables(false);
      offs2EncodeTable.updateTables(false);
      offs3EncodeTable.updateTables(fastMode);
      offs3NumSlots = offs3EncodeTable.getSlotCnt();
      offs3PrefixSize = offs3EncodeTable.getSlotPrefixSize(0);
    }
    // compress data by searching for repeated byte sequences,
    // and replacing them with length/distance codes
    std::vector< LZMatchParameters >  matchTable(nBytes);
    {
      std::vector< size_t > bitCountTable(nBytes + 1, 0);
      if (!firstPass) {
        std::vector< unsigned int > offsSumTable(nBytes + 1, 0U);
        lengthEncodeTable.setUnencodedSymbolSize(lengthNumSlots + 15);
        optimizeMatches(&(matchTable.front()), &(bitCountTable.front()),
                        &(offsSumTable.front()), offs, nBytes);
      }
      else {
        // first pass: no symbol size information is available yet
        optimizeMatches_noStats(&(matchTable.front()), &(bitCountTable.front()),
                                offs, nBytes);
      }
    }
    lengthEncodeTable.setUnencodedSymbolSize(8192);
    // generate optimal encode table for length values
    for (size_t i = offs; i < endPos; ) {
      LZMatchParameters&  tmp = matchTable[i - offs];
      if (tmp.d > 0) {
        long    unencodedCost = long(tmp.len) * 9L - 1L;
        unencodedCost -=
            (tmp.len > 1 ? long(offs2PrefixSize) : long(offs1PrefixSize));
        unencodedCost = (unencodedCost > 0L ? unencodedCost : 0L);
        lengthEncodeTable.addSymbol(tmp.len - minRepeatLen,
                                    size_t(unencodedCost));
      }
      i += size_t(tmp.len);
    }
    lengthEncodeTable.updateTables(false);
    // update LZ77 offset statistics for calculating encode tables later
    for (size_t i = offs; i < endPos; ) {
      LZMatchParameters&  tmp = matchTable[i - offs];
      if (tmp.d > 0) {
        if (lengthEncodeTable.getSymbolSize(tmp.len - minRepeatLen) <= 64) {
          long    unencodedCost = long(tmp.len) * 9L - 1L;
          unencodedCost -=
              long(lengthEncodeTable.getSymbolSize(tmp.len - minRepeatLen));
          unencodedCost = (unencodedCost > 0L ? unencodedCost : 0L);
          if (tmp.len > 2) {
            offs3EncodeTable.addSymbol(tmp.d - minRepeatDist,
                                       size_t(unencodedCost));
          }
          else if (tmp.len > 1) {
            offs2EncodeTable.addSymbol(tmp.d - minRepeatDist,
                                       size_t(unencodedCost));
          }
          else {
            offs1EncodeTable.addSymbol(tmp.d - minRepeatDist,
                                       size_t(unencodedCost));
          }
        }
      }
      i += size_t(tmp.len);
    }
    // first pass: there are no offset encode tables yet, so no data is written
    if (firstPass)
      return 0;
    // write encode tables
    tmpOutBuf.push_back(0x02000000U | (unsigned int) (offs3PrefixSize - 2));
    for (size_t i = 0; i < lengthNumSlots; i++) {
      unsigned int  c = (unsigned int) lengthEncodeTable.getSlotSize(i);
      tmpOutBuf.push_back(0x04000000U | c);
    }
    for (size_t i = 0; i < offs1NumSlots; i++) {
      unsigned int  c = (unsigned int) offs1EncodeTable.getSlotSize(i);
      tmpOutBuf.push_back(0x04000000U | c);
    }
    for (size_t i = 0; i < offs2NumSlots; i++) {
      unsigned int  c = (unsigned int) offs2EncodeTable.getSlotSize(i);
      tmpOutBuf.push_back(0x04000000U | c);
    }
    for (size_t i = 0; i < offs3NumSlots; i++) {
      unsigned int  c = (unsigned int) offs3EncodeTable.getSlotSize(i);
      tmpOutBuf.push_back(0x04000000U | c);
    }
    // write compressed data
    for (size_t i = offs; i < endPos; ) {
      LZMatchParameters&  tmp = matchTable[i - offs];
      if (tmp.d > 0) {
        // check if this match needs to be replaced with literals:
        size_t  nBits = getRepeatCodeLength(tmp.d, tmp.len);
        if (nBits > 64) {
          // if the match cannot be encoded, assume "infinite" size
          nBits = 0x7FFFFFFF;
        }
        if ((size_t(tmp.len) >= literalSequenceMinLength &&
             nBits > (literalSequenceMinLength + (size_t(tmp.len) * 8))) ||
            nBits >= (size_t(tmp.len) * 9)) {
          tmp.d = 0;
        }
      }
      if (tmp.d > 0) {
        // write LZ77 match
        writeRepeatCode(tmpOutBuf, tmp.d, tmp.len);
        i = i + tmp.len;
        nSymbols++;
      }
      else {
        while (size_t(tmp.len) >= literalSequenceMinLength) {
          // write literal sequence
          size_t  len = tmp.len;
          len = (len < (literalSequenceMinLength + 255) ?
                 len : (literalSequenceMinLength + 255));
          tmpOutBuf.push_back((unsigned int) ((lengthNumSlots + 1) << 24)
                              | ((1U << (unsigned int) (lengthNumSlots + 1))
                                 - 1U));
          tmpOutBuf.push_back(0x08000000U
                              | (unsigned int) (len
                                                - literalSequenceMinLength));
          for (size_t j = 0; j < len; j++) {
            tmpOutBuf.push_back(0x88000000U | (unsigned int) inBuf[i]);
            i++;
          }
          nSymbols++;
          tmp.len -= (unsigned int) len;
        }
        while (tmp.len > 0) {
          // write literal byte(s)
          tmpOutBuf.push_back(0x01000000U);
          tmpOutBuf.push_back(0x88000000U | (unsigned int) inBuf[i]);
          i++;
          nSymbols++;
          tmp.len--;
        }
      }
    }
    return nSymbols;
  }

  // --------------------------------------------------------------------------

  Compressor_M2::Compressor_M2()
    : lengthEncodeTable(lengthNumSlots, lengthMaxValue,
                        &(lengthPrefixSizeTable[0])),
      offs1EncodeTable(offs1NumSlots, offs1MaxValue, (size_t *) 0,
                       offs1PrefixSize),
      offs2EncodeTable(offs2NumSlots, offs2MaxValue, (size_t *) 0,
                       offs2PrefixSize),
      offs3EncodeTable(0, offs3MaxValue, (size_t *) 0,
                       2, 5, &(offs3SlotCntTable[0])),
      offs3NumSlots(4),
      offs3PrefixSize(2),
      searchTable((Plus4Compress::LZSearchTable *) 0)
  {
  }

  Compressor_M2::~Compressor_M2()
  {
    if (searchTable)
      delete searchTable;
  }

  void Compressor_M2::compressDataBlock(std::vector< unsigned int >& outBuf,
                                        const unsigned char *inBuf, size_t offs,
                                        size_t nBytes, size_t bufSize,
                                        bool isLastBlock, bool fastMode)
  {
    outBuf.clear();
    if ((offs + nBytes) > bufSize)
      nBytes = bufSize - offs;
    if (!inBuf || nBytes < 1)
      return;
    // FIXME: this assumes that the data is compressed in fixed size blocks
    // that maxRepeatDist is an integer multiple of
    {
      size_t  searchTableStart = (offs / maxRepeatDist) * maxRepeatDist;
      bool    searchTableNeeded = (offs == searchTableStart);
      if (searchTableNeeded) {
        if (!searchTable) {
          searchTable = new Plus4Compress::LZSearchTable(
                                minRepeatLen, maxRepeatLen, lengthMaxValue,
                                offs1MaxValue, offs2MaxValue, maxRepeatDist);
        }
        size_t  searchTableSize = bufSize - searchTableStart;
        if (searchTableSize > maxRepeatDist)
          searchTableSize = maxRepeatDist;
        searchTable->findMatches(inBuf, searchTableStart, searchTableSize);
      }
      inBuf = inBuf + searchTableStart;
      offs = offs - searchTableStart;
    }
    size_t  endPos = offs + nBytes;
    lengthEncodeTable.clear();
    offs1EncodeTable.clear();
    offs2EncodeTable.clear();
    offs3EncodeTable.clear();
    std::vector< uint64_t >     hashTable;
    std::vector< unsigned int > tmpBuf;
    const size_t  headerSize = 18;
    size_t  bestSize = 0x7FFFFFFF;
    size_t  nSymbols = 0;
    bool    doneFlag = false;
    for (size_t i = 0; i < 40; i++) {
      if (doneFlag)     // if the compression cannot be optimized further,
        continue;       // quit the loop earlier
      tmpBuf.clear();
      size_t  tmp =
          compressData(tmpBuf, inBuf, offs, nBytes, (i == 0), fastMode);
      if (i == 0)       // the first optimization pass writes no data
        continue;
      // calculate compressed size and hash value
      size_t    compressedSize = headerSize;
      uint64_t  h = 1UL;
      for (size_t j = 0; j < tmpBuf.size(); j++) {
        compressedSize += size_t((tmpBuf[j] & 0x7F000000U) >> 24);
        h = h ^ uint64_t(tmpBuf[j]);
        h = uint32_t(h) * uint64_t(0xC2B0C3CCUL);
        h = (h ^ (h >> 32)) & 0xFFFFFFFFUL;
      }
      h = h | (uint64_t(compressedSize) << 32);
      if (compressedSize < bestSize) {
        // found a better compression, so save it
        nSymbols = tmp;
        bestSize = compressedSize;
        outBuf.resize(tmpBuf.size() + 3);
        std::memcpy(&(outBuf.front()) + 3, &(tmpBuf.front()),
                    tmpBuf.size() * sizeof(unsigned int));
      }
      for (size_t j = 0; j < hashTable.size(); j++) {
        if (hashTable[j] == h) {
          // if the exact same compressed data was already generated earlier,
          // the remaining optimize iterations can be skipped
          doneFlag = true;
          break;
        }
      }
      if (!doneFlag)
        hashTable.push_back(h);         // save hash value
    }
    size_t  uncompressedSize = headerSize + (nBytes * 8);
    if (bestSize >= uncompressedSize) {
      // if cannot reduce the data size, store without compression
      outBuf.resize((endPos - offs) + 3);
      outBuf[0] = 0x10000000U | (unsigned int) (nBytes - 1);
      outBuf[1] = 0x01000000U | (unsigned int) isLastBlock;
      outBuf[2] = 0x01000000U;
      for (size_t i = offs; i < endPos; i++)
        outBuf[(i - offs) + 3] = 0x88000000U | (unsigned int) inBuf[i];
    }
    else {
      outBuf[0] = 0x10000000U | (unsigned int) (nSymbols - 1);
      outBuf[1] = 0x01000000U | (unsigned int) isLastBlock;
      outBuf[2] = 0x01000001U;
    }
  }

  // ==========================================================================

  class CompressorThread : public Thread {
   private:
    Compressor_M2 compressor;
   public:
    std::vector< unsigned int > outBuf;
    const unsigned char *inBuf;
    size_t  inBufSize;
    size_t  startPos;
    size_t  blockSize;
    bool    errorFlag;
    // --------
    CompressorThread();
    virtual ~CompressorThread();
    virtual void run();
  };

  CompressorThread::CompressorThread()
    : inBuf((unsigned char *) 0),
      inBufSize(0),
      startPos(0),
      blockSize(Compressor_M2::maxRepeatDist),
      errorFlag(false)
  {
  }

  CompressorThread::~CompressorThread()
  {
  }

  void CompressorThread::run()
  {
    try {
      if (!inBuf)
        return;
      std::vector< unsigned int > tmpBuf;
      size_t  dataSizePos = 0;
      while (startPos < inBufSize) {
        size_t  nBytes = blockSize;
        if ((startPos + nBytes) > inBufSize)
          nBytes = inBufSize - startPos;
        compressor.compressDataBlock(tmpBuf, inBuf, startPos, nBytes, inBufSize,
                                     ((startPos + nBytes) >= inBufSize), true);
        // append compressed data to output buffer
        size_t  prvSize = outBuf.size();
        if ((startPos % Compressor_M2::maxRepeatDist) == 0) {
          dataSizePos = prvSize;
          outBuf.resize(prvSize + tmpBuf.size() + 1);
          outBuf[dataSizePos] = (unsigned int) tmpBuf.size();
          prvSize++;
        }
        else {
          outBuf.resize(prvSize + tmpBuf.size());
          outBuf[dataSizePos] =
              outBuf[dataSizePos] + (unsigned int) tmpBuf.size();
        }
        std::memcpy(&(outBuf.front()) + prvSize, &(tmpBuf.front()),
                    tmpBuf.size() * sizeof(unsigned int));
        startPos = startPos + blockSize;
        if ((startPos % Compressor_M2::maxRepeatDist) == 0) {
          startPos = startPos + (Compressor_M2::maxRepeatDist
                                 * (COMPRESS_MAX_THREADS - 1));
        }
      }
    }
    catch (std::exception) {
      errorFlag = true;
    }
  }

  // --------------------------------------------------------------------------

  void compressData(std::vector< unsigned char >& outBuf,
                    const unsigned char *inBuf, size_t inBufSize)
  {
    outBuf.clear();
    if (inBufSize < 1 || !inBuf)
      return;
    CompressorThread  *compressorThreads[COMPRESS_MAX_THREADS];
    for (int i = 0; i < COMPRESS_MAX_THREADS; i++)
      compressorThreads[i] = (CompressorThread *) 0;
    try {
      size_t  startPos = 0;
      int     nThreads = 0;
      while (nThreads < COMPRESS_MAX_THREADS && startPos < inBufSize) {
        compressorThreads[nThreads] = new CompressorThread();
        compressorThreads[nThreads]->inBuf = inBuf;
        compressorThreads[nThreads]->inBufSize = inBufSize;
        compressorThreads[nThreads]->startPos = startPos;
        compressorThreads[nThreads]->blockSize = COMPRESS_BLOCK_SIZE;
        startPos = startPos + Compressor_M2::maxRepeatDist;
        nThreads++;
      }
      for (int i = 0; i < nThreads; i++)
        compressorThreads[i]->start();
      for (int i = 0; i < nThreads; i++) {
        compressorThreads[i]->join();
        // startPos is now the read position of the output buffer of the thread
        compressorThreads[i]->startPos = 0;
      }
      size_t        savedBufPos = 0x7FFFFFFF;
      unsigned char shiftReg = 0x01;
      for (int i = 0; true; i++) {
        if (i >= nThreads)
          i = 0;
        if (!compressorThreads[i]) {
          // end of compressed data for all threads
          if (shiftReg != 0x01) {
            while (!(shiftReg & 0x80))
              shiftReg = shiftReg << 1;
            shiftReg = (shiftReg & 0x7F) << 1;
            if (savedBufPos >= outBuf.size()) {
              outBuf.push_back(shiftReg);
            }
            else {
              // store at saved position if any literal bytes were inserted
              outBuf[savedBufPos] = shiftReg;
              savedBufPos = 0x7FFFFFFF;
            }
            shiftReg = 0x01;
          }
          // calculate checksum
          unsigned char crcVal = 0xFF;
          for (size_t j = outBuf.size() - 1; j > 0; j--) {
            unsigned char tmp = crcVal ^ outBuf[j];
            crcVal = (((tmp << 1) | ((tmp >> 7) & 0x01)) + 0xAC) & 0xFF;
          }
          crcVal = (unsigned char) ((0x0180 - 0xAC) >> 1) ^ crcVal;
          outBuf[0] = crcVal;
          break;
        }
        if (compressorThreads[i]->errorFlag)
          throw Exception("error compressing data");
        if (compressorThreads[i]->startPos
            >= compressorThreads[i]->outBuf.size()) {
          // end of compressed data for this thread
          delete compressorThreads[i];
          compressorThreads[i] = (CompressorThread *) 0;
          continue;
        }
        // pack output data
        if (outBuf.size() < 1)
          outBuf.push_back(0x00);       // reserve space for checksum byte
        startPos = compressorThreads[i]->startPos + 1;
        compressorThreads[i]->startPos =
            startPos + size_t(compressorThreads[i]->outBuf[startPos - 1]);
        for (size_t j = startPos; j < compressorThreads[i]->startPos; j++) {
          unsigned int  c = compressorThreads[i]->outBuf[j];
          if (c >= 0x80000000U) {
            // special case for literal bytes, which are stored byte-aligned
            if (shiftReg != 0x01 && savedBufPos >= outBuf.size()) {
              // reserve space for the shift register to be stored later when
              // it is full, and save the write position
              savedBufPos = outBuf.size();
              outBuf.push_back(0x00);
            }
            unsigned int  nBytes = ((c & 0x7F000000U) + 0x07000000U) >> 27;
            while (nBytes > 0U) {
              nBytes--;
              outBuf.push_back((unsigned char) ((c >> (nBytes * 8U)) & 0xFFU));
            }
          }
          else {
            unsigned int  nBits = c >> 24;
            c = c & 0x00FFFFFFU;
            for (unsigned int k = nBits; k > 0U; ) {
              k--;
              unsigned int  b = (unsigned int) (bool(c & (1U << k)));
              bool          srFull = bool(shiftReg & 0x80);
              shiftReg = ((shiftReg & 0x7F) << 1) | (unsigned char) b;
              if (srFull) {
                if (savedBufPos >= outBuf.size()) {
                  outBuf.push_back(shiftReg);
                }
                else {
                  // store at saved position if any literal bytes were inserted
                  outBuf[savedBufPos] = shiftReg;
                  savedBufPos = 0x7FFFFFFF;
                }
                shiftReg = 0x01;
              }
            }
          }
        }
      }
    }
    catch (...) {
      for (int i = 0; i < COMPRESS_MAX_THREADS; i++) {
        if (compressorThreads[i])
          delete compressorThreads[i];
      }
      outBuf.clear();
      throw;
    }
  }

}       // namespace Plus4Emu

