
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
#include "serial.hpp"
#include "printer.hpp"
#include "mps801.hpp"

#include <cmath>

namespace Plus4 {

  MPS801::MPS801(SerialBus& serialBus_, int devNum_)
    : Printer(serialBus_, devNum_),
      memory_rom((uint8_t *) 0),
      deviceNumber(devNum_ & 7),
      idleMode(false),
      atnState(true),
      listenFlag(false),
      pageFullFlag(false),
      serialBusState(0),
      secondaryAddress(0x00),
      shiftRegister(0x00),
      shiftRegisterBitCnt(0),
      defaultLowerCaseMode(false),
      lowerCaseMode(false),
      bitmapMode(false),
      doubleWidthMode(false),
      reverseMode(false),
      pixelPositionMode(false),
      positionMSB(0x00),
      positionLSB(0x00),
      positionBytesRemaining(0),
      bitmapRepeatBytesRemaining(0),
      bitmapRepeatCnt(0),
      eoiFlag(false),
      eoiTimer(0),
      headPosX(marginLeft),
      headPosY(marginTop),
      inputBufferReadPos(0),
      inputBufferBytesUsed(0),
      inputBufferSize(131072),
      inputBuffer((uint8_t *) 0),
      pageBuf((uint8_t *) 0),
      changeFlag(true),
      outFileASCIIMode(false),
      outFile((std::FILE *) 0)
  {
    inputBuffer = new uint8_t[inputBufferSize];
    for (size_t i = 0; i < inputBufferSize; i++)
      inputBuffer[i] = 0x00;
    try {
      pageBuf = new uint8_t[pageWidth * pageHeight];
    }
    catch (...) {
      delete[] inputBuffer;
      throw;
    }
    for (size_t i = 0; i < size_t(pageWidth * pageHeight); i++)
      pageBuf[i] = 0xFF;        // clear to white
    this->reset();
  }

  MPS801::~MPS801()
  {
    if (outFile) {
      // FIXME: errors are ignored here
      if (outFileASCIIMode)
        std::fputc('\n', outFile);
      std::fflush(outFile);
      std::fclose(outFile);
    }
    delete[] inputBuffer;
    delete[] pageBuf;
  }

  void MPS801::setROMImage(int n, const uint8_t *romData_)
  {
    if (n == 5)
      memory_rom = romData_;
  }

  const uint8_t * MPS801::getCharacterBitmap(uint8_t c)
  {
    if (memory_rom != (uint8_t *) 0) {
      if (!lowerCaseMode) {
        switch (c & 0xE0) {
        case 0x20:                      // space - ?
          return &(memory_rom[0x0800 | (int(c & 0x1F) * 6)]);
        case 0x40:                      // upper case
          return &(memory_rom[0x0900 | (int(c & 0x1F) * 6)]);
        case 0x60:                      // graphics 2
          return &(memory_rom[0x0B00 | (int(c & 0x1F) * 6)]);
        case 0xA0:                      // graphics 1
          return &(memory_rom[0x0A00 | (int(c & 0x1F) * 6)]);
        case 0xC0:                      // graphics 2
          return &(memory_rom[0x0B00 | (int(c & 0x1F) * 6)]);
        case 0xE0:                      // graphics 1
          return &(memory_rom[0x0A00 | (int(c & 0x1F) * 6)]);
        }
      }
      else {
        switch (c & 0xE0) {
        case 0x20:                      // space - ?
          return &(memory_rom[0x0800 | (int(c & 0x1F) * 6)]);
        case 0x40:                      // lower case
          return &(memory_rom[0x0D00 | (int(c & 0x1F) * 6)]);
        case 0x60:                      // upper case
          return &(memory_rom[0x0900 | (int(c & 0x1F) * 6)]);
        case 0xA0:                      // graphics 1
          return &(memory_rom[0x0A00 | (int(c & 0x1F) * 6)]);
        case 0xC0:                      // upper case
          return &(memory_rom[0x0900 | (int(c & 0x1F) * 6)]);
        case 0xE0:                      // graphics 1
          return &(memory_rom[0x0A00 | (int(c & 0x1F) * 6)]);
        }
      }
    }
    return (uint8_t *) 0;               // non-printable
  }

  void MPS801::lineFeed()
  {
    if (!bitmapMode)
      headPosY = headPosY + (((headPosY - marginTop) % 21) < 10 ? 10 : 11);
    else
      headPosY = headPosY + 7;
    if (headPosY > (pageHeight - marginBottom))
      pageFullFlag = true;
  }

