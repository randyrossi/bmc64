
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
#include "system.hpp"
#include "vc1551.hpp"
#include "iecdrive.hpp"

#include <cmath>
#include <map>
#include <sys/types.h>
#include <dirent.h>

static const size_t maxFileCnt = 4096;

static const unsigned char  directoryStartLine[32] = {
  0x01, 0x04, 0x01, 0x01, 0x00, 0x00, 0x12, 0x22,       // '......."'
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,       // '        '
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,       // '        '
  0x22, 0x20, 0x20, 0x20, 0x20, 0x32, 0x41, 0x00        // '"    2A.'
};

static const unsigned char  directoryEndLine[32] = {
  0x01, 0x01, 0xFF, 0xFF, 0x42, 0x4C, 0x4F, 0x43,       // '....BLOC'
  0x4B, 0x53, 0x20, 0x46, 0x52, 0x45, 0x45, 0x2E,       // 'KS FREE.'
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,       // '        '
  0x20, 0x20, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00        // '     ...'
};

static const char *errorMessages[75] = {
  " OK",                        // 00
  " FILES SCRATCHED",           // 01
  "",                           // 02
  "",                           // 03
  "",                           // 04
  "",                           // 05
  "",                           // 06
  "",                           // 07
  "",                           // 08
  "",                           // 09
  "",                           // 10
  "",                           // 11
  "",                           // 12
  "",                           // 13
  "",                           // 14
  "",                           // 15
  "",                           // 16
  "",                           // 17
  "",                           // 18
  "",                           // 19
  " READ ERROR",                // 20 (block header not found)
  " READ ERROR",                // 21 (no sync mark found on track)
  " READ ERROR",                // 22 (data block not present)
  " READ ERROR",                // 23 (checksum error in data block)
  " READ ERROR",                // 24 (byte decoding error)
  " WRITE ERROR",               // 25 (write verify error)
  " WRITE PROTECT ON",          // 26
  " READ ERROR",                // 27 (checksum error in header)
  " WRITE ERROR",               // 28 (no sync mark found after written block)
  " DISK ID MISMATCH",          // 29 (bad header)
  " SYNTAX ERROR",              // 30 (invalid command parameters)
  " SYNTAX ERROR",              // 31 (unrecognized command)
  " SYNTAX ERROR",              // 32 (command is too long)
  " SYNTAX ERROR",              // 33 (wildcards in SAVE filename)
  " SYNTAX ERROR",              // 34 (missing filename)
  "",                           // 35
  "",                           // 36
  "",                           // 37
  "",                           // 38
  " SYNTAX ERROR",              // 39 (unrecognized command)
  "",                           // 40
  "",                           // 41
  "",                           // 42
  "",                           // 43
  "",                           // 44
  "",                           // 45
  "",                           // 46
  "",                           // 47
  "",                           // 48
  "",                           // 49
  " RECORD NOT PRESENT",        // 50 (positioning REL file beyond last record)
  " OVERFLOW IN RECORD",        // 51 (writing exceeds record boundary)
  " FILE TOO LARGE",            // 52 (position in REL file exceeds disk space)
  "",                           // 53
  "",                           // 54
  "",                           // 55
  "",                           // 56
  "",                           // 57
  "",                           // 58
  "",                           // 59
  " WRITE FILE OPEN",           // 60 (file is already open for write)
  " FILE NOT OPEN",             // 61
  " FILE NOT FOUND",            // 62
  " FILE EXISTS",               // 63
  " FILE TYPE MISMATCH",        // 64
  " NO BLOCK",                  // 65
  " ILLEGAL TRACK AND SECTOR",  // 66
  " ILLEGAL SYSTEM T OR S",     // 67
  "",                           // 68
  "",                           // 69
  " NO CHANNEL",                // 70 (channel is already used)
  " DIRECTORY ERROR",           // 71
  " DISK FULL",                 // 72
  "PLUS4EMU 1.2.11 IEC DRIVE",  // 73
  " DRIVE NOT READY"            // 74
};

namespace Plus4 {

  ParallelIECDrive::Plus4FileName::Plus4FileName()
  {
    for (int i = 0; i < 16; i++)
      fileName[i] = 0x00;
    fileNameLen = 0;
  }

  ParallelIECDrive::Plus4FileName::Plus4FileName(const char *s)
  {
    fileNameLen = 0;
    for (int i = 0; s[i] != '\0' && i < 16; i++)
      fileName[fileNameLen++] = convertCharacterToPlus4(s[i]);
    for (int i = fileNameLen; i < 16; i++)
      fileName[i] = 0x00;
  }

  ParallelIECDrive::Plus4FileName::Plus4FileName(const std::string& s)
  {
    fileNameLen = 0;
    for (int i = 0; size_t(i) < s.length() && i < 16; i++)
      fileName[fileNameLen++] = convertCharacterToPlus4(s[i]);
    for (int i = fileNameLen; i < 16; i++)
      fileName[i] = 0x00;
  }

  ParallelIECDrive::Plus4FileName::Plus4FileName(const Plus4FileName& r)
  {
    for (int i = 0; i < 16; i++)
      fileName[i] = r.fileName[i];
    fileNameLen = r.fileNameLen;
  }

  ParallelIECDrive::Plus4FileName::~Plus4FileName()
  {
  }

  ParallelIECDrive::Plus4FileName&
      ParallelIECDrive::Plus4FileName::operator=(const char *s)
  {
    fileNameLen = 0;
    for (int i = 0; s[i] != '\0' && i < 16; i++)
      fileName[fileNameLen++] = convertCharacterToPlus4(s[i]);
    for (int i = fileNameLen; i < 16; i++)
      fileName[i] = 0x00;
    return (*this);
  }

  ParallelIECDrive::Plus4FileName&
      ParallelIECDrive::Plus4FileName::operator=(const std::string& s)
  {
    fileNameLen = 0;
    for (int i = 0; size_t(i) < s.length() && i < 16; i++)
      fileName[fileNameLen++] = convertCharacterToPlus4(s[i]);
    for (int i = fileNameLen; i < 16; i++)
      fileName[i] = 0x00;
    return (*this);
  }

  ParallelIECDrive::Plus4FileName&
      ParallelIECDrive::Plus4FileName::operator=(const Plus4FileName& r)
  {
    for (int i = 0; i < 16; i++)
      fileName[i] = r.fileName[i];
    fileNameLen = r.fileNameLen;
    return (*this);
  }

  ParallelIECDrive::Plus4FileName::operator std::string() const
  {
    std::string s = "";
    for (int i = 0; i < fileNameLen; i++)
      s += convertCharacterFromPlus4(fileName[i]);
    return s;
  }

  bool ParallelIECDrive::Plus4FileName::operator==(const Plus4FileName& r) const
  {
    if (fileNameLen != r.fileNameLen)
      return false;
    for (int i = 0; i < fileNameLen; i++) {
      if (fileName[i] != r.fileName[i])
        return false;
    }
    return true;
  }

  bool ParallelIECDrive::Plus4FileName::operator!=(const Plus4FileName& r) const
  {
    if (fileNameLen != r.fileNameLen)
      return true;
    for (int i = 0; i < fileNameLen; i++) {
      if (fileName[i] != r.fileName[i])
        return true;
    }
    return false;
  }

  bool ParallelIECDrive::Plus4FileName::operator<(const Plus4FileName& r) const
  {
    for (int i = 0; i < fileNameLen && i < r.fileNameLen; i++) {
      if (fileName[i] < r.fileName[i])
        return true;
      if (fileName[i] > r.fileName[i])
        return false;
    }
    return (fileNameLen < r.fileNameLen);
  }

  void ParallelIECDrive::Plus4FileName::appendCharacter(char c)
  {
    if (fileNameLen < 16)
      fileName[fileNameLen++] = convertCharacterToPlus4(c);
  }

  void ParallelIECDrive::Plus4FileName::appendPlus4Character(unsigned char c)
  {
    if (fileNameLen < 16)
      fileName[fileNameLen++] = c;
  }

  void ParallelIECDrive::Plus4FileName::clear()
  {
    for (int i = 0; i < 16; i++)
      fileName[i] = 0x00;
    fileNameLen = 0;
  }

