
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
#include "tape.hpp"
#include "system.hpp"
#include "fileio.hpp"
#include <cmath>
#include <sndfile.h>
#include "fileno_wrapper.h"

static const char *c16TAPFileMagic = "C16-TAPE-RAW";

static int cuePointCmpFunc(const void *p1, const void *p2)
{
  if (*((uint32_t *) p1) < *((uint32_t *) p2))
    return -1;
  if (*((uint32_t *) p1) > *((uint32_t *) p2))
    return 1;
  return 0;
}

namespace Plus4Emu {

  Tape::Tape(int bitsPerSample)
    : sampleRate(24000L),
      fileBitsPerSample(1),
      requestedBitsPerSample(bitsPerSample),
      isReadOnly(true),
      isPlaybackOn(false),
      isRecordOn(false),
      isMotorOn(false),
      tapeLength(0),
      tapePosition(0),
      inputState(0),
      outputState(0)
  {
    if (!(requestedBitsPerSample == 1 || requestedBitsPerSample == 2 ||
          requestedBitsPerSample == 4 || requestedBitsPerSample == 8))
      throw Exception("invalid tape sample size");
  }

  Tape::~Tape()
  {
  }

  void Tape::runOneSample_()
  {
  }

  void Tape::setIsMotorOn(bool newState)
  {
    isMotorOn = newState;
  }

  void Tape::stop()
  {
    isPlaybackOn = false;
    isRecordOn = false;
  }

  void Tape::seek(double t)
  {
    (void) t;
  }

  void Tape::seekToCuePoint(bool isForward, double t)
  {
    (void) isForward;
    (void) t;
  }

  void Tape::addCuePoint()
  {
  }

  void Tape::deleteNearestCuePoint()
  {
  }

  void Tape::deleteAllCuePoints()
  {
  }

  // --------------------------------------------------------------------------

  bool Tape_Plus4Emu::findCuePoint_(size_t& ndx_, size_t pos_)
  {
    uint32_t  *cuePointTable = &(fileHeader[4]);
    uint32_t  pos = (pos_ < 0xFFFFFFFEUL ?
                     uint32_t(pos_) : uint32_t(0xFFFFFFFEUL));
    size_t    min_ = 0;
    size_t    max_ = cuePointCnt;
    while ((min_ + 1) < max_) {
      size_t  tmp = (min_ + max_) >> 1;
      if (pos > cuePointTable[tmp])
        min_ = tmp;
      else if (pos < cuePointTable[tmp])
        max_ = tmp;
      else {
        ndx_ = tmp;
        return true;
      }
    }
    ndx_ = min_;
    return (cuePointTable[min_] == pos);
  }

  void Tape_Plus4Emu::packSamples_()
  {
    uint8_t maxValue = uint8_t((1 << requestedBitsPerSample) - 1);
    uint8_t *buf_ = buf;
    if (fileBitsPerSample == requestedBitsPerSample) {
      for (size_t i = 0; i < 4096; i++) {
        uint8_t c = buf_[i];
        buf_[i] = (c < maxValue ? c : maxValue);
      }
    }
    else if (fileBitsPerSample < requestedBitsPerSample) {
      uint8_t rShift = uint8_t(requestedBitsPerSample - fileBitsPerSample);
      for (size_t i = 0; i < 4096; i++) {
        uint8_t c = buf_[i];
        buf_[i] = (c < maxValue ? c : maxValue) >> rShift;
      }
    }
    else {
      uint8_t lShift = uint8_t(fileBitsPerSample - requestedBitsPerSample);
      for (size_t i = 0; i < 4096; i++) {
        uint8_t c = buf_[i];
        buf_[i] = (c < maxValue ? c : maxValue) << lShift;
      }
    }
    if (fileBitsPerSample != 8) {
      int     nBits = fileBitsPerSample;
      int     byteBuf = 0;
      int     bitCnt = 8;
      size_t  writePos = 0;
      for (size_t i = 0; i < 4096; i++) {
        byteBuf = (byteBuf << nBits) | int(buf_[i]);
        bitCnt = bitCnt - nBits;
        if (!bitCnt) {
          buf_[writePos++] = uint8_t(byteBuf);
          byteBuf = 0;
          bitCnt = 8;
        }
      }
    }
  }

  bool Tape_Plus4Emu::writeBuffer_()
  {
    // calculate file position
    size_t  blockSize = 512U * (unsigned int) fileBitsPerSample;
    size_t  filePos = (tapePosition >> 12) * blockSize;
    if (usingNewFormat)
      filePos = filePos + 4096;
    if (std::fseek(f, long(filePos), SEEK_SET) < 0)
      return false;
    // write data
    long    n = long(std::fwrite(buf, sizeof(uint8_t), blockSize, f));
    n = (n >= 0L ? n : 0L);
    size_t  endPos = filePos + size_t(n);
    if (usingNewFormat)
      endPos = endPos - 4096;
    endPos = endPos * (8U / (unsigned int) fileBitsPerSample);
    if (endPos > tapeLength)
      tapeLength = endPos;
    bool    err = (n != long(blockSize));
    if (std::fflush(f) != 0)
      err = true;
    return (!err);
  }

