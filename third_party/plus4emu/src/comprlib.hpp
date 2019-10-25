
// compression library for plus4emu and utilities
// Copyright (C) 2007-2019 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef P4COMPRESS_COMPRLIB_HPP
#define P4COMPRESS_COMPRLIB_HPP

#include "plus4emu.hpp"

#include <vector>

namespace Plus4Compress {

  // length limited Huffman coding

  class HuffmanEncoder {
   private:
    size_t  minSymbolCnt;
    size_t  symbolRangeUsed;
    std::vector< unsigned int > symbolCounts;
    std::vector< unsigned int > encodeTable;
    void    *nodeBuf;
   public:
    HuffmanEncoder(size_t maxSymbolCnt_ = 256, size_t minSymbolCnt_ = 0);
    virtual ~HuffmanEncoder();
    // Create encode table and reset symbol counts.
    // If codeLengthTable is not NULL, it specifies a preset encoding.
    void updateTables(bool reverseBits = true, size_t maxCodeLen = 16,
                      const unsigned char *codeLengthTable =
                          (unsigned char *) 0);
    inline void addSymbol(unsigned int c)
    {
      symbolCounts[c]++;
    }
    inline unsigned int encodeSymbol(unsigned int c) const
    {
      if (encodeTable[c] == 0U) {
        throw Plus4Emu::Exception("internal error in "
                                  "HuffmanEncoder::encodeSymbol()");
      }
      return encodeTable[c];
    }
    inline size_t getSymbolSize(unsigned int c) const
    {
      if (encodeTable[c] == 0U)
        return 0x3FFF;
      return size_t((encodeTable[c] >> 24) & 0x7FU);
    }
    inline size_t getSymbolRangeUsed() const
    {
      return symbolRangeUsed;
    }
    // reset symbol counts and clear encode table
    void clear();
  };

  // ==========================================================================

  // This class implements statistical compression of a range of values in
  // a simple prefix + extra bits format, where the prefix selects a slot in
  // a table that defines the number of additional bits to read. The prefix
  // itself can use a pre-defined variable length encoding. An example using a
  // fixed length prefix (2 bits) and an encode table with 4 slots (2,3,4,5):
  //   00b, 2 bits to read, range = 0 to 3
  //   01b, 3 bits to read, range = 4 to 11
  //   10b, 4 bits to read, range = 12 to 27
  //   11b, 5 bits to read, range = 28 to 59

  class EncodeTable {
   private:
    size_t    nSlots;
    size_t    nSymbols;
    size_t    nSymbolsUsed;
    size_t    nSymbolsEncoded;
    size_t    totalSlotWeight;
    size_t    unusedSymbolSize;
    size_t    minPrefixSize;
    size_t    maxPrefixSize;
    size_t    prefixOnlySymbolCnt;
    std::vector< size_t >   prefixSlotCntTable;
    std::vector< size_t >   slotPrefixSizeTable;
    std::vector< size_t >   slotWeightTable;
    std::vector< size_t >   slotBitsTable;
    std::vector< unsigned int >   slotBaseSymbolTable;
    std::vector< unsigned int >   symbolCntTable;
    std::vector< unsigned int >   unencodedSymbolCostTable;
    std::vector< unsigned char >  symbolSlotNumTable;
    std::vector< unsigned char >  symbolSizeTable;
    void setPrefixSize(size_t n);
    inline size_t calculateEncodedSize() const;
    inline size_t calculateEncodedSize(size_t firstSlot,
                                       unsigned int firstSymbol,
                                       size_t baseSize) const;
    size_t optimizeSlotBitsTable_fast();
    size_t optimizeSlotBitsTable();
   public:
    // If 'slotPrefixSizeTable_' is non-NULL, a variable prefix length
    // encoding is generated with 'nSlots_' slots, and the table is expected
    // to contain the prefix size in bits for each slot.
    // Otherwise, if 'minPrefixSize_' is greater than or equal to
    // 'maxPrefixSize_', a fixed prefix size of 'minPrefixSize_' bits will be
    // used with 'nSlots_' slots, and 'nSlots_' must be less than or equal to
    // 2 ^ 'minPrefixSize_'.
    // Finally, if 'maxPrefixSize_' is greater than 'minPrefixSize_', then
    // all fixed prefix sizes in the specified range are tried, and the one
    // that results in the smallest encoded size will be used. The number of
    // slots, which must be less than or equal to 2 ^ prefix_size, can be
    // specified for each prefix size in 'prefixSlotCntTable_' (the number of
    // elements is 'maxPrefixSize_' + 1 - 'minPrefixSize_'); if the table is
    // NULL, then the number of slots defaults to the maximum possible value
    // (2 ^ prefix_size).
    // In all cases, 'nSymbols_' is the highest value to be encoded + 1, so
    // the valid range will be 0 to 'nSymbols_' - 1.
    EncodeTable(size_t nSlots_, size_t nSymbols_,
                const size_t *slotPrefixSizeTable_ = (size_t *) 0,
                size_t minPrefixSize_ = 4,
                size_t maxPrefixSize_ = 0,
                const size_t *prefixSlotCntTable_ = (size_t *) 0);
    virtual ~EncodeTable();
    inline void addSymbol(unsigned int n, size_t unencodedCost = 16384)
    {
      symbolCntTable[n] += 1U;
      unencodedSymbolCostTable[n] += (unsigned int) unencodedCost;
      if (size_t(n) >= nSymbolsUsed)
        nSymbolsUsed = size_t(n) + 1;
    }
    // this function is for special symbols that use "reserved" slots
    // in the table (e.g. for repeating the previous match distance)
    inline void addPrefixOnlySymbol()
    {
      prefixOnlySymbolCnt++;
    }
    inline void setUnencodedSymbolSize(size_t n)
    {
      unusedSymbolSize = n;
    }
    inline size_t getSymbolsEncoded() const
    {
      return nSymbolsEncoded;
    }
    inline size_t getSymbolSize(unsigned int n) const
    {
      if (size_t(n) >= nSymbolsEncoded)
        return unusedSymbolSize;
      return symbolSizeTable[n];
    }
    inline size_t getSymbolSizeFast(unsigned int n) const
    {
      return symbolSizeTable[n];
    }
    inline size_t getSymbolSlotIndex(unsigned int n) const
    {
      if (size_t(n) >= nSymbolsEncoded)
        throw Plus4Emu::Exception("internal error: encoding invalid symbol");
      return symbolSlotNumTable[n];
    }
    inline unsigned int encodeSymbol(unsigned int n) const
    {
      size_t  slotNum = getSymbolSlotIndex(n);
      return ((unsigned int) (slotBitsTable[slotNum] << 24)
              | (n - slotBaseSymbolTable[slotNum]));
    }
    inline size_t getSlotCnt() const
    {
      return slotBitsTable.size();
    }
    inline size_t getSlotPrefixSize(size_t n) const
    {
      return slotPrefixSizeTable[n];
    }
    inline size_t getSlotSize(size_t n) const
    {
      return slotBitsTable[n];
    }
    void updateTables(bool fastMode = false);
    void clear();
  };

