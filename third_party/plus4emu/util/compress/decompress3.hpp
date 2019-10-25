
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

#ifndef P4COMPRESS_DECOMPRESS3_HPP
#define P4COMPRESS_DECOMPRESS3_HPP

#include "plus4emu.hpp"
#include "compress.hpp"
#include <vector>

namespace Plus4Compress {

  class Decompressor_M3 : public Decompressor {
   private:
    const unsigned char *inBufPtr;
    size_t        inBufPos;
    unsigned char inputShiftReg;
    // --------
    unsigned char readByte();
    unsigned int readBits(size_t n);
    unsigned int readLengthValue();
   public:
    Decompressor_M3();
    virtual ~Decompressor_M3();
    // both functions assume "raw" compressed data block with no start address
    virtual void decompressData(
        std::vector< std::vector< unsigned char > >& outBuf,
        const std::vector< unsigned char >& inBuf);
    virtual void decompressData(
        std::vector< unsigned char >& outBuf,
        const std::vector< unsigned char >& inBuf);
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_DECOMPRESS3_HPP

