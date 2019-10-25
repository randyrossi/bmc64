
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

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "system.hpp"
#include "decompm2.hpp"

#include <cmath>
#include <map>

static const unsigned char plus4EmuFile_Magic[16] = {
  0x5D, 0x12, 0xE4, 0xF4, 0xC9, 0xDA, 0xB6, 0x42,
  0x01, 0x33, 0xDE, 0x07, 0xD2, 0x34, 0xF2, 0x22
};

static void getFullPathFileName(const char *fileName, std::string& fullName)
{
  fullName = Plus4Emu::getPlus4EmuHomeDirectory();
#ifndef WIN32
  fullName += '/';
#else
  fullName += '\\';
#endif
  fullName += fileName;
}

// ----------------------------------------------------------------------------

namespace Plus4Emu {

  PLUS4EMU_REGPARM2 uint32_t File::hash_32(const unsigned char *buf,
                                           size_t nBytes)
  {
    size_t        n = nBytes >> 2;
    unsigned int  h = 1U;

    for (size_t i = 0; i < n; i++) {
      h ^=  ((unsigned int) buf[0] & 0xFFU);
      h ^= (((unsigned int) buf[1] & 0xFFU) << 8);
      h ^= (((unsigned int) buf[2] & 0xFFU) << 16);
      h ^= (((unsigned int) buf[3] & 0xFFU) << 24);
      buf += 4;
      uint64_t  tmp = (uint32_t) h * (uint64_t) 0xC2B0C3CCU;
      h = ((unsigned int) tmp ^ (unsigned int) (tmp >> 32)) & 0xFFFFFFFFU;
    }
    switch (uint8_t(nBytes) & 3) {
    case 3:
      h ^= (((unsigned int) buf[2] & 0xFFU) << 16);
    case 2:
      h ^= (((unsigned int) buf[1] & 0xFFU) << 8);
    case 1:
      h ^=  ((unsigned int) buf[0] & 0xFFU);
      {
        uint64_t  tmp = (uint32_t) h * (uint64_t) 0xC2B0C3CCU;
        h = ((unsigned int) tmp ^ (unsigned int) (tmp >> 32)) & 0xFFFFFFFFU;
      }
      break;
    default:
      break;
    }
    return uint32_t(h);
  }

  PLUS4EMU_REGPARM2 uint32_t File::crc_32(const unsigned char *buf,
                                          size_t nBytes)
  {
    static const uint32_t crc32Table[4] = {
      0x00000000U, 0x76DC4190U, 0xEDB88320U, 0x9B64C2B0U
    };
    uint32_t  crc = ~0U;
    for ( ; nBytes > 0; buf++, nBytes--) {
      unsigned char c = *buf;
      crc = (crc >> 2) ^ crc32Table[(crc ^ uint32_t(c)) & 3U];
      crc = (crc >> 2) ^ crc32Table[(crc ^ uint32_t(c >> 2)) & 3U];
      crc = (crc >> 2) ^ crc32Table[(crc ^ uint32_t(c >> 4)) & 3U];
      crc = (crc >> 2) ^ crc32Table[(crc ^ uint32_t(c >> 6)) & 3U];
    }
    return ~crc;
  }

  File::Buffer::Buffer()
  {
    buf = (unsigned char *) 0;
    this->clear();
  }

  File::Buffer::Buffer(const unsigned char *buf_, size_t nBytes)
  {
    buf = (unsigned char *) 0;
    this->clear();
    writeData(buf_, nBytes);
  }

  File::Buffer::~Buffer()
  {
    this->clear();
  }

  unsigned char File::Buffer::readByte()
  {
    if (curPos >= dataSize)
      throw Exception("unexpected end of data chunk");
    unsigned char c = buf[curPos++] & 0xFF;
    return c;
  }

  bool File::Buffer::readBoolean()
  {
    unsigned char c = readByte();
    return (c == 0 ? false : true);
  }

  int32_t File::Buffer::readInt32()
  {
    uint32_t  n = readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    return int32_t(n);
  }

  uint32_t File::Buffer::readUInt32()
  {
    uint32_t  n = readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    return n;
  }

  int64_t File::Buffer::readInt64()
  {
    uint64_t  n = readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    return int64_t(n);
  }

  uint64_t File::Buffer::readUInt64()
  {
    uint64_t  n = readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    n = (n << 8) | readByte();
    return n;
  }

  double File::Buffer::readFloat()
  {
    int32_t   i = readInt32();
    uint32_t  f = readUInt32();
    return (double(i) + (double(f) * (1.0 / 4294967296.0)));
  }

  std::string File::Buffer::readString()
  {
    size_t  i, j;

    for (i = curPos; i < dataSize; i++) {
      if (buf[i] == 0)
        break;
    }
    if (i >= dataSize)
      throw Exception("unexpected end of data chunk while reading string");
    j = curPos;
    curPos = i + 1;
    return std::string(reinterpret_cast<char *>(&buf[j]));
  }

