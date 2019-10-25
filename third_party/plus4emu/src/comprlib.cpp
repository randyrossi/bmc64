
// compression library for plus4emu and utilities
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
#include "comprlib.hpp"

namespace Plus4Compress {

  class HuffmanNode {
   private:
    // bits 10 to 31: weight
    // bits  0 to  9: value
    uint32_t    nodeData;
    // buffer position + 1 of this node
    uint16_t    bufPos;
    // buffer position + 1 of next node in package (0 = none)
    uint16_t    nextPos;
   public:
    HuffmanNode()
      : nodeData(0U),
        bufPos(0),
        nextPos(0)
    {
    }
    ~HuffmanNode()
    {
    }
    inline void initNode(unsigned int value_, size_t weight_)
    {
      nodeData = (uint32_t(weight_) << 10) | uint32_t(value_);
      bufPos = 0;
      nextPos = 0;
    }
    inline void setBufPos(HuffmanNode *buf)
    {
      bufPos = uint16_t((this - buf) + 1);
    }
    inline size_t getBufPos() const
    {
      return size_t(bufPos - 1);
    }
    inline bool operator<(const HuffmanNode& r) const
    {
      return ((nodeData & 0xFFFFFC00U) < (r.nodeData & 0xFFFFFC00U));
    }
    inline size_t weight() const
    {
      return size_t(nodeData >> 10);
    }
    inline unsigned int value() const
    {
      return (nodeData & 0x03FFU);
    }
    inline HuffmanNode *nextNode(HuffmanNode *buf)
    {
      if (!nextPos)
        return (HuffmanNode *) 0;
      return &(buf[nextPos - 1]);
    }
    inline void merge(HuffmanNode& r, HuffmanNode *buf)
    {
      nodeData = nodeData + (r.nodeData & 0xFFFFFC00U);
      HuffmanNode *p = this;
      if (nextPos) {
        if (!r.nextPos) {
          r.nextPos = nextPos;
        }
        else {
          do {
            p = &(buf[p->nextPos - 1]);
          } while (p->nextPos);
        }
      }
      p->nextPos = r.bufPos;
    }
    static void sortNodes(HuffmanNode *startPtr, HuffmanNode *endPtr,
                          HuffmanNode *tmpBuf);
  };

  void HuffmanNode::sortNodes(HuffmanNode *startPtr, HuffmanNode *endPtr,
                              HuffmanNode *tmpBuf)
  {
    size_t  n = size_t(endPtr - startPtr);
    if (n < 2)
      return;
    size_t  m = n >> 1;
    if (m > 1)
      sortNodes(startPtr, startPtr + m, tmpBuf);
    if ((n - m) > 1)
      sortNodes(startPtr + m, endPtr, tmpBuf);
    size_t  i = 0;
    size_t  j = m;
    for (size_t k = 0; k < n; k++) {
      if (PLUS4EMU_UNLIKELY(j >= n))
        tmpBuf[k].nodeData = startPtr[i++].nodeData;
      else if (PLUS4EMU_UNLIKELY(i >= m) || startPtr[j] < startPtr[i])
        tmpBuf[k].nodeData = startPtr[j++].nodeData;
      else
        tmpBuf[k].nodeData = startPtr[i++].nodeData;
    }
    for (i = 0; i < n; i++)
      startPtr[i].nodeData = tmpBuf[i].nodeData;
  }

  // --------------------------------------------------------------------------

  HuffmanEncoder::HuffmanEncoder(size_t maxSymbolCnt_, size_t minSymbolCnt_)
    : minSymbolCnt(minSymbolCnt_),
      symbolRangeUsed(minSymbolCnt_),
      nodeBuf((void *) 0)
  {
    symbolCounts.resize(maxSymbolCnt_, 0U);
    encodeTable.resize(maxSymbolCnt_, 0U);
    nodeBuf = new HuffmanNode[maxSymbolCnt_ * 20];
  }

  HuffmanEncoder::~HuffmanEncoder()
  {
    delete[] reinterpret_cast< HuffmanNode * >(nodeBuf);
  }

  void HuffmanEncoder::updateTables(
      bool reverseBits, size_t maxCodeLen, const unsigned char *codeLengthTable)
  {
    symbolRangeUsed = 0;
    if (codeLengthTable) {
      // use a preset encoding table
      for (size_t i = 0; i < encodeTable.size(); i++) {
        symbolCounts[i] = 0;
        if (codeLengthTable[i]) {
          encodeTable[i] = (unsigned int) codeLengthTable[i] << 24;
          symbolRangeUsed = i + 1;
        }
      }
    }
    else {
      for (size_t i = 0; i < encodeTable.size(); i++)
        encodeTable[i] = 0U;
      // calculate the size of the Huffman tree
      size_t  n = 0;
      for (size_t i = 0; i < symbolCounts.size(); i++) {
        if (symbolCounts[i] > 0) {
          symbolRangeUsed = i + 1;
          n++;
        }
      }
      // check for trivial cases (symbols used <= 2)
      if (n <= 2) {
        n = 0;
        for (size_t i = 0; i < symbolRangeUsed; i++) {
          if (symbolCounts[i] > 0) {
            symbolCounts[i] = 0;
            encodeTable[i] = 0x01000000U | (unsigned int) n;
            n++;
          }
        }
      }
      else {
        // build Huffman tree
        HuffmanNode *buf0 = reinterpret_cast< HuffmanNode * >(nodeBuf);
        HuffmanNode *buf1 = buf0 + n;
        HuffmanNode *buf2 = buf1 + n;
        HuffmanNode *allocPtr = buf2 + (n << 1);
        n = 0;
        for (size_t i = 0; i < symbolRangeUsed; i++) {
          if (symbolCounts[i] > 0) {
            buf0[n].initNode((unsigned int) i, symbolCounts[i]);
            symbolCounts[i] = 0;
            n++;
          }
        }
        HuffmanNode::sortNodes(buf0, buf0 + n, buf1);
        for (size_t i = 0; i < n; i++)
          buf0[i].setBufPos(buf0);
        size_t  buf1Size = 0;
        for (size_t i = 0; i < maxCodeLen; i++) {
          size_t  j, k, l;
          // merge buffers
          for (j = 0, k = 0, l = 0; j < n || k < buf1Size; l++) {
            if (PLUS4EMU_UNLIKELY(k >= buf1Size)) {
              *allocPtr = buf0[j++];
              allocPtr->setBufPos(buf0);
              buf2[l] = *allocPtr;
              allocPtr++;
            }
            else if (PLUS4EMU_UNLIKELY(j >= n) || buf1[k] < buf0[j]) {
              buf2[l] = buf1[k++];
            }
            else {
              // sort by weight first, then nodes not in a package (from buf0)
              // are sorted before packages to minimize code length variance
              *allocPtr = buf0[j++];
              allocPtr->setBufPos(buf0);
              buf2[l] = *allocPtr;
              allocPtr++;
            }
          }
          buf1Size = l >> 1;
          // package pairs of nodes
          for (j = 0; j < buf1Size; j++) {
            k = j << 1;
            l = k + 1;
            buf2[k].merge(buf2[l], buf0);
            buf0[buf2[k].getBufPos()] = buf2[k];
            buf0[buf2[l].getBufPos()] = buf2[l];
            buf1[j] = buf2[k];
          }
        }
        for (size_t i = 0; i < buf1Size; i++) {
          HuffmanNode *p = buf1 + i;
          do {
            encodeTable[p->value()] = encodeTable[p->value()] + 0x01000000U;
            p = p->nextNode(buf0);
          } while (p);
        }
      }
    }
    // convert Huffman tree to canonical codes
    unsigned int  sizeCounts[20];
    unsigned int  sizeCodes[20];
    for (size_t i = 0; i <= maxCodeLen; i++)
      sizeCounts[i] = 0U;
    for (size_t i = 0; i < symbolRangeUsed; i++) {
      size_t  symLen = encodeTable[i] >> 24;
      if (symLen > maxCodeLen) {
        throw Plus4Emu::Exception("internal error in "
                                  "HuffmanEncoder::updateTables()");
      }
      sizeCounts[symLen] = sizeCounts[symLen] + 1U;
    }
    sizeCounts[0] = 0U;
    sizeCodes[0] = 0U;
    for (size_t i = 1; i <= maxCodeLen; i++)
      sizeCodes[i] = (sizeCodes[i - 1] + sizeCounts[i - 1]) << 1;
    for (size_t i = 0; i < symbolRangeUsed; i++) {
      if (encodeTable[i] != 0U) {
        unsigned int  nBits = encodeTable[i] >> 24;
        unsigned int  huffCode = sizeCodes[nBits];
        sizeCodes[nBits]++;
        if (reverseBits) {
          // Deflate format stores Huffman codes in most significant bit first
          // order, but everything else is least significant bit first
          huffCode = ((huffCode & 0x00FFU) << 8) | ((huffCode & 0xFF00U) >> 8);
          huffCode = ((huffCode & 0x0F0FU) << 4) | ((huffCode & 0xF0F0U) >> 4);
          huffCode = ((huffCode & 0x3333U) << 2) | ((huffCode & 0xCCCCU) >> 2);
          huffCode = ((huffCode & 0x5555U) << 1) | ((huffCode & 0xAAAAU) >> 1);
          huffCode = huffCode >> (16U - nBits);
        }
        encodeTable[i] = encodeTable[i] | huffCode;
      }
    }
    if (symbolRangeUsed < minSymbolCnt)
      symbolRangeUsed = minSymbolCnt;
  }

