
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
#include "d64image.hpp"
#include "system.hpp"
#include "fileio.hpp"

namespace Plus4 {

  const int D64Image::d64TrackOffsetTable[44] = {
        -1,      0,   5376,  10752,  16128,  21504,  26880,  32256,
     37632,  43008,  48384,  53760,  59136,  64512,  69888,  75264,
     80640,  86016,  91392,  96256, 101120, 105984, 110848, 115712,
    120576, 125440, 130048, 134656, 139264, 143872, 148480, 153088,
    157440, 161792, 166144, 170496, 174848, 179200, 183552, 187904,
    192256, 196608, 200960, 205312
  };

  const int D64Image::sectorsPerTrackTable[44] = {
     0, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 19, 19, 19, 19, 19, 19,
    19, 18, 18, 18, 18, 18, 18, 17,
    17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17
  };

  const int D64Image::trackSizeTable[44] = {
    7692, 7692, 7692, 7692, 7692, 7692, 7692, 7692,
    7692, 7692, 7692, 7692, 7692, 7692, 7692, 7692,
    7692, 7692, 7143, 7143, 7143, 7143, 7143, 7143,
    7143, 6667, 6667, 6667, 6667, 6667, 6667, 6250,
    6250, 6250, 6250, 6250, 6250, 6250, 6250, 6250,
    6250, 6250, 6250, 6250
  };

  // number of 4 MHz cycles per bit
  const unsigned char D64Image::trackSpeedTable[44] = {
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 14, 14, 14, 14, 14, 14,
    14, 15, 15, 15, 15, 15, 15, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16
  };

  const uint8_t D64Image::gcrEncodeTable[16] = {
    0x0A, 0x0B, 0x12, 0x13, 0x0E, 0x0F, 0x16, 0x17,
    0x09, 0x19, 0x1A, 0x1B, 0x0D, 0x1D, 0x1E, 0x15
  };

  const uint8_t D64Image::gcrDecodeTable[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x08, 0x00, 0x01, 0xFF, 0x0C, 0x04, 0x05,
    0xFF, 0xFF, 0x02, 0x03, 0xFF, 0x0F, 0x06, 0x07,
    0xFF, 0x09, 0x0A, 0x0B, 0xFF, 0x0D, 0x0E, 0xFF
  };

  // --------------------------------------------------------------------------

  void D64Image::gcrEncodeFourBytes(uint8_t *outBuf, const uint8_t *inBuf)
  {
    uint8_t bitBuf = 0;
    uint8_t bitCnt = 0;
    for (int i = 0; i < 8; i++) {
      uint8_t n = inBuf[i >> 1];
      n = ((i & 1) == 0 ? (n >> 4) : n) & 0x0F;
      n = gcrEncodeTable[n];
      for (int j = 0; j < 5; j++) {
        bitBuf = (bitBuf << 1) | ((n & 0x10) >> 4);
        n = n << 1;
        if (++bitCnt >= 8) {
          *(outBuf++) = bitBuf;
          bitBuf = 0;
          bitCnt = 0;
        }
      }
    }
  }

  bool D64Image::gcrDecodeFourBytes(uint8_t *outBuf, const uint8_t *inBuf)
  {
    bool    retval = true;
    uint8_t bitBuf = 0;
    uint8_t bitCnt = 0;
    for (int i = 0; i < 8; i++) {
      uint8_t n = 0;
      for (int j = 0; j < 5; j++) {
        if (bitCnt == 0) {
          bitBuf = *(inBuf++);
          bitCnt = 8;
        }
        bitCnt--;
        n = (n << 1) | ((bitBuf & 0x80) >> 7);
        bitBuf = bitBuf << 1;
      }
      n = gcrDecodeTable[n];
      if (n >= 0x80) {          // return false on invalid GCR data
        n = 0x00;
        retval = false;
      }
      if (!(i & 1))
        outBuf[i >> 1] = n << 4;
      else
        outBuf[i >> 1] = outBuf[i >> 1] | n;
    }
    return retval;
  }