  // ==========================================================================

  class RadixTree {
   protected:
    friend class LZSearchTable;
    // Each node has a size of 3 elements in the buffer:
    //        0:  length of the sub-string at this node
    //        1:  start position of the full sequence in the input data
    //        2:  buffer position of table of child nodes (0: none) for the
    //            2 most significant bits of the next character.
    //            A chain of 4 such tables leads to the next node, each one
    //            contains 4 buffer positions for the next 2 bits of the
    //            character
    // The first four elements of the buffer are unused (always zero)
    std::vector< unsigned int > buf;
    unsigned int  bufPos;
    // --------
    PLUS4EMU_INLINE unsigned int findNextNode(unsigned char c);
    void allocNode(unsigned char c);
    // returns the matching prefix length between 'p1' and 'p2'
    static PLUS4EMU_INLINE unsigned int compareStrings(
        const unsigned char *p1, size_t l1, const unsigned char *p2, size_t l2);
   public:
    RadixTree(size_t bufSize_ = 0x00100000);
    ~RadixTree();
    // writes the shortest offset - 1 of matches found to offsTable[1..maxLen]
    // (each offset is stored only at its maximum length), and returns the
    // maximum length; for each inBufPos, findMatches() should be called first,
    // then addString()
    size_t findMatches(unsigned int *offsTable,
                       const unsigned char *inBuf, size_t inBufPos,
                       size_t maxLen, size_t maxDistance = 0xFFFFFFFFU);
    void addString(const unsigned char *inBuf, size_t inBufPos, size_t len);
    void clear();
  };

  // --------------------------------------------------------------------------

  class LZSearchTable {
   protected:
    // for each buffer position P, matchTableBuf[matchTable[P]] is the
    // first element of an array of match length/offset pairs packed in
    // unsigned integers as length | (offset << 10), terminated with zero;
    // if matches with length >= 1024 are supported, then the first one is
    // stored as two separate values (length followed by offset << 10)
    std::vector< unsigned int > matchTable;
    // space allocated for matchTable
    std::vector< unsigned int > matchTableBuf;
    RadixTree   rt;
    uint32_t    minLength_;
    uint32_t    maxLength_;
    uint32_t    lengthMaxValue_;
    uint32_t    maxOffs1_;
    uint32_t    maxOffs2_;
    uint32_t    maxOffs_;
    // --------
    static void sortFunc(unsigned int *startPtr, unsigned int *endPtr,
                         const unsigned char *buf, size_t bufSize,
                         unsigned int *tmpBuf, size_t maxLen,
                         const unsigned short *rleLenTable);
    void addMatches(size_t bufPos, unsigned int *offsTable, size_t maxLen);
   public:
    // minLength:   minimum match length
    // maxLength:   maximum match length for optimal search (must be <= 1023)
    // lengthMaxValue:  maximum match length supported by the encoder
    // maxOffs1:    maximum offset for matches with length == 1
    // maxOffs2:    maximum offset for matches with length == 2
    // maxOffs:     maximum offset for all matches (must be <= 0x003FFFFF)
    LZSearchTable(size_t minLength, size_t maxLength, size_t lengthMaxValue,
                  size_t maxOffs1, size_t maxOffs2, size_t maxOffs);
    virtual ~LZSearchTable();
    // buf:     input data to be searched
    // offs_:   start position in 'buf', this will be at bufPos == 0 in
    //          getMatches(), but up to 'maxOffs' bytes before 'offs_' are
    //          still searched
    // nBytes_: data size after 'offs_'
    void findMatches(const unsigned char *buf, size_t offs_, size_t nBytes_);
    // Returns pointer to an array of matches found at 'bufPos', sorted
    // in descending order by length and distance. Each match is stored as
    // an unsigned integer with the length in the lower 10 bits and the
    // distance shifted to the left by 10 bits. If 'lengthMaxValue' is
    // greater than 1023, then the first match is stored as separate length
    // and offset << 10. The list is terminated with a zero value
    inline const unsigned int * getMatches(size_t bufPos) const
    {
      return (&(matchTableBuf.front()) + matchTable[bufPos]);
    }
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_COMPRLIB_HPP

