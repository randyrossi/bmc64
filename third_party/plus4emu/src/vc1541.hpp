
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

#ifndef PLUS4EMU_VC1541_HPP
#define PLUS4EMU_VC1541_HPP

#include "plus4emu.hpp"
#include "cpu.hpp"
#include "via6522.hpp"
#include "serial.hpp"
#include "p4floppy.hpp"
#include "d64image.hpp"

namespace Plus4 {

  class VC1541 : public FloppyDrive, public D64Image {
   private:
    class M7501_ : public M7501 {
     private:
      VC1541& vc1541;
     public:
      M7501_(VC1541& vc1541_);
      virtual ~M7501_();
     protected:
      virtual void breakPointCallback(int type, uint16_t addr, uint8_t value);
    };
    class VIA6522_ : public VIA6522 {
     private:
      VC1541& vc1541;
      bool    interruptFlag;
     public:
      VIA6522_(VC1541& vc1541_);
      virtual ~VIA6522_();
      virtual void irqStateChangeCallback(bool newState);
    };
    M7501_      cpu;
    VIA6522_    via1;                   // serial port interface (1800..1BFF)
    VIA6522_    via2;                   // floppy control (1C00..1FFF)
    const uint8_t *memory_rom;          // 16K ROM, 8000..FFFF
    uint8_t     memory_ram[2048];       // 2K RAM, 0000..07FF
    uint32_t    serialBusDelay;
    uint8_t     deviceNumber;
    uint8_t     dataBusState;
    uint8_t     via1PortBInput;
    bool        via1PortBOutputChangeFlag;
    bool        halfCycleFlag;
    bool        headLoadedFlag;
    bool        prvByteWasFF;           // for finding sync
    uint8_t     via2PortBInput;         // bit 7: /SYNC, bit 4: /WPS
    int8_t      motorUpdateCnt;         // 63 to 0, motor update at 62.5 kHz
    uint8_t     shiftRegisterBitCnt;    // 0 to 7, byte ready on 0
    int8_t      shiftRegisterBitCntFrac;    // track speed (13..16) - 1 to 0
    int         headPosition;           // index to track buffer
    int         currentTrackFrac;       // -65536 to 65536 (-32768 and 32768
                                        // are "half tracks")
    int         steppingDirection;      // 1: stepping in, -1: stepping out,
                                        // 0: not stepping
    int         currentTrackStepperMotorPhase;
    int         spindleMotorSpeed;      // 0 (stopped) to 65536 (full speed)
    int         diskChangeCnt;          // decrements from 15625 to 0
    void        (*breakPointCallback)(void *userData,
                                      int debugContext_, int type,
                                      uint16_t addr, uint8_t value);
    void        *breakPointCallbackUserData;
    // ----------------
    static PLUS4EMU_REGPARM2 uint8_t readMemory_RAM_0000_07FF(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_RAM(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_Dummy(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_VIA1(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_VIA2(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_ROM_8000_BFFF(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM2 uint8_t readMemory_ROM_C000_FFFF(
        void *userData, uint16_t addr);
    static PLUS4EMU_REGPARM3 void writeMemory_RAM_0000_07FF(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeMemory_RAM(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeMemory_Dummy(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeMemory_VIA1(
        void *userData, uint16_t addr, uint8_t value);
    static PLUS4EMU_REGPARM3 void writeMemory_VIA2(
        void *userData, uint16_t addr, uint8_t value);
    bool updateMotors();
    void updateHead();
    PLUS4EMU_REGPARM1 void updateSerialBus();
    PLUS4EMU_INLINE void updateDrive();
    static PLUS4EMU_REGPARM1 void processCallback(void *userData);
    static PLUS4EMU_REGPARM1 void processCallbackHighAccuracy(void *userData);
   protected:
    virtual bool setCurrentTrack(int trackNum);
   public:
    VC1541(SerialBus& serialBus_, int driveNum_ = 8);
    virtual ~VC1541();
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
     * Returns pointer to an optional process function that is called
     * at twice the normal clock frequency, allowing for more accurate
     * emulation.
     */
    virtual SerialDevice::ProcessCallbackPtr getHighAccuracyProcessCallback();
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
    /*!
     * Set the serial bus delay offset to 'n' (-100 to 100) nanoseconds.
     */
    virtual void setSerialBusDelayOffset(int n);
    // snapshot save/load functions
    virtual void saveState(Plus4Emu::File::Buffer&);
    virtual void saveState(Plus4Emu::File&);
    virtual void loadState(Plus4Emu::File::Buffer&);
    virtual void registerChunkTypes(Plus4Emu::File&);
  };

}       // namespace Plus4

#endif  // PLUS4EMU_VC1541_HPP

