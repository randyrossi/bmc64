
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
#include "wd177x.hpp"
#include "system.hpp"
#include "fileio.hpp"

#include <vector>

namespace Plus4Emu {

  WD177x::WD177x()
    : imageFile((std::FILE *) 0),
      nTracks(0),
      nSides(0),
      nSectorsPerTrack(0),
      commandRegister(0),
      statusRegister(0),
      trackRegister(0),
      sectorRegister(0),
      dataRegister(0),
      currentTrack(0),
      currentSide(0),
      writeProtectFlag(false),
      diskChangeFlag(true),
      interruptRequestFlag(false),
      dataRequestFlag(false),
      isWD1773(false),
      steppingIn(false),
      busyFlagHackEnabled(false),
      busyFlagHack(false),
      bufPos(512)
  {
    buf.resize(512);
    this->reset();
    diskChangeFlag = true;
  }

  WD177x::~WD177x()
  {
    try {
      setDiskImageFile((std::FILE *) 0, 0, 0, 0);
    }
    catch (...) {
    }
  }

  bool WD177x::setFilePosition()
  {
    if (!imageFile)
      return false;
    if (currentTrack >= nTracks || currentTrack != trackRegister ||
        currentSide >= nSides ||
        sectorRegister < 1 || sectorRegister > nSectorsPerTrack)
      return false;
    size_t  filePos =
        ((size_t(currentTrack) * size_t(nSides) + size_t(currentSide))
         * size_t(nSectorsPerTrack) + size_t(sectorRegister - 1))
        * 512;
    if (std::fseek(imageFile, long(filePos), SEEK_SET) < 0)
      return false;
    return true;
  }

  void WD177x::doStep(bool updateFlag)
  {
    if (steppingIn) {
      currentTrack++;
      if (updateFlag)
        trackRegister++;
    }
    else {
      currentTrack--;
      if (updateFlag)
        trackRegister--;
      if (currentTrack == 0)
        trackRegister = 0;
    }
  }

