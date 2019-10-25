
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

#ifndef P4COMPRESS_DECOMPRESS5_HPP
#define P4COMPRESS_DECOMPRESS5_HPP

#include "plus4emu.hpp"
#include "compress.hpp"
#include <vector>

namespace Plus4Compress {

  class Decompressor_ZLib : public Decompressor {
   private:
    // main Huffman table (literal characters + length codes), size = 288
    unsigned int  *huffmanSymCntTable0;
    unsigned int  *huffmanOffsetTable0;
    unsigned int  *huffmanDecodeTable0;
    // Huffman table for distance codes, size = 32
    unsigned int  *huffmanSymCntTable1;
    unsigned int  *huffmanOffsetTable1;
    unsigned int  *huffmanDecodeTable1;
    unsigned char shiftRegister;
    int           shiftRegisterCnt;
    const unsigned char *inputBuffer;
    size_t        inputBufferSize;
    size_t        inputBufferPosition;
    // --------
    unsigned int readByte();
    PLUS4EMU_INLINE unsigned int readBit();
    unsigned int readBits(size_t nBits);
    unsigned int huffmanDecode(int huffTable);
    void buildDecodeTable(int huffTable,
                          const unsigned char *lenBuf, size_t nSymbols);
    void huffmanInit(unsigned char blockType);
    bool decompressDataBlock(std::vector< unsigned char >& buf);
   public:
    Decompressor_ZLib();
    virtual ~Decompressor_ZLib();
    virtual void decompressData(
        std::vector< std::vector< unsigned char > >& outBuf,
        const std::vector< unsigned char >& inBuf);
    // this function assumes "raw" compressed data blocks with no start address
    virtual void decompressData(
        std::vector< unsigned char >& outBuf,
        const std::vector< unsigned char >& inBuf);
    virtual void decompressData(
        std::vector< unsigned char >& outBuf,
        const std::vector< unsigned char >& inBuf, size_t *inBufPos);
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_DECOMPRESS5_HPP