  void File::Buffer::writeByte(unsigned char n)
  {
    if (curPos >= allocSize) {
      size_t        newSize = ((allocSize + (allocSize >> 3)) | 255) + 1;
      unsigned char *newBuf = new unsigned char[newSize];
      if (buf) {
        for (size_t i = 0; i < dataSize; i++)
          newBuf[i] = buf[i];
        delete[] buf;
      }
      buf = newBuf;
      allocSize = newSize;
    }
    buf[curPos++] = n & 0xFF;
    if (curPos > dataSize)
      dataSize = curPos;
  }

  void File::Buffer::writeBoolean(bool n)
  {
    writeByte(n ? 1 : 0);
  }

  void File::Buffer::writeInt32(int32_t n)
  {
    writeByte(uint8_t(uint32_t(n) >> 24));
    writeByte(uint8_t(uint32_t(n) >> 16));
    writeByte(uint8_t(uint32_t(n) >> 8));
    writeByte(uint8_t(uint32_t(n)));
  }

  void File::Buffer::writeUInt32(uint32_t n)
  {
    writeByte(uint8_t(n >> 24));
    writeByte(uint8_t(n >> 16));
    writeByte(uint8_t(n >> 8));
    writeByte(uint8_t(n));
  }

  void File::Buffer::writeInt64(int64_t n)
  {
    writeByte(uint8_t(uint64_t(n) >> 56));
    writeByte(uint8_t(uint64_t(n) >> 48));
    writeByte(uint8_t(uint64_t(n) >> 40));
    writeByte(uint8_t(uint64_t(n) >> 32));
    writeByte(uint8_t(uint64_t(n) >> 24));
    writeByte(uint8_t(uint64_t(n) >> 16));
    writeByte(uint8_t(uint64_t(n) >> 8));
    writeByte(uint8_t(uint64_t(n)));
  }

  void File::Buffer::writeUInt64(uint64_t n)
  {
    writeByte(uint8_t(n >> 56));
    writeByte(uint8_t(n >> 48));
    writeByte(uint8_t(n >> 40));
    writeByte(uint8_t(n >> 32));
    writeByte(uint8_t(n >> 24));
    writeByte(uint8_t(n >> 16));
    writeByte(uint8_t(n >> 8));
    writeByte(uint8_t(n));
  }

  void File::Buffer::writeFloat(double n)
  {
    int32_t   i = 0;
    uint32_t  f = 0;
    if (n > -2147483648.0 && n < 2147483648.0) {
      double  tmp_i, tmp_f;
      tmp_f = std::modf(n, &tmp_i) * 4294967296.0;
      i = int32_t(tmp_i);
      if (n >= 0.0) {
        tmp_f += 0.5;
        if (tmp_f >= 4294967296.0) {
          f = 0;
          if (i < 2147483647)
            i++;
          else
            f--;
        }
        else
          f = uint32_t(tmp_f);
      }
      else {
        i--;
        tmp_f += 4294967296.5;
        if (tmp_f >= 4294967296.0) {
          f = 0;
          i++;
        }
        else
          f = uint32_t(tmp_f);
      }
    }
    else if (n <= -2147483648.0) {
      i = (-2147483647 - 1);
    }
    else if (n >= 2147483648.0) {
      i = 2147483647;
      f--;
    }
    writeInt32(i);
    writeUInt32(f);
  }

  void File::Buffer::writeString(const std::string& n)
  {
    writeData(reinterpret_cast<const unsigned char *>(n.c_str()),
              n.length() + 1);
  }

  void File::Buffer::writeData(const unsigned char *buf_, size_t nBytes)
  {
    if ((curPos + nBytes) > allocSize) {
      size_t  newSize = allocSize;
      do {
        newSize = ((newSize + (newSize >> 3)) | 255) + 1;
      } while (newSize < (curPos + nBytes));
      unsigned char *newBuf = new unsigned char[newSize];
      if (buf) {
        for (size_t i = 0; i < dataSize; i++)
          newBuf[i] = buf[i];
        delete[] buf;
      }
      buf = newBuf;
      allocSize = newSize;
    }
    for (size_t i = 0; i < nBytes; i++)
      buf[curPos++] = buf_[i] & 0xFF;
    if (curPos > dataSize)
      dataSize = curPos;
  }

  void File::Buffer::setPosition(size_t pos)
  {
    if (pos > dataSize) {
      if (pos > allocSize) {
        size_t  newSize = allocSize;
        do {
          newSize = ((newSize + (newSize >> 3)) | 255) + 1;
        } while (newSize < pos);
        unsigned char *newBuf = new unsigned char[newSize];
        if (buf) {
          for (size_t i = 0; i < dataSize; i++)
            newBuf[i] = buf[i];
          delete[] buf;
        }
        buf = newBuf;
        allocSize = newSize;
      }
      for (size_t i = dataSize; i < pos; i++)
        buf[i] = 0;
      dataSize = pos;
    }
    curPos = pos;
  }

  void File::Buffer::clear()
  {
    if (buf)
      delete[] buf;
    buf = (unsigned char *) 0;
    curPos = 0;
    dataSize = 0;
    allocSize = 0;
  }

  // --------------------------------------------------------------------------