  void WD177x::setDiskImageFile(std::FILE *imageFile_, bool isReadOnly,
                                int nTracks_, int nSides_,
                                int nSectorsPerTrack_)
  {
    if (imageFile_ == imageFile &&
        (!imageFile_ ||
         (nTracks_ == int(nTracks) && nSides_ == int(nSides) &&
          nSectorsPerTrack_ == int(nSectorsPerTrack)))) {
      return;
    }
    if (imageFile) {
      std::fclose(imageFile);
      imageFile = (std::FILE *) 0;
    }
    nTracks = 0;
    nSides = 0;
    nSectorsPerTrack = 0;
    writeProtectFlag = false;
    this->reset();
    diskChangeFlag = true;
    if (!imageFile_)
      return;
    unsigned char tmpBuf[512];
    bool    nTracksValid = (nTracks_ >= 1 && nTracks_ <= 240);
    bool    nSidesValid = (nSides_ >= 1 && nSides_ <= 2);
    bool    nSectorsPerTrackValid =
                (nSectorsPerTrack_ >= 1 && nSectorsPerTrack_ <= 240);
    try {
      imageFile = imageFile_;
      writeProtectFlag = isReadOnly;
      long    fileSize = -1L;
      if (std::fseek(imageFile, 0L, SEEK_END) >= 0)
        fileSize = std::ftell(imageFile);
      if (fileSize > (240L * 2L * 240L * 512L))
        fileSize = -1L;
      long    nSectors_ = fileSize / 512L;
      if (!nTracksValid) {
        if (nSectors_ > 0L && nSidesValid && nSectorsPerTrackValid) {
          nTracks_ = int(nSectors_ / (long(nSides_) * long(nSectorsPerTrack_)));
          nTracksValid = true;
        }
      }
      else if (!nSidesValid) {
        if (nSectors_ > 0L && nTracksValid && nSectorsPerTrackValid) {
          nSides_ = int(nSectors_ / (long(nTracks_) * long(nSectorsPerTrack_)));
          nSidesValid = true;
        }
      }
      else if (!nSectorsPerTrackValid) {
        if (nSectors_ > 0L && nTracksValid && nSidesValid) {
          nSectorsPerTrack_ = int(nSectors_ / (long(nTracks_) * long(nSides_)));
          nSectorsPerTrackValid = true;
        }
      }
      if (!(nTracksValid && nSidesValid && nSectorsPerTrackValid)) {
        // try to find out geometry parameters from FAT filesystem
        if (std::fseek(imageFile, 0L, SEEK_SET) >= 0) {
          if (std::fread(&(tmpBuf[0]), 1, 512, imageFile) == 512) {
            nSectors_ = long(tmpBuf[0x13]) | (long(tmpBuf[0x14]) << 8);
            if (!nSectors_) {
              nSectors_ =
                  long(tmpBuf[0x20]) | (long(tmpBuf[0x21]) << 8)
                  | (long(tmpBuf[0x22]) << 16) | (long(tmpBuf[0x23]) << 24);
            }
            if (!nSidesValid)
              nSides_ = int(tmpBuf[0x1A]) | (int(tmpBuf[0x1B]) << 8);
            if (!nSectorsPerTrackValid)
              nSectorsPerTrack_ = int(tmpBuf[0x18]) | (int(tmpBuf[0x19]) << 8);
            if (!nTracksValid) {
              if (nSides_ >= 1 && nSides_ <= 2 &&
                  nSectorsPerTrack_ >= 1 && nSectorsPerTrack_ <= 240 &&
                  nSectors_ >= 1L && nSectors_ <= (240L * 2L * 240L)) {
                nTracks_ =
                    int(nSectors_ / (long(nSides_) * long(nSectorsPerTrack_)));
              }
            }
          }
        }
      }
      if (!(nTracks_ >= 1 && nTracks_ <= 240 &&
            nSides_ >= 1 && nSides_ <= 2 &&
            nSectorsPerTrack_ >= 1 && nSectorsPerTrack_ <= 240))
        throw Exception("wd177x: cannot determine size of disk image");
      nSectors_ = long(nTracks_) * long(nSides_) * long(nSectorsPerTrack_);
      fileSize = nSectors_ * 512L;
      bool    err = true;
      if (std::fseek(imageFile, fileSize - 512L, SEEK_SET) >= 0) {
        if (std::fread(&(tmpBuf[0]), 1, 512, imageFile) == 512) {
          if (std::fread(&(tmpBuf[0]), 1, 512, imageFile) == 0) {
            err = false;
          }
        }
      }
      if (err)
        throw Exception("wd177x: invalid or inconsistent "
                        "disk image size parameters");
      std::fseek(imageFile, 0L, SEEK_SET);
    }
    catch (...) {
      imageFile = (std::FILE *) 0;
      writeProtectFlag = false;
      throw;
    }
    nTracks = uint8_t(nTracks_);
    nSides = uint8_t(nSides_);
    nSectorsPerTrack = uint8_t(nSectorsPerTrack_);
    this->reset();
    diskChangeFlag = true;
  }

