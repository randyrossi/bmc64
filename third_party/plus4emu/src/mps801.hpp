
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

#ifndef PLUS4EMU_MPS801_HPP
#define PLUS4EMU_MPS801_HPP

#include "plus4emu.hpp"
#include "system.hpp"
#include "serial.hpp"
#include "printer.hpp"

namespace Plus4 {

  class MPS801 : public Printer {
   public:
    static const int    pageWidth = 520;
    static const int    pageHeight = 736;
    static const int    marginLeft = 20;
    static const int    marginRight = 20;
    static const int    marginTop = 32;
    static const int    marginBottom = 39;
   private:
    const uint8_t *memory_rom;  // 4K ROM
    int         deviceNumber;
    bool        idleMode;
    bool        atnState;
    bool        listenFlag;
    bool        pageFullFlag;
    uint8_t     serialBusState;
    uint8_t     secondaryAddress;
    uint8_t     shiftRegister;
    uint8_t     shiftRegisterBitCnt;
    bool        defaultLowerCaseMode;
    bool        lowerCaseMode;
    bool        bitmapMode;
    bool        doubleWidthMode;
    bool        reverseMode;
    bool        pixelPositionMode;      // set after CHR$(27)
    uint8_t     positionMSB;
    uint8_t     positionLSB;
    uint8_t     positionBytesRemaining; // decrements from 2 after CHR$(16)
    uint8_t     bitmapRepeatBytesRemaining;
    uint8_t     bitmapRepeatCnt;
    bool        eoiFlag;
    int         eoiTimer;
    int         headPosX;
    int         headPosY;
    size_t      inputBufferReadPos;
    size_t      inputBufferBytesUsed;
    size_t      inputBufferSize;
    uint8_t     *inputBuffer;
    uint8_t     *pageBuf;               // pageWidth * pageHeight bytes
    bool        changeFlag;
    bool        outFileASCIIMode;
    std::FILE   *outFile;
    Plus4Emu::Timer ledFlashTimer;
    // --------
    const uint8_t * getCharacterBitmap(uint8_t c);
    void lineFeed();
    void printBitmap(uint8_t b);
    void printCharacter(uint8_t c);
    bool processInputByte(uint8_t n, bool atnState_);
    void runOneCycle();
    static PLUS4EMU_REGPARM1 void processCallback(void *userData);
    void writeOutputFile(uint8_t c);
   public:
    MPS801(SerialBus& serialBus_, int devNum_ = 4);
    virtual ~MPS801();
    /*!
     * Use 'romData_' (should point to 16384 bytes of data which is expected
     * to remain valid until either a new address is set or the object is
     * destroyed, or can be NULL for no ROM data) for ROM bank 'n'; allowed
     * values for 'n' are:
     *   0: 1581 low
     *   1: 1581 high
     *   2: 1541
     *   3: 1551
     *   4: 1526/MPS-802 printer (data size is 8192 bytes)
     *   5: MPS-801 printer (data size is 4096 bytes)
     * if this device type does not use the selected ROM bank, the function
     * call is ignored.
     */
    virtual void setROMImage(int n, const uint8_t *romData_);
    /*!
     * Returns the process function to be called at the time interval
     * determined by serialBus.timesliceLength.
     */
    virtual SerialDevice::ProcessCallbackPtr getProcessCallback();
    /*!
     * Called when the state of the ATN line changes.
     */
    virtual void atnStateChangeCallback(bool newState);
    /*!
     * Returns a pointer to the page data, which is encoded as an 8-bit
     * greyscale image. The data size is getPageWidth() * getPageHeight()
     * bytes. The buffer should not be changed or freed, and remains valid
     * until the next runOneCycle() or clearPage() call.
     */
    virtual const uint8_t *getPageData() const;
    /*!
     * Returns the page width in pixels.
     */
    virtual int getPageWidth() const;
    /*!
     * Returns the page height in pixels.
     */
    virtual int getPageHeight() const;
    /*!
     * Clear page and set the head position at the top of the page.
     */
    virtual void clearPage();
    /*!
     * Returns the current state of the printer LED (0: off, 1: on).
     */
    virtual uint8_t getLEDState();
    /*!
     * Returns the current position of the head. 'xPos' is in the range
     * 0 (left) to getPageWidth()-1 (right), 'yPos' is in the range 0 (top)
     * to getPageHeight()-1 (bottom).
     */
    virtual void getHeadPosition(int& xPos, int& yPos);
    /*!
     * Returns true if the page has changed since the last call of
     * clearOutputChangedFlag().
     */
    virtual bool getIsOutputChanged() const;
    virtual void clearOutputChangedFlag();
    /*!
     * Write the printer output to 'fileName'. If 'asciiMode' is false, then
     * the raw character data received on the serial bus is written, otherwise
     * it is converted to ASCII and written as a standard text file.
     * If 'fileName' is NULL or empty, then a previously opened file is closed.
     * On error, such as not being able to open the output file,
     * Plus4Emu::Exception may be thrown.
     */
    virtual void setTextOutputFile(const char *fileName,
                                   bool asciiMode = false);
    /*!
     * Reset printer.
     */
    virtual void reset();
  };

}       // namespace Plus4

#endif  // PLUS4EMU_MPS801_HPP