  void HuffmanEncoder::clear()
  {
    symbolRangeUsed = minSymbolCnt;
    for (size_t i = 0; i < symbolCounts.size(); i++) {
      symbolCounts[i] = 0U;
      encodeTable[i] = 0U;
    }
  }

  // ==========================================================================

  EncodeTable::EncodeTable(size_t nSlots_, size_t nSymbols_,
                           const size_t *slotPrefixSizeTable_,
                           size_t minPrefixSize_, size_t maxPrefixSize_,
                           const size_t *prefixSlotCntTable_)
    : nSlots(nSlots_),
      nSymbols(nSymbols_),
      nSymbolsUsed(nSymbols_),
      nSymbolsEncoded(nSymbols_),
      totalSlotWeight(0),
      unusedSymbolSize(8192),
      minPrefixSize(minPrefixSize_),
      maxPrefixSize(maxPrefixSize_),
      prefixOnlySymbolCnt(0)
  {
    if (nSymbols < 1)
      throw Plus4Emu::Exception("EncodeTable::EncodeTable(): nSymbols < 1");
    if (slotPrefixSizeTable_ != (size_t *) 0) {
      minPrefixSize = 0;
      maxPrefixSize = 0;
    }
    if (maxPrefixSize <= minPrefixSize) {
      maxPrefixSize = minPrefixSize;
      prefixSlotCntTable_ = (size_t *) 0;
    }
    prefixSlotCntTable.resize((maxPrefixSize - minPrefixSize) + 1);
    symbolCntTable.resize(nSymbols + 1);
    unencodedSymbolCostTable.resize(nSymbols + 1);
    symbolSlotNumTable.resize(nSymbols);
    symbolSizeTable.resize(nSymbols);
    for (size_t i = minPrefixSize; i <= maxPrefixSize; i++) {
      if (prefixSlotCntTable_ != (size_t *) 0) {
        prefixSlotCntTable[i - minPrefixSize] =
            prefixSlotCntTable_[i - minPrefixSize];
      }
      else if (maxPrefixSize > minPrefixSize) {
        prefixSlotCntTable[i - minPrefixSize] = size_t(1) << i;
      }
      else {
        prefixSlotCntTable[i - minPrefixSize] = nSlots;
      }
    }
    if (slotPrefixSizeTable_ != (size_t *) 0) {
      if (nSlots < 1)
        throw Plus4Emu::Exception("EncodeTable::EncodeTable(): nSlots < 1");
      slotPrefixSizeTable.resize(nSlots);
      slotWeightTable.resize(nSlots);
      slotBitsTable.resize(nSlots);
      slotBaseSymbolTable.resize(nSlots);
      maxPrefixSize = 0;
      for (size_t i = 0; i < nSlots; i++) {
        slotPrefixSizeTable[i] = slotPrefixSizeTable_[i];
        if (slotPrefixSizeTable[i] > maxPrefixSize)
          maxPrefixSize = slotPrefixSizeTable[i];
      }
      for (size_t i = 0; i < nSlots; i++) {
        slotWeightTable[i] =
            size_t(1) << (maxPrefixSize - slotPrefixSizeTable[i]);
      }
      minPrefixSize = 0;
      maxPrefixSize = 0;
      totalSlotWeight = 0;
      for (size_t i = 0; i < nSlots; i++)
        totalSlotWeight = totalSlotWeight + slotWeightTable[i];
    }
    else {
      setPrefixSize(minPrefixSize);
    }
    this->clear();
  }

  EncodeTable::~EncodeTable()
  {
  }

  void EncodeTable::setPrefixSize(size_t n)
  {
    if (n < 1) {
      throw Plus4Emu::Exception("EncodeTable::setPrefixSize(): "
                                "prefix size < 1");
    }
    if (n < minPrefixSize || n > maxPrefixSize) {
      throw Plus4Emu::Exception("EncodeTable::setPrefixSize(): "
                                "prefix size is out of range");
    }
    nSlots = prefixSlotCntTable[n - minPrefixSize];
    if (nSlots < 1)
      throw Plus4Emu::Exception("EncodeTable::setPrefixSize(): nSlots < 1");
    slotPrefixSizeTable.resize(nSlots);
    slotWeightTable.resize(nSlots);
    slotBitsTable.resize(nSlots);
    slotBaseSymbolTable.resize(nSlots);
    for (size_t i = 0; i < nSlots; i++) {
      slotPrefixSizeTable[i] = n;
      slotWeightTable[i] = 1;
    }
    totalSlotWeight = nSlots;
  }