  void WD177x::writeCommandRegister(uint8_t n)
  {
    if ((statusRegister & 0x01) != 0 && (n & 0xF0) != 0xD0) {
      // ignore command if wd177x is busy, and the command is not
      // force interrupt
      return;
    }
    if ((statusRegister & 0x01) == 0)   // store new command
      commandRegister = n;
    if ((n & 0x80) == 0) {              // ---- Type I commands ----
      uint8_t r = n & 3;                // step rate (ignored)
      bool    v = ((n & 0x04) != 0);    // verify flag
      bool    h = ((n & 0x08) != 0);    // disable spin-up (ignored)
      bool    u = ((n & 0x10) != 0);    // update flag
      (void) r;
      (void) h;
      // set busy flag; reset CRC, seek error, data request, and IRQ
      dataRequestFlag = false;
      statusRegister = 0x21;
      if (interruptRequestFlag) {
        interruptRequestFlag = false;
        clearInterruptRequest();
      }
      if ((n & 0xF0) == 0) {            // RESTORE
        trackRegister = currentTrack;
        dataRegister = 0;
        if (dataRegister < trackRegister) {
          steppingIn = false;
          do {
            doStep(true);
          } while (trackRegister != dataRegister);
        }
      }
      else if ((n & 0xF0) == 0x10) {    // SEEK
        if (dataRegister > trackRegister) {
          steppingIn = true;
          do {
            doStep(true);
          } while (trackRegister != dataRegister);
        }
        else {
          steppingIn = false;
          do {
            doStep(true);
          } while (trackRegister != dataRegister);
        }
      }
      else {                            // STEP
        if ((n & 0xE0) == 0x40)         // STEP IN
          steppingIn = true;
        else if ((n & 0xE0) == 0x60)    // STEP OUT
          steppingIn = false;
        doStep(u);
      }
      // command done: update flags and trigger interrupt
      if (writeProtectFlag)
        statusRegister = statusRegister | 0x40;
      if (v && (imageFile == (std::FILE *) 0 ||
                currentTrack >= nTracks || currentTrack != trackRegister))
        statusRegister = statusRegister | 0x10; // seek error
      if (currentTrack == 0)
        statusRegister = statusRegister | 0x04;
      if (imageFile)
        statusRegister = statusRegister | 0x02; // index pulse
      statusRegister = statusRegister & 0xFE;   // clear busy flag
      if (!interruptRequestFlag) {
        interruptRequestFlag = true;
        interruptRequest();
      }
    }
    else if ((n & 0xC0) == 0x80) {      // ---- Type II commands ----
      bool    a0 = ((n & 0x01) != 0);   // write deleted data mark (ignored)
      bool    pc = ((n & 0x02) != 0);   // disable write precompensation
                                        // / enable side compare
      bool    e = ((n & 0x04) != 0);    // settling delay (ignored)
      bool    hs = ((n & 0x08) != 0);   // disable spin-up / side select
      bool    m = ((n & 0x10) != 0);    // multiple sectors
      (void) a0;
      (void) e;
      (void) m;
      // set busy flag; reset data request, lost data, record not found,
      // and interrupt request
      dataRequestFlag = false;
      statusRegister = 0x01;
      if (interruptRequestFlag) {
        interruptRequestFlag = false;
        clearInterruptRequest();
      }
      // select side (if enabled)
      if (isWD1773) {
        if (pc)
          currentSide = uint8_t(hs ? 1 : 0);
      }
      bufPos = 512;
      if ((n & 0x20) == 0) {            // READ SECTOR
        if (!setFilePosition())
          statusRegister = statusRegister | 0x10;   // record not found
        else if (std::fread(&(buf[0]), 1, 512, imageFile) != 512)
          statusRegister = statusRegister | 0x08;   // CRC error
        else {
          dataRequestFlag = true;
          statusRegister = statusRegister | 0x02;
          bufPos = 0;
        }
      }
      else {                            // WRITE SECTOR
        if (writeProtectFlag)
          statusRegister = statusRegister | 0x40;   // disk is write protected
        else if (!setFilePosition())
          statusRegister = statusRegister | 0x10;   // record not found
        else {
          dataRequestFlag = true;
          statusRegister = statusRegister | 0x02;
          bufPos = 0;
        }
      }
      if (bufPos >= 512) {
        // on error: clear busy flag, and trigger interrupt
        statusRegister = statusRegister & 0xFE;
        if (!interruptRequestFlag) {
          interruptRequestFlag = true;
          interruptRequest();
        }
      }
    }
    else if ((n & 0xF0) != 0xD0) {      // ---- Type III commands ----
      bool    p = ((n & 0x02) != 0);    // disable write precompensation
      bool    e = ((n & 0x04) != 0);    // settling delay (ignored)
      bool    h = ((n & 0x08) != 0);    // disable spin-up (ignored)
      (void) p;
      (void) e;
      (void) h;
      // set busy flag; reset data request, lost data, record not found,
      // and interrupt request
      dataRequestFlag = false;
      statusRegister = 0x01;
      if (interruptRequestFlag) {
        interruptRequestFlag = false;
        clearInterruptRequest();
      }
      bufPos = 512;
      if ((n & 0x20) == 0) {            // READ ADDRESS
        if (imageFile != (std::FILE *) 0 &&
            currentTrack < nTracks &&
            currentSide < nSides) {
          buf[506] = currentTrack;
          buf[507] = currentSide;
          buf[508] = 0x01;          // assume first sector of track
          buf[509] = 0x02;          // 512 bytes per sector
          uint16_t  tmp = calculateCRC(&(buf[506]), 4, 0xB230);
          buf[510] = uint8_t(tmp >> 8);         // CRC high byte
          buf[511] = uint8_t(tmp & 0xFF);       // CRC low byte
          bufPos = 506;
          dataRequestFlag = true;
          statusRegister = statusRegister | 0x02;
        }
        else
          statusRegister = statusRegister | 0x10;   // record not found
      }
      else if ((n & 0x10) == 0) {       // READ TRACK (FIXME: unimplemented)
        statusRegister = statusRegister | 0x80;     // not ready
      }
      else {                            // WRITE TRACK (FIXME: unimplemented)
        statusRegister = statusRegister | 0x20;     // write error
      }
      if (bufPos >= 512) {
        // on error: clear busy flag, and trigger interrupt
        statusRegister = statusRegister & 0xFE;
        if (!interruptRequestFlag) {
          interruptRequestFlag = true;
          interruptRequest();
        }
      }
    }
    else {                              // ---- Type IV commands ----
      bool    i0 = ((n & 0x01) != 0);
      bool    i1 = ((n & 0x02) != 0);
      bool    i2 = ((n & 0x04) != 0);
      bool    i3 = ((n & 0x08) != 0);
      (void) i0;                        // FORCE INTERRUPT
      (void) i1;
      // reset status
      dataRequestFlag = false;
      statusRegister = (statusRegister & 0x01) | 0x20;
      if (writeProtectFlag)
        statusRegister = statusRegister | 0x40;
      if (imageFile) {
        if (currentTrack == 0)
          statusRegister = statusRegister | 0x04;
        statusRegister = statusRegister | 0x02; // index pulse
      }
      // if a write sector command is interrupted, may need to flush buffer
      if ((commandRegister & 0xE0) == 0xA0 && (statusRegister & 0x01) != 0) {
        if (bufPos > 0) {
          for ( ; bufPos < 512; bufPos++)
            buf[bufPos] = 0;
          if (imageFile != (std::FILE *) 0 && !writeProtectFlag) {
            std::fwrite(&(buf[0]), 1, 512, imageFile);
            std::fflush(imageFile);
          }
        }
      }
      // FIXME: only immediate interrupt is implemented
      statusRegister = statusRegister & 0xFE;
      commandRegister = n;
      if (i2 || i3) {
        if (!interruptRequestFlag) {
          interruptRequestFlag = true;
          interruptRequest();
        }
      }
    }
  }