  void MPS801::printBitmap(uint8_t b)
  {
    changeFlag = true;
    if (headPosX >= (pageWidth - marginRight)) {
      headPosX = marginLeft;
      lineFeed();
    }
    for (int i = 0; i < 7; i++) {
      if (b & uint8_t(1 << i)) {
        int     y = headPosY + i;
        if (headPosX >= 0 && headPosX < pageWidth &&
            y >= 0 && y < pageHeight) {
          int     n = (y * pageWidth) + headPosX;
          pageBuf[n] = (pageBuf[n] >> 3) + (pageBuf[n] >> 4);
        }
      }
    }
    headPosX++;
  }

  void MPS801::printCharacter(uint8_t c)
  {
    const uint8_t *bp = getCharacterBitmap(c);
    if (bp == (uint8_t *) 0)
      return;
    for (int i = 0; i < 6; i++) {
      for (int j = 0; j <= int(doubleWidthMode); j++)
        printBitmap(bp[i] ^ uint8_t((-(int(reverseMode))) & 0xFF));
    }
  }

  bool MPS801::processInputByte(uint8_t n, bool atnState_)
  {
    if (!atnState_) {
      if (!listenFlag) {
        if (n != uint8_t(0x20 | deviceNumber)) {
          serialBusState = 0;           // not LISTEN to this device
          serialBus.setDATA(deviceNumber, true);
        }
        else {
          listenFlag = true;
          secondaryAddress = 0x00;
        }
      }
      else if (secondaryAddress == 0x00) {
        secondaryAddress = n | 0x60;    // store secondary address
        defaultLowerCaseMode = ((secondaryAddress & 0x0F) == 7);
        lowerCaseMode = defaultLowerCaseMode;
      }
      else if (n == 0x3F) {
        listenFlag = false;             // UNLSN received
        secondaryAddress = 0x00;
      }
      return true;
    }
    // data byte:
    if (positionBytesRemaining > 1) {
      positionMSB = n;
      positionBytesRemaining--;
      return true;
    }
    if (positionBytesRemaining > 0) {
      positionLSB = n;
      if (!pixelPositionMode) {
        headPosX =
            ((int(positionMSB & 0x0F) * 10) + int(positionLSB & 0x0F)) * 6;
      }
      else {
        headPosX = (int(positionMSB & 0x01) << 8) | int(positionLSB);
      }
      headPosX = headPosX + marginLeft;
      headPosX = (headPosX > marginLeft ?
                  (headPosX < (pageWidth - marginRight) ?
                   headPosX : (pageWidth - marginRight))
                  : marginLeft);
      pixelPositionMode = false;
      positionMSB = 0x00;
      positionLSB = 0x00;
      positionBytesRemaining = 0;
      return true;
    }
    if (bitmapRepeatBytesRemaining > 1) {
      bitmapRepeatCnt = n;
      bitmapRepeatBytesRemaining--;
      return true;
    }
    if (bitmapRepeatBytesRemaining > 0) {
      bitmapRepeatBytesRemaining = 0;
      if (bitmapMode) {
        while (bitmapRepeatCnt > 0) {
          printBitmap(n);
          bitmapRepeatCnt--;
        }
        return true;
      }
      bitmapRepeatCnt = 0;
    }
    if (pixelPositionMode) {
      if (n != 0x10)
        pixelPositionMode = false;
    }
    switch (n) {
    case 0x08:                          // enable bitmap mode
      lowerCaseMode = defaultLowerCaseMode;
      bitmapMode = true;
      doubleWidthMode = false;
      reverseMode = false;
      return true;
    case 0x0E:                          // enable double width mode
      if (!bitmapMode)
        doubleWidthMode = true;
      return true;
    case 0x0F:                          // restore normal character mode
      bitmapMode = false;
      doubleWidthMode = false;
      return true;
    case 0x10:                          // set head position
      positionMSB = 0x00;
      positionLSB = 0x00;
      positionBytesRemaining = 2;
      return true;
    case 0x11:                          // lower case mode
      if (!bitmapMode)
        lowerCaseMode = true;
      return true;
    case 0x12:                          // enable reverse mode
      if (!bitmapMode)
        reverseMode = true;
      return true;
    case 0x1A:                          // repeat bitmap
      bitmapRepeatBytesRemaining = uint8_t(bitmapMode) + 1;
      bitmapRepeatCnt = 0;
      return true;
    case 0x1B:                          // set head position in pixels
      pixelPositionMode = true;
      return true;
    case 0x91:                          // upper case mode
      if (!bitmapMode) {
        lowerCaseMode = false;
        return true;
      }
      break;
    case 0x92:                          // disable reverse mode
      if (!bitmapMode) {
        reverseMode = false;
        return true;
      }
      break;
    }
    if (bitmapMode && (n < 0x80 && n != 0x0A && n != 0x0D))
      return true;                      // ignore invalid bytes in bitmap mode
    if (n == 0x0A && headPosX > marginLeft)
      return true;
    if (pageFullFlag)                   // if the page is already full:
      return false;                     // cannot print this character
    if (n == 0x0A) {                    // line feed
      lineFeed();
    }
    else if (n == 0x0D) {               // carriage return
      lowerCaseMode = defaultLowerCaseMode;
      doubleWidthMode = false;
      reverseMode = false;
      headPosX = marginLeft;
      lineFeed();
    }
    else if (!bitmapMode) {             // character mode
      printCharacter(n);
    }
    else {                              // bitmap mode: print single bitmap
      printBitmap(n);
    }
    return true;
  }

