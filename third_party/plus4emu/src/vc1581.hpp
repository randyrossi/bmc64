
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

#ifndef PLUS4EMU_VC1581_HPP
#define PLUS4EMU_VC1581_HPP

#include "plus4emu.hpp"
#include "p4floppy.hpp"
#include "cpu.hpp"
#include "cia8520.hpp"
#include "wd177x.hpp"
#include "serial.hpp"

namespace Plus4 {

  class VC1581 : public FloppyDrive {
   private:
    class M7501_ : public M7501 {
     private:
      VC1581& vc1581;
     public:
      M7501_(VC1581& vc1581_);
      virtual ~M7501_();
     protected:
      virtual void breakPointCallback(int type, uint16_t addr, uint8_t value);
    };
    class CIA8520_ : public CIA8520 {
     private:
      VC1581& vc1581;
     public:
      CIA8520_(VC1581& vc1581_)
        : CIA8520(),
          vc1581(vc1581_)
      {
      }
      virtual ~CIA8520_();
     protected:
      virtual void interruptCallback(bool irqState);
    };
    M7501_      cpu;
    CIA8520_    cia;                    // 4000 to 43FF
    Plus4Emu::WD177x  wd177x;           // 6000 to 63FF
    const uint8_t *memory_rom_0;        // 8000 to BFFF
    const uint8_t *memory_rom_1;        // C000 to FFFF
    uint8_t     memory_ram[8192];       // 0000 to 1FFF
    int         deviceNumber;
    uint8_t     dataBusState;
    uint8_t     ciaPortAInput;
    uint8_t     ciaPortBInput;
    size_t      diskChangeCnt;
    void        (*breakPointCallback)(void *userData,
                                      int debugContext_, int type,
                                      uint16_t addr, uint8_t value);
    void        *breakPointCallbackUserData;
    // memory read/write callbacks
    static PLUS4EMU_REGPARM2 uint8_t readRAM(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readDummy(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readCIA8520(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readWD177x(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readROM0(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readROM1(void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeRAM(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeDummy(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeCIA8520(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeWD177x(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM1 void processCallback(void *userData);
   public:
    VC1581(SerialBus& serialBus_, int driveNum_ = 8);
    virtual ~VC1581();
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
     * Use disk image file 'imageFile_' (imageFile_ == NULL means no disk).
     */
    virtual void setDiskImageFile(std::FILE *imageFile_, bool isReadOnly);
    /*!
     * Returns true if there is a disk image file opened.
     */
    virtual bool haveDisk() const;
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
     * Reset floppy drive.
     */
    virtual void reset();
    /*!
     * Returns pointer to the drive CPU.
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
     * Read a byte from drive memory (used for debugging).
     */
    virtual uint8_t readMemoryDebug(uint16_t addr) const;
    /*!
     * Write a byte to drive memory (used for debugging).
     */
    virtual void writeMemoryDebug(uint16_t addr, uint8_t value);
    /*!
     * Returns the current state of drive LEDs. Bit 0 is set if the red LED
     * is on, bit 1 is set if the green LED is on, and bit 2 is set if the
     * blue LED is on.
     */
    virtual uint8_t getLEDState() const;
    /*!
     * Returns the current head position (track * 256 + sector), or 0xFFFF
     * if there is no disk. Bit 7 is set depending on which side is selected
     * (1581 only), and bit 15 is set if there are 80 tracks (i.e. emulating
     * the 1581).
     */
    virtual uint16_t getHeadPosition() const;
    // snapshot save/load functions
    virtual void saveState(Plus4Emu::File::Buffer&);
    virtual void saveState(Plus4Emu::File&);
    virtual void loadState(Plus4Emu::File::Buffer&);
    virtual void registerChunkTypes(Plus4Emu::File&);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_VC1581_HPP