  uint8_t WD177x::readStatusRegister()
  {
    if (interruptRequestFlag && (commandRegister & 0xF8) != 0xD8) {
      interruptRequestFlag = false;
      clearInterruptRequest();
    }
    uint8_t n = statusRegister;
    if (isWD1773)
      n = n & 0x7F;             // always ready
    else
      n = n | 0x80;             // motor is always on
    if (busyFlagHackEnabled) {
      busyFlagHack = !busyFlagHack;
      if (busyFlagHack)
        n = n | 0x01;
    }
    return n;
  }

  void WD177x::writeTrackRegister(uint8_t n)
  {
    if ((statusRegister & 0x01) == 0)
      trackRegister = n;
  }

  uint8_t WD177x::readTrackRegister() const
  {
    return trackRegister;
  }

  void WD177x::writeSectorRegister(uint8_t n)
  {
    if ((statusRegister & 0x01) == 0)
      sectorRegister = n;
  }

  uint8_t WD177x::readSectorRegister() const
  {
    return sectorRegister;
  }

  void WD177x::writeDataRegister(uint8_t n)
  {
    dataRegister = n;
    if (dataRequestFlag && (commandRegister & 0xE0) == 0xA0 && bufPos < 512) {
      // writing sector
      buf[bufPos++] = dataRegister;
      if (bufPos >= 512) {
        bufPos = 0;
        // clear data request and busy flag
        dataRequestFlag = false;
        statusRegister = statusRegister & 0xFC;
        if (setFilePosition()) {
          size_t  bytesWritten = std::fwrite(&(buf[0]), 1, 512, imageFile);
          std::fflush(imageFile);
          if (bytesWritten == 512) {
            if (commandRegister & 0x10) {
              // multiple sectors: continue with writing next sector
              sectorRegister++;
              writeCommandRegister(commandRegister);
              return;
            }
          }
          else
            statusRegister = statusRegister | 0x20; // write error
        }
        else
          statusRegister = statusRegister | 0x10;   // record not found
        commandRegister = 0x00;
        if (!interruptRequestFlag) {
          interruptRequestFlag = true;
          interruptRequest();
        }
      }
    }
  }