  inline size_t EncodeTable::calculateEncodedSize() const
  {
    size_t  totalSize = 0;
    size_t  p = 0;
    for (size_t i = 0; i < nSlots; i++) {
      size_t  symbolCnt = size_t(symbolCntTable[p]);
      p = p + (size_t(1) << slotBitsTable[i]);
      size_t  symbolSize = slotPrefixSizeTable[i] + slotBitsTable[i];
      if (p >= nSymbolsUsed) {
        return (totalSize + ((size_t(symbolCntTable[nSymbolsUsed]) - symbolCnt)
                             * symbolSize));
      }
      totalSize += ((size_t(symbolCntTable[p]) - symbolCnt) * symbolSize);
    }
    // add the cost of any symbols that could not be encoded
    totalSize += (size_t(unencodedSymbolCostTable[nSymbolsUsed])
                  - size_t(unencodedSymbolCostTable[p]));
    return totalSize;
  }

  inline size_t EncodeTable::calculateEncodedSize(
      size_t firstSlot, unsigned int firstSymbol, size_t baseSize) const
  {
    size_t  totalSize = baseSize;
    size_t  p = size_t(firstSymbol);
    for (size_t i = firstSlot; i < nSlots; i++) {
      size_t  symbolCnt = size_t(symbolCntTable[p]);
      p = p + (size_t(1) << slotBitsTable[i]);
      size_t  symbolSize = slotPrefixSizeTable[i] + slotBitsTable[i];
      if (p >= nSymbolsUsed) {
        return (totalSize + ((size_t(symbolCntTable[nSymbolsUsed]) - symbolCnt)
                             * symbolSize));
      }
      totalSize += ((size_t(symbolCntTable[p]) - symbolCnt) * symbolSize);
    }
    // add the cost of any symbols that could not be encoded
    totalSize += (size_t(unencodedSymbolCostTable[nSymbolsUsed])
                  - size_t(unencodedSymbolCostTable[p]));
    return totalSize;
  }

  size_t EncodeTable::optimizeSlotBitsTable_fast()
  {
    size_t  totalSymbolCnt = symbolCntTable[nSymbolsUsed];
    size_t  slotWeightSum = totalSlotWeight;
    size_t  slotBegin = 0;
    size_t  slotEnd = 0;
    for (size_t i = 0; i < nSlots; i++) {
      slotBegin = slotEnd;
      if (totalSymbolCnt < 1) {
        slotBitsTable[i] = 0;
        continue;
      }
      if ((i + 1) < nSlots) {
        size_t  bestSize = 0;
        long    bestDiff = 0x7FFFFFFFL;
        for (size_t j = 0; j <= 15; j++) {
          slotEnd = slotBegin + (size_t(1) << j);
          if (slotEnd > nSymbolsUsed)
            slotEnd = nSymbolsUsed;
          if ((slotEnd + ((nSlots - (i + 1)) << 15)) < nSymbolsUsed)
            continue;
          size_t  tmp = symbolCntTable[slotEnd] - symbolCntTable[slotBegin];
          tmp = size_t(tmp * uint64_t(0x01000000U) / totalSymbolCnt);
          size_t  tmp2 = size_t(slotWeightTable[i] * uint64_t(0x01000000U)
                                / slotWeightSum);
          long    d = long(tmp) - long(tmp2);
          if (d < 0L)
            d = (-d);
          if (d < bestDiff || (tmp == 0 && d == bestDiff)) {
            bestSize = j;
            bestDiff = d;
          }
        }
        slotBitsTable[i] = bestSize;
      }
      else {
        for (size_t j = 0; true; j++) {
          slotEnd = slotBegin + (size_t(1) << j);
          if (slotEnd > nSymbolsUsed)
            slotEnd = nSymbolsUsed;
          size_t  tmp = symbolCntTable[slotEnd] - symbolCntTable[slotBegin];
          if (tmp >= totalSymbolCnt) {
            slotBitsTable[i] = j;
            break;
          }
          if (j >= 15) {
            throw Plus4Emu::Exception("internal error in "
                                      "EncodeTable::optimizeSlotBitsTable()");
          }
        }
      }
      slotEnd = slotBegin + (size_t(1) << slotBitsTable[i]);
      if (slotEnd > nSymbolsUsed)
        slotEnd = nSymbolsUsed;
      size_t  symbolsUsed =
          symbolCntTable[slotEnd] - symbolCntTable[slotBegin];
      totalSymbolCnt = totalSymbolCnt - symbolsUsed;
      slotWeightSum = slotWeightSum - slotWeightTable[i];
    }
    size_t  bestSize = calculateEncodedSize();
    for (int l = 0; l < 4; l++) {
      int     offs = ((l & 1) == 0 ? 1 : -1);
      while (true) {
        size_t  bestSlot = 0;
        bool    doneFlag = true;
        for (size_t i = 0; i < nSlots; i++) {
          if (!((slotBitsTable[i] >= 15 && offs > 0) ||
                (slotBitsTable[i] < 1 && offs < 0))) {
            slotBitsTable[i] = size_t(int(slotBitsTable[i]) + offs);
            size_t  newSize = calculateEncodedSize();
            slotBitsTable[i] = size_t(int(slotBitsTable[i]) - offs);
            if (newSize < bestSize) {
              bestSize = newSize;
              bestSlot = i;
              doneFlag = false;
            }
          }
        }
        if (doneFlag)
          break;
        slotBitsTable[bestSlot] = size_t(int(slotBitsTable[bestSlot]) + offs);
      }
    }
    std::vector< size_t > bestEncodeTable(nSlots);
    for (size_t i = 0; i < nSlots; i++)
      bestEncodeTable[i] = slotBitsTable[i];
    bool    doneFlag = false;
    do {
      doneFlag = true;
      for (size_t i = 0; (i + 1) < nSlots; i++) {
        size_t  firstSlot = i;
        size_t  baseSize = 0;
        unsigned int  firstSymbol = 0U;
        if (firstSlot > 0) {
          for (size_t j = 0; j < firstSlot; j++) {
            size_t  symbolCnt = size_t(symbolCntTable[firstSymbol]);
            firstSymbol = firstSymbol + (1U << (unsigned int) slotBitsTable[j]);
            size_t  symbolSize = slotPrefixSizeTable[j] + slotBitsTable[j];
            if (size_t(firstSymbol) >= nSymbolsUsed)
              break;
            baseSize += ((size_t(symbolCntTable[firstSymbol]) - symbolCnt)
                         * symbolSize);
          }
          if (size_t(firstSymbol) >= nSymbolsUsed)
            continue;
        }
        for (size_t j = i + 1; j < nSlots; j++) {
          if (bestEncodeTable[i] == bestEncodeTable[j])
            continue;
          slotBitsTable[i] = bestEncodeTable[j];
          slotBitsTable[j] = bestEncodeTable[i];
          size_t  newSize =
              calculateEncodedSize(firstSlot, firstSymbol, baseSize);
          if (newSize < bestSize) {
            bestSize = newSize;
            doneFlag = false;
            bestEncodeTable[i] = slotBitsTable[i];
            bestEncodeTable[j] = slotBitsTable[j];
          }
          else {
            slotBitsTable[i] = bestEncodeTable[i];
            slotBitsTable[j] = bestEncodeTable[j];
          }
        }
      }
      for (size_t i = nSlots; i > 0; ) {
        i--;
        int     bestOffsets[3];
        bestOffsets[0] = 0;
        bestOffsets[1] = 0;
        bestOffsets[2] = 0;
        size_t  firstSlot = (i > 2 ? (i - 2) : 0);
        size_t  baseSize = 0;
        unsigned int  firstSymbol = 0U;
        if (firstSlot > 0) {
          for (size_t j = 0; j < firstSlot; j++) {
            size_t  symbolCnt = size_t(symbolCntTable[firstSymbol]);
            firstSymbol = firstSymbol + (1U << (unsigned int) slotBitsTable[j]);
            size_t  symbolSize = slotPrefixSizeTable[j] + slotBitsTable[j];
            if (size_t(firstSymbol) >= nSymbolsUsed)
              break;
            baseSize += ((size_t(symbolCntTable[firstSymbol]) - symbolCnt)
                         * symbolSize);
          }
          if (size_t(firstSymbol) >= nSymbolsUsed)
            continue;
        }
        for (int offs2 = (i >= 2 ? -1 : 1); offs2 <= 1; offs2++) {
          if (i >= 2) {
            int     tmp = int(bestEncodeTable[i - 2]) + offs2;
            if (tmp < 0 || tmp > 15)
              continue;
            slotBitsTable[i - 2] = size_t(tmp);
          }
          for (int offs1 = (i >= 1 ? -1 : 1); offs1 <= 1; offs1++) {
            if (i >= 1) {
              int     tmp = int(bestEncodeTable[i - 1]) + offs1;
              if (tmp < 0 || tmp > 15)
                continue;
              slotBitsTable[i - 1] = size_t(tmp);
            }
            for (int offs0 = -1; offs0 <= 1; offs0++) {
              int     tmp = int(bestEncodeTable[i]) + offs0;
              if (tmp < 0 || tmp > 15)
                continue;
              slotBitsTable[i] = size_t(tmp);
              size_t  newSize =
                  calculateEncodedSize(firstSlot, firstSymbol, baseSize);
              if (newSize < bestSize) {
                bestSize = newSize;
                doneFlag = false;
                bestOffsets[0] = offs0;
                bestOffsets[1] = offs1;
                bestOffsets[2] = offs2;
              }
            }
          }
        }
        int     tmp = int(bestEncodeTable[i]) + bestOffsets[0];
        slotBitsTable[i] = size_t(tmp);
        bestEncodeTable[i] = size_t(tmp);
        if (i >= 1) {
          tmp = int(bestEncodeTable[i - 1]) + bestOffsets[1];
          slotBitsTable[i - 1] = size_t(tmp);
          bestEncodeTable[i - 1] = size_t(tmp);
        }
        if (i >= 2) {
          tmp = int(bestEncodeTable[i - 2]) + bestOffsets[2];
          slotBitsTable[i - 2] = size_t(tmp);
          bestEncodeTable[i - 2] = size_t(tmp);
        }
      }
    } while (!doneFlag);
    for (size_t i = nSlots; i-- > 0; ) {
      while (true) {
        if (slotBitsTable[i] < 1)
          break;
        slotBitsTable[i] = slotBitsTable[i] - 1;
        size_t  newSize = calculateEncodedSize();
        if (newSize > bestSize) {
          slotBitsTable[i] = slotBitsTable[i] + 1;
          break;
        }
        else {
          bestSize = newSize;
        }
      }
    }
    return bestSize;
  }

