
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

#ifndef PLUS4EMU_SERIAL_HPP
#define PLUS4EMU_SERIAL_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"

namespace Plus4 {

  class M7501;

  class SerialBus {
   public:
    // The time interval at which the process callback function of each
    // serial device is called, in 2^-32 microsecond units. This variable is
    // written by the virtual machine class, and read by the serial devices.
    int64_t   timesliceLength;
   private:
    uint16_t  clkStateMask;
    uint16_t  dataStateMask;
    uint8_t   atnState;
   public:
    SerialBus()
      : timesliceLength(int64_t(1) << 32),
        clkStateMask(0x0000),
        dataStateMask(0x0000),
        atnState(0xFF)
    {
    }
    // returns the current state of the CLK line (0: low, 0xFF: high)
    inline uint8_t getCLK() const
    {
      return (uint8_t(bool(clkStateMask)) - uint8_t(1));
    }
    // returns the current state of the DATA line (0: low, 0xFF: high)
    inline uint8_t getDATA() const
    {
      return (uint8_t(bool(dataStateMask)) - uint8_t(1));
    }
    // returns the current state of the ATN line (0: low, 0xFF: high)
    inline uint8_t getATN() const
    {
      return atnState;
    }
    // set the CLK output (false: low, true: high) for device 'n' (0 to 15)
    inline void setCLK(int n, bool newState)
    {
      clkStateMask =
          (clkStateMask | (uint16_t(1) << n)) ^ (uint16_t(newState) << n);
    }
    // set the DATA output (false: low, true: high) for device 'n' (0 to 15)
    inline void setDATA(int n, bool newState)
    {
      dataStateMask =
          (dataStateMask | (uint16_t(1) << n)) ^ (uint16_t(newState) << n);
    }
    // set the CLK and DATA output (false: low, true: high)
    // for device 'n' (0 to 15)
    inline void setCLKAndDATA(int n, bool newCLKState, bool newDATAState)
    {
      uint16_t  mask_ = uint16_t(1) << n;
      clkStateMask = (clkStateMask | mask_) ^ (uint16_t(newCLKState) << n);
      dataStateMask = (dataStateMask | mask_) ^ (uint16_t(newDATAState) << n);
    }
    // set the state of the ATN line (false: low, true: high)
    inline void setATN(bool newState)
    {
      atnState = uint8_t(-(int8_t(newState)));
    }
    // remove device 'n' (0 to 15) from the bus, setting its outputs to high
    inline void removeDevice(int n)
    {
      uint16_t  mask_ = (uint16_t(1) << n) ^ uint16_t(0xFFFF);
      clkStateMask &= mask_;
      dataStateMask &= mask_;
    }
    // remove devices defined by 'mask_' (bit N of 'mask_' corresponds to
    // device N) from the bus
    inline void removeDevices(uint16_t mask_)
    {
      clkStateMask &= (mask_ ^ uint16_t(0xFFFF));
      dataStateMask &= (mask_ ^ uint16_t(0xFFFF));
    }
  };

  class SerialDevice {
   protected:
    SerialBus&  serialBus;
    int64_t     timeRemaining;
   public:
    typedef PLUS4EMU_REGPARM1 void (*ProcessCallbackPtr)(void *);
    // --------
    SerialDevice(SerialBus& serialBus_)
      : serialBus(serialBus_),
        timeRemaining(0)
    {
    }
    virtual ~SerialDevice()
    {
    }
    /*!
     * Returns the pointer that should be passed to the process callback.
     */
    virtual void * getProcessCallbackUserData()
    {
      return (void *) this;
    }
    /*!
     * Returns the process function to be called at the time interval
     * determined by serialBus.timesliceLength.
     */
    virtual ProcessCallbackPtr getProcessCallback()
    {
      return (ProcessCallbackPtr) 0;
    }
    /*!
     * Returns pointer to an optional process function that is called
     * at twice the normal clock frequency, allowing for more accurate
     * emulation.
     */
    virtual ProcessCallbackPtr getHighAccuracyProcessCallback()
    {
      return (ProcessCallbackPtr) 0;
    }
    /*!
     * Called when the state of the ATN line changes.
     */
    virtual void atnStateChangeCallback(bool newState)
    {
      (void) newState;
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
     * Reset serial device.
     */
    virtual void reset()
    {
    }
    /*!
     * Returns pointer to the CPU of the device, or NULL if there is no CPU.
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
     * Read a byte from memory (used for debugging).
     */
    virtual uint8_t readMemoryDebug(uint16_t addr) const
    {
      (void) addr;
      return uint8_t(0xFF);
    }
    /*!
     * Write a byte to memory (used for debugging).
     */
    virtual void writeMemoryDebug(uint16_t addr, uint8_t value)
    {
      (void) addr;
      (void) value;
    }
    // snapshot save/load functions
    virtual void saveState(Plus4Emu::File::Buffer& buf)
    {
      (void) buf;
    }
    virtual void saveState(Plus4Emu::File& f)
    {
      (void) f;
    }
    virtual void loadState(Plus4Emu::File::Buffer& buf)
    {
      (void) buf;
    }
    virtual void registerChunkTypes(Plus4Emu::File& f)
    {
      (void) f;
    }
  };

}       // namespace Plus4

#endif  // PLUS4EMU_SERIAL_HPP

