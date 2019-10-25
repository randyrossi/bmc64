
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2017 Istvan Varga <istvanv@users.sourceforge.net>
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

#ifndef PLUS4EMU_FILEIO_HPP
#define PLUS4EMU_FILEIO_HPP

#include "plus4emu.hpp"
#include <vector>
#include <map>

namespace Plus4Emu {

  class File {
   public:
    class Buffer {
     private:
      unsigned char *buf;
      size_t  curPos, dataSize, allocSize;
     public:
      unsigned char readByte();
      bool readBoolean();
      int32_t readInt32();
      uint32_t readUInt32();
      int64_t readInt64();
      uint64_t readUInt64();
      double readFloat();
      std::string readString();
      void writeByte(unsigned char n);
      void writeBoolean(bool n);
      void writeInt32(int32_t n);
      void writeUInt32(uint32_t n);
      void writeInt64(int64_t n);
      void writeUInt64(uint64_t n);
      void writeFloat(double n);
      void writeString(const std::string& n);
      void writeData(const unsigned char *buf_, size_t nBytes);
      void setPosition(size_t pos);
      void clear();
      inline size_t getPosition() const
      {
        return curPos;
      }
      inline size_t getDataSize() const
      {
        return dataSize;
      }
      inline const unsigned char * getData() const
      {
        return buf;
      }
      Buffer();
      Buffer(const unsigned char *buf_, size_t nBytes);
      ~Buffer();
    };
    // ----------------
    typedef enum {
      PLUS4EMU_CHUNKTYPE_END_OF_FILE =  0x00000000,
      PLUS4EMU_CHUNKTYPE_Z80_STATE =    0x45508001,
      PLUS4EMU_CHUNKTYPE_MEMORY_STATE = 0x45508002,
      PLUS4EMU_CHUNKTYPE_IO_STATE =     0x45508003,
      PLUS4EMU_CHUNKTYPE_DAVE_STATE =   0x45508004,
      PLUS4EMU_CHUNKTYPE_NICK_STATE =   0x45508005,
      PLUS4EMU_CHUNKTYPE_BREAKPOINTS =  0x45508006,
      PLUS4EMU_CHUNKTYPE_CONFIG_DB =    0x45508007,
      PLUS4EMU_CHUNKTYPE_VM_CONFIG =    0x45508008,
      PLUS4EMU_CHUNKTYPE_VM_STATE =     0x45508009,
      PLUS4EMU_CHUNKTYPE_DEMO_STREAM =  0x4550800A,
      PLUS4EMU_CHUNKTYPE_M7501_STATE =  0x4550800B,
      PLUS4EMU_CHUNKTYPE_TED_STATE =    0x4550800C,
      PLUS4EMU_CHUNKTYPE_P4VM_CONFIG =  0x4550800D,
      PLUS4EMU_CHUNKTYPE_P4VM_STATE =   0x4550800E,
      PLUS4EMU_CHUNKTYPE_PLUS4_DEMO =   0x4550800F,
      PLUS4EMU_CHUNKTYPE_PLUS4_PRG =    0x45508010,
      PLUS4EMU_CHUNKTYPE_SID_STATE =    0x45508011
    } ChunkType;
    // ----------------
    class ChunkTypeHandler {
     public:
      ChunkTypeHandler()
      {
      }
      virtual ~ChunkTypeHandler();
      virtual ChunkType getChunkType() const = 0;
      virtual void processChunk(Buffer& buf) = 0;
    };
   private:
    Buffer  buf;
    std::map< int, ChunkTypeHandler * > chunkTypeDB;
    void loadCompressedFile(std::FILE *f);
   public:
    void addChunk(ChunkType type, const Buffer& buf_);
    void processAllChunks();
    void writeFile(const char *fileName, bool useHomeDirectory = false,
                   bool enableCompression = false);
    void registerChunkType(ChunkTypeHandler *);
    File();
    File(const char *fileName, bool useHomeDirectory = false);
    ~File();
    inline size_t getBufferDataSize() const
    {
      return buf.getDataSize();
    }
    inline const unsigned char *getBufferData() const
    {
      return buf.getData();
    }
    static PLUS4EMU_REGPARM2 uint32_t hash_32(const unsigned char *buf,
                                              size_t nBytes);
    static PLUS4EMU_REGPARM2 uint32_t crc_32(const unsigned char *buf,
                                             size_t nBytes);
  };

  class ZIPFile {
   protected:
    std::vector< unsigned char >  inBuf;
    size_t  inBufPos;
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
    // --------
    unsigned char readByte();
    uint16_t readUInt16();
    uint32_t readUInt32();
    PLUS4EMU_INLINE unsigned int readBit();
    unsigned int readBits(size_t nBits);
    unsigned int huffmanDecode(int huffTable);
    void buildDecodeTable(int huffTable,
                          const unsigned char *lenBuf, size_t nSymbols);
    void huffmanInit(unsigned char blockType);
    bool decompressDataBlock(std::vector< unsigned char >& buf);
   public:
    ZIPFile(const char *fileName);
    virtual ~ZIPFile();
    /*!
     * Uncompress the next file of the specified type from the archive to
     * 'buf', the file name is stored in 'fileName'.
     *
     * fileType == -1: any type (updated on return)
     * fileType == 0: program (.PRG or .P00)
     * fileType == 1: disk (.D64 or .D81)
     * fileType == 2: tape (.TAP)
     *
     * Returns false on EOF.
     */
    bool getFile(std::vector< unsigned char >& buf, std::string& fileName,
                 int& fileType);
  };

  /*!
   * Create empty D64 or D81 disk image.
   */
  std::FILE *createDiskImage(const char *fileName);

  /*!
   * Open or create a D64, D81, TAP, PRG or P00 file. If the file name ends
   * with ".zip" (case insensitive), then the first matching file from the
   * archive is extracted to a temporary file created with std::tmpfile().
   */
  std::FILE *openPlus4ImageFile(const char *fileName,
                                int& fileType, bool& isReadOnly);

}       // namespace Plus4Emu

#endif  // PLUS4EMU_FILEIO_HPP