  bool Tape_Plus4Emu::readBuffer_()
  {
    bool    err = false;
    long    n = 0L;
    size_t  blockSize = 512U * (unsigned int) fileBitsPerSample;
    if ((tapePosition & 0xFFFFF000UL) < tapeLength) {
      // calculate file position
      size_t  filePos = (tapePosition >> 12) * blockSize;
      if (usingNewFormat)
        filePos = filePos + 4096;
      if (std::fseek(f, long(filePos), SEEK_SET) >= 0) {
        // read data
        n = long(std::fread(buf, sizeof(uint8_t), blockSize, f));
        n = (n >= 0L ? n : 0L);
      }
    }
    if (n < long(blockSize)) {
      err = true;
      // pad with zero bytes
      do {
        buf[n] = 0;
      } while (++n < long(blockSize));
    }
    return (!err);
  }

  void Tape_Plus4Emu::unpackSamples_()
  {
    uint8_t *buf_ = buf;
    if (fileBitsPerSample != 8) {
      int     nBits = fileBitsPerSample;
      int     byteBuf = 0;
      int     bitCnt = 0;
      int     bitMask = (1 << fileBitsPerSample) - 1;
      size_t  readPos = 512U * (unsigned int) fileBitsPerSample;
      for (size_t i = 4096; i != 0; ) {
        if (!bitCnt) {
          byteBuf = int(buf_[--readPos]);
          bitCnt = 8;
        }
        buf_[--i] = uint8_t(byteBuf & bitMask);
        byteBuf = byteBuf >> nBits;
        bitCnt = bitCnt - nBits;
      }
    }
    if (fileBitsPerSample < requestedBitsPerSample) {
      uint8_t lShift = uint8_t(requestedBitsPerSample - fileBitsPerSample);
      for (size_t i = 0; i < 4096; i++)
        buf_[i] = buf_[i] << lShift;
    }
    else if (fileBitsPerSample > requestedBitsPerSample) {
      uint8_t rShift = uint8_t(fileBitsPerSample - requestedBitsPerSample);
      for (size_t i = 0; i < 4096; i++)
        buf_[i] = buf_[i] >> rShift;
    }
  }

  void Tape_Plus4Emu::seek_(size_t pos_)
  {
    // clamp position to tape length
    size_t  pos = (pos_ < tapeLength ? pos_ : tapeLength);
    size_t  oldBlockNum = (tapePosition >> 12);
    size_t  newBlockNum = (pos >> 12);

    if (newBlockNum == oldBlockNum) {
      tapePosition = pos;
      return;
    }
    bool    err = false;
    try {
      // flush any pending file changes
      flushBuffer_();
    }
    catch (...) {
      err = true;
    }
    tapePosition = pos;
    readBuffer_();
    unpackSamples_();
    if (err)
      throw Exception("error writing tape file - is the disk full ?");
  }

  bool Tape_Plus4Emu::writeHeader_()
  {
    if (!usingNewFormat)
      return true;
    if (std::fseek(f, 0L, SEEK_SET) < 0)
      return false;
    bool      err = false;
    uint32_t  tmp = 0U;
    for (size_t i = 0; i < 4096; i++) {
      if (!(i & 3)) {
        tmp = fileHeader[i >> 2];
        tmp = ((tmp & 0xFF000000U) >> 24) | ((tmp & 0x00FF0000U) >> 8)
              | ((tmp & 0x0000FF00U) << 8) | ((tmp & 0x000000FFU) << 24);
      }
      if (std::fputc(int(tmp & 0xFFU), f) == EOF) {
        err = true;
        break;
      }
      tmp = tmp >> 8;
    }
    if (std::fflush(f) != 0)
      err = true;
    return (!err);
  }

  void Tape_Plus4Emu::flushBuffer_()
  {
    if (isBufferDirty) {
      packSamples_();
      bool    err = !(writeBuffer_());
      unpackSamples_();
      isBufferDirty = false;
      if (err)
        throw Exception("error writing tape file - is the disk full ?");
    }
  }

