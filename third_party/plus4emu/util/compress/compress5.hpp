
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

#ifndef P4COMPRESS_COMPRESS5_HPP
#define P4COMPRESS_COMPRESS5_HPP

#include "plus4emu.hpp"
#include "compress.hpp"
#include "comprlib.hpp"
#include "compress0.hpp"

#include <vector>

namespace Plus4Compress {

  class Compressor_ZLib : public Compressor_M0 {
   private:
    static const size_t minMatchDist = 1;
    static const size_t maxMatchDist = 65536;
    static const size_t minMatchLen = 2;
    static const size_t maxMatchLen = 258;
    // --------
    struct LZMatchParameters {
      unsigned int    d;
      unsigned short  len;
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
    struct BitCountTableEntry {
      long    totalBits;
      unsigned int  prvDistance;
    };
    // --------
    // for symbol length codes in the header (size = 19)
    HuffmanEncoder  huffmanEncoder3;
    unsigned int    adler32Sum;
    // --------
    void calculateHuffmanEncoding(std::vector< unsigned int >& ioBuf);
    virtual void huffmanEncodeBlock(std::vector< unsigned int >& ioBuf,
                                    const unsigned char *inBuf,
                                    size_t uncompressedBytes);
    void initializeLengthCodeTables();
    PLUS4EMU_INLINE void encodeSymbol(std::vector< unsigned int >& buf,
                                      unsigned int c);
    void writeMatchCode(std::vector< unsigned int >& buf, size_t d, size_t n);
    void optimizeMatches_RND(
        LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
        const size_t *lengthBitsTable_, const unsigned char *inBuf,
        size_t offs, size_t nBytes);
    void optimizeMatches(
        LZMatchParameters *matchTable, BitCountTableEntry *bitCountTable,
        const size_t *lengthBitsTable_, const unsigned char *inBuf,
        size_t offs, size_t nBytes);
    void compressData_(std::vector< unsigned int >& tmpOutBuf,
                       const std::vector< unsigned char >& inBuf,
                       size_t offs, size_t nBytes);
    virtual bool compressData(std::vector< unsigned int >& tmpOutBuf,
                              const std::vector< unsigned char >& inBuf,
                              unsigned int startAddr, bool isLastBlock,
                              size_t offs = 0, size_t nBytes = 0x7FFFFFFFUL);
    virtual void packOutputData(const std::vector< unsigned int >& tmpBuf,
                                bool isLastBlock);
   public:
    Compressor_ZLib(std::vector< unsigned char >& outBuf_);
    virtual ~Compressor_ZLib();
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_COMPRESS5_HPP