  void File::loadCompressedFile(std::FILE *f)
  {
    long    fileSize = 0L;
    if (std::fseek(f, 0L, SEEK_END) < 0 || (fileSize = std::ftell(f)) < 0L ||
        std::fseek(f, 0L, SEEK_SET) < 0) {
      throw Exception("error seeking file");
    }
    if (fileSize < 20L || fileSize >= 0x00300000L)
      throw Exception("invalid file header");
    std::vector< unsigned char >  tmpBuf;
    {
      std::vector< unsigned char >  inBuf(fileSize);
      if (std::fread(&(inBuf.front()), sizeof(unsigned char), size_t(fileSize),
                     f) != size_t(fileSize)) {
        throw Exception("error reading file");
      }
      tmpBuf.reserve(fileSize);
      try {
        Plus4Emu::decompressData(tmpBuf, &(inBuf.front()), inBuf.size());
      }
      catch (...) {
        throw Exception("invalid file header or error in compressed file");
      }
    }
    for (size_t i = 0; i < 16; i++) {
      if (i >= tmpBuf.size() || tmpBuf[i] != plus4EmuFile_Magic[i])
        throw Exception("invalid file header");
    }
    buf.clear();
    buf.setPosition(tmpBuf.size() - 16);
    buf.setPosition(0);
    if (buf.getDataSize() > 0) {
      std::memcpy(const_cast< unsigned char * >(buf.getData()),
                  &(tmpBuf.front()) + 16, buf.getDataSize());
    }
  }

  File::File()
  {
  }

  File::File(const char *fileName, bool useHomeDirectory)
  {
    bool    err = false;

    if (fileName != (char*) 0 && fileName[0] != '\0') {
      std::string fullName;
      if (useHomeDirectory)
        getFullPathFileName(fileName, fullName);
      else
        fullName = fileName;
      std::FILE *f = fileOpen(fullName.c_str(), "rb");
      if (f) {
        try {
          int     c;
          for (int i = 0; i < 16; i++) {
            c = std::fgetc(f);
            if (c == EOF ||
                (unsigned char) (c & 0xFF) != plus4EmuFile_Magic[i]) {
              // check for compressed file format
              loadCompressedFile(f);
              std::fclose(f);
              return;
            }
          }
          while ((c = std::fgetc(f)) != EOF)
            buf.writeByte((unsigned char) (c & 0xFF));
        }
        catch (...) {
          buf.clear();
          std::fclose(f);
          throw;
        }
        buf.setPosition(0);
        if (std::ferror(f))
          err = true;
        if (std::fclose(f) != 0)
          err = true;
      }
      else
        err = true;
    }
    else
      err = true;
    if (err) {
      buf.clear();
      throw Exception("error opening or reading file");
    }
  }

  File::~File()
  {
    std::map< int, ChunkTypeHandler * >::iterator   i;

    for (i = chunkTypeDB.begin(); i != chunkTypeDB.end(); i++)
      delete (*i).second;
    chunkTypeDB.clear();
  }

  void File::addChunk(ChunkType type, const Buffer& buf_)
  {
    if (type == PLUS4EMU_CHUNKTYPE_END_OF_FILE)
      throw Exception("internal error: invalid chunk type");
    size_t  startPos = buf.getPosition();
    buf.setPosition(startPos + buf_.getDataSize() + 12);
    buf.setPosition(startPos);
    buf.writeUInt32(uint32_t(type));
    buf.writeUInt32(uint32_t(buf_.getDataSize()));
    buf.writeData(buf_.getData(), buf_.getDataSize());
    buf.writeUInt32(hash_32(buf.getData() + startPos, buf_.getDataSize() + 8));
  }

  void File::processAllChunks()
  {
    if (buf.getDataSize() < 12)
      throw Exception("file is too short (no data)");
    buf.setPosition(0);
    while (buf.getPosition() < (buf.getDataSize() - 12)) {
      size_t  startPos = buf.getPosition();
      int     type = int(buf.readInt32());
      size_t  len = buf.readUInt32();
      if (len > (buf.getDataSize() - (startPos + 12)))
        throw Exception("unexpected end of file");
      buf.setPosition(startPos + len + 8);
      if (buf.readUInt32() != hash_32(buf.getData() + startPos, len + 8))
        throw Exception("CRC error in file data");
      if (ChunkType(type) == PLUS4EMU_CHUNKTYPE_END_OF_FILE)
        throw Exception("unexpected 'end of file' chunk");
      if (chunkTypeDB.find(type) != chunkTypeDB.end()) {
        Buffer  tmpBuf(buf.getData() + (startPos + 8), len);
        tmpBuf.setPosition(0);
        chunkTypeDB[type]->processChunk(tmpBuf);
      }
    }
    if (buf.getPosition() != (buf.getDataSize() - 12))
      throw Exception("file is truncated (missing 'end of file' chunk)");
    if (ChunkType(buf.readUInt32()) != PLUS4EMU_CHUNKTYPE_END_OF_FILE)
      throw Exception("file is truncated (missing 'end of file' chunk)");
    if (buf.readUInt32() != 0)
      throw Exception("invalid length for 'end of file' chunk (must be zero)");
    if (buf.readUInt32()
        != hash_32(buf.getData() + (buf.getDataSize() - 12), 8))
      throw Exception("CRC error in file data");
  }

