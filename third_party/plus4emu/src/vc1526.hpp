
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

#ifndef PLUS4EMU_VC1526_HPP
#define PLUS4EMU_VC1526_HPP

#include "plus4emu.hpp"
#include "serial.hpp"
#include "printer.hpp"
#include "cpu.hpp"
#include "via6522.hpp"
#include "riot6532.hpp"

namespace Plus4 {

  class VC1526 : public Printer {
   public:
    static const int    pageWidth = 700;
    static const int    pageHeight = 990;
    static const int    marginLeft = 36;
    static const int    marginRight = 24;
    static const int    marginTop = 30;
    static const int    marginBottom = 140;
   private:
    static const int    updatePinReload = 10;           // 100 kHz
    static const int    updateMotorReload = 100;        // 10 kHz
    class M6504_ : public M7501 {
     private:
      VC1526& vc1526;
     public:
      M6504_(VC1526& vc1526_);
      virtual ~M6504_();
     protected:
      virtual void breakPointCallback(int type, uint16_t addr, uint8_t value);
    };
    class VIA6522_ : public VIA6522 {
     private:
      VC1526& vc1526;
      bool    interruptFlag;
     public:
      VIA6522_(VC1526& vc1526_);
      virtual ~VIA6522_();
      virtual void irqStateChangeCallback(bool newState);
      inline bool getInterruptFlag() const
      {
        return interruptFlag;
      }
    };
    class RIOT6532_ : public RIOT6532 {
     private:
      VC1526& vc1526;
     public:
      RIOT6532_(VC1526& vc1526_);
      virtual ~RIOT6532_();
      virtual void irqStateChangeCallback(bool newState);
    };
    M6504_      cpu;            // NOTE: A8, A13, A14, and A15 are ignored
    VIA6522_    via;            // 0240..027F, 02C0..02FF
    RIOT6532_   riot1;          // I/O 0200..023F, RAM 0000..007F
    RIOT6532_   riot2;          // I/O 0280..02BF, RAM 0080..00FF
    const uint8_t *memory_rom;  // 8K ROM (0400..1FFF)
    int         deviceNumber;
    int         updatePinCnt;
    int         updateMotorCnt;
    int         headPosX;
    int         headPosY;
    int         motorXPhase;
    int         prvMotorXPhase;
    int         motorXCnt;
    int         motorYPhase;
    int         prvMotorYPhase;
    int         motorYCnt;
    bool        riot1PortAOutputChangeFlag;
    uint8_t     pinState;
    uint8_t     prvPinState;
    bool        changeFlag;
    uint8_t     *pageBuf;       // pageWidth * pageHeight bytes
    void        (*breakPointCallback)(void *userData,
                                      int debugContext_, int type,
                                      uint16_t addr, uint8_t value);
    void        *breakPointCallbackUserData;
    bool        outFileASCIIMode;
    std::FILE   *outFile;
    // --------
    static PLUS4EMU_REGPARM2 uint8_t readRIOT1RAM(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeRIOT1RAM(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM2 uint8_t readRIOT2RAM(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeRIOT2RAM(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeRIOT2RAMAndFile(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM2 uint8_t readMemoryROM(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeMemoryDummy(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM2 uint8_t readVIARegister(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeVIARegister(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM2 uint8_t readRIOT1Register(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeRIOT1Register(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM2 uint8_t readRIOT2Register(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeRIOT2Register(
        void *userData, uint16_t addr, uint8_t value);
    void updatePins();
    void updateMotors();
    static PLUS4EMU_REGPARM1 void processCallback(void *userData);
    static inline int yPosToPixel(int n)
    {
      return ((n * 7) / 18);
    }
    static inline int pixelToYPos(int n)
    {
      return ((n * 18) / 7);
    }
   public:
    VC1526(SerialBus& serialBus_, int devNum_ = 4);
    virtual ~VC1526();
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
     * Set printer mode (false: 1526, true: 1525).
     */
    virtual void setEnable1525Mode(bool isEnabled);
    /*!
     * Set the state of the form feed button (false: off, true: on).
     */
    virtual void setFormFeedOn(bool isEnabled);
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
    /*!
     * Returns pointer to the printer CPU.
     */
    virtual M7501 * getCPU();
    virtual const M7501 * getCPU() const;
    /*!
     * Set function to be called when a breakpoint is triggered.
     * 'type' can be one of the following values:
     *   0: breakpoint at opcode read
     *   1: memory read
     *   2: memory write
     *   3: opcode read in single step mode
     */
    virtual void setBreakPointCallback(void (*breakPointCallback_)(
                                           void *userData,
                                           int debugContext_, int type,
                                           uint16_t addr, uint8_t value),
                                       void *userData_);
    /*!
     * Read a byte from printer memory (used for debugging).
     */
    virtual uint8_t readMemoryDebug(uint16_t addr) const;
    /*!
     * Write a byte to printer memory (used for debugging).
     */
    virtual void writeMemoryDebug(uint16_t addr, uint8_t value);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_VC1526_HPP

