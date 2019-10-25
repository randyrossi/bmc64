
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

#ifndef PLUS4EMU_DECOMPM2_HPP
#define PLUS4EMU_DECOMPM2_HPP

#include "plus4emu.hpp"
#include <vector>

namespace Plus4Emu {

  class Decompressor {
   private:
    unsigned int  lengthDecodeTable[8 * 2];
    unsigned int  offs1DecodeTable[4 * 2];
    unsigned int  offs2DecodeTable[8 * 2];
    unsigned int  offs3DecodeTable[32 * 2];
    size_t        offs3PrefixSize;
    unsigned char shiftRegister;
    const unsigned char *inputBuffer;
    size_t        inputBufferSize;
    size_t        inputBufferPosition;
    // --------
    unsigned int readBits(size_t nBits);
    unsigned char readLiteralByte();
    // returns LZ match length (1..65535),
    // or length + 0x80000000 for literal sequence
    unsigned int readMatchLength();
    unsigned int readLZMatchParameter(unsigned char slotNum,
                                      const unsigned int *decodeTable);
    void readDecodeTables();
    bool decompressDataBlock(std::vector< unsigned char >& buf);
   public:
    Decompressor();
    virtual ~Decompressor();
    virtual void decompressData(std::vector< unsigned char >& outBuf,
                                const unsigned char *inBuf, size_t inBufSize);
  };

  extern void decompressData(std::vector< unsigned char >& outBuf,
                             const unsigned char *inBuf, size_t inBufSize);

  // --------------------------------------------------------------------------

  extern void compressData(std::vector< unsigned char >& outBuf,
                           const unsigned char *inBuf, size_t inBufSize);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_DECOMPM2_HPP