  void File::writeFile(const char *fileName, bool useHomeDirectory,
                       bool enableCompression)
  {
    size_t  startPos = buf.getPosition();
    bool    err = true;

    if (enableCompression) {
      buf.setPosition(startPos + 28);
      if (startPos > 0) {
        std::memmove(const_cast< unsigned char * >(buf.getData() + 16),
                     buf.getData(), startPos);
      }
      std::memcpy(const_cast< unsigned char * >(buf.getData()),
                  &(plus4EmuFile_Magic[0]), 16);
      startPos = startPos + 16;
    }
    else {
      buf.setPosition(startPos + 12);
    }
    buf.setPosition(startPos);
    buf.writeUInt32(uint32_t(PLUS4EMU_CHUNKTYPE_END_OF_FILE));
    buf.writeUInt32(0U);
    buf.writeUInt32(hash_32(buf.getData() + startPos, 8));
    if (enableCompression) {
      try {
        std::vector< unsigned char >  tmpBuf;
        compressData(tmpBuf, buf.getData(), startPos + 12);
        buf.clear();
        buf.setPosition(tmpBuf.size());
        std::memcpy(const_cast< unsigned char * >(buf.getData()),
                    &(tmpBuf.front()), tmpBuf.size());
      }
      catch (...) {
        buf.clear();
        throw Exception("error compressing file");
      }
    }
    if (fileName != (char*) 0 && fileName[0] != '\0') {
      std::string fullName;
      if (useHomeDirectory)
        getFullPathFileName(fileName, fullName);
      else
        fullName = fileName;
      std::FILE *f = fileOpen(fullName.c_str(), "wb");
      if (f) {
        err = !(enableCompression ||
                std::fwrite(&(plus4EmuFile_Magic[0]), 1, 16, f) == 16);
        if (!err) {
          if (std::fwrite(buf.getData(),
                          sizeof(unsigned char), buf.getDataSize(), f)
              != buf.getDataSize()) {
            err = true;
          }
        }
        if (std::fclose(f) != 0)
          err = true;
        if (err)
          fileRemove(fullName.c_str());
      }
    }
    buf.clear();
    if (err)
      throw Exception("error opening or writing file");
  }

  // --------------------------------------------------------------------------

  File::ChunkTypeHandler::~ChunkTypeHandler()
  {
  }

  void File::registerChunkType(ChunkTypeHandler *p)
  {
    if (!p)
      throw Exception("internal error: NULL chunk type handler");

    int     type = int(p->getChunkType());

    if (chunkTypeDB.find(type) != chunkTypeDB.end()) {
      delete chunkTypeDB[type];
      chunkTypeDB.erase(type);
    }
    chunkTypeDB[type] = p;
  }

  // --------------------------------------------------------------------------

  static const unsigned char d64ImageData[89] = {
    0xFA, 0x00, 0x1F, 0xC0, 0x02, 0x00, 0x00, 0x00,
    0x02, 0x35, 0x32, 0x05, 0x30, 0x00, 0x45, 0x30,
    0xA9, 0x00, 0x01, 0x4F, 0x15, 0xED, 0xC4, 0x32,
    0xF8, 0x4A, 0x07, 0xF8, 0x41, 0x25, 0x41, 0x56,
    0xFA, 0x71, 0xE7, 0xBF, 0xE2, 0xF7, 0x10, 0x8F,
    0x74, 0x7F, 0x04, 0x12, 0x01, 0x41, 0xC8, 0x15,
    0xFF, 0x39, 0x1F, 0xD8, 0x11, 0xFC, 0x99, 0x07,
    0x13, 0x2D, 0x39, 0x12, 0x5B, 0x03, 0x46, 0x56,
    0x11, 0x01, 0xCD, 0x99, 0xA0, 0xC2, 0x30, 0x12,
    0x2E, 0x32, 0x41, 0xA3, 0xB3, 0xFF, 0xE6, 0x33,
    0x0F, 0x19, 0xFC, 0x84, 0x20, 0x7B, 0x6B, 0xB4,
    0x00
  };

  static const unsigned char d81ImageData[134] = {
    0xB4, 0x00, 0x6A, 0xC0, 0x03, 0x39, 0x60, 0x42,
    0x01, 0x02, 0x04, 0x45, 0x79, 0x05, 0x15, 0x10,
    0x6E, 0x00, 0x01, 0x4D, 0x0A, 0xA9, 0x87, 0x65,
    0x50, 0xF3, 0x00, 0x07, 0x84, 0xFB, 0xC0, 0x88,
    0x02, 0x6D, 0x0E, 0x4C, 0x3B, 0x2A, 0x80, 0x84,
    0x4C, 0x7F, 0xBE, 0x30, 0x26, 0xD4, 0xC3, 0xB2,
    0x75, 0xA8, 0x67, 0xFB, 0xE0, 0xF2, 0x06, 0xFA,
    0x0A, 0xBB, 0x1E, 0xD2, 0x76, 0x11, 0xA0, 0x28,
    0xA0, 0x81, 0x24, 0x04, 0x09, 0x03, 0x44, 0x58,
    0xB3, 0x84, 0x30, 0x24, 0x2C, 0x33, 0x52, 0xBE,
    0x3F, 0x24, 0xC8, 0x02, 0xBB, 0xB6, 0x01, 0xC0,
    0xC7, 0xC2, 0x58, 0xFF, 0x27, 0xC6, 0x96, 0x80,
    0xF0, 0x64, 0x00, 0x74, 0x7F, 0x9F, 0xBD, 0x17,
    0x7E, 0x3F, 0xE9, 0x00, 0x09, 0xB5, 0x30, 0x39,
    0xEC, 0xAA, 0xFD, 0xCC, 0xFE, 0xF8, 0x41, 0x9B,
    0x53, 0x0E, 0xCA, 0xA0, 0xD0, 0x80, 0x1F, 0xEF,
    0x80, 0xFF, 0x7A, 0x8F, 0xD1, 0x00
  };

