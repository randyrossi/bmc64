
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

#ifndef PLUS4EMU_PNGWRITE_HPP
#define PLUS4EMU_PNGWRITE_HPP

#include "plus4emu.hpp"
#include "comprlib.hpp"
#include <vector>

namespace Plus4Emu {

  class Compressor_ZLib {
   public:
    static const size_t minMatchDist = 1;
    static const size_t maxMatchDist = 32768;
    static const size_t minMatchLen = 3;
    static const size_t maxMatchLen = 258;
    // --------------------------------
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
    // --------------------------------
   private:
    Plus4Compress::LZSearchTable  *searchTable;
    // for literals and length codes (size = 288)
    Plus4Compress::HuffmanEncoder huffmanEncoderL;
    // for distance codes (size = 32)
    Plus4Compress::HuffmanEncoder huffmanEncoderD;
    // for symbol length codes in the header (size = 19)
    Plus4Compress::HuffmanEncoder huffmanEncoderC;
    // --------
    // initialize Huffman encoders with the fixed codes
    void setDefaultEncoding();
    void writeHuffmanEncoding(std::vector< unsigned int >& buf);
    void writeMatchCode(std::vector< unsigned int >& buf, size_t d, size_t n);
    inline size_t getLengthCodeLength(size_t n) const;
    inline size_t getDistanceCodeLength(size_t d) const;
    inline size_t getMatchCodeLength(size_t n, size_t d) const;
    void optimizeMatches(LZMatchParameters *matchTable,
                         const unsigned char *inBuf, size_t *bitCountTable,
                         size_t offs, size_t nBytes);
    void compressData(std::vector< unsigned int >& outBuf,
                      const unsigned char *inBuf, size_t offs, size_t nBytes,
                      bool firstPass);
   public:
    Compressor_ZLib();
    virtual ~Compressor_ZLib();
    void compressDataBlock(std::vector< unsigned int >& outBuf,
                           const unsigned char *inBuf, size_t offs,
                           size_t nBytes, size_t bufSize, bool isLastBlock);
    // 'blockSize' must be a power of two and not greater than 32768,
    // the optimal value depends on the input data
    static void compressData(std::vector< unsigned char >& outBuf,
                             const unsigned char *inBuf, size_t inBufSize,
                             size_t blockSize = 16384);
  };

  // ==========================================================================

  /*!
   * Save a PNG format image to 'fileName' from the data in 'inBuf'.
   * 'w' and 'h' are the image width and height in pixels, 'nColors' is the
   * palette size (0 to 256, 0 means RGB format), and 'blockSize' is the block
   * size to be used by Compressor_ZLib::compressData(). If 'optimizePalette'
   * is true, unused colors are removed from the palette.
   * The size of 'inBuf' should be nColors * 3 + w * h bytes if a palette is
   * used, and w * h * 3 in the case of RGB format. If a palette is present,
   * it is expected to be at the beginning of 'inBuf' as nColors * 3
   * interleaved R, G, B values.
   */
  void writePNGImage(const char *fileName,
                     const unsigned char *inBuf, int w, int h, int nColors,
                     bool optimizePalette = false, size_t blockSize = 16384);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_PNGWRITE_HPP