  void MPS801::runOneCycle()
  {
    if (inputBufferBytesUsed > 0) {
      // if there is buffered data, process that first
      do {
        if (processInputByte(inputBuffer[inputBufferReadPos], true)) {
          if (++inputBufferReadPos >= inputBufferSize)
            inputBufferReadPos = 0;
          inputBufferBytesUsed--;
        }
        else {
          pageFullFlag = true;
          break;
        }
      } while (inputBufferBytesUsed > 0);
    }
    if (atnState && !listenFlag)
      serialBusState = 0;
    switch (serialBusState) {
    case 0:                             // idle (waiting for input)
      if (!atnState) {
        idleMode = false;
        serialBusState = 1;
        serialBus.setDATA(deviceNumber, false);
      }
      else {
        idleMode = (inputBufferBytesUsed < 1);
      }
      break;
    case 1:                             // waiting for CLK = high before byte
      if (serialBus.getCLK() != 0) {
        if (inputBufferBytesUsed < inputBufferSize) {
          serialBusState = 2;
          eoiFlag = false;
          eoiTimer = 224;               // EOI after 250 us
          shiftRegister = 0x00;
          shiftRegisterBitCnt = 0;
          serialBus.setDATA(deviceNumber, true);
        }
      }
      break;
    case 2:                             // waiting for CLK = low before 1st bit
      if (--eoiTimer < 0) {
        eoiFlag = true;
        serialBusState = 3;
        eoiTimer = 72;
        serialBus.setDATA(deviceNumber, false);
      }
      else if (serialBus.getCLK() == 0) {
        eoiFlag = false;
        serialBusState = 4;
      }
      break;
    case 3:                             // EOI handshake (80 us)
      eoiTimer--;
      if (eoiTimer < 0) {
        serialBus.setDATA(deviceNumber, true);
        if (serialBus.getCLK() == 0) {  // wait for CLK = low before first bit
          serialBusState = 4;
        }
        else if (eoiTimer < -179) {
          // if the talker does not respond after 200 us, reset serial bus I/O
          idleMode = false;
          atnState = bool(serialBus.getATN());
          listenFlag = false;
          serialBusState = 0;
          secondaryAddress = 0x00;
          shiftRegister = 0x00;
          shiftRegisterBitCnt = 0;
          eoiFlag = false;
          eoiTimer = 0;
        }
      }
      break;
    case 4:                             // wait for CLK = high before next bit
      if (serialBus.getCLK() != 0) {
        shiftRegister = (shiftRegister >> 1) & 0x7F;
        shiftRegister |= uint8_t(serialBus.getDATA() & 0x80);
        shiftRegisterBitCnt++;
        serialBusState = 5;
      }
      break;
    case 5:                             // wait for CLK = low after input bit
      if (serialBus.getCLK() == 0) {
        if (shiftRegisterBitCnt < 8) {
          serialBusState = 4;           // continue with next bit
        }
        else {                          // all bits have been received:
          if (!eoiFlag)
            serialBusState = 1;         // continue with next byte
          else
            serialBusState = 0;         // EOI, wait for ATN
          uint8_t c = shiftRegister;
          eoiTimer = 0;
          shiftRegister = 0x00;
          shiftRegisterBitCnt = 0;
          serialBus.setDATA(deviceNumber, false);
          if (atnState) {
#if 0
            if ((secondaryAddress & 0xF0) != 0x60 &&
                (secondaryAddress & 0xF0) != 0xF0) {
              break;                    // not listening
            }
#endif
            if (outFile != (std::FILE *) 0)
              writeOutputFile(c);       // write printer output to text file
            if (!pageFullFlag) {
              if (processInputByte(c, true))
                break;
              pageFullFlag = true;
            }
            // page is full, store input data in buffer
            if (inputBufferBytesUsed < inputBufferSize) {
              size_t  writePos = inputBufferReadPos + inputBufferBytesUsed;
              if (writePos >= inputBufferSize)
                writePos -= inputBufferSize;
              inputBuffer[writePos] = c;
              inputBufferBytesUsed++;
            }
          }
          else {
            processInputByte(c, false);
          }
        }
      }
      break;
    }
  }

