
// compressor utility for Commodore Plus/4 programs
// Copyright (C) 2007-2017 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "compress0.hpp"
#include "compress5.hpp"

#include <list>
#include <map>

namespace Plus4Compress {

  // 0:       unused symbol
  // 1 to 15: code length in bits
  // 16:      repeat the last code length 3 to 6 times (2 extra bits)
  // 17:      repeat unused symbol 3 to 10 times (3 extra bits)
  // 18:      repeat unused symbol 11 to 138 times (7 extra bits)

  static const unsigned char deflateCodeLengthCodeTable[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
  };

  static const unsigned char deflateCodeLengthIndexTable[19] = {
    3, 17, 15, 13, 11, 9, 7, 5, 4, 6, 8, 10, 12, 14, 16, 18, 0, 1, 2
  };

  // --------------------------------------------------------------------------

  void Compressor_ZLib::calculateHuffmanEncoding(
      std::vector< unsigned int >& ioBuf)
  {
    size_t  maxLen = 15;
    for (int h = 0; h < 2; h++) {
      HuffmanEncoder& huffmanEncoder =
          (h == 0 ? huffmanEncoder1 : huffmanEncoder2);
      unsigned int  *symbolCnts = (h == 0 ? symbolCntTable1 : symbolCntTable2);
      unsigned int  *encodeTable = (h == 0 ? encodeTable1 : encodeTable2);
      size_t  nSymbols = (h == 0 ? 288 : 32);
      huffmanEncoder.clear();
      for (size_t i = 0; i < nSymbols; i++) {
        for (unsigned int j = symbolCnts[i]; j-- > 0U; )
          huffmanEncoder.addSymbol((unsigned int) i);
      }
      // create optimal encoding table
      huffmanEncoder.updateTables(true, maxLen);
      for (unsigned int c = 0U; c < (unsigned int) nSymbols; c++) {
        encodeTable[c] = 0U;
        if (huffmanEncoder.getSymbolSize(c) <= maxLen)
          encodeTable[c] = huffmanEncoder.encodeSymbol(c);
      }
      for (unsigned int l = 1U; l <= 15U; l++) {
        size_t  sizeCnt = 0;
        for (unsigned int c = 0U; c < (unsigned int) nSymbols; c++)
          sizeCnt += size_t((encodeTable[c] >> 24) == l);
        if (PLUS4EMU_UNLIKELY(sizeCnt >= 256)) {
          // compatibility hack for new FAST_HUFFMAN decompressor code
          if (PLUS4EMU_UNLIKELY(l >= 15U)) {
            maxLen--;
            h--;
            break;
          }
          huffmanCompatibilityHack(encodeTable, symbolCnts, nSymbols, true);
          sizeCnt = 255;
        }
      }
      if (PLUS4EMU_UNLIKELY(h < 0))
        continue;                       // try again with shorter maximum length
      maxLen = 15;
      // update symbol lengths
      for (size_t i = 0; i < nSymbols; i++) {
        symbolCnts[i] = 0U;
        unsigned int  c = (unsigned int) (h == 0 ? i : (i + 0x0180));
        tmpCharBitsTable[c] = 16383;
        if (encodeTable[i])
          tmpCharBitsTable[c] = size_t(encodeTable[i] >> 24);
      }
    }
    unsigned char lenBuf[320];          // 288 + 32
    size_t  litCnt = huffmanEncoder1.getSymbolRangeUsed();
    size_t  distCnt = huffmanEncoder2.getSymbolRangeUsed();
    size_t  totalCnt = litCnt + distCnt;
    for (size_t i = 0; i < totalCnt; i++) {
      if (i < litCnt)
        lenBuf[i] = (unsigned char) (encodeTable1[i] >> 24);
      else
        lenBuf[i] = (unsigned char) (encodeTable2[i - litCnt] >> 24);
    }
    // calculate symbol usage statistics for the code length encoder
    huffmanEncoder3.clear();
    unsigned int  prvCode = 0xFFFFFFFFU;
    for (size_t i = 0; i < totalCnt; i++) {
      unsigned int  codeLength = lenBuf[i];
      unsigned int  rleCode = (codeLength == 0U ? codeLength : prvCode);
      unsigned int  rleLength = 0U;
      unsigned int  maxLength = (!rleCode ? 138U : 6U);
      while ((i + rleLength) < totalCnt && lenBuf[i + rleLength] == rleCode) {
        if (++rleLength >= maxLength)
          break;
      }
      prvCode = codeLength;
      if (rleLength < 3U) {
        huffmanEncoder3.addSymbol(codeLength);
        continue;
      }
      huffmanEncoder3.addSymbol(!rleCode ? (rleLength < 11U ? 17U : 18U) : 16U);
      i = i + (rleLength - 1U);
    }
    huffmanEncoder3.updateTables(true, 7);
    // calculate the size of the code length encoding table
    size_t  codeCnt = 4;
    for (size_t i = 0; i < 19; i++) {
      if (huffmanEncoder3.getSymbolSize((unsigned int) i) <= 7) {
        if (size_t(deflateCodeLengthIndexTable[i]) >= codeCnt)
          codeCnt = size_t(deflateCodeLengthIndexTable[i]) + 1;
      }
    }
    // write header
    ioBuf.push_back(0x11000004U | (unsigned int) ((litCnt - 257) << 3)
                                | (unsigned int) ((distCnt - 1) << 8)
                                | (unsigned int) ((codeCnt - 4) << 13));
    // write code length encoding table
    for (size_t i = 0; i < codeCnt; i++) {
      size_t  len = huffmanEncoder3.getSymbolSize(
                        (unsigned int) deflateCodeLengthCodeTable[i]);
      ioBuf.push_back(0x03000000U | (unsigned int) (len <= 7 ? len : 0));
    }
    // write RLE and Huffman compressed literal and distance encoding tables
    prvCode = 0xFFFFFFFFU;
    for (size_t i = 0; i < totalCnt; i++) {
      unsigned int  codeLength = lenBuf[i];
      unsigned int  rleCode = (codeLength == 0U ? codeLength : prvCode);
      unsigned int  rleLength = 0U;
      unsigned int  maxLength = (!rleCode ? 138U : 6U);
      while ((i + rleLength) < totalCnt && lenBuf[i + rleLength] == rleCode) {
        if (++rleLength >= maxLength)
          break;
      }
      prvCode = codeLength;
      if (rleLength >= 3U) {
        rleCode = 16U + (unsigned int) (!rleCode)
                  + (unsigned int) (rleLength >= 11U);
        unsigned int  nBits = (rleCode != 18U ? (rleCode - 14U) : 7U);
        size_t  symLen = huffmanEncoder3.getSymbolSize(rleCode);
        if ((symLen + size_t(nBits))
            <= (huffmanEncoder3.getSymbolSize(codeLength) * rleLength)) {
          // use RLE only if it does not expand the data size
          rleCode = huffmanEncoder3.encodeSymbol(rleCode);
          rleCode = (rleCode + (nBits << 24))
                    | ((rleLength - (((nBits + 1U) & 8U) + 3U)) << symLen);
          ioBuf.push_back(rleCode);
          i = i + (rleLength - 1U);
          continue;
        }
      }
      ioBuf.push_back(huffmanEncoder3.encodeSymbol(codeLength));
    }
  }