  void Tape_Plus4Emu::setImageFile(std::FILE *imageFile_, const char *fileName,
                                   int mode, long sampleRate_,
                                   int bitsPerSample)
  {
    (void) bitsPerSample;
    f = (std::FILE *) 0;
    buf = (uint8_t *) 0;
    fileHeader = (uint32_t *) 0;
    cuePointCnt = 0;
    isBufferDirty = false;
    usingNewFormat = false;
    isReadOnly = false;
    try {
      if (!imageFile_ && (!fileName || fileName[0] == '\0'))
        throw Exception("invalid tape file name");
      if (sampleRate_ < 10000L || sampleRate_ > 120000L)
        throw Exception("invalid tape sample rate");
      if (!(mode >= 0 && mode <= 3))
        throw Exception("invalid tape open mode parameter");
      buf = new uint8_t[4096];
      for (size_t i = 0; i < 4096; i++)
        buf[i] = 0;
      fileHeader = new uint32_t[1024];
      if (imageFile_) {
        f = imageFile_;
      }
      else {
        f = fileOpen(fileName, (mode < 2 ? "r+b" : (mode == 2 ? "rb" : "w+b")));
        if (!f && mode < 2) {
          f = fileOpen(fileName, "rb");
          if (f) {
            mode = 2;
          }
          else if (!mode) {
            f = fileOpen(fileName, "w+b");
            mode = 3;
          }
        }
      }
      isReadOnly = (mode == 2);
      if (mode == 3 && f) {
        // create new tape file
        usingNewFormat = true;
        sampleRate = sampleRate_;
        fileBitsPerSample = requestedBitsPerSample;
        fileHeader[0] = 0x0275CD72U;
        fileHeader[1] = 0x1C445126U;
        fileHeader[2] = uint32_t(fileBitsPerSample);
        fileHeader[3] = uint32_t(sampleRate);
        for (size_t i = 4; i < 1024; i++)
          fileHeader[i] = 0xFFFFFFFFU;
        if (!writeHeader_()) {
          if (!imageFile_) {
            std::fclose(f);
            fileRemove(fileName);
            f = (std::FILE *) 0;
          }
        }
      }
      if (!f)
        throw Exception("error opening tape file");
      if (!usingNewFormat) {            // if opening an already existing file:
        if (std::fseek(f, 0L, SEEK_END) < 0)
          throw Exception("error setting tape file position");
        long  fSize = std::ftell(f);
        if (fSize < 0L)
          throw Exception("cannot find out length of tape file");
        std::fseek(f, 0L, SEEK_SET);
        // check file header
        if (fSize >= 4096L) {
          std::fread(buf, 1, 4096, f);
          for (size_t i = 0; i < 4096; i += 4) {
            uint32_t  tmp;
            tmp =   (uint32_t(buf[i + 0]) << 24) | (uint32_t(buf[i + 1]) << 16)
                  | (uint32_t(buf[i + 2]) << 8)  |  uint32_t(buf[i + 3]);
            fileHeader[i >> 2] = tmp;
          }
          if (fileHeader[0] == 0x0275CD72U && fileHeader[1] == 0x1C445126U &&
              (fileHeader[2] == 1U || fileHeader[2] == 2U ||
               fileHeader[2] == 4U || fileHeader[2] == 8U) &&
              (fileHeader[3] >= 10000U && fileHeader[3] <= 120000U) &&
              fileHeader[1023] == 0xFFFFFFFFU) {
            usingNewFormat = true;
            sampleRate = long(fileHeader[3]);
            fileBitsPerSample = int(fileHeader[2]);
            std::qsort(&(fileHeader[4]), 1020, sizeof(uint32_t),
                       &cuePointCmpFunc);
            while (fileHeader[cuePointCnt + 4] != uint32_t(0xFFFFFFFFUL))
              cuePointCnt++;
          }
        }
        tapeLength = size_t(fSize);
        if (usingNewFormat)
          tapeLength = tapeLength - 4096;
        tapeLength = (tapeLength << 3) / (unsigned int) fileBitsPerSample;
        if (!usingNewFormat) {
          fileHeader[0] = 0x0275CD72U;
          fileHeader[1] = 0x1C445126U;
          fileHeader[2] = uint32_t(fileBitsPerSample);
          fileHeader[3] = uint32_t(sampleRate);
          for (size_t i = 4; i < 1024; i++)
            fileHeader[i] = 0xFFFFFFFFU;
        }
        // fill buffer
        readBuffer_();
        unpackSamples_();
      }
    }
    catch (...) {
      if (f && !imageFile_)
        std::fclose(f);
      if (fileHeader)
        delete[] fileHeader;
      if (buf)
        delete[] buf;
      throw;
    }
  }

  Tape_Plus4Emu::Tape_Plus4Emu(const char *fileName, int mode,
                               long sampleRate_, int bitsPerSample)
    : Tape(bitsPerSample)
  {
    setImageFile((std::FILE *) 0, fileName, mode, sampleRate_, bitsPerSample);
  }

  Tape_Plus4Emu::Tape_Plus4Emu(std::FILE *imageFile_, int mode,
                               long sampleRate_, int bitsPerSample)
    : Tape(bitsPerSample)
  {
    setImageFile(imageFile_, (char *) 0, mode, sampleRate_, bitsPerSample);
  }

  Tape_Plus4Emu::~Tape_Plus4Emu()
  {
    // flush any pending file changes, and close file
    // FIXME: errors are not handled here
    try {
      flushBuffer_();
    }
    catch (...) {
    }
    std::fclose(f);
    delete[] fileHeader;
    delete[] buf;
  }

  void Tape_Plus4Emu::runOneSample_()
  {
    outputState = buf[tapePosition & 0x0FFF];
    if (isRecordOn) {
      buf[tapePosition & 0x0FFF] =
          uint8_t(inputState > 0 ? (inputState < 255 ? inputState : 255) : 0);
      isBufferDirty = true;
    }
    size_t  pos = tapePosition + 1;
    // unless recording, clamp position to tape length
    pos = ((pos < tapeLength || isRecordOn) ? pos : tapeLength);
    size_t  oldBlockNum = (tapePosition >> 12);
    size_t  newBlockNum = (pos >> 12);

    if (isRecordOn && cuePointCnt > 0) {
      size_t  ndx = 0;
      if (findCuePoint_(ndx, pos)) {
        // if recording over a cue point, delete it
        size_t  savedPos = tapePosition;
        tapePosition = pos;
        deleteNearestCuePoint();
        tapePosition = savedPos;
      }
    }
    if (newBlockNum == oldBlockNum) {
      tapePosition = pos;
      return;
    }
    bool    err = false;
    try {
      // flush any pending file changes
      flushBuffer_();
    }
    catch (...) {
      err = true;
    }
    tapePosition = pos;
    readBuffer_();
    unpackSamples_();
    if (err)
      throw Exception("error writing tape file - is the disk full ?");
  }