  std::FILE *createDiskImage(const char *fileName)
  {
    bool    isD81 = checkFileNameExtension(fileName, ".d81");
    if (!isD81 && !checkFileNameExtension(fileName, ".d64"))
      throw Exception("error opening disk image file");

    const unsigned char *compressedImage =
        (isD81 ? &(d81ImageData[0]) : &(d64ImageData[0]));
    size_t  compressedImageSize =
        (isD81 ? sizeof(d81ImageData) : sizeof(d64ImageData))
        / sizeof(unsigned char);
    std::vector< unsigned char >  buf;
    {
      std::vector< unsigned char >  tmpBuf;
      decompressData(tmpBuf, compressedImage, compressedImageSize);
      decompressData(buf, &(tmpBuf.front()), tmpBuf.size());
    }

    const char  *s = fileName + (std::strlen(fileName) - 4);
    size_t  n = 0;
    for ( ; s > fileName; s--, n++) {
      char    c = *(s - 1);
      if (c == '/' || c == '\\')
        break;
#ifdef WIN32
      if (c == ':')
        break;
#endif
    }
    unsigned char *diskName =
        &(buf.front()) + (isD81 ? 0x00061804 : 0x00016590);
    for (size_t i = 0; i < n && i < 16; i++) {
      char    c = s[i];
      if (c >= 'a' && c <= 'z') {
        c = c - ('a' - 'A');
      }
      else if (c == '_') {
        c = ' ';
      }
      else if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
                 c == '+' || c == '-' || c == '.' || c == ' ')) {
        c = char(0xA4);
      }
      diskName[i] = (unsigned char) c;
    }

    unsigned char *diskID = diskName + 18;
    uint32_t  tmp = File::crc_32(diskName, 16);
    tmp = tmp ^ (tmp >> 16);
    tmp = tmp ^ (tmp >> 8);
    for (int i = 0; i < 2; i++) {
      diskID[i] = (unsigned char) (((tmp >> (i << 2)) & 0x0F) | '0');
      if (diskID[i] > '9')
        diskID[i] = diskID[i] + ('A' - ('9' + 1));
      if (isD81) {
        buf[0x00061904 + i] = diskID[i];
        buf[0x00061A04 + i] = diskID[i];
      }
    }

    std::FILE *f = fileOpen(fileName, "w+b");
    if (!f ||
        std::fwrite(&(buf.front()), sizeof(unsigned char), buf.size(), f)
        != buf.size() ||
        std::fflush(f) != 0 ||
        std::fseek(f, 0L, SEEK_SET) < 0) {
      if (f) {
        std::fclose(f);
        fileRemove(fileName);
      }
      throw Exception("error creating disk image file");
    }

    return f;
  }

  // --------------------------------------------------------------------------

  // 0:       unused symbol
  // 1 to 15: code length in bits
  // 16:      repeat the last code length 3 to 6 times (2 extra bits)
  // 17:      repeat unused symbol 3 to 10 times (3 extra bits)
  // 18:      repeat unused symbol 11 to 138 times (7 extra bits)

  static const unsigned char deflateCodeLengthCodeTable[19] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
  };

  unsigned char ZIPFile::readByte()
  {
    if (inBufPos >= inBuf.size())
      throw Exception("unexpected end of file");
    unsigned char c = inBuf[inBufPos];
    inBufPos++;
    return c;
  }

  uint16_t ZIPFile::readUInt16()
  {
    uint16_t  n = readByte();
    n = n | (uint16_t(readByte()) << 8);
    return n;
  }

  uint32_t ZIPFile::readUInt32()
  {
    uint32_t  n = readByte();
    n = n | (uint32_t(readByte()) << 8);
    n = n | (uint32_t(readByte()) << 16);
    n = n | (uint32_t(readByte()) << 24);
    return n;
  }

  PLUS4EMU_INLINE unsigned int ZIPFile::readBit()
  {
    if (PLUS4EMU_UNLIKELY(shiftRegisterCnt < 1)) {
      shiftRegister = readByte();
      shiftRegisterCnt = 8;
    }
    unsigned int  retval = (unsigned int) (shiftRegister & 0x01);
    shiftRegister = shiftRegister >> 1;
    shiftRegisterCnt--;
    return retval;
  }

  unsigned int ZIPFile::readBits(size_t nBits)
  {
    unsigned int  retval = 0U;
    for (unsigned char i = 0; i < (unsigned char) nBits; ) {
      if (PLUS4EMU_UNLIKELY(shiftRegisterCnt < 1)) {
        shiftRegister = readByte();
        shiftRegisterCnt = 8;
      }
      unsigned char n = (unsigned char) nBits - i;
      if (n > (unsigned char) shiftRegisterCnt)
        n = (unsigned char) shiftRegisterCnt;
      retval |= (((unsigned int) shiftRegister & ((1U << n) - 1U)) << i);
      shiftRegister = shiftRegister >> n;
      shiftRegisterCnt = shiftRegisterCnt - n;
      i = i + n;
    }
    return retval;
  }

  unsigned int ZIPFile::huffmanDecode(int huffTable)
  {
    int     tmp = 0;
    int     cnt = -1;
    const unsigned int  *symCntTable =
        (huffTable == 0 ? huffmanSymCntTable0 : huffmanSymCntTable1);
    const unsigned int  *offsetTable =
        (huffTable == 0 ? huffmanOffsetTable0 : huffmanOffsetTable1);
    const unsigned int  *decodeTable =
        (huffTable == 0 ? huffmanDecodeTable0 : huffmanDecodeTable1);
    do {
      if (++cnt >= 15)
        throw Plus4Emu::Exception("error in compressed data");
      tmp = ((tmp << 1) | int(readBit())) - int(symCntTable[cnt]);
    } while (tmp >= 0);
    tmp = tmp + int(offsetTable[cnt]);
    if (decodeTable[tmp] == 0xFFFFFFFFU)
      throw Plus4Emu::Exception("error in compressed data");
    return decodeTable[tmp];
  }

  void ZIPFile::buildDecodeTable(int huffTable,
                                 const unsigned char *lenBuf, size_t nSymbols)
  {
    unsigned int  *symCntTable =
        (huffTable == 0 ? huffmanSymCntTable0 : huffmanSymCntTable1);
    unsigned int  *offsetTable =
        (huffTable == 0 ? huffmanOffsetTable0 : huffmanOffsetTable1);
    unsigned int  *decodeTable =
        (huffTable == 0 ? huffmanDecodeTable0 : huffmanDecodeTable1);
    for (size_t i = 0; i < 15; i++)
      symCntTable[i] = 0U;
    for (size_t i = 0; i < nSymbols; i++) {
      decodeTable[i] = 0xFFFFFFFFU;
      if (lenBuf[i])
        symCntTable[lenBuf[i] - 1] = symCntTable[lenBuf[i] - 1] + 1U;
    }
    {
      unsigned int  offs = 0U;
      for (size_t i = 0; i < 15; i++) {
        offsetTable[i] = offs;
        offs = offs + symCntTable[i];
      }
    }
    for (size_t i = 0; i < nSymbols; i++) {
      unsigned char len = lenBuf[i];
      if (len) {
        len--;
        unsigned int  offs = offsetTable[len];
        decodeTable[offs] = (unsigned int) i;
        offsetTable[len] = offs + 1U;
      }
    }
  }

  void ZIPFile::huffmanInit(unsigned char blockType)
  {
    unsigned char lenBuf[320];
    if (blockType == 1) {
      // fixed Huffman codes
      for (int i = 0; i < 320; i++) {
        lenBuf[i] =
            (i < 144 ? 8 : (i < 256 ? 9 : (i < 280 ? 7 : (i < 288 ? 8 : 5))));
      }
    }
    else {
      for (int i = 0; i < 320; i++)
        lenBuf[i] = 0;
      size_t  litCnt = size_t(readBits(5)) + 257;
      size_t  distCnt = size_t(readBits(5)) + 1;
      size_t  codeCnt = size_t(readBits(4)) + 4;
      for (size_t i = 0; i < 19; i++)
        lenBuf[i] = 0;
      for (size_t i = 0; i < codeCnt; i++)
        lenBuf[deflateCodeLengthCodeTable[i]] = (unsigned char) readBits(3);
      buildDecodeTable(1, lenBuf, 19);
      size_t  totalCnt = distCnt + 288;
      size_t  rleCnt = 0;
      unsigned char rleCode = 0x00;
      for (size_t i = 0; i < totalCnt; i++) {
        if (i == litCnt)
          i = 288;
        if (rleCnt > 0) {
          lenBuf[i] = rleCode;
          rleCnt--;
          continue;
        }
        unsigned char c = (unsigned char) huffmanDecode(1);
        if (c < 16) {
          lenBuf[i] = c;
          rleCode = c;
          continue;
        }
        else if (c == 16) {
          if (!i)
            throw Plus4Emu::Exception("error in compressed data");
          rleCnt = size_t(readBits(2)) + 2;
        }
        else {
          if (c == 17)
            rleCnt = size_t(readBits(3)) + 2;
          else
            rleCnt = size_t(readBits(7)) + 10;
          rleCode = 0x00;
        }
        lenBuf[i] = rleCode;
      }
      if (rleCnt > 0 || lenBuf[256] == 0)
        throw Plus4Emu::Exception("error in compressed data");
    }
    buildDecodeTable(0, lenBuf, 288);
    buildDecodeTable(1, &(lenBuf[288]), 32);
  }

  bool ZIPFile::decompressDataBlock(std::vector< unsigned char >& buf)
  {
    static const size_t maxDataSize = 0x04000000;
    bool    isLastBlock = bool(readBit());
    unsigned char blockType = (unsigned char) readBits(2);
    if (blockType == 3)
      throw Plus4Emu::Exception("error in compressed data");
    if ((buf.size() + 65536) > buf.capacity())
      buf.reserve(((buf.size() + (buf.size() >> 2)) | 0xFFFF) + 1);
    if (!blockType) {
      // uncompressed data
      shiftRegisterCnt = 0;
      unsigned int  blockSize = readUInt32();
      blockSize = blockSize ^ ((~blockSize & 0xFFFFU) << 16);
      if (!(blockSize >= 1U && blockSize <= 0xFFFFU))
        throw Plus4Emu::Exception("error in compressed data");
      do {
        if (PLUS4EMU_UNLIKELY(buf.size() >= maxDataSize))
          throw Plus4Emu::Exception("error in compressed data");
        buf.push_back((unsigned char) readByte());
      } while (--blockSize);
      return isLastBlock;
    }
    huffmanInit(blockType);
    unsigned int  prvDistance = 0U;
    while (true) {
      unsigned int  c = huffmanDecode(0);
      if (c == 0x0100U)
        break;
      if (c < 0x0100U) {
        // literal character
        if (PLUS4EMU_UNLIKELY(buf.size() >= maxDataSize))
          throw Plus4Emu::Exception("error in compressed data");
        buf.push_back((unsigned char) c);
        continue;
      }
      // decode length:
      //   0x0101..0x0108: 3 to 10
      //   0x0109..0x010C: 11 to 18 (1 extra bit)
      //   0x010D..0x0110: 19 to 34 (2 extra bits)
      //   0x0111..0x0114: 35 to 66 (3 extra bits)
      //   0x0115..0x0118: 67 to 130 (4 extra bits)
      //   0x0119..0x011C: 131 to 258 (5 extra bits)
      //   0x011D:         258, no extra bits
      //   0x011E:         2 bytes (non-standard extension)
      //   0x011F:         2 bytes, repeat prv. offset (non-standard extension)
      unsigned int  len = c - 0x0100U;
      unsigned int  d = prvDistance;
      if (len > 8U) {
        if (len > 28U) {
          len = (len == 29U ? 256U : 0U);
        }
        else {
          unsigned char nBits = (unsigned char) ((len - 5U) >> 2);
          len = (((((len - 1U) & 3U) | 4U) << nBits) | readBits(nBits)) + 1U;
        }
      }
      len = len + 2U;
      // decode offset:
      //     0..3: 1 to 4
      //     4..5: 5 to 8 (1 extra bit)
      //     ...
      //   28..29: 16385 to 32768 (13 extra bits)
      //   30..31: 32769 to 65536 (14 extra bits, non-standard extension)
      if (c != 0x011FU) {
        c = huffmanDecode(1);
        if (c < 4U) {
          d = c + 1U;
        }
        else {
          unsigned char nBits = (unsigned char) ((c - 2U) >> 1);
          d = ((((c & 1U) | 2U) << nBits) | readBits(nBits)) + 1U;
        }
      }
      if (!(d > 0U && d <= (unsigned int) buf.size()))
        throw Plus4Emu::Exception("error in compressed data");
      prvDistance = d;
      if (PLUS4EMU_UNLIKELY((buf.size() + size_t(len)) > maxDataSize))
        throw Plus4Emu::Exception("error in compressed data");
      do {
        buf.push_back(buf[buf.size() - size_t(d)]);
      } while (--len);
    }
    return isLastBlock;
  }

  ZIPFile::ZIPFile(const char *fileName)
    : inBufPos(0),
      huffmanSymCntTable0((unsigned int *) 0),
      huffmanOffsetTable0((unsigned int *) 0),
      huffmanDecodeTable0((unsigned int *) 0),
      huffmanSymCntTable1((unsigned int *) 0),
      huffmanOffsetTable1((unsigned int *) 0),
      huffmanDecodeTable1((unsigned int *) 0),
      shiftRegister(0x00),
      shiftRegisterCnt(0)
  {
    size_t  totalTableSize = 15 + 15 + 288 + 15 + 15 + 32;
    huffmanSymCntTable0 = new unsigned int[totalTableSize];
    for (size_t i = 0; i < totalTableSize; i++)
      huffmanSymCntTable0[i] = 0U;
    huffmanOffsetTable0 = &(huffmanSymCntTable0[15]);
    huffmanDecodeTable0 = &(huffmanOffsetTable0[15]);
    huffmanSymCntTable1 = &(huffmanDecodeTable0[288]);
    huffmanOffsetTable1 = &(huffmanSymCntTable1[15]);
    huffmanDecodeTable1 = &(huffmanOffsetTable1[15]);

    std::FILE *f = fileOpen(fileName, "rb");
    if (!f)
      throw Exception("error opening or reading file");
    try {
      long    fileSize = 0L;
      if (std::fseek(f, 0L, SEEK_END) < 0 ||
          (unsigned long) (fileSize = std::ftell(f)) > 0x04000000UL ||
          std::fseek(f, 0L, SEEK_SET) < 0) {
        throw Exception("error opening or reading file");
      }
      inBuf.resize(size_t(fileSize));
      if (std::fread(&(inBuf.front()), sizeof(unsigned char), size_t(fileSize),
                     f) != size_t(fileSize)) {
        throw Exception("error opening or reading file");
      }
      std::fclose(f);
    }
    catch (...) {
      delete[] huffmanSymCntTable0;
      if (f)
        std::fclose(f);
      throw;
    }
  }

  ZIPFile::~ZIPFile()
  {
    delete[] huffmanSymCntTable0;
  }

  bool ZIPFile::getFile(std::vector< unsigned char >& buf,
                        std::string& fileName, int& fileType)
  {
    buf.clear();
    if (!(fileType >= 0 && fileType <= 2))
      fileType = -1;
    while (true) {
      fileName.clear();
      // check header
      uint32_t  h = readUInt32();
      if (h == 0x02014B50)              // central directory file header
        return false;
      else if (h != 0x04034B50)         // local file header
        throw Exception("invalid file header");
      (void) readUInt16();              // version needed to extract
      uint16_t  flg = readUInt16();     // general purpose bit flag
      if (flg & 0x0008)                 // unknown file size: not supported
        throw Exception("invalid file header");
      uint16_t  m = readUInt16();       // compression method
      (void) readUInt32();              // last modification time and date
      uint32_t  crc = readUInt32();     // CRC-32
      uint32_t  compressedSize = readUInt32();
      uint32_t  uncompressedSize = readUInt32();
      uint16_t  nameLen = readUInt16(); // file name length
      uint16_t  extLen = readUInt16();  // extra field length
      for ( ; nameLen > 0; nameLen--) {
        char    c = char(readByte());
        if ((unsigned char) c < 0x20)
          c = ' ';
        fileName += c;
      }
      for ( ; extLen > 0; extLen--)
        (void) readByte();

      // check file type
      int     t = -1;
      if ((fileType < 0 || fileType == 0) &&
          (checkFileNameExtension(fileName.c_str(), ".prg") ||
           checkFileNameExtension(fileName.c_str(), ".p00"))) {
        t = 0;
      }
      else if ((fileType < 0 || fileType == 1) &&
               (checkFileNameExtension(fileName.c_str(), ".d64") ||
                checkFileNameExtension(fileName.c_str(), ".d81"))) {
        t = 1;
      }
      else if ((fileType < 0 || fileType == 2) &&
               checkFileNameExtension(fileName.c_str(), ".tap")) {
        t = 2;
      }
      if (fileType >= 0 && t != fileType) {
        // file type does not match: skip file
        if (inBufPos > inBuf.size() ||
            size_t(compressedSize) > (inBuf.size() - inBufPos)) {
          throw Exception("unexpected end of file");
        }
        inBufPos = inBufPos + compressedSize;
        continue;
      }
      fileType = t;

      if (m == 0x0000) {                // Store
        if (compressedSize != uncompressedSize)
          throw Exception("error in compressed data");
        for ( ; compressedSize > 0U; compressedSize--)
          buf.push_back(readByte());
      }
      else if (m == 0x0008) {           // Deflate
        size_t  endPos = inBufPos + compressedSize;
        shiftRegister = 0x00;
        shiftRegisterCnt = 0;
        // decompress all data blocks
        while (!decompressDataBlock(buf))
          ;
        // on successful decompression, all input data must be consumed
        if (inBufPos != endPos)
          throw Exception("error in compressed data");
      }
      else {                            // unsupported method
        throw Exception("error in compressed data");
      }

      // verify CRC-32 checksum
      if (crc != File::crc_32(&(buf.front()), buf.size()))
        throw Exception("error in compressed data");

      break;
    }
    return true;
  }

  // --------------------------------------------------------------------------

  std::FILE *openPlus4ImageFile(const char *fileName,
                                int& fileType, bool& isReadOnly)
  {
    if (!(fileType >= 0 && fileType <= 2))
      fileType = -1;
    if (checkFileNameExtension(fileName, ".zip")) {
      isReadOnly = true;
      ZIPFile zipFile(fileName);
      std::vector< unsigned char >  buf;
      std::string s;
      if (!zipFile.getFile(buf, s, fileType))
        throw Exception("no matching file found in archive");
      std::FILE *f = std::tmpfile();
      if (!f ||
          std::fwrite(&(buf.front()), sizeof(unsigned char), buf.size(), f)
          != buf.size() ||
          std::fflush(f) != 0 ||
          std::fseek(f, 0L, SEEK_SET) < 0) {
        if (f)
          std::fclose(f);
        throw Exception("error creating temporary file");
      }
      return f;
    }
    std::FILE *f = (std::FILE *) 0;
    if (!isReadOnly)
      f = fileOpen(fileName, "r+b");
    if (!f) {
      f = fileOpen(fileName, "rb");
      if (f)
        isReadOnly = true;
    }
    if (fileType == 1) {
      if (f) {
#ifdef WIN32
        if (!(fileName[0] == '\\' && fileName[1] == '\\' &&
              fileName[2] == '.' && fileName[3] == '\\'))
#elif defined(__linux) || defined(__linux__)
        if (std::strncmp(fileName, "/dev/", 5) == 0)
#endif
        {
          std::setvbuf(f, (char *) 0, _IONBF, 0);
        }
      }
      else {
        isReadOnly = false;
        f = createDiskImage(fileName);
      }
    }
    return f;
  }

}       // namespace Plus4Emu