  uint8_t WD177x::readDataRegister()
  {
    if (dataRequestFlag && bufPos < 512) {
      // reading sector
      dataRegister = buf[bufPos++];
      if (bufPos >= 512) {
        bufPos = 0;
        // clear data request and busy flag
        dataRequestFlag = false;
        statusRegister = statusRegister & 0xFC;
        if ((commandRegister & 0xF0) == 0x90) {
          // multiple sectors: continue with reading next sector
          sectorRegister++;
          writeCommandRegister(commandRegister);
        }
        else {
          commandRegister = 0x00;
          if (!interruptRequestFlag) {
            interruptRequestFlag = true;
            interruptRequest();
          }
        }
      }
    }
    return dataRegister;
  }

  uint8_t WD177x::readStatusRegisterDebug() const
  {
    uint8_t n = statusRegister;
    if (isWD1773)
      n = n & 0x7F;             // always ready
    else
      n = n | 0x80;             // motor is always on
    return n;
  }

  uint8_t WD177x::readDataRegisterDebug() const
  {
    return dataRegister;
  }

  bool WD177x::getDiskChangeFlag() const
  {
    return diskChangeFlag;
  }

  void WD177x::clearDiskChangeFlag()
  {
    diskChangeFlag = false;
  }

  void WD177x::setIsWD1773(bool isEnabled)
  {
    isWD1773 = isEnabled;
  }

  bool WD177x::getInterruptRequestFlag() const
  {
    return interruptRequestFlag;
  }

  bool WD177x::getDataRequestFlag() const
  {
    return dataRequestFlag;
  }

  bool WD177x::haveDisk() const
  {
    return (imageFile != (std::FILE *) 0);
  }

  bool WD177x::getIsWriteProtected() const
  {
    return writeProtectFlag;
  }

  void WD177x::setEnableBusyFlagHack(bool isEnabled)
  {
    busyFlagHackEnabled = isEnabled;
  }

  uint16_t WD177x::calculateCRC(const uint8_t *buf_, size_t nBytes, uint16_t n)
  {
    size_t  nBits = nBytes << 3;
    int     bitCnt = 0;
    uint8_t bitBuf = 0;

    while (nBits--) {
      if (bitCnt == 0) {
        bitBuf = *(buf_++);
        bitCnt = 8;
      }
      if ((bitBuf ^ uint8_t(n >> 8)) & 0x80)
        n = (n << 1) ^ 0x1021;
      else
        n = (n << 1);
      n = n & 0xFFFF;
      bitBuf = (bitBuf << 1) & 0xFF;
      bitCnt--;
    }
    return n;
  }

  void WD177x::reset()
  {
    if (interruptRequestFlag) {
      interruptRequestFlag = false;
      clearInterruptRequest();
    }
    if ((statusRegister & 0x01) != 0) {
      // terminate any unfinished command
      interruptRequestFlag = true;  // hack to avoid triggering an interrupt
      writeCommandRegister(0xD8);
    }
    commandRegister = 0;
    statusRegister = 0x20;
    if (writeProtectFlag)
      statusRegister = statusRegister | 0x40;
    if (imageFile)
      statusRegister = statusRegister | 0x06;   // track 0, index pulse
    trackRegister = 0;
    sectorRegister = 0;
    dataRegister = 0;
    currentTrack = 0;
    currentSide = 0;
    diskChangeFlag = true;
    interruptRequestFlag = false;
    dataRequestFlag = false;
    steppingIn = false;
    busyFlagHack = false;
    bufPos = 512;
  }

  void WD177x::interruptRequest()
  {
  }

  void WD177x::clearInterruptRequest()
  {
  }

}       // namespace Plus4Emu