  void Tape_Plus4Emu::setIsMotorOn(bool newState)
  {
    isMotorOn = newState;
    if (!isMotorOn)
      flushBuffer_();
  }

  void Tape_Plus4Emu::stop()
  {
    isPlaybackOn = false;
    isRecordOn = false;
    flushBuffer_();
  }

  void Tape_Plus4Emu::seek(double t)
  {
    this->seek_(size_t(long(t > 0.0 ? (t * double(sampleRate) + 0.5) : 0.0)));
  }

  void Tape_Plus4Emu::seekToCuePoint(bool isForward, double t)
  {
    if (cuePointCnt > 0) {
      size_t    ndx = 0;
      findCuePoint_(ndx, tapePosition);
      uint32_t  *cuePointTable = &(fileHeader[4]);
      if ((cuePointTable[ndx] < tapePosition && !isForward) ||
          (cuePointTable[ndx] > tapePosition && isForward)) {
        this->seek_(cuePointTable[ndx]);
        return;
      }
      else if (ndx > 0 && !isForward) {
        this->seek_(cuePointTable[ndx - 1]);
        return;
      }
      else if ((ndx + 1) < cuePointCnt && isForward) {
        this->seek_(cuePointTable[ndx + 1]);
        return;
      }
    }
    if (isForward)
      this->seek(getPosition() + (t > 0.0 ? t : 0.0));
    else
      this->seek(getPosition() - (t < 0.0 ? t : 0.0));
  }

  void Tape_Plus4Emu::addCuePoint()
  {
    if (isReadOnly || cuePointCnt >= 1019 || !usingNewFormat)
      return;
    uint32_t  *cuePointTable = &(fileHeader[4]);
    uint32_t  pos = (tapePosition < 0xFFFFFFFEUL ?
                     uint32_t(tapePosition) : uint32_t(0xFFFFFFFEUL));
    size_t    ndx = 0;
    if (findCuePoint_(ndx, pos))
      return;           // there is already a cue point at this position
    cuePointTable[cuePointCnt++] = pos;
    // sort table
    for (size_t i = cuePointCnt - 1; i != 0; ) {
      uint32_t  tmp = cuePointTable[--i];
      if (tmp < cuePointTable[i + 1])
        break;
      cuePointTable[i] = cuePointTable[i + 1];
      cuePointTable[i + 1] = tmp;
    }
    if (!writeHeader_())
      throw Exception("error updating cue point table");
  }

  void Tape_Plus4Emu::deleteNearestCuePoint()
  {
    if (isReadOnly || cuePointCnt < 1)
      return;
    uint32_t  *cuePointTable = &(fileHeader[4]);
    uint32_t  pos = (tapePosition < 0xFFFFFFFEUL ?
                     uint32_t(tapePosition) : uint32_t(0xFFFFFFFEUL));
    size_t    ndx = 0;
    uint32_t  nearestDiff = uint32_t(0xFFFFFFFFUL);
    for (size_t i = 0; i < cuePointCnt; i++) {
      uint32_t  tmp = (cuePointTable[i] >= pos ?
                       (cuePointTable[i] - pos) : (pos - cuePointTable[i]));
      if (tmp >= nearestDiff)
        break;
      nearestDiff = tmp;
      ndx = i;
    }
    for (size_t i = (ndx + 1); i < cuePointCnt; i++)
      cuePointTable[i - 1] = cuePointTable[i];
    cuePointTable[cuePointCnt--] = uint32_t(0xFFFFFFFFUL);
    if (!writeHeader_())
      throw Exception("error updating cue point table");
  }

  void Tape_Plus4Emu::deleteAllCuePoints()
  {
    if (isReadOnly || cuePointCnt < 1)
      return;
    for (size_t i = 4; i < 1024; i++)
      fileHeader[i] = uint32_t(0xFFFFFFFFUL);
    cuePointCnt = 0;
    if (!writeHeader_())
      throw Exception("error updating cue point table");
  }

  // --------------------------------------------------------------------------

  Tape_C16::Tape_C16(std::FILE *imageFile_, int bitsPerSample)
    : Tape(bitsPerSample),
      f(imageFile_),
      endOfTape(false),
      usingOldTapFormat(false),
      inputSignal(0),
      inputCnt(0),
      savedInputCnt(0)
  {
    memset(seekIndex,0,sizeof(seekIndex));
    memset(posIndex,0,sizeof(posIndex));
    bool    isC16TAPFile = false;
    if (std::fseek(f, 0L, SEEK_END) >= 0) {
      if (std::ftell(f) >= 20L) {
        std::fseek(f, 0L, SEEK_SET);
        for (int i = 0; i <= 12; i++) {
          if (i == 12) {
            int   c = std::fgetc(f);
            isC16TAPFile = true;
            if (c == 1)
              usingOldTapFormat = true;
            else if (c == 2)
              usingOldTapFormat = false;
            else
              isC16TAPFile = false;
            break;
          }
          if (std::fgetc(f) != int(c16TAPFileMagic[i]))
            break;
        }
      }
    }
    if (!isC16TAPFile)
      throw Exception("invalid tape file header");
    sampleRate = 55420L;

   if (std::fseek(f, 20L, SEEK_SET) < 0)
      throw Exception("invalid tape file header");

    // Calculate index
    int indexSeconds = 0;
    while (!endOfTape) {
      long pos = std::ftell(f);
      if (indexSeconds >= 2700) {
         break;
      }
      seekIndex[indexSeconds] = pos;
      posIndex[indexSeconds] = tapePosition;
      for (int q=0;q<sampleRate;q++) {
         tapeLength = tapePosition;
         runOneSample_();
      }
      indexSeconds++;
    }
    this->seek(0.0);
  }

