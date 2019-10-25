
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2008 Istvan Varga <istvanv@users.sourceforge.net>
// http://sourceforge.net/projects/plus4emu/
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

#ifndef PLUS4EMU_IECDRIVE_HPP
#define PLUS4EMU_IECDRIVE_HPP

#include "plus4emu.hpp"
#include "system.hpp"
#include "vc1551.hpp"

#include <map>

namespace Plus4 {

  class ParallelIECDrive {
   protected:
    struct Plus4FileName {
      unsigned char fileName[16];
      int           fileNameLen;
      // --------
      Plus4FileName();
      Plus4FileName(const char *s);
      Plus4FileName(const std::string& s);
      Plus4FileName(const Plus4FileName& r);
      ~Plus4FileName();
      Plus4FileName& operator=(const char *s);
      Plus4FileName& operator=(const std::string& s);
      Plus4FileName& operator=(const Plus4FileName& r);
      operator std::string() const;
      bool operator==(const Plus4FileName& r) const;
      bool operator!=(const Plus4FileName& r) const;
      bool operator<(const Plus4FileName& r) const;
      void appendCharacter(char c);
      void appendPlus4Character(unsigned char c);
      void clear();
      static unsigned char convertCharacterToPlus4(char c);
      static char convertCharacterFromPlus4(unsigned char c);
    };
    struct FileDBEntry {
      std::string   fullName;   // the actual file name on the file system
      char          fileType;   // 'p': PRG, 'P': P00, 'R': R00,
                                // 'S': S00, 'U': U00
      unsigned char recordSize; // record size for REL files, zero otherwise
    };
    struct FileTableEntry {
      Plus4FileName fileName;
      std::FILE     *f;
      char          fileType;   // like above, but can also be '$' (directory),
                                // or '\0' if no file is opened
      char          openMode;   // 'A', 'M', 'R', 'W', or '\0' if not opened
      unsigned char recordSize; // record size for REL files, zero for others
      unsigned char recordPos;  // REL file position within record (from zero)
      int           recordNum;  // current REL file record number (from zero)
      long          fileSize;   // file size - 26 (for REL files only)
      FileTableEntry();
      ~FileTableEntry();
      void clear();             // close file and reset all struct members
    };
    TPI6523 tpi;
    int     deviceNumber;
    int     iecBusStatus;       // FEC1 bits 0 and 1
                                // 0: OK, 1: wr timeout, 2: rd timeout, 3: EOI
    uint8_t dataRegisterIn;     // data received from FEC0
    uint8_t dataRegisterOut;    // data sent to FEC0
    bool    handShake1;         // Plus/4 -> drive (FEC2 bit 6)
    bool    handShake2;         // drive -> Plus/4 (FEC2 bit 7)
    int     currentBusMode;     // 0: default / inactive
                                // 1: received byte 0x81 (talk/listen)
                                // 2: received byte 0x82 (tksa/second)
                                // 3: received byte 0x83 (ciout)
                                // 4: received byte 0x84 (acptr)
                                // 5: acptr end
    int     currentIOMode;      // 0: default / inactive
                                // 1: listen
                                // 2: talk
    uint8_t secondaryAddress;
    bool    recordDirtyFlag;    // true if data was written to this REL record
    bool    fileDBUpdateFlag;   // true if file database needs to be rebuilt
    bool    writeProtectFlag;   // do not allow any write operations if true
    std::map< Plus4FileName, FileDBEntry >  fileDB;
    std::string     currentWorkingDirectory;
    FileTableEntry  filesOpened[16];
    unsigned char   buf[256];
    size_t          bufPos;
    size_t          bufBytes;
    size_t          bufMaxBytes;
    char            errMsg[64];
    size_t          errMsgPos;
    size_t          errMsgBytes;
    std::map< Plus4FileName, FileDBEntry >::iterator  directoryIterator;
    int             errorCode;
    int             recordLength;
    unsigned char   recordData[256];
    Plus4FileName   diskName;
    unsigned char   diskID[2];
    Plus4Emu::Timer ledFlashTimer;
    // --------
    void updateParallelInterface();
    int listenNextByte(uint8_t n);
    int talkNextByte(uint8_t& n);
    void updateFileDB();
    // returns 0 on success, -1 if open failed, -2 on too many files
    int createFile(std::FILE*& f, const Plus4FileName& fileName, char fileType,
                   int recSize = 254);
    void addFileToDB(const Plus4FileName& fileName, const std::string& fullName,
                     char fileType, int recordSize);
    Plus4FileName findFile(const Plus4FileName& fileName);
    void setErrorMessage(int n, int t = 0, int s = 0);
    void openFile();
    void dosCommand();
    void closeFile();
    void openRelativeFile(const Plus4FileName& fileName, int recordSize = 0);
    bool readRelativeFileRecord(int channelNum);
    void flushRelativeFile();
    bool createRelativeFileRecord(int channelNum);
    int writeRelativeFile(uint8_t n);
    int readRelativeFile(uint8_t& n);
    int readDirectory(uint8_t& n);
    bool scratchFile(const Plus4FileName& fileName);
    bool copyFile(const Plus4FileName& dstFileName,
                  const Plus4FileName& srcFileName);
    bool appendFile(const Plus4FileName& dstFileName,
                    const Plus4FileName& srcFileName);
   public:
    ParallelIECDrive(int deviceNumber_ = 8);
    virtual ~ParallelIECDrive();
    virtual void reset();
    virtual void setReadOnlyMode(bool isReadOnly);
    virtual void setWorkingDirectory(const std::string& dirName_);
    virtual bool parallelIECRead(uint16_t addr, uint8_t& value);
    virtual bool parallelIECWrite(uint16_t addr, uint8_t value);
    /*!
     * Returns the current state of drive LEDs. Bit 0 is set if the red LED
     * is on, bit 1 is set if the green LED is on, and bit 2 is set if the
     * blue LED is on.
     */
    virtual uint8_t getLEDState();
  };

}       // namespace Plus4

#endif  // PLUS4EMU_IECDRIVE_HPP