  unsigned char ParallelIECDrive::Plus4FileName::convertCharacterToPlus4(char c)
  {
    if (c >= 'a' && c <= 'z') {
      c = (c - 'a') + 'A';
    }
    else if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
               c == '+' || c == '-' || c == '.' || c == ' ')) {
      c = char(0xA4);
    }
    return (unsigned char) c;
  }

  char ParallelIECDrive::Plus4FileName::convertCharacterFromPlus4(
      unsigned char c)
  {
    if (c >= 0x41 && c <= 0x5A)
      return (char(c - 0x41) + 'a');
    if (c >= 0x61 && c <= 0x7A)
      return (char(c - 0x61) + 'A');
    if (c >= 0xC1 && c <= 0xDA)
      return (char(c - 0xC1) + 'A');
    if ((c >= 0x30 && c <= 0x39) || c == 0x2B || c == 0x2D || c == 0x2E)
      return char(c);
    return '_';
  }

  // --------------------------------------------------------------------------

  ParallelIECDrive::FileTableEntry::FileTableEntry()
    : fileName(Plus4FileName("")),
      f((std::FILE *) 0),
      fileType('\0'),
      openMode('\0'),
      recordSize(0),
      recordPos(0),
      recordNum(0),
      fileSize(0L)
  {
  }

  ParallelIECDrive::FileTableEntry::~FileTableEntry()
  {
    this->clear();
  }

  void ParallelIECDrive::FileTableEntry::clear()
  {
    fileName.clear();
    if (f) {
      std::fclose(f);
      f = (std::FILE *) 0;
    }
    fileType = '\0';
    openMode = '\0';
    recordSize = 0;
    recordPos = 0;
    recordNum = 0;
    fileSize = 0L;
  }

  // --------------------------------------------------------------------------

  ParallelIECDrive::ParallelIECDrive(int deviceNumber_)
    : deviceNumber((deviceNumber_ & 3) | 8),
      iecBusStatus(0),
      dataRegisterIn(0xFF),
      dataRegisterOut(0xFF),
      handShake1(true),
      handShake2(true),
      currentBusMode(0),
      currentIOMode(0),
      secondaryAddress(0x00),
      recordDirtyFlag(false),
      fileDBUpdateFlag(true),
      writeProtectFlag(false),
      currentWorkingDirectory(""),
      bufPos(0),
      bufBytes(0),
      bufMaxBytes(256),
      errMsgPos(0),
      errMsgBytes(0),
      errorCode(0),
      recordLength(0),
      diskName("plus4emu disk")
  {
    for (int i = 0; i < 256; i++)
      recordData[i] = 0x00;
    diskID[0] = 0x30;
    diskID[1] = 0x30;
    directoryIterator = fileDB.end();
    this->reset();
  }

  ParallelIECDrive::~ParallelIECDrive()
  {
    for (int i = 0; i < 16; i++)
      filesOpened[i].clear();
  }

  void ParallelIECDrive::reset()
  {
    flushRelativeFile();
    tpi.reset();
    tpi.writeRegister(0x0002, 0x40);
    tpi.writeRegister(0x0003, 0xFF);
    tpi.writeRegister(0x0004, 0x00);
    tpi.writeRegister(0x0005, 0x40);
    iecBusStatus = 0;
    dataRegisterOut = 0xFF;
    handShake2 = true;
    updateParallelInterface();
    currentBusMode = 0;
    currentIOMode = 0;
    secondaryAddress = 0x00;
    recordDirtyFlag = false;
    fileDBUpdateFlag = true;
    bufPos = 0;
    bufBytes = 0;
    setErrorMessage(73);
    ledFlashTimer.reset();
    for (int i = 0; i < 16; i++)
      filesOpened[i].clear();
    recordLength = 0;
    fileDB.clear();
    directoryIterator = fileDB.end();
  }

  void ParallelIECDrive::setReadOnlyMode(bool isReadOnly)
  {
    writeProtectFlag = isReadOnly;
  }

  void ParallelIECDrive::setWorkingDirectory(const std::string& dirName_)
  {
    fileDBUpdateFlag = true;
    currentWorkingDirectory = dirName_;
#ifdef WIN32
    if (currentWorkingDirectory.length() > 0) {
      size_t  len = currentWorkingDirectory.length();
      // convert forward slashes to backslash characters
      for (size_t i = 0; i < len; i++) {
        if (currentWorkingDirectory[i] == '/')
          currentWorkingDirectory[i] = '\\';
      }
      // remove trailing backslash
      if (currentWorkingDirectory[len - 1] == '\\') {
        if (len > 1 && currentWorkingDirectory[len - 2] != ':')
          currentWorkingDirectory.resize(len - 1);
      }
    }
#endif
  }

  bool ParallelIECDrive::parallelIECRead(uint16_t addr, uint8_t& value)
  {
    if (!((deviceNumber == 8 && addr >= 0xFEE0 && addr <= 0xFEFF) ||
          (deviceNumber == 9 && addr >= 0xFEC0 && addr <= 0xFEDF))) {
      return false;
    }
    addr = addr & 0x0007;
    value = tpi.readRegister(addr);
    return true;
  }

  bool ParallelIECDrive::parallelIECWrite(uint16_t addr, uint8_t value)
  {
    if (!((deviceNumber == 8 && addr >= 0xFEE0 && addr <= 0xFEFF) ||
          (deviceNumber == 9 && addr >= 0xFEC0 && addr <= 0xFEDF))) {
      return false;
    }
    addr = addr & 0x0007;
    tpi.writeRegister(addr, value);
    updateParallelInterface();
    if (currentBusMode == 0 &&
        dataRegisterIn >= 0x81 && dataRegisterIn <= 0x84 &&
        handShake1) {
      switch (dataRegisterIn) {
      case 0x81:                                // talk / listen
        currentBusMode = 1;
        break;
      case 0x82:                                // tksa / second
        currentBusMode = 2;
        break;
      case 0x83:                                // ciout
        currentBusMode = 3;
        break;
      case 0x84:                                // acptr
        currentBusMode = 4;
        break;
      }
      handShake2 = false;
      updateParallelInterface();
    }
    else if (currentBusMode >= 1 && currentBusMode <= 3 && !handShake1) {
      // receive data byte from Plus/4
      updateParallelInterface();
      switch (currentBusMode) {
      case 1:
        switch (dataRegisterIn & 0xF0) {
        case 0x20:                              // listen
          if (currentIOMode == 0) {
            currentIOMode = 1;
            secondaryAddress = 0x00;
          }
          break;
        case 0x30:                              // unlsn
          if (currentIOMode == 1) {
            currentIOMode = 0;
            if (errorCode != 32) {
              if (secondaryAddress == 0x6F || secondaryAddress == 0xFF)
                dosCommand();
              else if (secondaryAddress >= 0xF0)
                openFile();
            }
            bufPos = 0;
            bufBytes = 0;
            if (recordDirtyFlag)
              flushRelativeFile();
            secondaryAddress = 0x00;
          }
          break;
        case 0x40:                              // talk
          if (currentIOMode == 0) {
            currentIOMode = 2;
            secondaryAddress = 0x00;
          }
          break;
        case 0x50:                              // untlk
          if (currentIOMode == 2) {
            currentIOMode = 0;
            secondaryAddress = 0x00;
          }
          break;
        }
        iecBusStatus = 0;
        break;
      case 2:
        if (secondaryAddress == 0x00 && currentIOMode != 0)
          secondaryAddress = dataRegisterIn & 0xFF;
        iecBusStatus = 0;
        if ((secondaryAddress & 0xF0) == 0x60) {
          int     channelNum = int(secondaryAddress & 0x0F);
          if (currentIOMode == 2 && channelNum == 15 &&
              filesOpened[15].fileType == '\0') {
            errMsgPos = 0;              // read error message
          }
          else if (currentIOMode == 1 && channelNum == 15 &&
                   filesOpened[15].fileType == '\0') {
            bufPos = 0;                 // initialize buffer position
            bufBytes = 0;               // for receiving DOS command
          }
          else {
            if (filesOpened[channelNum].fileType == '\0') {
              if (errorCode == 0 || errorCode == 73)
                setErrorMessage(61);    // "file not open"
            }
            else if (filesOpened[channelNum].fileType == 'R' &&
                     filesOpened[channelNum].f != (std::FILE *) 0) {
              // load relative file record (FIXME: this is slow)
              (void) readRelativeFileRecord(channelNum);
            }
          }
        }
        else if (currentIOMode == 1 && secondaryAddress >= 0xF0) {
          bufPos = 0;                   // initialize buffer position
          bufBytes = 0;                 // for receiving 'open' string
        }
        else if ((secondaryAddress & 0xF0) == 0xE0) {
          closeFile();
        }
        break;
      case 3:
        iecBusStatus = listenNextByte(dataRegisterIn);
        break;
      }
      handShake2 = true;
      currentBusMode = 0;
      updateParallelInterface();
    }
    else if (currentBusMode == 4 && !handShake1) {
      // send data byte to Plus/4
      iecBusStatus = talkNextByte(dataRegisterOut);
      handShake2 = true;
      currentBusMode = 5;
      if (iecBusStatus == 2) {
        currentIOMode = 0;
        secondaryAddress = 0x00;
      }
      updateParallelInterface();
    }
    else if (currentBusMode == 5 && handShake1) {
      // end of acptr
      iecBusStatus = 0;
      handShake2 = false;
      dataRegisterOut = 0xFF;
      updateParallelInterface();
    }
    else if (currentBusMode == 5 && !handShake1) {
      handShake2 = true;
      currentBusMode = 0;
      updateParallelInterface();
    }
    return true;
  }

  void ParallelIECDrive::updateParallelInterface()
  {
    tpi.setPortA(dataRegisterOut);
    tpi.setPortB(uint8_t(iecBusStatus & 3));
    tpi.setPortC(uint8_t((int(handShake2) << 7) | 0x40));
    dataRegisterIn = tpi.getPortA();
    handShake1 = bool(tpi.getPortC() & 0x40);
  }

  uint8_t ParallelIECDrive::getLEDState()
  {
    if (errorCode < 20 || errorCode == 73)
      return uint8_t(currentIOMode == 0 ? 0 : 6);
    double  t = ledFlashTimer.getRealTime();
    if (t > 0.333333) {
      t = std::fmod(t, 0.333333);
      ledFlashTimer.reset(t);
    }
    return uint8_t(t < 0.166667 ? 6 : 0);
  }

  // --------------------------------------------------------------------------

  int ParallelIECDrive::listenNextByte(uint8_t n)
  {
    if ((secondaryAddress & 0xF0) == 0x60) {    // DOS command or write file
      int     channelNum = int(secondaryAddress & 0x0F);
      if (channelNum != 15) {           // write file
        std::FILE *f = filesOpened[channelNum].f;
        if (f != (std::FILE *) 0) {
          if (filesOpened[channelNum].fileType != 'R') {
            if (filesOpened[channelNum].openMode == 'A' ||
                filesOpened[channelNum].openMode == 'W') {
              // write sequential file
              if (writeProtectFlag) {
                setErrorMessage(26);    // "write protect on"
                return 0;
              }
              if (std::fputc(int(n), f) == EOF)
                setErrorMessage(72);    // "disk full"
            }
          }
          else {
            // write relative file
            return writeRelativeFile(n);
          }
        }
      }
      else {                            // DOS command
        if (bufBytes < bufMaxBytes) {
          buf[bufBytes++] = n;
          bufPos = bufBytes;
        }
        else if (errorCode != 32)
          setErrorMessage(32);
      }
    }
    else if (secondaryAddress >= 0xF0) {        // open
      if (bufBytes < bufMaxBytes) {
        buf[bufBytes++] = n;
        bufPos = bufBytes;
      }
      else if (errorCode != 32)
        setErrorMessage(32);
    }
    return 0;
  }

  int ParallelIECDrive::talkNextByte(uint8_t& n)
  {
    if ((secondaryAddress & 0xF0) == 0x60) {
      int     channelNum = int(secondaryAddress & 0x0F);
      if (channelNum == 15 && filesOpened[15].fileType == '\0') {
        // read error message
        n = 0x0D;
        if (errMsgPos >= errMsgBytes)
          return 3;
        n = uint8_t(errMsg[errMsgPos++]);
        if (errMsgPos >= errMsgBytes) {
          setErrorMessage(0);
          errMsgPos = errMsgBytes;
          return 3;
        }
        return 0;
      }
      else if (filesOpened[channelNum].fileType == '$') {
        // read directory
        return readDirectory(n);
      }
      else if (filesOpened[channelNum].f != (std::FILE *) 0) {
        if (filesOpened[channelNum].fileType != 'R') {
          if (filesOpened[channelNum].openMode == 'M' ||
              filesOpened[channelNum].openMode == 'R') {
            // read sequential file
            std::FILE *f = filesOpened[channelNum].f;
            int     c = std::fgetc(f);
            if (c == EOF) {
              n = 0x0D;
              return 2;
            }
            n = uint8_t(c & 0xFF);
            long    savedPos = std::ftell(f);
            c = std::fgetc(f);
            if (c == EOF)
              return 3;
            if (savedPos >= 0L)
              std::fseek(f, savedPos, SEEK_SET);
            return 0;
          }
        }
        else {
          // read relative file
          return readRelativeFile(n);
        }
      }
    }
    n = 0x00;
    return 2;
  }

  void ParallelIECDrive::updateFileDB()
  {
    DIR       *d = (DIR *) 0;
    std::FILE *f = (std::FILE *) 0;
    try {
      fileDB.clear();
      directoryIterator = fileDB.end();
      if (currentWorkingDirectory.length() < 1) {
        fileDBUpdateFlag = false;
        return;
      }
      d = opendir(currentWorkingDirectory.c_str());
      if (!d) {
        fileDBUpdateFlag = false;
        return;
      }
      do {
        struct dirent *e = readdir(d);
        if (!e)
          break;
        const char    *baseName = &(e->d_name[0]);
        size_t        nameLen = std::strlen(baseName);
        if (nameLen < 5)
          continue;
        const char    *s = &(baseName[nameLen - 4]);
        if (s[0] != '.')
          continue;
        char          fileType = '\0';
        unsigned char recordSize = 0;
        if ((s[1] == 'P' || s[1] == 'p') &&
            (s[2] == 'R' || s[2] == 'r') &&
            (s[3] == 'G' || s[3] == 'g')) {
          fileType = 'p';
        }
        else if ((s[2] >= '0' && s[2] <= '9') &&
                 (s[3] >= '0' && s[3] <= '9')) {
          if (s[1] == 'P' || s[1] == 'p')
            fileType = 'P';
          if (s[1] == 'R' || s[1] == 'r')
            fileType = 'R';
          if (s[1] == 'S' || s[1] == 's')
            fileType = 'S';
          if (s[1] == 'U' || s[1] == 'u')
            fileType = 'U';
        }
        if (fileType == '\0')
          continue;
        Plus4FileName plus4Name;
        for (size_t i = 0; i < (nameLen - 4); i++)
          plus4Name.appendCharacter(baseName[i]);
        std::string   fullName = currentWorkingDirectory;
        if (fullName[fullName.length() - 1] != '/' &&
            fullName[fullName.length() - 1] != '\\') {
#ifdef WIN32
          fullName += '\\';
#else
          fullName += '/';
#endif
        }
        fullName += baseName;
        f = Plus4Emu::fileOpen(fullName.c_str(), "rb");
        if (!f)
          continue;
        if (fileType != 'p') {
          // check header of P00, R00, S00, and U00 files
          unsigned char hdrData[26];
          size_t        hdrLen =
              std::fread(&(hdrData[0]), sizeof(unsigned char), 26, f);
          std::fclose(f);
          f = (std::FILE *) 0;
          if (hdrLen < 26)
            continue;
          if (!(hdrData[0] == 0x43 && hdrData[1] == 0x36 &&     // "C6"
                hdrData[2] == 0x34 && hdrData[3] == 0x46 &&     // "4F"
                hdrData[4] == 0x69 && hdrData[5] == 0x6C &&     // "il"
                hdrData[6] == 0x65 && hdrData[7] == 0x00)) {    // "e\0"
            continue;
          }
          if (hdrData[8] == 0x00)
            continue;                   // empty name is invalid
          if (fileType == 'R') {
            recordSize = hdrData[25];
            if (recordSize < 1 || recordSize > 254)
              continue;                 // REL file with invalid record size
          }
          plus4Name.clear();
          for (int i = 8; i < 24 && hdrData[i] != 0x00; i++)
            plus4Name.appendPlus4Character(hdrData[i]);
        }
        else {
          std::fclose(f);
          f = (std::FILE *) 0;
        }
        addFileToDB(plus4Name, fullName, fileType, recordSize);
      } while (fileDB.size() < maxFileCnt);
      closedir(d);
      d = (DIR *) 0;
    }
    catch (...) {
      if (f)
        std::fclose(f);
      if (d)
        closedir(d);
      throw;
    }
    fileDBUpdateFlag = false;
  }

  int ParallelIECDrive::createFile(std::FILE*& f,
                                   const Plus4FileName& fileName, char fileType,
                                   int recSize)
  {
    f = (std::FILE *) 0;
    try {
      if (writeProtectFlag)
        return -1;
      if (fileDB.size() >= maxFileCnt)
        return -2;
      if (fileType != 'P' && fileType != 'R' &&
          fileType != 'S' && fileType != 'U') {
        fileType = 'P';
      }
      recSize = (recSize > 1 ? (recSize < 254 ? recSize : 254) : 1);
      unsigned char   hdrData[26];
      hdrData[0] = 0x43;                // 'C'
      hdrData[1] = 0x36;                // '6'
      hdrData[2] = 0x34;                // '4'
      hdrData[3] = 0x46;                // 'F'
      hdrData[4] = 0x69;                // 'i'
      hdrData[5] = 0x6C;                // 'l'
      hdrData[6] = 0x65;                // 'e'
      hdrData[7] = 0x00;                // '\0'
      for (int i = 8; i < 26; i++)
        hdrData[i] = 0x00;
      for (int i = 0; i < fileName.fileNameLen; i++)
        hdrData[i + 8] = fileName.fileName[i];
      if (fileType == 'R')
        hdrData[25] = (unsigned char) recSize;
      std::string     fullName = currentWorkingDirectory;
      if (fullName[fullName.length() - 1] != '/' &&
          fullName[fullName.length() - 1] != '\\') {
#ifdef WIN32
        fullName += '\\';
#else
        fullName += '/';
#endif
      }
      for (int i = 0; i < 8 && i < fileName.fileNameLen; i++) {
        char    c =
            Plus4FileName::convertCharacterFromPlus4(fileName.fileName[i]);
        if (c >= 'A' && c <= 'Z')
          c = (c - 'A') + 'a';
        if (c == '.')
          c = '_';
        fullName += c;
      }
      fullName += '.';
      fullName += ((fileType - 'A') + 'a');
      fullName += "00";
      unsigned int    fileIndex = 0U;
      do {
        // change suffix if a file with the same name already exists
        fullName[fullName.length() - 2] = (char(fileIndex / 10) + '0');
        fullName[fullName.length() - 1] = (char(fileIndex % 10) + '0');
        f = Plus4Emu::fileOpen(fullName.c_str(), "rb");
        if (!f)
          break;
        std::fclose(f);
        f = (std::FILE *) 0;
      } while (++fileIndex <= 99U);
      if (fileIndex > 99U)
        return -2;
      f = Plus4Emu::fileOpen(fullName.c_str(),
                             (fileType != 'R' ? "wb" : "w+b"));
      if (!f)
        return -1;
      if (std::fwrite(&(hdrData[0]), sizeof(unsigned char), 26, f) != 26 ||
          std::fflush(f) != 0) {
        std::fclose(f);
        f = (std::FILE *) 0;
        Plus4Emu::fileRemove(fullName.c_str());
        return -1;
      }
      addFileToDB(fileName, fullName, fileType, recSize);
    }
    catch (...) {
      if (f) {
        std::fclose(f);
        f = (std::FILE *) 0;
      }
      throw;
    }
    return 0;
  }

  void ParallelIECDrive::addFileToDB(const Plus4FileName& fileName,
                                     const std::string& fullName,
                                     char fileType, int recordSize)
  {
    std::map< Plus4FileName, FileDBEntry >::iterator  i_ =
        fileDB.find(fileName);
    if (fileType == 'R')
      recordSize = (recordSize > 1 ? (recordSize < 254 ? recordSize : 254) : 0);
    else
      recordSize = 0;
    if (i_ == fileDB.end()) {
      // add new entry
      FileDBEntry   tmp;
      tmp.fullName = fullName;
      tmp.fileType = fileType;
      tmp.recordSize = (unsigned char) recordSize;
      fileDB.insert(std::pair< Plus4FileName, FileDBEntry >(fileName, tmp));
    }
    else {
      // or modify already existing one
      (*i_).second.fullName = fullName;
      (*i_).second.fileType = fileType;
      (*i_).second.recordSize = (unsigned char) recordSize;
    }
    directoryIterator = fileDB.end();
  }

  ParallelIECDrive::Plus4FileName
      ParallelIECDrive::findFile(const Plus4FileName& fileName)
  {
    if (fileName.fileNameLen < 1)
      return Plus4FileName("");         // invalid (empty) name
    for (int i = 0;
         fileName.fileName[i] != 0x2A && fileName.fileName[i] != 0x3F;
         i++) {
      if ((i + 1) == fileName.fileNameLen) {
        // if there are no wildcard characters:
        std::map< Plus4FileName, FileDBEntry >::iterator  i_ =
            fileDB.find(fileName);
        if (i_ == fileDB.end())
          return Plus4FileName("");
        return (*i_).first;
      }
    }
    // find file name with wildcards
    std::map< Plus4FileName, FileDBEntry >::iterator  i_ = fileDB.begin();
    while (i_ != fileDB.end()) {
      const Plus4FileName&  fName = (*i_).first;
      int     i = 0;
      while (true) {
        if (i >= fileName.fileNameLen) {
          if (i >= fName.fileNameLen)
            return (*i_).first;
          break;
        }
        // '*': match zero or more characters, ignore the rest of the file name
        if (fileName.fileName[i] == 0x2A)
          return (*i_).first;
        if (i >= fName.fileNameLen)
          break;
        // '?': match exactly one character
        if (fileName.fileName[i] != 0x3F) {
          if (fileName.fileName[i] != fName.fileName[i])
            break;
        }
        i++;
      }
      i_++;
    }
    return Plus4FileName("");           // not found
  }

  void ParallelIECDrive::setErrorMessage(int n, int t, int s)
  {
    if (n < 0 || n > 74)
      n = 73;
    t = (t > 0 ? (t < 99 ? t : 99) : 0);
    s = (s > 0 ? (s < 99 ? s : 99) : 0);
    errorCode = n;
    errMsgBytes = size_t(std::sprintf(&(errMsg[0]), "%02d,%s,%02d,%02d\r",
                                      n, errorMessages[n], t, s));
    errMsgPos = errMsgBytes;
  }

  void ParallelIECDrive::openFile()
  {
    int     channelNum = int(secondaryAddress & 0x0F);
    if (filesOpened[channelNum].fileType != '\0') {
      setErrorMessage(70);              // "no channel"
      return;
    }
    filesOpened[channelNum].clear();
    if (bufBytes < 1) {
      setErrorMessage(34);              // missing filename
      return;
    }
    if (currentWorkingDirectory.length() < 1) {
      updateFileDB();
      setErrorMessage(74);              // "drive not ready"
      return;
    }
    bool    fileDBUpdated = false;
    if (fileDBUpdateFlag) {
      updateFileDB();
      fileDBUpdated = true;
    }
    char            fileType = (channelNum < 2 ? 'P' : '\0');
    char            openMode = (channelNum == 1 ? 'W' : 'R');
    bool            allowOverwrite = false;
    unsigned char   recordSize = 0;
    Plus4FileName   fileName;
    bufPos = 0;
    if (buf[0] == 0x23) {               // '#': buffers
      // TODO: implement this
      setErrorMessage(34);
      return;
    }
    else if (buf[0] == 0x40) {          // '@'
      allowOverwrite = true;
      bufPos++;
    }
    if ((bufPos + 1) <= bufBytes && buf[bufPos] == 0x3A) {
      bufPos = bufPos + 1;
    }
    else if ((bufPos + 2) <= bufBytes &&
             buf[bufPos] >= 0x30 && buf[bufPos] <= 0x39 &&
             buf[bufPos + 1] == 0x3A) { // skip drive number
      bufPos = bufPos + 2;
    }
    while (bufPos < bufBytes) {         // file name
      if (buf[bufPos] == 0x2C || buf[bufPos] == 0x00 || buf[bufPos] == 0x0D)
        break;
      fileName.appendPlus4Character(buf[bufPos]);
      bufPos++;
    }
    if (fileName.fileNameLen < 1) {
      setErrorMessage(34);              // missing filename
      return;
    }
    for (int j = 0; j < 2 && bufPos < bufBytes; j++) {
      if (buf[bufPos] == 0x2C) {        // file type / open mode / record size
        bufPos++;
        if (fileType == 'R') {
          if (bufPos >= bufBytes) {
            setErrorMessage(30);        // invalid parameters
            return;
          }
          recordSize = buf[bufPos++];
          if (recordSize < 1 || recordSize > 254) {
            setErrorMessage(30);
            return;
          }
          break;
        }
        char    tmpBuf[8];
        int     i = 0;
        while (i < 7 && bufPos < bufBytes) {
          if (buf[bufPos] == 0x2C || buf[bufPos] == 0x00 || buf[bufPos] == 0x0D)
            break;
          tmpBuf[i++] = char(buf[bufPos++]);
        }
        tmpBuf[i] = '\0';
        if (std::strcmp(&(tmpBuf[0]), "A") == 0)
          openMode = 'A';
        else if (std::strcmp(&(tmpBuf[0]), "L") == 0)
          fileType = 'R';
        else if (std::strcmp(&(tmpBuf[0]), "M") == 0)
          openMode = 'M';
        else if (std::strcmp(&(tmpBuf[0]), "P") == 0)
          fileType = 'P';
        else if (std::strcmp(&(tmpBuf[0]), "PRG") == 0)
          fileType = 'P';
        else if (std::strcmp(&(tmpBuf[0]), "R") == 0)
          openMode = 'R';
        else if (std::strcmp(&(tmpBuf[0]), "READ") == 0)
          openMode = 'R';
        else if (std::strcmp(&(tmpBuf[0]), "REL") == 0)
          fileType = 'R';
        else if (std::strcmp(&(tmpBuf[0]), "S") == 0)
          fileType = 'S';
        else if (std::strcmp(&(tmpBuf[0]), "SEQ") == 0)
          fileType = 'S';
        else if (std::strcmp(&(tmpBuf[0]), "U") == 0)
          fileType = 'U';
        else if (std::strcmp(&(tmpBuf[0]), "USR") == 0)
          fileType = 'U';
        else if (std::strcmp(&(tmpBuf[0]), "W") == 0)
          openMode = 'W';
        else if (std::strcmp(&(tmpBuf[0]), "WRITE") == 0)
          openMode = 'W';
        else {
          setErrorMessage(30);
          return;
        }
      }
    }
    if (bufPos < bufBytes) {
      if (buf[bufPos] != 0x00 && buf[bufPos] != 0x0D) {
        setErrorMessage(30);
        return;
      }
    }
    if (fileType == 'R')
      openMode = 'W';
    if (fileName.fileName[0] == 0x24 && (openMode == 'R' || openMode == 'M')) {
      // directory
      if (fileType != '\0' && fileType != 'P') {
        setErrorMessage(64);
        return;
      }
      int     nameOffs = 1;
      if (fileName.fileNameLen >= 2 && fileName.fileName[1] == 0x3A)
        nameOffs = 2;
      else if (fileName.fileNameLen >= 3 && fileName.fileName[2] == 0x3A)
        nameOffs = 3;                   // skip drive number
      for (int i = nameOffs; i < fileName.fileNameLen && i < 16; i++) {
        filesOpened[channelNum].fileName.appendPlus4Character(
            fileName.fileName[i]);
      }
      filesOpened[channelNum].fileType = '$';
      filesOpened[channelNum].openMode = 'R';
      bufPos = 0;
      bufBytes = 0;
      setErrorMessage(0);
      if (!fileDBUpdated)
        updateFileDB();
      directoryIterator = fileDB.begin();
      return;
    }
    std::FILE *f = (std::FILE *) 0;
    if (openMode != 'W') {              // ---- read or append mode ----
      Plus4FileName nameFound = findFile(fileName);
      if (nameFound.fileNameLen < 1 && !fileDBUpdated) {
        // if not found, try reloading the directory first
        updateFileDB();
        nameFound = findFile(fileName);
      }
      if (nameFound.fileNameLen < 1) {
        setErrorMessage(62);            // "file not found"
        return;
      }
      fileName = nameFound;
      if (fileType != '\0') {
        if (fileType != (fileDB[fileName].fileType & char(0xDF))) {
          setErrorMessage(64);          // "file type mismatch"
          return;
        }
      }
      fileType = fileDB[fileName].fileType;
      if (fileType == 'R') {
        openRelativeFile(fileName, recordSize);
        return;
      }
      if (openMode != 'A') {            // -- read mode --
        // check if this file is currently open
        if (openMode != 'M') {
          for (int i = 0; i < 16; i++) {
            if (filesOpened[i].f != (std::FILE *) 0 &&
                filesOpened[i].fileName == fileName &&
                (filesOpened[i].openMode == 'A' ||
                 filesOpened[i].openMode == 'W')) {
              setErrorMessage(60);      // "write file open"
              return;
            }
          }
        }
        f = Plus4Emu::fileOpen(fileDB[fileName].fullName.c_str(), "rb");
        if (!f) {
          fileDBUpdateFlag = true;
          setErrorMessage(62);          // "file not found"
          return;
        }
        if (fileType != 'p') {
          // skip header of .P00 etc. files
          std::fseek(f, 26L, SEEK_SET);
        }
      }
      else {                            // -- append mode --
        if (writeProtectFlag) {
          setErrorMessage(26);          // "write protect on"
          return;
        }
        // check if this file is currently open
        for (int i = 0; i < 16; i++) {
          if (filesOpened[i].f != (std::FILE *) 0 &&
              filesOpened[i].fileName == fileName) {
            setErrorMessage(60);        // "write file open"
            return;
          }
        }
        f = Plus4Emu::fileOpen(fileDB[fileName].fullName.c_str(), "ab");
        if (!f) {
          fileDBUpdateFlag = true;
          setErrorMessage(26);          // error opening file for write
          return;
        }
      }
    }
    else {                              // ---- write mode ----
      if (writeProtectFlag) {
        setErrorMessage(26);            // "write protect on"
        return;
      }
      for (int i = 0; i < fileName.fileNameLen; i++) {
        // no wildcards are allowed for save
        if (fileName.fileName[i] == 0x2A || fileName.fileName[i] == 0x3F) {
          setErrorMessage(33);
          return;
        }
      }
      bool    fileExists = (findFile(fileName).fileNameLen > 0);
      if (fileExists && !fileDBUpdated) {
        updateFileDB();
        fileExists = (findFile(fileName).fileNameLen > 0);
      }
      if (fileExists) {
        if (fileDB[fileName].fileType == 'R') {
          if (fileType != '\0' && fileType != 'R') {
            setErrorMessage(64);        // "file type mismatch"
            return;
          }
          openRelativeFile(fileName, recordSize);
          return;
        }
        if (!allowOverwrite) {
          setErrorMessage(63);          // "file exists"
          return;
        }
      }
      if (fileType == 'R' && (recordSize < 1 || recordSize > 254)) {
        setErrorMessage(30);            // invalid record size
        return;
      }
      if (fileExists) {
        // check if this file is currently open
        for (int i = 0; i < 16; i++) {
          if (filesOpened[i].f != (std::FILE *) 0 &&
              filesOpened[i].fileName == fileName) {
            setErrorMessage(60);        // "write file open"
            return;
          }
        }
        // remove old entry from directory
        if (!scratchFile(fileName))
          return;
      }
      int       err = createFile(f, fileName, fileType, recordSize);
      if (err == -1) {
        fileDBUpdateFlag = true;
        setErrorMessage(26);            // error opening file for write
        return;
      }
      if (err == -2) {
        fileDBUpdateFlag = true;
        setErrorMessage(72);            // too many directory entries
        return;
      }
    }
    filesOpened[channelNum].fileName = fileName;
    filesOpened[channelNum].f = f;
    filesOpened[channelNum].fileType = fileType;
    filesOpened[channelNum].openMode = openMode;
    filesOpened[channelNum].recordSize = recordSize;
    setErrorMessage(0);
  }

  void ParallelIECDrive::dosCommand()
  {
    const unsigned char *cmdBuf = &(buf[0]);
    size_t  cmdLen = bufBytes;
    bufPos = 0;
    bufBytes = 0;
    if (cmdLen < 1)                     // no command
      return;
    setErrorMessage(0);
    if (cmdBuf[cmdLen - 1] == 0x0D)     // ignore trailing CR character
      cmdLen--;
    if (cmdLen < 1) {                   // CR only: error (invalid command)
      setErrorMessage(31);
      return;
    }
    if (currentWorkingDirectory.length() < 1) {
      updateFileDB();
      setErrorMessage(74);              // "drive not ready"
      return;
    }
    bool    fileDBUpdated = false;
    if (fileDBUpdateFlag && !(cmdBuf[0] == 0x49 || cmdBuf[0] == 0x56)) {
      updateFileDB();
      fileDBUpdated = true;
    }
    size_t  nameOffset = 0;
    size_t  nameLen = 0;
    for (size_t i = 1; i < cmdLen; i++) {
      if (cmdBuf[i] == 0x3A && nameOffset == 0) {
        nameOffset = i + 1;
        nameLen = cmdLen - nameOffset;
      }
      else if (nameOffset != 0 &&
               (cmdBuf[i] == 0x2C || cmdBuf[i] == 0x3A || cmdBuf[i] == 0x3D)) {
        nameLen = i - nameOffset;
        break;
      }
    }
    bool    haveWildcards = false;
    for (size_t i = nameOffset; i < (nameOffset + nameLen); i++) {
      if (cmdBuf[i] == 0x2A || cmdBuf[i] == 0x3F)
        haveWildcards = true;
    }
    switch (cmdBuf[0]) {
    case 0x43:                          // COPY
      if (nameLen < 1) {
        setErrorMessage(34);            // missing filename
      }
      else {
        if ((nameOffset + nameLen) >= cmdLen ||
            cmdBuf[nameOffset + nameLen] != 0x3D) {
          setErrorMessage(30);          // invalid command parameters
          return;
        }
        Plus4FileName dstFileName;
        Plus4FileName srcFileName;
        for (size_t i = nameOffset; i < (nameOffset + nameLen); i++)
          dstFileName.appendPlus4Character(cmdBuf[i]);
        size_t  nameEndPos = nameOffset + nameLen;
        bool    appendFlag = false;
        do {
          size_t  srcNameOffset = nameEndPos + 1;
          nameEndPos = cmdLen;
          for (size_t i = 0; i < 2 && (srcNameOffset + i) < cmdLen; i++) {
            if (cmdBuf[srcNameOffset + i] == 0x3A) {
              // ignore drive number
              srcNameOffset = srcNameOffset + i + 1;
              break;
            }
          }
          for (size_t i = srcNameOffset; i < cmdLen; i++) {
            if (cmdBuf[i] == 0x2C) {
              nameEndPos = i;
              break;
            }
          }
          srcFileName.clear();
          for (size_t i = srcNameOffset; i < nameEndPos; i++)
            srcFileName.appendPlus4Character(cmdBuf[i]);
          if (!appendFlag) {
            if (!copyFile(dstFileName, srcFileName))
              return;
          }
          else {
            if (!appendFile(dstFileName, srcFileName))
              return;
          }
          appendFlag = true;
        } while (nameEndPos < cmdLen);
      }
      break;
    case 0x49:                          // INITIALIZE
      for (int i = 0; i < 16; i++)
        filesOpened[i].clear();
      fileDBUpdateFlag = true;
      directoryIterator = fileDB.end();
      setErrorMessage(73);
      break;
    case 0x4E:                          // NEW
      if (nameLen < 1) {
        setErrorMessage(34);            // missing disk name
        return;
      }
      if ((nameOffset + nameLen) < cmdLen) {
        if (cmdBuf[nameOffset + nameLen] == 0x2C &&
            (nameOffset + nameLen + 3) == cmdLen) {
          diskID[0] = cmdBuf[nameOffset + nameLen + 1];
          diskID[1] = cmdBuf[nameOffset + nameLen + 2];
        }
        else {
          setErrorMessage(30);          // invalid command parameters
          return;
        }
      }
      diskName.clear();
      for (size_t i = nameOffset; i < (nameOffset + nameLen); i++)
        diskName.appendPlus4Character(cmdBuf[i]);
      if (writeProtectFlag) {
        setErrorMessage(26);            // "write protect on"
        return;
      }
      if (!fileDBUpdated)
        updateFileDB();
      while (errorCode == 0 && fileDB.begin() != fileDB.end()) {
        // delete all files
        Plus4FileName fileName = (*(fileDB.begin())).first;
        scratchFile(fileName);
      }
      break;
    case 0x50:                          // POSITION
      while (cmdLen > 0 && cmdBuf[0] >= 0x41 && cmdBuf[0] <= 0x5A) {
        // skip command name
        cmdBuf++;
        cmdLen--;
      }
      if (cmdLen < 3) {
        setErrorMessage(30);
        return;
      }
      else {
        int     channelNum = int(cmdBuf[0]);
        if (channelNum < 96 || channelNum > 111) {
          setErrorMessage(70);          // "no channel"
          return;
        }
        channelNum = channelNum & 15;
        if (channelNum == 15) {
          setErrorMessage(64);          // "file type mismatch"
          return;
        }
        if (filesOpened[channelNum].fileType == '\0') {
          setErrorMessage(70);          // "no channel"
          return;
        }
        int     recordSize = filesOpened[channelNum].recordSize;
        if (filesOpened[channelNum].f == (std::FILE *) 0 ||
            filesOpened[channelNum].fileType != 'R' ||
            recordSize < 1) {
          setErrorMessage(64);          // "file type mismatch"
          return;
        }
        int     recordNum = int(cmdBuf[1]) | (int(cmdBuf[2]) << 8);
        recordNum = (recordNum > 0 ? (recordNum - 1) : 0);
        int     recordPos = 0;
        if (cmdLen > 3) {
          recordPos = int(cmdBuf[3]);
          recordPos = (recordPos > 0 ? (recordPos - 1) : 0);
        }
        if (recordPos >= recordSize)
          setErrorMessage(51);          // "overflow in record"
        if (((long(recordNum) * recordSize) + recordPos)
            >= filesOpened[channelNum].fileSize) {
          setErrorMessage(50);          // "record not present"
        }
        filesOpened[channelNum].recordNum = recordNum;
        filesOpened[channelNum].recordPos = (unsigned char) recordPos;
      }
      break;
    case 0x52:                          // RENAME
      if (nameLen < 1) {
        setErrorMessage(34);            // missing filename
      }
      else {
        if ((nameOffset + nameLen) >= cmdLen ||
            cmdBuf[nameOffset + nameLen] != 0x3D) {
          setErrorMessage(30);          // invalid command parameters
          return;
        }
        Plus4FileName dstFileName;
        Plus4FileName srcFileName;
        for (size_t i = nameOffset; i < (nameOffset + nameLen); i++)
          dstFileName.appendPlus4Character(cmdBuf[i]);
        size_t  srcNameOffset = nameOffset + nameLen + 1;
        for (size_t i = 0; i < 2 && (srcNameOffset + i) < cmdLen; i++) {
          if (cmdBuf[srcNameOffset + i] == 0x3A) {
            // ignore drive number
            srcNameOffset = srcNameOffset + i + 1;
            break;
          }
        }
        for (size_t i = srcNameOffset; i < cmdLen; i++)
          srcFileName.appendPlus4Character(cmdBuf[i]);
        if (copyFile(dstFileName, srcFileName))
          scratchFile(srcFileName);
      }
      break;
    case 0x53:                          // SCRATCH
      if (nameLen < 1) {
        setErrorMessage(34);            // missing filename
        return;
      }
      if ((nameOffset + nameLen) < cmdLen) {
        setErrorMessage(30);            // invalid command parameters
        return;
      }
      else {
        if (haveWildcards && !fileDBUpdated) {
          updateFileDB();
          fileDBUpdated = true;
        }
        Plus4FileName fileName;
        for (size_t i = nameOffset; i < (nameOffset + nameLen); i++)
          fileName.appendPlus4Character(cmdBuf[i]);
        Plus4FileName nameFound = findFile(fileName);
        if (nameFound.fileNameLen < 1 && !fileDBUpdated) {
          updateFileDB();
          nameFound = findFile(fileName);
        }
        int     scratchCnt = 0;
        if (nameFound.fileNameLen > 0) {
          if (scratchFile(nameFound))
            scratchCnt++;
          if (haveWildcards) {
            while (errorCode == 0) {
              nameFound = findFile(fileName);
              if (nameFound.fileNameLen < 1)
                break;
              if (scratchFile(nameFound))
                scratchCnt++;
            }
          }
        }
        if (errorCode == 0)
          setErrorMessage(1, scratchCnt);
      }
      break;
    case 0x56:                          // VALIDATE
      fileDBUpdateFlag = true;
      break;
    default:
      setErrorMessage(31);              // unrecognized command
    }
  }

  void ParallelIECDrive::closeFile()
  {
    int     channelNum = int(secondaryAddress & 0x0F);
    if (channelNum == 15) {
      for (int i = 0; i < 16; i++)
        filesOpened[i].clear();
    }
    else if (filesOpened[channelNum].fileType == '\0') {
      if (errorCode == 0 || errorCode == 73)
        setErrorMessage(61);            // "file not open"
      return;
    }
    filesOpened[channelNum].clear();
  }

  void ParallelIECDrive::openRelativeFile(const Plus4FileName& fileName,
                                          int recordSize)
  {
    if (fileName.fileNameLen < 1) {
      setErrorMessage(62);              // "file not found"
      return;
    }
    int     channelNum = int(secondaryAddress & 0x0F);
    filesOpened[channelNum].clear();
    if (recordSize == 0) {
      recordSize = fileDB[fileName].recordSize;
    }
    else if (recordSize != int(fileDB[fileName].recordSize)) {
      setErrorMessage(64);              // record size does not match file
      return;
    }
    char    openMode = 'W';
    std::FILE *f = (std::FILE *) 0;
    if (!writeProtectFlag)
      f = Plus4Emu::fileOpen(fileDB[fileName].fullName.c_str(), "r+b");
    if (!f) {
      openMode = 'R';
      f = Plus4Emu::fileOpen(fileDB[fileName].fullName.c_str(), "rb");
    }
    if (!f) {
      fileDBUpdateFlag = true;
      setErrorMessage(62);              // "file not found"
      return;
    }
    std::fseek(f, 0L, SEEK_END);
    long    fileSize = std::ftell(f);
    if (fileSize < 26L) {
      std::fclose(f);
      setErrorMessage(27);              // "read error"
      return;
    }
    // skip .R00 header
    std::fseek(f, 26L, SEEK_SET);
    filesOpened[channelNum].fileName = fileName;
    filesOpened[channelNum].f = f;
    filesOpened[channelNum].fileType = 'R';
    filesOpened[channelNum].openMode = openMode;
    filesOpened[channelNum].recordSize = (unsigned char) recordSize;
    filesOpened[channelNum].fileSize = fileSize - 26L;
    (void) readRelativeFileRecord(channelNum);  // read first record
    setErrorMessage(0);
  }

  bool ParallelIECDrive::readRelativeFileRecord(int channelNum)
  {
    recordDirtyFlag = false;
    recordLength = 0;
    int     recordSize = filesOpened[channelNum].recordSize;
    if (filesOpened[channelNum].f == (std::FILE *) 0 ||
        filesOpened[channelNum].fileType != 'R' || recordSize < 1) {
      return false;
    }
    int     recordNum = filesOpened[channelNum].recordNum;
    long    filePos = long(recordNum) * recordSize;
    if (filePos >= filesOpened[channelNum].fileSize)
      return false;
    std::FILE *f = filesOpened[channelNum].f;
    std::fseek(f, filePos + 26L, SEEK_SET);
    if (std::ftell(f) != (filePos + 26L)) {
      setErrorMessage(27);              // "read error"
      return false;
    }
    size_t  nBytes = std::fread(&(recordData[0]),
                                sizeof(unsigned char), size_t(recordSize), f);
    if (nBytes < 1)
      return false;
    for (size_t i = nBytes; i < size_t(recordSize); i++)
      recordData[i] = 0x00;
    for (recordLength = recordSize - 1; recordLength > 0; recordLength--) {
      if (recordData[recordLength] != 0x00)
        break;
    }
    recordLength++;
    return true;
  }

  void ParallelIECDrive::flushRelativeFile()
  {
    if (!recordDirtyFlag)
      return;
    recordDirtyFlag = false;
    int     channelNum = int(secondaryAddress & 0x0F);
    int     recordSize = filesOpened[channelNum].recordSize;
    if (filesOpened[channelNum].f == (std::FILE *) 0 ||
        filesOpened[channelNum].fileType != 'R' || recordSize < 1) {
      setErrorMessage(64);              // "file type mismatch"
      recordLength = 0;
      return;
    }
    // pad record with zero bytes
    for (int i = recordLength; i <= recordSize; i++)
      recordData[i] = 0x00;
    filesOpened[channelNum].recordPos = 0;
    recordLength = 0;
    if (writeProtectFlag || filesOpened[channelNum].openMode == 'R') {
      setErrorMessage(26);              // "write protect on"
      return;
    }
    int     recordNum = filesOpened[channelNum].recordNum;
    if (recordNum >= 65535) {
      setErrorMessage(52);              // "file too large"
      return;
    }
    long    filePos = long(recordNum) * recordSize;
    if (filePos > filesOpened[channelNum].fileSize) {
      filesOpened[channelNum].recordPos = 0;    // reset record position
      if (!createRelativeFileRecord(channelNum))
        return;
    }
    std::FILE *f = filesOpened[channelNum].f;
    std::fseek(f, filePos + 26L, SEEK_SET);
    if (std::ftell(f) != (filePos + 26L)) {
      setErrorMessage(28);              // "write error"
      return;
    }
    size_t  bytesWritten = std::fwrite(&(recordData[0]), sizeof(unsigned char),
                                       size_t(recordSize), f);
    if ((filePos + long(bytesWritten)) > filesOpened[channelNum].fileSize)
      filesOpened[channelNum].fileSize = filePos + long(bytesWritten);
    if (bytesWritten != size_t(recordSize))
      setErrorMessage(72);              // "disk full"
    else
      filesOpened[channelNum].recordNum++;
  }

  bool ParallelIECDrive::createRelativeFileRecord(int channelNum)
  {
    if (writeProtectFlag) {
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    int     recordNum = filesOpened[channelNum].recordNum;
    int     recordSize = filesOpened[channelNum].recordSize;
    long&   fileSize = filesOpened[channelNum].fileSize;
    std::FILE *f = filesOpened[channelNum].f;
    std::fseek(f, fileSize + 26L, SEEK_SET);
    if (std::ftell(f) != (fileSize + 26L)) {
      setErrorMessage(28);              // "write error"
      return false;
    }
    long    fileSizeRequired = long(recordNum) * recordSize;
    while (fileSize < fileSizeRequired) {
      // pad file to required length
      int     c = 0x00;
      if ((fileSize % long(recordSize)) == 0L)
        c = 0xFF;
      if (std::fputc(c, f) == EOF) {
        setErrorMessage(72);            // "disk full"
        return false;
      }
      fileSize++;
    }
    return true;
  }

  int ParallelIECDrive::writeRelativeFile(uint8_t n)
  {
    int     channelNum = int(secondaryAddress & 0x0F);
    if (writeProtectFlag || filesOpened[channelNum].openMode == 'R') {
      setErrorMessage(26);              // "write protect on"
      return 0;
    }
    int     recordSize = filesOpened[channelNum].recordSize;
    if (recordSize < 1) {
      setErrorMessage(28);              // "write error"
      return 0;
    }
    if (filesOpened[channelNum].recordNum >= 65535) {
      setErrorMessage(52);              // "file too large"
      return 0;
    }
    if (int(filesOpened[channelNum].recordPos) >= recordSize) {
      flushRelativeFile();
      if (errorCode == 0 || errorCode == 50)
        setErrorMessage(51);            // "overflow in record"
    }
    recordDirtyFlag = true;
    recordData[filesOpened[channelNum].recordPos] = n;
    recordLength = int(filesOpened[channelNum].recordPos) + 1;
    if (errorCode != 51)
      filesOpened[channelNum].recordPos++;
    return 0;
  }

  int ParallelIECDrive::readRelativeFile(uint8_t& n)
  {
    int     channelNum = int(secondaryAddress & 0x0F);
    int     recordSize = filesOpened[channelNum].recordSize;
    n = 0x0D;
    if (recordSize < 1) {
      setErrorMessage(27);              // "read error"
      return 2;
    }
    if (filesOpened[channelNum].recordNum >= 65535 ||
        (long(filesOpened[channelNum].recordNum) * recordSize)
        >= filesOpened[channelNum].fileSize) {
      setErrorMessage(50);              // "record not present"
      return 2;
    }
    if (int(filesOpened[channelNum].recordPos) >= recordLength) {
      filesOpened[channelNum].recordPos = 0;
      filesOpened[channelNum].recordNum++;
      if (!readRelativeFileRecord(channelNum)) {
        setErrorMessage(50);            // "record not present"
        return 2;
      }
    }
    n = uint8_t(recordData[filesOpened[channelNum].recordPos++]);
    return (int(filesOpened[channelNum].recordPos) < recordLength ? 0 : 3);
  }

  int ParallelIECDrive::readDirectory(uint8_t& n)
  {
    if (bufPos >= bufBytes) {
      // load next directory entry
      bufPos = 0;
      bufBytes = 0;
      if (directoryIterator == fileDB.begin()) {
        for (int i = 0; i < 32; i++)
          buf[bufBytes++] = directoryStartLine[i];
        for (int i = 0; i < diskName.fileNameLen && i < 16; i++)
          buf[i + 8] = diskName.fileName[i];
        buf[0x1A] = diskID[0];
        buf[0x1B] = diskID[1];
      }
      while (directoryIterator != fileDB.end()) {
        long          fSize = 0L;
        const Plus4FileName&  fileName = (*directoryIterator).first;
        const Plus4FileName&  fName =
            filesOpened[secondaryAddress & 0x0F].fileName;
        if (fName.fileNameLen > 0) {
          // check if the file matches the specified name/wildcards
          bool    foundFile = false;
          for (int i = 0; true; i++) {
            if (i >= fName.fileNameLen) {
              foundFile = (i >= fileName.fileNameLen);
              break;
            }
            // '*': match zero or more characters,
            // ignore the rest of the file name
            if (fName.fileName[i] == 0x2A) {
              foundFile = true;
              break;
            }
            if (i >= fileName.fileNameLen)
              break;
            // '?': match exactly one character
            if (fName.fileName[i] != 0x3F) {
              if (fileName.fileName[i] != fName.fileName[i])
                break;
            }
          }
          if (!foundFile) {
            directoryIterator++;
            continue;
          }
        }
        FileDBEntry&  fileDBEntry = (*directoryIterator).second;
        std::FILE     *f =
            Plus4Emu::fileOpen(fileDBEntry.fullName.c_str(), "rb");
        if (f) {
          // get file size
          std::fseek(f, 0L, SEEK_END);
          fSize = std::ftell(f);
          std::fclose(f);
          f = (std::FILE *) 0;
        }
        if (fileDBEntry.fileType != 'p')
          fSize -= 26L;
        if (fSize < 0L)
          fSize = 0L;
        if (fSize > (254L * 65535L))
          fSize = 254L * 65535L;
        fSize = (fSize + 253L) / 254L;
        size_t  savedBufBytes = bufBytes;
        buf[bufBytes++] = 0x01;
        buf[bufBytes++] = 0x01;
        buf[bufBytes++] = (unsigned char) (fSize & 255L);
        buf[bufBytes++] = (unsigned char) (fSize >> 8);
        if (fSize < 1000L)
          buf[bufBytes++] = 0x20;
        if (fSize < 100L)
          buf[bufBytes++] = 0x20;
        if (fSize < 10L)
          buf[bufBytes++] = 0x20;
        buf[bufBytes++] = 0x22;
        for (int i = 0; i < fileName.fileNameLen; i++) {
          unsigned char c = fileName.fileName[i];
          if (c < 0x20)
            c = 0x20;
          buf[bufBytes++] = c;
        }
        buf[bufBytes++] = 0x22;
        for (int i = fileName.fileNameLen; i < 16; i++)
          buf[bufBytes++] = 0x20;
        buf[bufBytes] = 0x20;
        for (int i = 0; i < 16; i++) {
          if (filesOpened[i].f != (std::FILE *) 0 &&
              (filesOpened[i].openMode == 'A' ||
               filesOpened[i].openMode == 'W') &&
              filesOpened[i].fileName == fileName) {
            buf[bufBytes] = 0x2A;       // file is currently open for write
            break;
          }
        }
        bufBytes++;
        switch (fileDBEntry.fileType) {
        case 'R':
          buf[bufBytes++] = 0x52;       // 'R'
          buf[bufBytes++] = 0x45;       // 'E'
          buf[bufBytes++] = 0x4C;       // 'L'
          break;
        case 'S':
          buf[bufBytes++] = 0x53;       // 'S'
          buf[bufBytes++] = 0x45;       // 'E'
          buf[bufBytes++] = 0x51;       // 'Q'
          break;
        case 'U':
          buf[bufBytes++] = 0x55;       // 'U'
          buf[bufBytes++] = 0x53;       // 'S'
          buf[bufBytes++] = 0x52;       // 'R'
          break;
        default:
          buf[bufBytes++] = 0x50;       // 'P'
          buf[bufBytes++] = 0x52;       // 'R'
          buf[bufBytes++] = 0x47;       // 'G'
          break;
        }
        while (bufBytes < (savedBufBytes + 31))
          buf[bufBytes++] = 0x20;
        buf[bufBytes++] = 0x00;
        directoryIterator++;
        break;
      }
      if (directoryIterator == fileDB.end()) {
        for (int i = 0; i < 32; i++)
          buf[bufBytes++] = directoryEndLine[i];
      }
    }
    n = uint8_t(buf[bufPos++]);
    if (bufPos >= bufBytes) {
      bufPos = 0;
      bufBytes = 0;
      if (directoryIterator == fileDB.end())
        return 3;
    }
    return 0;
  }

  bool ParallelIECDrive::scratchFile(const Plus4FileName& fileName)
  {
    if (writeProtectFlag) {
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    if (fileName.fileNameLen < 1) {
      setErrorMessage(34);
      return false;
    }
    for (int i = 0; i < 16; i++) {
      // check if this file is currently opened
      if (filesOpened[i].f != (std::FILE *) 0 &&
          fileName == filesOpened[i].fileName) {
        setErrorMessage(60);
        return false;
      }
    }
    if (Plus4Emu::fileRemove(fileDB[fileName].fullName.c_str()) != 0) {
      fileDBUpdateFlag = true;
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    fileDB.erase(fileName);
    directoryIterator = fileDB.end();
    return true;
  }

  bool ParallelIECDrive::copyFile(const Plus4FileName& dstFileName,
                                  const Plus4FileName& srcFileName)
  {
    if (writeProtectFlag) {
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    if (dstFileName.fileNameLen < 1 || srcFileName.fileNameLen < 1) {
      setErrorMessage(34);              // missing filename
      return false;
    }
    for (int i = 0; i < dstFileName.fileNameLen; i++) {
      if (dstFileName.fileName[i] == 0x2A || dstFileName.fileName[i] == 0x3F) {
        setErrorMessage(30);            // wildcards are not allowed
        return false;
      }
    }
    for (int i = 0; i < srcFileName.fileNameLen; i++) {
      if (srcFileName.fileName[i] == 0x2A || srcFileName.fileName[i] == 0x3F) {
        setErrorMessage(30);            // wildcards are not allowed
        return false;
      }
    }
    bool    fileDBUpdated = false;
    if (findFile(srcFileName).fileNameLen < 1) {
      updateFileDB();
      fileDBUpdated = true;
    }
    if (findFile(srcFileName).fileNameLen < 1) {
      setErrorMessage(62);              // "file not found"
      return false;
    }
    if (findFile(dstFileName).fileNameLen > 0 && !fileDBUpdated) {
      updateFileDB();
      fileDBUpdated = true;
    }
    if (findFile(dstFileName).fileNameLen > 0) {
      setErrorMessage(63);              // "file exists"
      return false;
    }
    for (int i = 0; i < 16; i++) {
      // check for currently opened files:
      if (filesOpened[i].f != (std::FILE *) 0 &&
          (filesOpened[i].fileName == dstFileName ||
           (filesOpened[i].fileName == srcFileName &&
            (filesOpened[i].openMode == 'A' ||
             filesOpened[i].openMode == 'W')))) {
        setErrorMessage(60);            // "write file open"
        return false;
      }
    }
    if (dstFileName == srcFileName)     // nothing to do
      return true;
    FileTableEntry  dstFile;    // use FileTableEntry structures so that
    FileTableEntry  srcFile;    // the files are automatically closed on return
    srcFile.f = Plus4Emu::fileOpen(fileDB[srcFileName].fullName.c_str(), "rb");
    if (srcFile.f == (std::FILE *) 0) {
      fileDBUpdateFlag = !fileDBUpdated;
      setErrorMessage(62);              // "file not found"
      return false;
    }
    char    fileType = fileDB[srcFileName].fileType;
    if (fileType != 'p') {
      std::fseek(srcFile.f, 26L, SEEK_SET);
      if (std::ftell(srcFile.f) != 26L) {
        setErrorMessage(27);            // "read error"
        return false;
      }
    }
    else
      fileType = 'P';
    int     recordSize = fileDB[srcFileName].recordSize;
    int     err = createFile(dstFile.f, dstFileName, fileType, recordSize);
    if (err != 0) {
      // "disk full" or "write protect on"
      setErrorMessage(err == -2 ? 72 : 26);
      return false;
    }
    while (true) {
      int     c = std::fgetc(srcFile.f);
      if (c == EOF)
        break;
      c = c & 0xFF;
      if (std::fputc(c, dstFile.f) == EOF) {
        setErrorMessage(72);            // "disk full"
        return false;
      }
    }
    if (std::fflush(dstFile.f) != 0) {
      setErrorMessage(72);              // "disk full"
      return false;
    }
    return true;
  }

  bool ParallelIECDrive::appendFile(const Plus4FileName& dstFileName,
                                    const Plus4FileName& srcFileName)
  {
    if (writeProtectFlag) {
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    if (dstFileName.fileNameLen < 1 || srcFileName.fileNameLen < 1) {
      setErrorMessage(34);              // missing filename
      return false;
    }
    for (int i = 0; i < dstFileName.fileNameLen; i++) {
      if (dstFileName.fileName[i] == 0x2A || dstFileName.fileName[i] == 0x3F) {
        setErrorMessage(30);            // wildcards are not allowed
        return false;
      }
    }
    for (int i = 0; i < srcFileName.fileNameLen; i++) {
      if (srcFileName.fileName[i] == 0x2A || srcFileName.fileName[i] == 0x3F) {
        setErrorMessage(30);            // wildcards are not allowed
        return false;
      }
    }
    bool    fileDBUpdated = false;
    if (findFile(srcFileName).fileNameLen < 1 ||
        findFile(dstFileName).fileNameLen < 1) {
      updateFileDB();
      fileDBUpdated = true;
    }
    if (findFile(srcFileName).fileNameLen < 1 ||
        findFile(dstFileName).fileNameLen < 1) {
      setErrorMessage(62);              // "file not found"
      return false;
    }
    if (dstFileName == srcFileName) {
      setErrorMessage(63);              // "file exists"
      return false;
    }
    for (int i = 0; i < 16; i++) {
      // check for currently opened files:
      if (filesOpened[i].f != (std::FILE *) 0 &&
          (filesOpened[i].fileName == dstFileName ||
           (filesOpened[i].fileName == srcFileName &&
            (filesOpened[i].openMode == 'A' ||
             filesOpened[i].openMode == 'W')))) {
        setErrorMessage(60);            // "write file open"
        return false;
      }
    }
    FileTableEntry  dstFile;    // use FileTableEntry structures so that
    FileTableEntry  srcFile;    // the files are automatically closed on return
    srcFile.f = Plus4Emu::fileOpen(fileDB[srcFileName].fullName.c_str(), "rb");
    if (srcFile.f == (std::FILE *) 0) {
      fileDBUpdateFlag = !fileDBUpdated;
      setErrorMessage(62);              // "file not found"
      return false;
    }
    if (fileDB[srcFileName].fileType != 'p') {
      std::fseek(srcFile.f, 26L, SEEK_SET);
      if (std::ftell(srcFile.f) != 26L) {
        setErrorMessage(27);            // "read error"
        return false;
      }
    }
    dstFile.f = Plus4Emu::fileOpen(fileDB[dstFileName].fullName.c_str(), "ab");
    if (dstFile.f == (std::FILE *) 0) {
      fileDBUpdateFlag = !fileDBUpdated;
      setErrorMessage(26);              // "write protect on"
      return false;
    }
    while (true) {
      int     c = std::fgetc(srcFile.f);
      if (c == EOF)
        break;
      c = c & 0xFF;
      if (std::fputc(c, dstFile.f) == EOF) {
        setErrorMessage(72);            // "disk full"
        return false;
      }
    }
    if (std::fflush(dstFile.f) != 0) {
      setErrorMessage(72);              // "disk full"
      return false;
    }
    return true;
  }

}       // namespace Plus4