  void D64Image::gcrEncodeTrack(int trackNum, int nSectors, int nBytes)
  {
    int     readPos = 0;
    int     writePos = 0;
    uint8_t tmpBuf1[8];
    uint8_t tmpBuf2[5];
    for (int i = 0; i < nSectors; i++) {
      int     gapSize = 9;
      if (i & 1) {
        switch (nSectors) {
        case 19:
          gapSize = 19;
          break;
        case 18:
          gapSize = 13;
          break;
        case 17:
          gapSize = 10;
          break;
        }
      }
      uint8_t errorCode = badSectorTable[i];
      // write header sync
      if (errorCode != 0x03) {
        for (int j = 0; j < 5; j++)
          trackBuffer_GCR[writePos++] = 0xFF;
      }
      // write header
      tmpBuf1[0] = 0x08;                // block ID
      if (errorCode == 0x02)
        tmpBuf1[0] = 0x00;          // header block not found
      tmpBuf1[2] = uint8_t(i);          // sector (0 to 20)
      tmpBuf1[3] = uint8_t(trackNum);   // track (1 to 35)
      tmpBuf1[4] = idCharacter2;        // format ID
      tmpBuf1[5] = idCharacter1;        // -"-
      tmpBuf1[6] = 0x0F;                // padding
      tmpBuf1[7] = 0x0F;                // -"-
      uint8_t crcValue = 0;
      for (int j = 2; j < 6; j++)
        crcValue = crcValue ^ tmpBuf1[j];
      if (errorCode == 0x09)
        crcValue = (~crcValue);     // CRC error in header
      tmpBuf1[1] = crcValue;            // checksum
      gcrEncodeFourBytes(&(tmpBuf2[0]), &(tmpBuf1[0]));
      for (int j = 0; j < 5; j++)
        trackBuffer_GCR[writePos++] = tmpBuf2[j];
      gcrEncodeFourBytes(&(tmpBuf2[0]), &(tmpBuf1[4]));
      for (int j = 0; j < 5; j++)
        trackBuffer_GCR[writePos++] = tmpBuf2[j];
      // write gap
      for (int j = 0; j < 9; j++)
        trackBuffer_GCR[writePos++] = 0x55;
      // write sector data sync
      if (errorCode != 0x03) {
        for (int j = 0; j < 5; j++)
          trackBuffer_GCR[writePos++] = 0xFF;
      }
      int     bufPos = 0;
      tmpBuf1[bufPos++] = 0x07;         // block ID
      if (errorCode == 0x04)
        tmpBuf1[0] = 0x00;          // data block not found
      // write sector data
      crcValue = 0;
      for (int j = 0; j < 256; j++) {
        uint8_t tmp = trackBuffer_D64[readPos++];
        tmpBuf1[bufPos++] = tmp;
        crcValue = crcValue ^ tmp;
        if (bufPos >= 4) {
          bufPos = 0;
          gcrEncodeFourBytes(&(tmpBuf2[0]), &(tmpBuf1[0]));
          for (int k = 0; k < 5; k++)
            trackBuffer_GCR[writePos++] = tmpBuf2[k];
        }
      }
      if (errorCode == 0x05)
        crcValue = (~crcValue);     // CRC error in data block
      tmpBuf1[1] = crcValue;            // checksum
      tmpBuf1[2] = 0x00;                // padding
      tmpBuf1[3] = 0x00;                // -"-
      gcrEncodeFourBytes(&(tmpBuf2[0]), &(tmpBuf1[0]));
      if (errorCode == 0x10)
        tmpBuf2[0] = 0x00;          // GCR decoding error
      for (int j = 0; j < 5; j++)
        trackBuffer_GCR[writePos++] = tmpBuf2[j];
      // write gap
      do {
        trackBuffer_GCR[writePos++] = 0x55;
      } while (--gapSize);
    }
    // pad track data to requested length
    for ( ; writePos < nBytes; writePos++)
      trackBuffer_GCR[writePos] = 0x55;
  }