  Tape_C16::~Tape_C16()
  {
    if (f)
      std::fclose(f);
  }

  void Tape_C16::runOneSample_()
  {
    if (endOfTape) {
      tapePosition = tapeLength;
      outputState = 0;
      return;
    }
    // downsample by a factor of 16
    int   outputSignal = 0;
    for (int i = 0; i < 16; i++) {
      if (!inputCnt) {
        inputSignal = -inputSignal;
        if (!(usingOldTapFormat && inputSignal > 0)) {
          int   c = std::fgetc(f);
          if (c == EOF) {
            endOfTape = true;
            break;
          }
          if (!c) {
            savedInputCnt = 0;
            c = std::fgetc(f);
            if (c == EOF) {
              endOfTape = true;
              break;
            }
            savedInputCnt = savedInputCnt | (c & 0xFF);
            c = std::fgetc(f);
            if (c == EOF) {
              endOfTape = true;
              break;
            }
            savedInputCnt = savedInputCnt | ((c & 0xFF) << 8);
            c = std::fgetc(f);
            if (c == EOF) {
              endOfTape = true;
              break;
            }
            savedInputCnt = savedInputCnt | ((c & 0xFF) << 16);
          }
          else
            savedInputCnt = (c & 0xFF) << 3;
          if (usingOldTapFormat)
            savedInputCnt = savedInputCnt >> 1;
        }
        inputCnt = savedInputCnt;
      }
      outputSignal += inputSignal;
      if (inputCnt)
        inputCnt--;
    }
    tapePosition++;
    //tapeLength = tapePosition + 1;
    outputState = (outputSignal > 0 ? (1 << (requestedBitsPerSample - 1)) : 0);
  }

  void Tape_C16::setIsMotorOn(bool newState)
  {
    isMotorOn = newState;
  }

  void Tape_C16::stop()
  {
    isPlaybackOn = false;
    isRecordOn = false;
  }

  void Tape_C16::seek(double t)
  {
    if (t <= 0.0) {
       t = 0.0;
    }
      //tapeLength = 1;
      tapePosition = posIndex[(int)t];
      outputState = 0;
      endOfTape = false;
      inputSignal = (usingOldTapFormat ? -1 : 1);
      inputCnt = 0;
      savedInputCnt = 0;
      if (std::fseek(f, seekIndex[(int)t], SEEK_SET) < 0)
        endOfTape = true;
  }

  void Tape_C16::seekToCuePoint(bool isForward, double t)
  {
    (void) t;
    if (!isForward)
      this->seek(0.0);
  }

  void Tape_C16::addCuePoint()
  {
  }

  void Tape_C16::deleteNearestCuePoint()
  {
  }

  void Tape_C16::deleteAllCuePoints()
  {
  }

  // --------------------------------------------------------------------------

  Tape_SoundFile::TapeFilter::TapeFilter(size_t irSamples)
  {
    if (irSamples < 16 || irSamples > 32768 ||
        (irSamples & (irSamples - 1)) != 0)
      throw std::exception();
    irFFT.resize(irSamples << 2);
    fftBuf.resize(irSamples << 2);
    outBuf.resize(irSamples);
    sampleCnt = 0;
    for (size_t i = 0; i < (irSamples << 2); i++)
      irFFT[i] = 0.0f;
    irFFT[irSamples >> 1] = 1.0f;
    this->fft(&(irFFT.front()), (irSamples << 1), false);
    for (size_t i = 0; i < (irSamples << 2); i++)
      fftBuf[i] = 0.0f;
    for (size_t i = 0; i < irSamples; i++)
      outBuf[i] = 0.0f;
  }

  Tape_SoundFile::TapeFilter::~TapeFilter()
  {
  }

  void Tape_SoundFile::TapeFilter::setFilterParameters(float sampleRate,
                                                       float minFreq,
                                                       float maxFreq)
  {
    size_t  n = outBuf.size();
    // calculate linear phase filter
    size_t  fhpp, fhps, flpp, flps;
    fhpp = size_t(long(float(long(n)) * (minFreq / sampleRate) + 0.5f));
    fhps = size_t(long(float(long(n)) * (0.25f * minFreq / sampleRate) + 0.5f));
    flpp = size_t(long(float(long(n)) * (maxFreq / sampleRate) + 0.5f));
    flps = size_t(long(float(long(n)) * (4.0f * maxFreq / sampleRate) + 0.5f));
    for (size_t i = 0; i <= (n >> 1); i++) {
      double  a = 1.0 / double(long(n));
      if (i <= fhps || i >= flps)
        a = 0.0;
      else {
        if (i > fhps && i < fhpp) {
          double  w = std::sin(2.0 * std::atan(1.0)
                               * double(long(i - fhps))
                               / double(long(fhpp - fhps)));
          a *= (w * w);
        }
        if (i > flpp && i < flps) {
          double  w = std::cos(2.0 * std::atan(1.0)
                               * double(long(i - flpp))
                               / double(long(flps - flpp)));
          a *= (w * w);
        }
      }
      irFFT[(i << 1) + 0] = float((i & 1) == 0 ? a : -a);
      irFFT[(i << 1) + 1] = 0.0f;
    }
    // apply von Hann window to impulse response
    this->fft(&(irFFT.front()), n, true);
    for (size_t i = 0; i < n; i++) {
      double  w = std::sin(4.0 * std::atan(1.0) * double(long(i))
                                                / double(long(n)));
      irFFT[i] *= float(w * w);
    }
    // pad to double length
    for (size_t i = n; i < (n << 1); i++)
      irFFT[i] = 0.0f;
    this->fft(&(irFFT.front()), (n << 1), false);
  }