  void Compressor_ZLib::huffmanEncodeBlock(std::vector< unsigned int >& ioBuf,
                                         const unsigned char *inBuf,
                                         size_t uncompressedBytes)
  {
    if (ioBuf.size() < 1 || uncompressedBytes < 1)
      return;
    size_t  uncompressedSize = (uncompressedBytes << 3) + 39;
    size_t  compressedSize = 0;
    for (size_t i = 0; i < ioBuf.size(); i++)
      compressedSize += size_t((ioBuf[i] >> 24) & 0x1FU);
    if (compressedSize >= uncompressedSize) {
      // if the size cannot be reduced, store the data without compression
      ioBuf.resize(uncompressedBytes + 5);
      ioBuf[0] = (ioBuf[0] & 1U) | 0x03000000U;
      ioBuf[1] = 0x88000000U | (unsigned int) (uncompressedBytes & 0xFF);
      ioBuf[2] = 0x88000000U | (unsigned int) ((uncompressedBytes >> 8) & 0xFF);
      ioBuf[3] = ioBuf[1] ^ 0xFFU;
      ioBuf[4] = ioBuf[2] ^ 0xFFU;
      for (size_t i = 0; i < uncompressedBytes; i++)
        ioBuf[i + 5] = 0x88000000U | (unsigned int) inBuf[i];
    }
    // update Adler-32 checksum
    unsigned int  tmp1 = adler32Sum & 0xFFFFU;
    unsigned int  tmp2 = adler32Sum >> 16;
    for (size_t i = 0; i < uncompressedBytes; i++) {
      tmp1 += (unsigned int) inBuf[i];
      tmp1 = (tmp1 < 65521U ? tmp1 : (tmp1 - 65521U));
      tmp2 += tmp1;
      tmp2 = (tmp2 < 65521U ? tmp2 : (tmp2 - 65521U));
    }
    adler32Sum = tmp1 | (tmp2 << 16);
  }