  int D64Image::gcrDecodeTrack(int trackNum, int nSectors, int nBytes)
  {
    int     sectorsDecoded = 0;
    int     readPos = 0;
    int     firstSyncPos = -1;
    uint8_t errorCode = 0x03;       // "sync not found"
    // find first header sync
    while (readPos <= (nBytes - 4)) {
      if (trackBuffer_GCR[readPos] == 0xFF) {
        if (trackBuffer_GCR[readPos + 1] == 0xFF) {
          if (trackBuffer_GCR[readPos + 2] == 0x52) {
            if ((trackBuffer_GCR[readPos + 3] & 0xC0) == 0x40) {
              firstSyncPos = readPos;
              errorCode = 0x02;     // "header block not found"
              break;
            }
          }
        }
      }
      readPos++;
    }
    for (int i = 0; i < nSectors; i++)
      badSectorTable[i] = errorCode;
    if (firstSyncPos < 0 || nBytes <= 0)
      return 0;
    // process track data
    readPos = firstSyncPos;
    int     syncCnt = 0;
    // 0: searching for header sync, 1: reading header,
    // 2: searching for sector data sync, 3: reading sector data
    int     currentMode = 0;
    int     gcrBytesToDecode = 0;
    int     gcrByteCnt = 0;
    int     currentSector = 0;
    uint8_t tmpBuf1[325];
    uint8_t tmpBuf2[260];
    do {
      uint8_t c = trackBuffer_GCR[readPos];
      switch (currentMode) {
      case 0:                           // search for header sync
        if (c == 0xFF)
          syncCnt++;
        else {
          if (syncCnt >= 2) {
            errorCode = 0x01;       // no error
            currentMode = 1;
            gcrBytesToDecode = 10;
            gcrByteCnt = 0;
            readPos = (readPos != 0 ? readPos : nBytes) - 1;
          }
          syncCnt = 0;
        }
        break;
      case 1:                           // read header
        if (gcrByteCnt < gcrBytesToDecode) {
          tmpBuf1[gcrByteCnt++] = c;
        }
        else {
          int     j = 0;
          for (int i = 0; i < gcrBytesToDecode; i += 5) {
            if (!gcrDecodeFourBytes(&(tmpBuf2[j]), &(tmpBuf1[i])))
              errorCode = 0x10;     // "GCR decoding error"
            j += 4;
          }
          uint8_t crcValue = 0;
          for (int i = 1; i < 6; i++)
            crcValue = crcValue ^ tmpBuf2[i];
          if (errorCode == 0x01) {
            if (tmpBuf2[0] != 0x08)
              errorCode = 0x02;     // "header block not found"
            else if (crcValue != 0x00)
              errorCode = 0x09;     // "CRC error in header"
          }
          if (errorCode != 0x01 ||
              int(tmpBuf2[3]) != trackNum || int(tmpBuf2[2]) >= nSectors) {
            currentMode = 0;
          }
          else {
            currentSector = int(tmpBuf2[2]);
            currentMode = 2;
            idCharacter2 = tmpBuf2[4];
            idCharacter1 = tmpBuf2[5];
          }
        }
        break;
      case 2:                           // search for sector data sync
        if (c == 0xFF)
          syncCnt++;
        else {
          if (syncCnt >= 2) {
            currentMode = 3;
            gcrBytesToDecode = 325;
            gcrByteCnt = 0;
            readPos = (readPos != 0 ? readPos : nBytes) - 1;
          }
          syncCnt = 0;
        }
        break;
      case 3:                           // read sector data
        if (gcrByteCnt < gcrBytesToDecode) {
          tmpBuf1[gcrByteCnt++] = c;
        }
        else {
          int     j = 0;
          for (int i = 0; i < gcrBytesToDecode; i += 5) {
            if (!gcrDecodeFourBytes(&(tmpBuf2[j]), &(tmpBuf1[i])))
              errorCode = 0x10;     // "GCR decoding error"
            j += 4;
          }
          uint8_t crcValue = 0;
          for (int i = 1; i < 258; i++)
            crcValue = crcValue ^ tmpBuf2[i];
          if (errorCode == 0x01) {
            if (tmpBuf2[0] != 0x07)
              errorCode = 0x04;     // "data block not found"
            else if (crcValue != 0x00)
              errorCode = 0x05;     // "CRC error in data block"
          }
          if (errorCode == 0x01 || errorCode == 0x05) {
            j = currentSector * 256;
            for (int i = 0; i < 256; i++)
              trackBuffer_D64[j + i] = tmpBuf2[i + 1];
            sectorsDecoded++;
          }
          badSectorTable[currentSector] = errorCode;
          currentSector = 0;
          currentMode = 0;
        }
        break;
      }
      if (++readPos >= nBytes)
        readPos = 0;
    } while (readPos != firstSyncPos);
    // return the number of sectors successfully decoded
    return sectorsDecoded;
  }

  bool D64Image::readTrack(int trackNum)
  {
    bool    retval = true;
    if (trackNum < 0)
      trackNum = currentTrack;
    for (int i = 0; i < trackSizeTable[trackNum]; i++)
      trackBuffer_GCR[i] = 0x00;
    for (int i = 0; i < 24; i++)
      badSectorTable[i] = 0x00;
    if (trackNum >= 1 && trackNum <= nTracks) {
      retval = false;
      int     nSectors = sectorsPerTrackTable[trackNum];
      if (haveBadSectorTable) {
        // read bad sector table (FIXME: errors are ignored)
        if (std::fseek(imageFile, long((d64TrackOffsetTable[trackNum] >> 8)
                                       + d64TrackOffsetTable[nTracks + 1]),
                       SEEK_SET) >= 0) {
          std::fread(&(badSectorTable[0]),
                     sizeof(uint8_t), size_t(nSectors), imageFile);
        }
      }
      if (std::fseek(imageFile, long(d64TrackOffsetTable[trackNum]),
                     SEEK_SET) >= 0) {
        if (std::fread(&(trackBuffer_D64[0]), sizeof(uint8_t),
                       (size_t(nSectors) * 256), imageFile)
            == (size_t(nSectors) * 256)) {
          gcrEncodeTrack(trackNum, nSectors, trackSizeTable[trackNum]);
          retval = true;
        }
      }
    }
    // return true on success
    return retval;
  }