  float Tape_SoundFile::TapeFilter::processSample(float inputSignal)
  {
    size_t  n = outBuf.size();
    if (sampleCnt >= n) {
      sampleCnt = 0;
      // copy remaining samples from previous buffer
      for (size_t i = 0; i < n; i++)
        outBuf[i] = fftBuf[i + n];
      // pad input to double length
      for (size_t i = n; i < (n << 1); i++)
        fftBuf[i] = 0.0f;
      // convolve
      this->fft(&(fftBuf.front()), (n << 1), false);
      for (size_t i = 0; i <= n; i++) {
        double  re1 = fftBuf[(i << 1) + 0];
        double  im1 = fftBuf[(i << 1) + 1];
        double  re2 = irFFT[(i << 1) + 0];
        double  im2 = irFFT[(i << 1) + 1];
        fftBuf[(i << 1) + 0] = re1 * re2 - im1 * im2;
        fftBuf[(i << 1) + 1] = re1 * im2 + im1 * re2;
      }
      this->fft(&(fftBuf.front()), (n << 1), true);
      // mix new buffer to output
      for (size_t i = 0; i < n; i++)
        outBuf[i] += fftBuf[i];
    }
    fftBuf[sampleCnt] = inputSignal;
    float   outputSignal = outBuf[sampleCnt] / float(long(n));
    sampleCnt++;
    return outputSignal;
  }

  void Tape_SoundFile::TapeFilter::fft(float *buf, size_t n, bool isInverse)
  {
    // check FFT size
    if (n < 16 || n > 65536 || (n & (n - 1)) != 0)
      throw std::exception();
    if (!isInverse) {
      // convert real data to interleaved real/imaginary format
      size_t  i = n;
      do {
        i--;
        buf[(i << 1) + 0] = buf[i];
        buf[(i << 1) + 1] = 0.0f;
      } while (i);
    }
    else {
      buf[1] = 0.0f;
      buf[n + 1] = 0.0f;
      for (size_t i = 1; i < (n >> 1); i++) {
        buf[((n - i) << 1) + 0] = buf[(i << 1) + 0];
        buf[((n - i) << 1) + 1] = -(buf[(i << 1) + 1]);
      }
    }
    // pack data in reverse bit order
    size_t  i, j;
    for (i = 0, j = 0; i < n; i++) {
      if (i < j) {
        float   tmp1 = buf[(i << 1) + 0];
        float   tmp2 = buf[(i << 1) + 1];
        buf[(i << 1) + 0] = buf[(j << 1) + 0];
        buf[(i << 1) + 1] = buf[(j << 1) + 1];
        buf[(j << 1) + 0] = tmp1;
        buf[(j << 1) + 1] = tmp2;
      }
      for (size_t k = (n >> 1); k > 0; k >>= 1) {
        j ^= k;
        if ((j & k) != 0)
          break;
      }
    }
    // calculate FFT
    for (size_t k = 1; k < n; k <<= 1) {
      double  ph_inc_re = std::cos(4.0 * std::atan(1.0) / double(long(k)));
      double  ph_inc_im = std::sin((isInverse ? 4.0 : -4.0)
                                   * std::atan(1.0) / double(long(k)));
      for (j = 0; j < n; j += (k << 1)) {
        double  ph_re = 1.0;
        double  ph_im = 0.0;
        for (i = j; i < (j + k); i++) {
          double  re1 = buf[(i << 1) + 0];
          double  im1 = buf[(i << 1) + 1];
          double  re2 = buf[((i + k) << 1) + 0];
          double  im2 = buf[((i + k) << 1) + 1];
          buf[(i << 1) + 0] = float(re1 + re2 * ph_re - im2 * ph_im);
          buf[(i << 1) + 1] = float(im1 + re2 * ph_im + im2 * ph_re);
          buf[((i + k) << 1) + 0] = float(re1 - re2 * ph_re + im2 * ph_im);
          buf[((i + k) << 1) + 1] = float(im1 - re2 * ph_im - im2 * ph_re);
          double  tmp = ph_re * ph_inc_re - ph_im * ph_inc_im;
          ph_im = ph_re * ph_inc_im + ph_im * ph_inc_re;
          ph_re = tmp;
        }
      }
    }
    if (!isInverse) {
      buf[1] = 0.0f;
      buf[n + 1] = 0.0f;
    }
    else {
      // convert from interleaved real/imaginary format to pure real data
      for (i = 0; i < n; i++)
        buf[i] = buf[(i << 1) + 0];
      for (i = n; i < (n << 1); i++)
        buf[i] = 0.0f;
    }
  }