  size_t EncodeTable::optimizeSlotBitsTable()
  {
    for (size_t i = 0; i < nSlots; i++)
      slotBitsTable[i] = 0;
    if (nSymbolsUsed < 1)
      return 0;
    size_t  nSlotsD2 = (nSlots + 1) >> 1;
    std::vector< uint8_t >  slotBitsBuffer(nSlotsD2 * nSymbolsUsed, 0x00);
    std::vector< uint32_t > encodedSizeBuffer(nSymbolsUsed + 1);
    std::vector< uint8_t >  minSlotSizeTable(nSymbolsUsed, 0x00);
    // minSlotNumTable[N] is the binary weight of N, this is used to skip
    // calculating the encoded cost at any symbol index that cannot be the
    // end point with a given number of slots
    std::vector< uint8_t >  minSlotNumTable(nSymbolsUsed, 0x00);
    for (size_t i = 0; i <= nSymbolsUsed; i++) {
      encodedSizeBuffer[i] = uint32_t(unencodedSymbolCostTable[nSymbolsUsed]
                                      - unencodedSymbolCostTable[i]);
    }
    for (size_t i = 0; i < nSymbolsUsed; i++) {
      size_t  j = 0;
      size_t  nxtSlotEnd = i + 2;
      while (j < 15 && nxtSlotEnd < nSymbolsUsed) {
        if (symbolCntTable[nxtSlotEnd] != symbolCntTable[i])
          break;
        nxtSlotEnd = nxtSlotEnd * 2 - i;
        j++;
      }
      minSlotSizeTable[i] = uint8_t(j);
      size_t  bitCnt = (i & 0x55555555) + ((i >> 1) & 0x55555555);
      bitCnt = (bitCnt & 0x33333333) + ((bitCnt >> 2) & 0x33333333);
      bitCnt = (bitCnt & 0x07070707) + ((bitCnt >> 4) & 0x07070707);
      bitCnt = bitCnt + (bitCnt >> 8);
      bitCnt = (bitCnt + (bitCnt >> 16)) & 0xFF;
      minSlotNumTable[i] = uint8_t(bitCnt);
    }
    for (size_t slotNum = (nSlots < nSymbolsUsed ? nSlots : nSymbolsUsed);
         slotNum-- > 0; ) {
      size_t  maxSlotSize = 0;
      while ((size_t(1) << maxSlotSize) < nSymbolsUsed && maxSlotSize < 15)
        maxSlotSize++;
      size_t  maxPos = nSymbolsUsed - ((size_t(1) << maxSlotSize) >> 1);
      while (slotNum >= maxPos) {
        maxSlotSize--;
        maxPos = nSymbolsUsed - ((nSymbolsUsed - maxPos) >> 1);
      }
      size_t  endPos = (slotNum << 15) + 1;
      endPos = (endPos < nSymbolsUsed ? endPos : nSymbolsUsed);
      maxPos = (maxPos < endPos ? maxPos : endPos);
      for (size_t i = slotNum; true; i++) {
        if (i >= maxPos) {
          if (i >= endPos)
            break;
          maxSlotSize--;
          maxPos = nSymbolsUsed - ((nSymbolsUsed - maxPos) >> 1);
        }
        if (size_t(minSlotNumTable[i]) > slotNum)
          continue;
        size_t  baseSymbolCnt = size_t(symbolCntTable[i]);
        size_t  slotEnd = i + (size_t(1) << minSlotSizeTable[i]);
        size_t  maxSymbolSize = slotPrefixSizeTable[slotNum] + maxSlotSize;
        size_t  bestSize = 0x7FFFFFFF;
        size_t  bestSlotSize = 0;
        size_t  nBits;
        switch (maxSlotSize - size_t(minSlotSizeTable[i])) {
        case 15:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 15))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 15 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 14:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 14))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 14 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 13:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 13))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 13 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 12:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 12))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 12 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 11:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 11))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 11 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 10:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 10))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 10 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 9:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 9))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 9 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 8:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 8))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 8 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 7:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 7))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 7 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 6:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 6))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 6 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 5:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 5))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 5 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 4:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 4))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 4 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 3:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 3))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 3 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 2:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 2))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 2 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 1:
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * (maxSymbolSize - 1))
                  + size_t(encodedSizeBuffer[slotEnd]);
          slotEnd = slotEnd * 2 - i;
          bestSlotSize = (nBits < bestSize ? 1 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        case 0:
          slotEnd = (slotEnd < nSymbolsUsed ? slotEnd : nSymbolsUsed);
          nBits = ((size_t(symbolCntTable[slotEnd]) - baseSymbolCnt)
                   * maxSymbolSize)
                  + size_t(encodedSizeBuffer[slotEnd]);
          bestSlotSize = (nBits < bestSize ? 0 : bestSlotSize);
          bestSize = (nBits < bestSize ? nBits : bestSize);
        }
        slotBitsBuffer[(slotNum >> 1) * nSymbolsUsed + i] |=
            uint8_t((maxSlotSize - bestSlotSize) << ((slotNum & 1) << 2));
        encodedSizeBuffer[i] = uint32_t(bestSize);
      }
    }
    size_t  slotBegin = 0;
    for (size_t i = 0; i < nSlots; i++) {
      slotBitsTable[i] =
          size_t((slotBitsBuffer[(i >> 1) * nSymbolsUsed + slotBegin]
                 >> ((i & 1) << 2)) & 0x0F);
      slotBegin += (size_t(1) << slotBitsTable[i]);
      if (slotBegin >= nSymbolsUsed)
        break;
    }
    for (size_t i = nSlots; i-- > 0; ) {
      while (true) {
        if (slotBitsTable[i] < 1)
          break;
        slotBitsTable[i] = slotBitsTable[i] - 1;
        if (calculateEncodedSize() != size_t(encodedSizeBuffer[0])) {
          slotBitsTable[i] = slotBitsTable[i] + 1;
          break;
        }
      }
    }
    return size_t(encodedSizeBuffer[0]);
  }

  void EncodeTable::updateTables(bool fastMode)
  {
    try {
      size_t  totalSymbolCnt = 0;
      size_t  totalUnencodedSymbolCost = 0;
      for (size_t i = 0; i < nSymbolsUsed; i++) {
        size_t  tmp = size_t(symbolCntTable[i]);
        symbolCntTable[i] = (unsigned int) totalSymbolCnt;
        totalSymbolCnt += tmp;
        tmp = size_t(unencodedSymbolCostTable[i]);
        unencodedSymbolCostTable[i] = (unsigned int) totalUnencodedSymbolCost;
        totalUnencodedSymbolCost += tmp;
      }
      symbolCntTable[nSymbolsUsed] = totalSymbolCnt;
      unencodedSymbolCostTable[nSymbolsUsed] =
          (unsigned int) totalUnencodedSymbolCost;
      size_t  bestSize = 0x7FFFFFFF;
      size_t  bestPrefixSize = minPrefixSize;
      std::vector< size_t > bestSlotBitsTable;
      for (size_t prefixSize = minPrefixSize;
           prefixSize <= maxPrefixSize;
           prefixSize++) {
        if (maxPrefixSize > minPrefixSize)
          setPrefixSize(prefixSize);
        if (nSymbolsUsed > (size_t(0x8000) << prefixSize) && prefixSize > 0) {
          if (prefixSize >= maxPrefixSize) {
            throw Plus4Emu::Exception("internal error in "
                                      "EncodeTable::updateTables()");
          }
          continue;
        }
        size_t  encodedSize = 0;
        if (fastMode)
          encodedSize = optimizeSlotBitsTable_fast();
        else
          encodedSize = optimizeSlotBitsTable();
        if (maxPrefixSize > minPrefixSize) {
          encodedSize += (nSlots * 4);
          encodedSize += (prefixOnlySymbolCnt * prefixSize);
        }
        if (encodedSize < bestSize) {
          bestSize = encodedSize;
          bestPrefixSize = prefixSize;
          bestSlotBitsTable.resize(nSlots);
          for (size_t i = 0; i < nSlots; i++)
            bestSlotBitsTable[i] = slotBitsTable[i];
        }
      }
      if (maxPrefixSize > minPrefixSize)
        setPrefixSize(bestPrefixSize);
      for (size_t i = 0; i < nSlots; i++)
        slotBitsTable[i] = bestSlotBitsTable[i];
      unsigned int  baseSymbol = 0U;
      for (size_t i = 0; i < nSlots; i++) {
        slotBaseSymbolTable[i] = baseSymbol;
        unsigned int  prvBaseSymbol = baseSymbol;
        baseSymbol = prvBaseSymbol + (1U << (unsigned int) slotBitsTable[i]);
        if (baseSymbol > nSymbols)
          baseSymbol = nSymbols;
        size_t  symbolSize = slotPrefixSizeTable[i] + slotBitsTable[i];
        for (unsigned int j = prvBaseSymbol; j < baseSymbol; j++) {
          symbolSlotNumTable[j] = (unsigned char) i;
          symbolSizeTable[j] = (unsigned char) symbolSize;
        }
      }
      for (size_t i = 0; i <= nSymbolsUsed; i++) {
        symbolCntTable[i] = 0U;
        unencodedSymbolCostTable[i] = 0U;
      }
      nSymbolsUsed = 0;
      nSymbolsEncoded = baseSymbol;
      prefixOnlySymbolCnt = 0;
    }
    catch (...) {
      this->clear();
      throw;
    }
  }

  void EncodeTable::clear()
  {
    for (size_t i = 0; i < nSlots; i++) {
      slotBitsTable[i] = 0;
      slotBaseSymbolTable[i] = 0U;
    }
    for (size_t i = 0; true; i++) {
      symbolCntTable[i] = 0U;
      unencodedSymbolCostTable[i] = 0U;
      if (i >= nSymbolsUsed && i >= nSymbolsEncoded)
        break;
      symbolSlotNumTable[i] = 0;
      symbolSizeTable[i] = 1;
    }
    nSymbolsUsed = 0;
    nSymbolsEncoded = 0;
    prefixOnlySymbolCnt = 0;
  }

  // ==========================================================================

  PLUS4EMU_INLINE unsigned int RadixTree::findNextNode(unsigned char c)
  {
    unsigned int  nextPos = buf[bufPos + 2U];
    nextPos = buf[nextPos + ((c >> 6) & 3)];
    nextPos = buf[nextPos + ((c >> 4) & 3)];
    nextPos = buf[nextPos + ((c >> 2) & 3)];
    nextPos = buf[nextPos + (c & 3)];
    return nextPos;
  }

  PLUS4EMU_INLINE unsigned int RadixTree::compareStrings(
      const unsigned char *p1, size_t l1, const unsigned char *p2, size_t l2)
  {
    size_t  l = (l1 < l2 ? l1 : l2);
    if (l < 1 || p1[0] != p2[0])
      return 0U;
    if (l < 2 || p1[1] != p2[1])
      return 1U;
    if (l >= 8) {
      if (std::memcmp(p1 + 2, p2 + 2, l - 2) == 0)
        return l;
    }
    size_t  i = 2;
    for ( ; (i + 4) <= l; i = i + 4) {
      if (((p1[i] ^ p2[i]) | (p1[i + 1] ^ p2[i + 1])
           | (p1[i + 2] ^ p2[i + 2]) | (p1[i + 3] ^ p2[i + 3])) != 0) {
        break;
      }
    }
    for ( ; i < l; i++) {
      if (p1[i] != p2[i])
        return (unsigned int) i;
    }
    return (unsigned int) l;
  }

  RadixTree::RadixTree(size_t bufSize_)
    : bufPos(4U)
  {
    buf.reserve(bufSize_);
    buf.resize(7, 0U);
  }

  RadixTree::~RadixTree()
  {
  }

  size_t RadixTree::findMatches(unsigned int *offsTable,
                                const unsigned char *inBuf, size_t inBufPos,
                                size_t maxLen, size_t maxDistance)
  {
    bufPos = 4U;
    if (buf[bufPos] == 0U) {
      bufPos = findNextNode(inBuf[inBufPos]);
      if (!bufPos)
        return 0;
    }
    else if (inBuf[inBufPos] != inBuf[buf[bufPos + 1U]]) {
      return 0;
    }
    size_t  len = 0;
    do {
      unsigned int  matchPos = buf[bufPos + 1U];
      unsigned int  d = (unsigned int) inBufPos - matchPos;
      if (d > maxDistance)
        return len;
      do {
        unsigned int  l =
            RadixTree::compareStrings(
                inBuf + (inBufPos + len), maxLen - len,
                inBuf + size_t(buf[bufPos + 1U]) + len, size_t(buf[bufPos]));
        len = len + size_t(l);
        if (l < buf[bufPos] || len >= maxLen) {
          bufPos = 0U;
          if (!l)
            return len;
          break;
        }
        bufPos = findNextNode(inBuf[inBufPos + len]);
      } while (bufPos && buf[bufPos + 1U] == matchPos);
      offsTable[len] = d - 1U;
    } while (bufPos);
    return len;
  }

  void RadixTree::allocNode(unsigned char c)
  {
    unsigned int  nextPos = buf[bufPos + 2U];
    unsigned char nBits = 6;
    unsigned int  bufSize = (unsigned int) buf.size();
    if (nextPos) {
      unsigned int  nextPos_;
      while ((nextPos_ = buf[nextPos + ((c >> nBits) & 3)]) != 0U) {
        nextPos = nextPos_;
        nBits = nBits - 2;
      }
    }
    else {
      nextPos = bufSize;
      bufSize = bufSize + 4U;
      buf[bufPos + 2U] = nextPos;
    }
    unsigned int  nextPos_ = bufSize;
    bufSize = bufSize + (3U + (nBits << 1));
    if (PLUS4EMU_UNLIKELY(size_t(bufSize) > buf.capacity()))
      buf.reserve(((bufSize + (bufSize >> 2)) | 0x3FFFU) + 1U);
    buf.resize(bufSize, 0U);
    do {
      buf[nextPos + ((c >> nBits) & 3)] = nextPos_;
      nextPos = nextPos_;
      nextPos_ = nextPos_ + 4U;
      nBits = nBits - 2;
    } while ((signed char) nBits >= 0);
    bufPos = nextPos;
  }

  void RadixTree::addString(const unsigned char *inBuf, size_t inBufPos,
                            size_t len)
  {
    bufPos = 4U;
    if (buf[bufPos] == 0U) {
      unsigned char c = inBuf[inBufPos];
      unsigned int  nextPos = findNextNode(c);
      if (!nextPos) {
        // empty tree or new leaf node
        if (buf[bufPos + 2U])
          allocNode(c);
        buf[bufPos] = (unsigned int) len;
        buf[bufPos + 1U] = (unsigned int) inBufPos;
        buf[bufPos + 2U] = 0U;
        return;
      }
      bufPos = nextPos;
    }
    for (size_t n = 0; n < len; ) {
      if (buf[bufPos] == 1U &&
          inBuf[inBufPos + n] == inBuf[size_t(buf[bufPos + 1U]) + n]) {
        do {
          buf[bufPos + 1U] = (unsigned int) inBufPos;
          if (++n >= len)
            return;
          unsigned int  nextPos = findNextNode(inBuf[inBufPos + n]);
          if (!nextPos) {
            allocNode(inBuf[inBufPos + n]);
            buf[bufPos] = (unsigned int) (len - n);
            buf[bufPos + 1U] = (unsigned int) inBufPos;
            return;
          }
          bufPos = nextPos;
        } while (buf[bufPos] == 1U);
      }
      unsigned int  l =
          RadixTree::compareStrings(inBuf + (inBufPos + n), len - n,
                                    inBuf + (size_t(buf[bufPos + 1U]) + n),
                                    size_t(buf[bufPos]));
      n = n + size_t(l);
      if (l >= buf[bufPos]) {
        // full match, update position
        buf[bufPos + 1U] = (unsigned int) inBufPos;
        if (n >= len)
          break;
        unsigned char c = inBuf[inBufPos + n];
        unsigned int  nextPos = findNextNode(c);
        if (nextPos) {
          bufPos = nextPos;
          continue;
        }
        allocNode(c);
        if (!buf[bufPos]) {
          // new leaf node
          buf[bufPos] = (unsigned int) (len - n);
          buf[bufPos + 1U] = (unsigned int) inBufPos;
          break;
        }
      }
      else {
        // partial match, need to split the original sub-string of the node
        unsigned int  savedBufPos = bufPos;
        unsigned int  savedChildrenPos = buf[bufPos + 2U];
        buf[bufPos + 2U] = 0U;
        allocNode(inBuf[buf[bufPos + 1U] + (unsigned int) n]);
        buf[bufPos] = buf[savedBufPos] - l;
        buf[bufPos + 1U] = buf[savedBufPos + 1U];
        buf[bufPos + 2U] = savedChildrenPos;
        buf[savedBufPos] = l;
        buf[savedBufPos + 1U] = (unsigned int) inBufPos;
        if (n < len) {
          // create new leaf node
          bufPos = savedBufPos;
          allocNode(inBuf[inBufPos + n]);
          buf[bufPos] = (unsigned int) (len - n);
          buf[bufPos + 1U] = (unsigned int) inBufPos;
        }
        break;
      }
    }
  }

  void RadixTree::clear()
  {
    buf.clear();
    buf.resize(7, 0U);
    bufPos = 4U;
  }

  // --------------------------------------------------------------------------

  void LZSearchTable::sortFunc(unsigned int *startPtr, unsigned int *endPtr,
                               const unsigned char *buf, size_t bufSize,
                               unsigned int *tmpBuf, size_t maxLen,
                               const unsigned short *rleLenTable)
  {
    // create suffix array using merge sort algorithm
    unsigned int  *midPtr = startPtr + (size_t(endPtr - startPtr) >> 1);
    if (size_t(midPtr - startPtr) > 1)
      sortFunc(startPtr, midPtr, buf, bufSize, tmpBuf, maxLen, rleLenTable);
    if (size_t(endPtr - midPtr) > 1)
      sortFunc(midPtr, endPtr, buf, bufSize, tmpBuf, maxLen, rleLenTable);
    const unsigned int  *p1 = startPtr;
    const unsigned int  *p2 = midPtr;
    for (size_t k = 0; k < size_t(endPtr - startPtr); k++) {
      if (PLUS4EMU_UNLIKELY(p1 >= midPtr)) {
        tmpBuf[k] = *(p2++);
      }
      else if (PLUS4EMU_UNLIKELY(p2 >= endPtr)) {
        tmpBuf[k] = *(p1++);
      }
      else {
        size_t  pos1 = *p1;
        size_t  pos2 = *p2;
        size_t  l = bufSize - (pos1 > pos2 ? pos1 : pos2);
        l = (l < maxLen ? l : maxLen);
        int     c = 0;
        if (buf[pos1] == buf[pos2]) {
          size_t  rl1 = rleLenTable[pos1];
          size_t  rl2 = rleLenTable[pos2];
          size_t  rleLen = (rl1 < rl2 ? rl1 : rl2);
          if (l > rleLen) {
            c = std::memcmp(buf + (pos1 + rleLen), buf + (pos2 + rleLen),
                            l - rleLen);
          }
        }
        else {
          c = int(buf[pos1]) - int(buf[pos2]);
        }
        if (c < 0 || (c == 0 && pos1 < pos2))
          tmpBuf[k] = *(p1++);
        else
          tmpBuf[k] = *(p2++);
      }
    }
    std::memcpy(startPtr, tmpBuf,
                size_t(endPtr - startPtr) * sizeof(unsigned int));
  }

  void LZSearchTable::addMatches(size_t bufPos,
                                 unsigned int *offsTable, size_t maxLen)
  {
    matchTable[bufPos] = 0U;
    unsigned int  maxOffs = maxOffs_;
    unsigned int  prvDist = maxOffs;
    size_t  minLen = minLength_;
    bool    firstMatch = true;
    for (size_t k = maxLen; k >= minLen; k--) {
      unsigned int  d = offsTable[k];
      offsTable[k] = maxOffs;
      if (d < prvDist) {
        prvDist = d;
        if (k < 3) {
          if (d >= (k == 1 ? maxOffs1_ : maxOffs2_))
            continue;
        }
        if (PLUS4EMU_UNLIKELY((matchTableBuf.size() + 3)
                              > matchTableBuf.capacity())) {
          matchTableBuf.reserve(((matchTableBuf.size()
                                  + (matchTableBuf.size() >> 1)) | 0x03FF) + 1);
        }
        unsigned int  l = (unsigned int) k;
        if (PLUS4EMU_UNLIKELY(firstMatch)) {
          firstMatch = false;
          matchTable[bufPos] = matchTableBuf.size();
          if (lengthMaxValue_ > 1023) {
            matchTableBuf.push_back(l);
            l = 0U;
          }
        }
        matchTableBuf.push_back(l | ((d + 1U) << 10));
        if (!d)
          break;
      }
    }
    if (!firstMatch)
      matchTableBuf.push_back(0U);
  }

  // --------------------------------------------------------------------------

  LZSearchTable::LZSearchTable(size_t minLength, size_t maxLength,
                               size_t lengthMaxValue, size_t maxOffs1,
                               size_t maxOffs2, size_t maxOffs)
    : rt(maxOffs << 4),
      minLength_(uint32_t(minLength)),
      maxLength_(uint32_t(maxLength)),
      lengthMaxValue_(uint32_t(lengthMaxValue)),
      maxOffs1_(uint32_t(maxOffs1)),
      maxOffs2_(uint32_t(maxOffs2)),
      maxOffs_(uint32_t(maxOffs))
  {
    if (minLength < 1 || minLength > maxLength || maxLength > 1023 ||
        lengthMaxValue < maxLength || maxOffs < 1U || maxOffs > 0x003FFFFFU ||
        (minLength < 3 && maxOffs2 < 1U) ||
        (minLength < 2 && (maxOffs1 < 1U || maxOffs1 > maxOffs2))) {
      throw Plus4Emu::Exception("LZSearchTable::LZSearchTable(): "
                                "invalid length or offset range");
    }
    maxOffs1_ =
        (maxOffs1_ > 1U ? (maxOffs1_ < maxOffs_ ? maxOffs1_ : maxOffs_) : 1U);
    maxOffs2_ =
        (maxOffs2_ > 1U ? (maxOffs2_ < maxOffs_ ? maxOffs2_ : maxOffs_) : 1U);
  }

  void LZSearchTable::findMatches(const unsigned char *buf,
                                  size_t offs_, size_t nBytes_)
  {
    if (!buf || nBytes_ < 1 ||
        ((offs_ | nBytes_ | (offs_ + nBytes_)) & ~(size_t(0x7FFFFFFF))) != 0) {
      throw Plus4Emu::Exception("LZSearchTable::findMatches(): "
                                "invalid input buffer size");
    }
    if (matchTable.size() > 0) {
      matchTable.clear();
      matchTableBuf.clear();
    }
    matchTable.resize(nBytes_, 0xFFFFFFFFU);
    if (matchTableBuf.capacity() < 1024)
      matchTableBuf.reserve(1024);
    matchTableBuf.push_back(0U);
    size_t  maxLength = maxLength_;
    unsigned int  maxOffs = maxOffs_;
    size_t  bufSize = offs_ + nBytes_;
    // matches up to this length are found using the radix tree,
    // the suffix array based search is used only for the longer ones
    size_t  rtMaxLen = (maxLength < 15 ? maxLength : 15);
    std::vector< unsigned short > rleLengthTable(nBytes_ + 1, 0);
    std::vector< unsigned int >   suffixArray;
    std::vector< unsigned int >   invSuffixArray;
    std::vector< unsigned short > prvMatchLenTable;
    std::vector< unsigned int >   offsTable(maxLength + 1, maxOffs);
    // find RLE (offset = 1) matches
    for (size_t i = nBytes_; i-- > 1; ) {
      if (buf[offs_ + i] == buf[offs_ + i - 1]) {
        unsigned short  rleLength = rleLengthTable[i + 1];
        rleLength += (unsigned short) (rleLength < maxLength);
        rleLengthTable[i] = rleLength;
      }
    }
    for (size_t startPos = offs_; startPos < bufSize; ) {
      size_t  startPos_ =
          (startPos > size_t(maxOffs) ? (startPos - size_t(maxOffs)) : 0);
      size_t  endPos = startPos + size_t(maxOffs);
      if (endPos > bufSize || nBytes_ <= size_t(maxOffs * 2U))
        endPos = bufSize;
      size_t  nBytes = endPos - startPos_;
      // sort buffer positions alphabetically by bytes at each position
      suffixArray.resize(nBytes);
      invSuffixArray.resize(nBytes);
      prvMatchLenTable.resize(nBytes + 1);
      // create temporary RLE length table to optimize sorting the suffix array
      prvMatchLenTable[nBytes - 1] = 1;
      for (size_t i = nBytes - 1; i-- > 0; ) {
        prvMatchLenTable[i] = 1;
        if (buf[startPos_ + i] == buf[startPos_ + i + 1]) {
          unsigned short  l = prvMatchLenTable[i + 1];
          prvMatchLenTable[i] = l + (unsigned short) (l < maxLength);
        }
      }
      for (size_t i = 0; i < nBytes; i++)
        suffixArray[i] = (unsigned int) (startPos_ + i);
      if (nBytes > 1) {
        sortFunc(&(suffixArray.front()), &(suffixArray.front()) + nBytes,
                 buf, bufSize, &(invSuffixArray.front()), maxLength,
                 &(prvMatchLenTable.front()) - startPos_);
      }
      // invert suffix array
      for (size_t i = 0; i < nBytes; i++)
        invSuffixArray[suffixArray[i] - startPos_] = (unsigned int) i;
      // suffixArray[n] matches prvMatchLenTable[n] characters with
      // suffixArray[n - 1], and nxtMatchLenTable[n] characters with
      // suffixArray[n + 1]
      const unsigned short  *nxtMatchLenTable = &(prvMatchLenTable.front()) + 1;
      prvMatchLenTable[0] = 0;
      for (size_t i = 1; i < nBytes; i++) {
        const unsigned char *p1 = &(buf[suffixArray[i - 1]]);
        const unsigned char *p2 = &(buf[suffixArray[i]]);
        size_t  maxLen = size_t((buf + bufSize) - (p1 > p2 ? p1 : p2));
        maxLen = (maxLen < maxLength ? maxLen : maxLength);
        size_t  minLen = rtMaxLen + 1;
        // find longest common prefix
        prvMatchLenTable[i] = 0U;
        if (maxLen >= minLen && std::memcmp(p1, p2, minLen) == 0) {
          maxLen = maxLen - minLen;
          prvMatchLenTable[i] =
              (unsigned short) (minLen + RadixTree::compareStrings(
                                             p1 + minLen, maxLen,
                                             p2 + minLen, maxLen));
        }
      }
      prvMatchLenTable[nBytes] = 0;
      // find all matches:
      for (size_t i = startPos_; i < startPos; i++) {
        size_t  maxLen = ((bufSize - i) < rtMaxLen ? (bufSize - i) : rtMaxLen);
        rt.addString(buf, i, maxLen);
      }
      for (size_t i = startPos; i < endPos; i++) {
        size_t  maxLen = bufSize - i;
        maxLen = (maxLen < rtMaxLen ? maxLen : rtMaxLen);
        {
          size_t  rleLen = rleLengthTable[i - offs_];
          size_t  rtLen = 0;
          if (rleLen < maxLen) {
            rtLen = rt.findMatches(&(offsTable.front()), buf, i,
                                   maxLen, maxOffs);
          }
          if (rleLen > rtLen) {
            rtLen = rleLen;
            offsTable[rleLen] = 0U;
          }
          rt.addString(buf, i, maxLen);
          if (rtLen < rtMaxLen) {
            // all matches have already been found at this position
            addMatches(i - offs_, &(offsTable.front()), rtLen);
            continue;
          }
          maxLen = rtLen;
        }
        unsigned int  i_ = invSuffixArray[i - startPos_];
        size_t  matchLen = prvMatchLenTable[i_];
        if (matchLen >= maxLength && (i - suffixArray[i_ - 1]) < matchLen) {
          // hack to work around highly redundant input data: since maximum
          // length matches are sorted by buffer position in the suffix array,
          // the nearest one is always at the shortest distance, and if it
          // overlaps with the current position, anything else is skipped
          if (((unsigned int) i - suffixArray[i_ - 1]) <= maxOffs) {
            maxLen = matchLen;
            offsTable[matchLen] = ((unsigned int) i - suffixArray[i_ - 1]) - 1U;
          }
        }
        size_t  minLen = maxLen + 1;
        minLen = (minLen > (rtMaxLen + 1) ? minLen : (rtMaxLen + 1));
        if (matchLen >= minLen) {
          if (matchLen > maxLen)
            maxLen = matchLen;
          unsigned int  d = maxOffs;
          size_t  ndx = i_;
          while (true) {
            ndx--;
            unsigned int  tmp = (unsigned int) i - (suffixArray[ndx] + 1U);
            d = (tmp < d ? tmp : d);
            if (size_t(prvMatchLenTable[ndx]) < matchLen) {
              if (d < offsTable[matchLen])
                offsTable[matchLen] = d;
              matchLen = prvMatchLenTable[ndx];
              if (matchLen < minLen)
                break;
            }
          }
        }
        matchLen = nxtMatchLenTable[i_];
        if (matchLen >= minLen) {
          if (matchLen > maxLen)
            maxLen = matchLen;
          unsigned int  d = offsTable[matchLen];
          size_t  ndx = i_;
          while (true) {
            ndx++;
            unsigned int  tmp = (unsigned int) i - (suffixArray[ndx] + 1U);
            d = (tmp < d ? tmp : d);
            if (size_t(nxtMatchLenTable[ndx]) < matchLen) {
              if (d < offsTable[matchLen])
                offsTable[matchLen] = d;
              matchLen = nxtMatchLenTable[ndx];
              if (matchLen < minLen)
                break;
            }
          }
        }
        // store the matches that were found
        addMatches(i - offs_, &(offsTable.front()), maxLen);
      }
      rt.clear();
      startPos = endPos;
    }
    // find very long matches
    size_t  lengthMaxValue = lengthMaxValue_;
    if (lengthMaxValue <= maxLength || nBytes_ < 2)
      return;
    unsigned int  lenMask = (lengthMaxValue < 1024 ? 0x03FFU : 0xFFFFFFFFU);
    unsigned int  distOffs = (unsigned int) (lengthMaxValue >= 1024);
    for (size_t i = nBytes_ - 1; i-- > 0; ) {
      unsigned int  *m0 = &(matchTableBuf.front()) + size_t(matchTable[i]);
      unsigned int  *m1 = &(matchTableBuf.front()) + size_t(matchTable[i + 1]);
      if ((*m0 & lenMask) >= (unsigned int) maxLength &&
          (*m1 & lenMask) >= (unsigned int) maxLength &&
          ((m0[distOffs] ^ m1[distOffs]) & 0xFFFFFC00U) == 0U) {
        *m0 = ((*m1 & lenMask) < (unsigned int) lengthMaxValue ?
               (*m1 + 1U) : *m1);
      }
    }
  }

  LZSearchTable::~LZSearchTable()
  {
  }

}       // namespace Plus4Compress