  bool D64Image::flushTrack(int trackNum)
  {
    bool    retval = true;
    if (trackNum < 0)
      trackNum = currentTrack;
    if (trackDirtyFlag && !writeProtectFlag &&
        (trackNum >= 1 && trackNum <= nTracks)) {
      int     nSectors = sectorsPerTrackTable[trackNum];
      if (gcrDecodeTrack(trackNum, nSectors, trackSizeTable[trackNum]) > 0) {
        retval = false;
        if (std::fseek(imageFile, long(d64TrackOffsetTable[trackNum]),
                       SEEK_SET) >= 0) {
          if (std::fwrite(&(trackBuffer_D64[0]), sizeof(uint8_t),
                          (size_t(nSectors) * 256), imageFile)
              == (size_t(nSectors) * 256)) {
            if (std::fflush(imageFile) == 0)
              retval = true;
          }
        }
      }
      if (haveBadSectorTable) {
        retval = false;
        // update bad sector table
        if (std::fseek(imageFile, long((d64TrackOffsetTable[trackNum] >> 8)
                                       + d64TrackOffsetTable[nTracks + 1]),
                       SEEK_SET) >= 0) {
          if (std::fwrite(&(badSectorTable[0]),
                          sizeof(uint8_t), size_t(nSectors), imageFile)
              == size_t(nSectors)) {
            if (std::fflush(imageFile) == 0)
              retval = true;
          }
        }
      }
    }
    trackDirtyFlag = false;
    // return true on success
    return retval;
  }

  bool D64Image::setCurrentTrack(int trackNum)
  {
    bool    retval = true;
    trackNum = (trackNum >= 1 ? (trackNum <= 42 ? trackNum : 42) : 1);
    if (trackNum != currentTrack) {
      // write old track to disk if it has been changed
      if (!flushTrack(currentTrack))
        retval = false;
      // read new track from disk
      currentTrack = trackNum;
      if (!readTrack(currentTrack))
        retval = false;
    }
    return retval;
  }

  // --------------------------------------------------------------------------

  D64Image::D64Image()
    : trackDirtyFlag(false),
      currentTrack(42),
      nTracks(0),
      imageFile((std::FILE *) 0),
      writeProtectFlag(false),
      diskID(0x00),
      idCharacter1(0x41),
      idCharacter2(0x41),
      haveBadSectorTable(false)
  {
    // clear track buffers
    for (int i = 0; i < 8192; i++)
      trackBuffer_GCR[i] = 0x00;
    for (int i = 0; i < 5376; i++)
      trackBuffer_D64[i] = 0x00;
    for (int i = 0; i < 24; i++)
      badSectorTable[i] = 0x00;
  }

  D64Image::~D64Image()
  {
    if (imageFile) {
      (void) flushTrack();
      std::fclose(imageFile);
      imageFile = (std::FILE *) 0;
      nTracks = 0;
    }
  }

  void D64Image::setImageFile(std::FILE *imageFile_, bool isReadOnly)
  {
    if (imageFile) {
      (void) flushTrack();              // FIXME: should report errors ?
      std::fclose(imageFile);
      imageFile = (std::FILE *) 0;
      nTracks = 0;
    }
    writeProtectFlag = false;
    haveBadSectorTable = false;
    (void) setCurrentTrack(18);         // FIXME: should report errors ?
    if (imageFile_) {
      if (std::fseek(imageFile_, 0L, SEEK_END) < 0)
        throw Plus4Emu::Exception("error seeking to end of disk image file");
      long    fSize = std::ftell(imageFile_);
      long    nSectors = fSize / 256L;
      if ((nSectors * 256L) != fSize) {
        nSectors = fSize / 257L;
        if ((nSectors * 257L) != fSize)
          nSectors = 0L;
      }
      nSectors -= 683L;
      // allow any number of tracks from 35 to 42
      if (nSectors < 0L || nSectors > 119L ||
          ((nSectors / 17L) * 17L) != nSectors) {
        throw Plus4Emu::Exception("D64 image file has invalid length");
      }
      imageFile = imageFile_;
      writeProtectFlag = isReadOnly;
      std::fseek(imageFile, 0L, SEEK_SET);
      nTracks = 35L + (nSectors / 17L);
      haveBadSectorTable = (((nSectors + 683L) * 256L) < fSize);
      diskID = (diskID + 1) & 0xFF;
      if (((diskID >> 4) + 0x41) == idCharacter1 &&
          ((diskID & 0x0F) + 0x41) == idCharacter2)
        diskID = (diskID + 1) & 0xFF;   // make sure that the disk ID changes
      idCharacter1 = (diskID >> 4) + 0x41;
      idCharacter2 = (diskID & 0x0F) + 0x41;
      currentTrack = 42;
      (void) setCurrentTrack(18);       // FIXME: should report errors ?
    }
  }

}       // namespace Plus4