  // --------------------------------------------------------------------------

  void Compressor_ZLib::initializeLengthCodeTables()
  {
    lengthCodeTable[2] = 0x011E;
    lengthBitsTable[2] = 0;
    lengthValueTable[2] = 0U;
    for (size_t i = 3; i < maxMatchLen; i++) {
      unsigned short  lCode = 0;
      unsigned char   lBits = 0;
      unsigned int    lValue = (unsigned int) (i - 3);
      if (lValue < 8U) {
        lCode = (unsigned short) (lValue + 0x0101U);
        lValue = 0U;
      }
      else {
        lCode = 0x0105;
        do {
          lCode = lCode + 4;
          lBits++;
        } while (lValue >= (8U << lBits));
        lCode = lCode + (unsigned short) ((lValue >> lBits) & 3U);
        lValue = (lValue & ((1U << lBits) - 1U)) | ((unsigned int) lBits << 24);
      }
      lengthCodeTable[i] = lCode;
      lengthBitsTable[i] = lBits;
      lengthValueTable[i] = lValue;
    }
    lengthCodeTable[maxMatchLen] = 0x011D;
    lengthBitsTable[maxMatchLen] = 0;
    lengthValueTable[maxMatchLen] = 0U;
    for (size_t i = minMatchDist; i <= maxMatchDist; i++) {
      unsigned short  dCode = 0;
      unsigned char   dBits = 0;
      unsigned int    dValue = (unsigned int) (i - minMatchDist);
      if (dValue < 4U) {
        dCode = (unsigned short) (dValue + 0x0180U);
        dValue = 0U;
      }
      else {
        dCode = 0x0182;
        do {
          dCode = dCode + 2;
          dBits++;
        } while (dValue >= (4U << dBits));
        dCode = dCode + (unsigned short) ((dValue >> dBits) & 1U);
        dValue = (dValue & ((1U << dBits) - 1U)) | ((unsigned int) dBits << 24);
      }
      distanceCodeTable[i] = dCode;
      distanceBitsTable[i] = dBits;
      distanceValueTable[i] = dValue;
    }
  }

  PLUS4EMU_INLINE void Compressor_ZLib::encodeSymbol(
      std::vector< unsigned int >& buf, unsigned int c)
  {
    if (c < 0x0180U) {
      symbolCntTable1[c] = symbolCntTable1[c] + 1U;
      buf.push_back(encodeTable1[c]);
    }
    else {
      c = c & 0x7FU;
      symbolCntTable2[c] = symbolCntTable2[c] + 1U;
      buf.push_back(encodeTable2[c]);
    }
  }