  // --------------------------------------------------------------------------

  bool Tape_SoundFile::writeBuffer_()
  {
    sf_count_t  filePos = sf_count_t((tapePosition >> 10) << 10);
    if (sf_seek(sf, filePos, SEEK_SET) != filePos)
      return false;
    // write data
    if (invertSignal)
      invertBuffer_();
    long    n = long(sf_writef_short(sf, &(buf.front()), sf_count_t(1024)));
    if (invertSignal)
      invertBuffer_();
    n = (n >= 0L ? n : 0L);
    size_t  endPos = filePos + size_t(n);
    if (endPos > tapeLength)
      tapeLength = endPos;
    return (n == 1024L);
  }

  void Tape_SoundFile::seek_(size_t pos_)
  {
    // clamp position to tape length
    size_t  pos = (pos_ < tapeLength ? pos_ : tapeLength);
    size_t  oldBlockNum = (tapePosition >> 10);
    size_t  newBlockNum = (pos >> 10);

    if (newBlockNum == oldBlockNum) {
      tapePosition = pos;
      return;
    }
    bool    err = false;
    try {
      // flush any pending file changes
      flushBuffer_();
    }
    catch (...) {
      err = true;
    }
    tapePosition = pos;
    // FIXME: should check for errors ?
    (void) sf_seek(sf, sf_count_t(newBlockNum << 10), SEEK_SET);
    // fill buffer
    int   n = int(sf_readf_short(sf, &(buf.front()), sf_count_t(1024)));
    n = (n >= 0 ? n : 0) * nChannels;
    for ( ; n < int(buf.size()); n++)
      buf[n] = 0;
    if (invertSignal)
      invertBuffer_();
    if (err)
      throw Exception("error writing tape file - is the disk full ?");
  }

  void Tape_SoundFile::flushBuffer_()
  {
    if (isBufferDirty) {
      bool    err = !(writeBuffer_());
      isBufferDirty = false;
      if (err)
        throw Exception("error writing tape file - is the disk full ?");
    }
  }

  void Tape_SoundFile::invertBuffer_()
  {
    for (size_t i = 0; i < buf.size(); i++) {
      buf[i] = short(-1 - int(buf[i]));
    }
  }

  Tape_SoundFile::Tape_SoundFile(std::FILE *imageFile_,
                                 int mode, int bitsPerSample)
    : Tape(bitsPerSample),
      f(imageFile_),
      sf((SNDFILE *) 0),
      nChannels(1),
      requestedChannel(0),
      invertSignal(false),
      enableFIRFilter(false),
      isBufferDirty(false),
      firFilter(2048)
  {
    if (!(mode >= 0 && mode <= 2))
      throw Exception("invalid tape open mode parameter");
    isReadOnly = (mode == 2);
    SF_INFO sfinfo;
    std::memset(&sfinfo, 0, sizeof(SF_INFO));
    sf = sf_open_fd(
#ifdef WIN32
             _fileno(f),
#else
             plus4_fileno(f),
#endif
             (isReadOnly ? SFM_READ : SFM_RDWR), &sfinfo, SF_FALSE);
    if (!sf)
      throw Exception("error opening tape file");
    try {
      int32_t   nFrames = int32_t(sfinfo.frames);
      if (sf_count_t(nFrames) != sfinfo.frames)
        throw Exception("invalid tape file length");
      if (nFrames < 0 || nFrames >= 0x40000000)
        throw Exception("invalid tape file length");
      tapeLength = size_t(nFrames);
      if (sfinfo.samplerate < 10000 || sfinfo.samplerate > 120000)
        throw Exception("invalid tape file sample rate");
      sampleRate = sfinfo.samplerate;
      if (sfinfo.channels < 1 || sfinfo.channels > 16)
        throw Exception("invalid number of channels in tape file");
      nChannels = sfinfo.channels;
      if (!sfinfo.seekable)
        throw Exception("invalid tape file");
      switch (sfinfo.format & SF_FORMAT_SUBMASK) {
      case SF_FORMAT_IMA_ADPCM:
      case SF_FORMAT_MS_ADPCM:
        fileBitsPerSample = 4;
        break;
      case SF_FORMAT_PCM_S8:
      case SF_FORMAT_PCM_U8:
      case SF_FORMAT_ULAW:
      case SF_FORMAT_ALAW:
        fileBitsPerSample = 8;
        break;
      case SF_FORMAT_PCM_16:
        fileBitsPerSample = 16;
        break;
      case SF_FORMAT_PCM_24:
        fileBitsPerSample = 24;
        break;
      case SF_FORMAT_PCM_32:
      case SF_FORMAT_FLOAT:
        fileBitsPerSample = 32;
        break;
      case SF_FORMAT_DOUBLE:
        fileBitsPerSample = 64;
        break;
      default:
        throw Exception("invalid tape file format");
      }
      buf.resize(size_t(nChannels * 1024));
      // fill buffer
      int   n = int(sf_readf_short(sf, &(buf.front()), sf_count_t(1024)));
      n = (n >= 0 ? n : 0) * nChannels;
      for ( ; n < int(buf.size()); n++)
        buf[n] = 0;
    }
    catch (...) {
      (void) sf_close(sf);
      throw;
    }
  }

