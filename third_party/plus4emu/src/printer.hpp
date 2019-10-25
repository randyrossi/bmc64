
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

#ifndef PLUS4EMU_PRINTER_HPP
#define PLUS4EMU_PRINTER_HPP

#include "plus4emu.hpp"
#include "serial.hpp"

namespace Plus4 {

  class Printer : public SerialDevice {
   public:
    Printer(SerialBus& serialBus_, int devNum_ = 4)
      : SerialDevice(serialBus_)
    {
      (void) devNum_;
    }
    virtual ~Printer()
    {
    }
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
    virtual void setROMImage(int n, const uint8_t *romData_)
    {
      (void) n;
      (void) romData_;
    }
    /*!
     * Returns the process function to be called at the time interval
     * determined by serialBus.timesliceLength.
     */
    virtual SerialDevice::ProcessCallbackPtr getProcessCallback() = 0;
    /*!
     * Called when the state of the ATN line changes.
     */
    virtual void atnStateChangeCallback(bool newState)
    {
      (void) newState;
    }
    /*!
     * Returns a pointer to the page data, which is encoded as an 8-bit
     * greyscale image. The data size is getPageWidth() * getPageHeight()
     * bytes. The buffer should not be changed or freed, and remains valid
     * until the next runOneCycle() or clearPage() call.
     */
    virtual const uint8_t *getPageData() const = 0;
    /*!
     * Returns the page width in pixels.
     */
    virtual int getPageWidth() const = 0;
    /*!
     * Returns the page height in pixels.
     */
    virtual int getPageHeight() const = 0;
    /*!
     * Clear page and set the head position at the top of the page.
     */
    virtual void clearPage()
    {
    }
    /*!
     * Returns the current state of the printer LED (0: off, 1: on).
     */
    virtual uint8_t getLEDState()
    {
      return uint8_t(0);
    }
    /*!
     * Returns the current position of the head. 'xPos' is in the range
     * 0 (left) to getPageWidth()-1 (right), 'yPos' is in the range 0 (top)
     * to getPageHeight()-1 (bottom).
     */
    virtual void getHeadPosition(int& xPos, int& yPos)
    {
      xPos = 0;
      yPos = 0;
    }
    /*!
     * Returns true if the page has changed since the last call of
     * clearOutputChangedFlag().
     */
    virtual bool getIsOutputChanged() const
    {
      return false;
    }
    virtual void clearOutputChangedFlag()
    {
    }
    /*!
     * Write the printer output to 'fileName'. If 'asciiMode' is false, then
     * the raw character data received on the serial bus is written, otherwise
     * it is converted to ASCII and written as a standard text file.
     * If 'fileName' is NULL or empty, then a previously opened file is closed.
     * On error, such as not being able to open the output file,
     * Plus4Emu::Exception may be thrown.
     */
    virtual void setTextOutputFile(const char *fileName,
                                   bool asciiMode = false)
    {
      (void) fileName;
      (void) asciiMode;
    }
    /*!
     * Reset printer.
     */
    virtual void reset()
    {
    }
    /*!
     * Returns pointer to the printer CPU.
     */
    virtual M7501 * getCPU()
    {
      return (M7501 *) 0;
    }
    virtual const M7501 * getCPU() const
    {
      return (M7501 *) 0;
    }
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
                                       void *userData_)
    {
      (void) breakPointCallback_;
      (void) userData_;
    }
    /*!
     * Read a byte from printer memory (used for debugging).
     */
    virtual uint8_t readMemoryDebug(uint16_t addr) const
    {
      (void) addr;
      return uint8_t(0xFF);
    }
    /*!
     * Write a byte to printer memory (used for debugging).
     */
    virtual void writeMemoryDebug(uint16_t addr, uint8_t value)
    {
      (void) addr;
      (void) value;
    }
  };

}       // namespace Plus4

#endif  // PLUS4EMU_PRINTER_HPP