  void Compressor_ZLib::writeMatchCode(std::vector< unsigned int >& buf,
                                       size_t d, size_t n)
  {
    if (n == 2 && d == prvDistances[0]) {
      unsigned int  c = 0x011FU;
      if (tmpCharBitsTable[c]
          < (tmpCharBitsTable[lengthCodeTable[n]] + lengthBitsTable[n]
             + tmpCharBitsTable[distanceCodeTable[d]] + distanceBitsTable[d])) {
        encodeSymbol(buf, c);
        return;
      }
    }
    prvDistances[0] = d;
    unsigned int  c = (unsigned int) lengthCodeTable[n];
    encodeSymbol(buf, c);
    if (lengthBitsTable[n] > 0)
      buf.push_back(lengthValueTable[n]);
    c = (unsigned int) distanceCodeTable[d];
    encodeSymbol(buf, c);
    if (distanceBitsTable[d] > 0)
      buf.push_back(distanceValueTable[d]);
  }

  PLUS4EMU_INLINE long Compressor_M0::rndBit()
  {
    unsigned int  b = ((lfsrState >> 30) ^ (lfsrState >> 27)) & 1U;
    lfsrState = (lfsrState << 1) | b;
    return long(b);
  }

  void Compressor_ZLib::optimizeMatches_RND(
      LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
      const size_t *lengthBitsTable_, const unsigned char *inBuf,
      size_t offs, size_t nBytes)
  {
    for (size_t i = nBytes; i-- > 0; ) {
      // check literal byte
      long    bestSize = long(tmpCharBitsTable[inBuf[offs + i]])
                         + bitCountTable[i + 1].totalBits;
      size_t  bestLen = 1;
      size_t  bestOffs = 0;
#if 0
      size_t  minLen = (config.maxOffset <= 32768 ? 3 : 2);
      minLen = (minLen > config.minLength ? minLen : config.minLength);
#else
      // TODO: add configuration support for extended Deflate format
      size_t  minLen = 3;
#endif
      size_t  maxLen = nBytes - i;
      // check LZ77 matches
      const unsigned int  *matchPtr = searchTable->getMatches(offs + i);
      while (size_t(*matchPtr & 0x03FFU) >= minLen) {
        size_t  len = *matchPtr & 0x03FFU;
        size_t  d = *(matchPtr++) >> 10;
        len = (len < maxLen ? len : maxLen);
        size_t  offsBits = tmpCharBitsTable[distanceCodeTable[d]];
        offsBits += size_t(distanceBitsTable[d]);
        for ( ; len >= minLen; len--) {
          const BitCountTableEntry& nxtMatch = bitCountTable[i + len];
          long    nBits = long(lengthBitsTable_[len] + offsBits)
                          + nxtMatch.totalBits;
          if (size_t(nxtMatch.prvDistance) == d) {
            if (tmpCharBitsTable[0x011F] < (lengthBitsTable_[2] + offsBits)) {
              nBits -= long(lengthBitsTable_[2] + offsBits
                            - tmpCharBitsTable[0x011F]);
            }
          }
          if ((nBits + rndBit()) <= bestSize) {
            bestSize = nBits;
            bestLen = len;
            bestOffs = d;
          }
        }
      }
      matchTable[i].d = (unsigned int) bestOffs;
      matchTable[i].len = (unsigned short) bestLen;
      bitCountTable[i] = bitCountTable[i + bestLen];
      bitCountTable[i].totalBits = bestSize;
      if (bestOffs > 0) {
        bitCountTable[i].prvDistance =
            (bestLen == 2 ? (unsigned int) bestOffs : 0U);
      }
    }
  }