  PLUS4EMU_REGPARM1 void MPS801::processCallback(void *userData)
  {
    MPS801& mps801 = *(reinterpret_cast<MPS801 *>(userData));
    if (mps801.idleMode)
      return;
    mps801.runOneCycle();
  }

  void MPS801::writeOutputFile(uint8_t c)
  {
    if (!outFile)
      return;                   // file is closed
    if (outFileASCIIMode) {
      // convert character to ASCII format
      if ((c >= 0x41 && c <= 0x5A) || (c >= 0x61 && c <= 0x7A))
        c = c ^ 0x20;           // swap upper/lower case
      else if (c == 0x0A || c == 0x0D)
        c = 0x0A;               // CR -> LF
      else if (c == 0x09 || c == 0xA0)
        c = 0x20;               // tab -> space
      else if (c < 0x20 || (c >= 0x80 && c <= 0x9F))
        return;                 // strip any other non-printable characters
      else if (!(c >= 0x20 && c <= 0x5F && c != 0x5C))
        c = 0x5F;               // replace invalid characters with underscores
    }
    // FIXME: errors are ignored here
    std::fputc(c, outFile);
  }

  SerialDevice::ProcessCallbackPtr MPS801::getProcessCallback()
  {
    return &processCallback;
  }

  void MPS801::atnStateChangeCallback(bool newState)
  {
    atnState = newState;
    if (!atnState)
      idleMode = false;
  }

  const uint8_t * MPS801::getPageData() const
  {
    return pageBuf;
  }

  int MPS801::getPageWidth() const
  {
    return pageWidth;
  }

  int MPS801::getPageHeight() const
  {
    return pageHeight;
  }

  void MPS801::clearPage()
  {
    changeFlag = true;
    for (size_t i = 0; i < size_t(pageWidth * pageHeight); i++)
      pageBuf[i] = 0xFF;        // clear to white
    headPosX = marginLeft;
    headPosY = marginTop;
    pageFullFlag = false;
  }

  uint8_t MPS801::getLEDState()
  {
    if (!pageFullFlag)
      return 1;
    double  t = ledFlashTimer.getRealTime();
    if (inputBufferBytesUsed < inputBufferSize) {
      if (t >= 0.5) {
        t = std::fmod(t, 0.5);
        ledFlashTimer.reset(t);
      }
      return uint8_t(t < 0.25);
    }
    if (t >= 0.25) {
      t = std::fmod(t, 0.25);
      ledFlashTimer.reset(t);
    }
    return uint8_t(t < 0.125);
  }

  void MPS801::getHeadPosition(int& xPos, int& yPos)
  {
    xPos = headPosX;
    yPos = headPosY;
  }

  bool MPS801::getIsOutputChanged() const
  {
    return changeFlag;
  }

  void MPS801::clearOutputChangedFlag()
  {
    changeFlag = false;
  }

  void MPS801::setTextOutputFile(const char *fileName, bool asciiMode)
  {
    if (fileName == (char *) 0 || fileName[0] == '\0') {
      // if closing output file:
      if (outFile) {
        bool    err = false;
        if (outFileASCIIMode)
          err = (std::fputc('\n', outFile) == EOF);
        if (std::fflush(outFile) != 0)
          err = true;
        if (std::fclose(outFile) != 0)
          err = true;
        outFile = (std::FILE *) 0;
        outFileASCIIMode = false;
        if (err)
          throw Plus4Emu::Exception("error closing printer output file");
      }
      return;
    }
    if (outFile)
      setTextOutputFile((char *) 0);    // close old output file first
    std::FILE *f = Plus4Emu::fileOpen(fileName, (asciiMode ? "w" : "wb"));
    if (!f)
      throw Plus4Emu::Exception("error opening printer output file");
    outFile = f;
    outFileASCIIMode = asciiMode;
  }

  void MPS801::reset()
  {
    idleMode = false;
    atnState = bool(serialBus.getATN());
    listenFlag = false;
    serialBusState = 0;
    secondaryAddress = 0x00;
    shiftRegister = 0x00;
    shiftRegisterBitCnt = 0;
    defaultLowerCaseMode = false;
    lowerCaseMode = false;
    bitmapMode = false;
    doubleWidthMode = false;
    reverseMode = false;
    pixelPositionMode = false;
    positionMSB = 0x00;
    positionLSB = 0x00;
    positionBytesRemaining = 0;
    bitmapRepeatBytesRemaining = 0;
    bitmapRepeatCnt = 0;
    eoiFlag = false;
    eoiTimer = 0;
    headPosX = marginLeft;
    ledFlashTimer.reset();
  }

}       // namespace Plus4