  Tape_SoundFile::~Tape_SoundFile()
  {
    // flush any pending file changes, and close file
    // FIXME: errors are not handled here
    try {
      flushBuffer_();
    }
    catch (...) {
    }
    (void) sf_close(sf);
    (void) std::fclose(f);
  }

  void Tape_SoundFile::runOneSample_()
  {
    int   bufPos = (int(tapePosition & 0x03FF) * nChannels) + requestedChannel;
    int   tmp = buf[bufPos];
    if (isRecordOn) {
      int   tmp2 = (inputState >= 0 ? inputState : 0);
      switch (requestedBitsPerSample) {
      case 1:
        tmp2 = (tmp2 > 0 ? 32767 : -32768);
        break;
      case 2:
        tmp2 = ((tmp2 <= 3 ? tmp2 : 3) << 14) - 24576;
        break;
      case 4:
        tmp2 = ((tmp2 <= 15 ? tmp2 : 15) << 12) - 30720;
        break;
      case 8:
        tmp2 = ((tmp2 <= 255 ? tmp2 : 255) << 8) - 32640;
        break;
      }
      buf[bufPos] = short(tmp2);
      isBufferDirty = true;
    }
    if (enableFIRFilter) {
      float tmp2 = firFilter.processSample(float(tmp));
      tmp = int(tmp2 + (tmp2 >= 0.0f ? 0.5f : -0.5f));
    }
    tmp = tmp + 32768;
    tmp = (tmp >= 0 ? (tmp <= 65535 ? tmp : 65535) : 0);
    outputState = tmp >> (16 - requestedBitsPerSample);

    size_t  pos = tapePosition + 1;
    // unless recording, clamp position to tape length
    pos = ((pos < tapeLength || isRecordOn) ? pos : tapeLength);
    size_t  oldBlockNum = (tapePosition >> 10);
    size_t  newBlockNum = (pos >> 10);

    if (newBlockNum == oldBlockNum) {
      tapePosition = pos;
      return;
    }
    bool    err = false;
    try {
      // flush any pending file changes
      flushBuffer_();
    }
    catch (...) {
      err = true;
    }
    tapePosition = pos;
    // FIXME: should check for errors ?
    (void) sf_seek(sf, sf_count_t(newBlockNum << 10), SEEK_SET);
    // fill buffer
    int   n = int(sf_readf_short(sf, &(buf.front()), sf_count_t(1024)));
    n = (n >= 0 ? n : 0) * nChannels;
    for ( ; n < int(buf.size()); n++)
      buf[n] = 0;
    if (invertSignal)
      invertBuffer_();
    if (err)
      throw Exception("error writing tape file - is the disk full ?");
  }

  void Tape_SoundFile::setIsMotorOn(bool newState)
  {
    isMotorOn = newState;
    if (!isMotorOn)
      flushBuffer_();
  }

  void Tape_SoundFile::stop()
  {
    isPlaybackOn = false;
    isRecordOn = false;
    flushBuffer_();
  }

  void Tape_SoundFile::seek(double t)
  {
    this->seek_(size_t(long(t > 0.0 ? (t * double(sampleRate) + 0.5) : 0.0)));
  }

  void Tape_SoundFile::seekToCuePoint(bool isForward, double t)
  {
    if (isForward)
      this->seek(getPosition() + (t > 0.0 ? t : 0.0));
    else
      this->seek(getPosition() - (t < 0.0 ? t : 0.0));
  }

  void Tape_SoundFile::addCuePoint()
  {
  }

  void Tape_SoundFile::deleteNearestCuePoint()
  {
  }

  void Tape_SoundFile::deleteAllCuePoints()
  {
  }

  void Tape_SoundFile::setParameters(int requestedChannel_,
                                     bool invertSignal_,
                                     bool enableFIRFilter_,
                                     float filterMinFreq_,
                                     float filterMaxFreq_)
  {
    requestedChannel =
        (requestedChannel_ >= 0 ?
         (requestedChannel_ < nChannels ? requestedChannel_ : (nChannels - 1))
         : 0);
    if (invertSignal_ != invertSignal) {
      // FIXME: errors are not handled here
      try {
        flushBuffer_();
      }
      catch (...) {
      }
      invertSignal = invertSignal_;
      invertBuffer_();
    }
    enableFIRFilter = enableFIRFilter_;
    if (enableFIRFilter) {
      firFilter.setFilterParameters(float(sampleRate),
                                    filterMinFreq_, filterMaxFreq_);
    }
  }

  // --------------------------------------------------------------------------

  Tape *openTapeFile(const char *fileName, int mode,
                     long sampleRate_, int bitsPerSample)
  {
    Tape    *t = (Tape *) 0;
    if (mode != 3) {
      int     fileType = 2;
      bool    isReadOnly = (mode == 2);
      std::FILE *f = openPlus4ImageFile(fileName, fileType, isReadOnly);
      if (f) {
        try {
          t = new Tape_C16(f, bitsPerSample);
        }
        catch (...) {
          try {
            t = new Tape_SoundFile(f, mode, bitsPerSample);
          }
          catch (...) {
            try {
              t = new Tape_Plus4Emu(f, mode, sampleRate_, bitsPerSample);
            }
            catch (...) {
              std::fclose(f);
              throw;
            }
          }
        }
        return t;
      }
    }
    t = new Tape_Plus4Emu(fileName, mode, sampleRate_, bitsPerSample);
    return t;
  }

}       // namespace Plus4Emu