  void Compressor_ZLib::optimizeMatches(
      LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
      const size_t *lengthBitsTable_, const unsigned char *inBuf,
      size_t offs, size_t nBytes)
  {
    for (size_t i = nBytes; i-- > 0; ) {
      // check literal byte
      long    bestSize = long(tmpCharBitsTable[inBuf[offs + i]])
                         + bitCountTable[i + 1].totalBits;
      size_t  bestLen = 1;
      size_t  bestOffs = 0;
#if 0
      size_t  minLen = (config.maxOffset <= 32768 ? 3 : 2);
      minLen = (minLen > config.minLength ? minLen : config.minLength);
#else
      // TODO: add configuration support for extended Deflate format
      size_t  minLen = 3;
#endif
      size_t  maxLen = nBytes - i;
      // check LZ77 matches
      const unsigned int  *matchPtr = searchTable->getMatches(offs + i);
      while (size_t(*matchPtr & 0x03FFU) >= minLen) {
        size_t  len = *matchPtr & 0x03FFU;
        size_t  d = *(matchPtr++) >> 10;
        len = (len < maxLen ? len : maxLen);
        size_t  offsBits = tmpCharBitsTable[distanceCodeTable[d]];
        offsBits += size_t(distanceBitsTable[d]);
        for ( ; len >= minLen; len--) {
          const BitCountTableEntry& nxtMatch = bitCountTable[i + len];
          long    nBits = long(lengthBitsTable_[len] + offsBits)
                          + nxtMatch.totalBits;
          if (size_t(nxtMatch.prvDistance) == d) {
            if (tmpCharBitsTable[0x011F] < (lengthBitsTable_[2] + offsBits)) {
              nBits -= long(lengthBitsTable_[2] + offsBits
                            - tmpCharBitsTable[0x011F]);
            }
          }
          if (nBits <= bestSize) {
            bestSize = nBits;
            bestLen = len;
            bestOffs = d;
          }
        }
      }
      matchTable[i].d = (unsigned int) bestOffs;
      matchTable[i].len = (unsigned short) bestLen;
      bitCountTable[i] = bitCountTable[i + bestLen];
      bitCountTable[i].totalBits = bestSize;
      if (bestOffs > 0) {
        bitCountTable[i].prvDistance =
            (bestLen == 2 ? (unsigned int) bestOffs : 0U);
      }
    }
  }

  void Compressor_ZLib::compressData_(std::vector< unsigned int >& tmpOutBuf,
                                      const std::vector< unsigned char >& inBuf,
                                      size_t offs, size_t nBytes)
  {
    size_t  endPos = offs + nBytes;
    // compress data by searching for repeated byte sequences,
    // and replacing them with length/distance codes
    for (size_t i = 0; i < 4; i++)
      prvDistances[i] = 0;
    std::vector< LZMatchParameters >  matchTable(nBytes);
    {
      std::vector< BitCountTableEntry > bitCountTable(nBytes + 1);
      std::vector< size_t > lengthBitsTable_(maxMatchLen + 1, 0x7FFF);
      for (size_t i = minMatchLen; i <= maxMatchLen; i++) {
        lengthBitsTable_[i] = tmpCharBitsTable[lengthCodeTable[i]]
                              + size_t(lengthBitsTable[i]);
      }
      bitCountTable[nBytes].totalBits = 0L;
      bitCountTable[nBytes].prvDistance = 0;
      if (config.splitOptimizationDepth >= 9) {
        optimizeMatches_RND(&(matchTable.front()), &(bitCountTable.front()),
                            &(lengthBitsTable_.front()), &(inBuf.front()),
                            offs, nBytes);
      }
      else {
        optimizeMatches(&(matchTable.front()), &(bitCountTable.front()),
                        &(lengthBitsTable_.front()), &(inBuf.front()),
                        offs, nBytes);
      }
    }
    for (size_t i = offs; i < endPos; ) {
      LZMatchParameters&  tmp = matchTable[i - offs];
      if (tmp.len >= minMatchLen) {
        writeMatchCode(tmpOutBuf, tmp.d, tmp.len);
        i = i + tmp.len;
      }
      else {
        encodeSymbol(tmpOutBuf, (unsigned int) inBuf[i]);
        i++;
      }
    }
    // end of block
    encodeSymbol(tmpOutBuf, 0x0100U);
  }

