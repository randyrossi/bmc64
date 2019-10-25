
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

#ifndef P4COMPRESS_COMPRESS3_HPP
#define P4COMPRESS_COMPRESS3_HPP

#include "plus4emu.hpp"
#include "compress.hpp"
#include "comprlib.hpp"

#include <vector>

namespace Plus4Compress {

  class Compressor_M3 : public Compressor {
   private:
    static const size_t minRepeatDist = 1;
    static const size_t maxRepeatDist = 65535;
    static const size_t minRepeatLen = 2;
    static const size_t maxRepeatLen = 1023;
    static const unsigned int lengthMaxValue = 65535U;
    // --------
    struct LZMatchParameters {
      unsigned short  d;
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
    LZSearchTable   *searchTable;
    // --------
    void writeRepeatCode(std::vector< unsigned int >& buf, size_t d, size_t n);
    inline size_t getRepeatCodeLength(size_t d, size_t n) const;
    void optimizeMatches(LZMatchParameters *matchTable,
                         size_t *bitCountTable, unsigned char *bitIncMaxTable,
                         size_t nBytes);
    void compressData_(std::vector< unsigned int >& tmpOutBuf,
                       const std::vector< unsigned char >& inBuf);
   public:
    Compressor_M3(std::vector< unsigned char >& outBuf_);
    virtual ~Compressor_M3();
    // 'startAddr' must be 0xFFFFFFFF
    // 'isLastBlock' must be true
    // 'enableProgressDisplay' is ignored
    virtual bool compressData(const std::vector< unsigned char >& inBuf,
                              unsigned int startAddr, bool isLastBlock,
                              bool enableProgressDisplay = false);
  };

}       // namespace Plus4Compress

#endif  // P4COMPRESS_COMPRESS3_HPP