  bool Compressor_ZLib::compressData(std::vector< unsigned int >& tmpOutBuf,
                                   const std::vector< unsigned char >& inBuf,
                                   unsigned int startAddr, bool isLastBlock,
                                   size_t offs, size_t nBytes)
  {
    (void) startAddr;
    // the 'offs' and 'nBytes' parameters allow compressing a buffer
    // as multiple chunks for possibly improved statistical compression
    if (nBytes < 1 || offs >= inBuf.size())
      return true;
    if (nBytes > (inBuf.size() - offs))
      nBytes = inBuf.size() - offs;
    // use fixed Huffman encoding on first pass
    for (int i = 0; i < 288; i++)
      symbolCntTable1[i] = (i < 144 ? 2 : (i < 256 ? 1 : (i < 280 ? 4 : 2)));
    for (int i = 0; i < 32; i++)
      symbolCntTable2[i] = 1;
    std::vector< uint64_t >     hashTable;
    std::vector< unsigned int > bestBuf;
    std::vector< unsigned int > tmpBuf;
    size_t  bestSize = 0x7FFFFFFF;
    bool    doneFlag = false;
    for (size_t i = 0; i < config.optimizeIterations; i++) {
      if (progressDisplayEnabled) {
        if (!setProgressPercentage(int(progressCnt * uint64_t(100)
                                       / progressMax))) {
          return false;
        }
        progressCnt += nBytes;
      }
      if (doneFlag)     // if the compression cannot be optimized further,
        continue;       // quit the loop earlier
      tmpBuf.clear();
      // apply statistical compression
      calculateHuffmanEncoding(tmpBuf);
      if (i == 0) {
        tmpBuf.resize(1);
        // bit 0 of the block header is the last block flag, b1 and b2
        // set the compression type (00b = none, 01b = fixed Huffman codes,
        // 10b = dynamic Huffman codes, 11b = invalid)
        tmpBuf[0] = 0x03000002U;
      }
      compressData_(tmpBuf, inBuf, offs, nBytes);
      // calculate compressed size and hash value
      size_t    compressedSize = 0;
      uint64_t  h = 1UL;
      for (size_t j = 0; j < tmpBuf.size(); j++) {
        compressedSize += size_t((tmpBuf[j] >> 24) & 0x1FU);
        h = h ^ uint64_t(tmpBuf[j]);
        h = uint32_t(h) * uint64_t(0xC2B0C3CCUL);
        h = (h ^ (h >> 32)) & 0xFFFFFFFFUL;
      }
      h = h | (uint64_t(compressedSize) << 32);
      if (compressedSize < bestSize) {
        // found a better compression, so save it
        bestSize = compressedSize;
        bestBuf.resize(tmpBuf.size());
        std::memcpy(&(bestBuf.front()), &(tmpBuf.front()),
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
    // append compressed data to output buffer
    bestBuf[0] = bestBuf[0] | (unsigned int) isLastBlock;
    for (size_t i = 0; i < bestBuf.size(); i++)
      tmpOutBuf.push_back(bestBuf[i]);
    return true;
  }

  void Compressor_ZLib::packOutputData(
      const std::vector< unsigned int >& tmpBuf, bool isLastBlock)
  {
    if (outBuf.size() < 1) {
      // write ZLib header:
      //   CINFO = log2(dictionary size) - 8
      //   CM = 8 (Deflate method)
      //   FLEVEL = 3 (high compression level)
      //   FDICT = 0 (no preset dictionary)
      unsigned char log2DictSize = 8;
#if 0
      while ((size_t(1) << log2DictSize) < config.maxOffset)
        log2DictSize++;
#else
      // TODO: add configuration support for extended Deflate format
      while ((size_t(1) << log2DictSize) < 32768)
        log2DictSize++;
#endif
      unsigned int  tmp = ((unsigned int) (log2DictSize - 8) << 12) | 0x08C0;
      // calculate FCHECK
      while (tmp % 31U)
        tmp++;
      outBuf.push_back((unsigned char) (tmp >> 8));
      outBuf.push_back((unsigned char) (tmp & 0xFFU));
    }
    for (size_t i = 0; i < tmpBuf.size(); i++) {
      unsigned int  c = tmpBuf[i];
      if (c >= 0x80000000U) {
        // special case for literal bytes, which are stored byte-aligned
        if (outputBitCnt != 0) {
          do {
            outputShiftReg = outputShiftReg >> 1;
            outputBitCnt = (outputBitCnt + 1) & 7;
          } while (outputBitCnt);
          outBuf.push_back(outputShiftReg);
          outputShiftReg = 0x00;
        }
        outBuf.push_back((unsigned char) (c & 0xFFU));
      }
      else {
        unsigned int  nBits = c >> 24;
        while (nBits-- > 0U) {
          outputShiftReg =
              ((outputShiftReg >> 1) & 0x7F) | (unsigned char) ((c & 1U) << 7);
          if (++outputBitCnt >= 8) {
            outBuf.push_back(outputShiftReg);
            outputShiftReg = 0x00;
            outputBitCnt = 0;
          }
          c = c >> 1;
        }
      }
    }
    if (isLastBlock) {
      if (outputBitCnt != 0) {
        do {
          outputShiftReg = outputShiftReg >> 1;
          outputBitCnt = (outputBitCnt + 1) & 7;
        } while (outputBitCnt);
        outBuf.push_back(outputShiftReg);
        outputShiftReg = 0x00;
      }
      // write Adler-32 checksum after last block
      outBuf.push_back((unsigned char) ((adler32Sum >> 24) & 0xFFU));
      outBuf.push_back((unsigned char) ((adler32Sum >> 16) & 0xFFU));
      outBuf.push_back((unsigned char) ((adler32Sum >> 8) & 0xFFU));
      outBuf.push_back((unsigned char) (adler32Sum & 0xFFU));
      // set output buffer closed flag
      outputBitCnt = -1;
    }
  }

  // --------------------------------------------------------------------------

  Compressor_ZLib::Compressor_ZLib(std::vector< unsigned char >& outBuf_)
    : Compressor_M0(outBuf_, 288, 257, 32, 1),
      huffmanEncoder3(19, 4),
      adler32Sum(1U)
  {
    try {
      lengthCodeTable = new unsigned short[maxMatchLen + maxMatchDist + 2];
      lengthBitsTable = new unsigned char[maxMatchLen + maxMatchDist + 2];
      lengthValueTable = new unsigned int[maxMatchLen + maxMatchDist + 642];
      distanceCodeTable = lengthCodeTable + (maxMatchLen + 1);
      distanceBitsTable = lengthBitsTable + (maxMatchLen + 1);
      distanceValueTable = lengthValueTable + (maxMatchLen + 1);
      symbolCntTable1 = distanceValueTable + (maxMatchDist + 1);
      symbolCntTable2 = symbolCntTable1 + 288;
      encodeTable1 = symbolCntTable2 + 32;
      encodeTable2 = encodeTable1 + 288;
      tmpCharBitsTable = new size_t[384 + 32];
      initializeLengthCodeTables();
      for (size_t i = 0; i < (384 + 32); i++)
        tmpCharBitsTable[i] = 16383;
      for (size_t i = 0; i < 4; i++)
        prvDistances[i] = 0;
    }
    catch (...) {
      if (lengthCodeTable)
        delete[] lengthCodeTable;
      if (lengthBitsTable)
        delete[] lengthBitsTable;
      if (lengthValueTable)
        delete[] lengthValueTable;
      if (tmpCharBitsTable)
        delete[] tmpCharBitsTable;
      throw;
    }
  }

  Compressor_ZLib::~Compressor_ZLib()
  {
  }

}       // namespace Plus4Compress

