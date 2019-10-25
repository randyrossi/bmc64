
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

#ifndef PLUS4EMU_P4FLOPPY_HPP
#define PLUS4EMU_P4FLOPPY_HPP

#include "plus4emu.hpp"
#include "fileio.hpp"
#include "serial.hpp"

namespace Plus4 {

  class FloppyDrive : public SerialDevice {
   public:
    FloppyDrive(SerialBus& serialBus_, int driveNum_ = 8)
      : SerialDevice(serialBus_)
    {
      (void) driveNum_;
    }
    virtual ~FloppyDrive()
    {
    }
    /*!
     * Use disk image file 'imageFile_' (imageFile_ == NULL means no disk).
     */
    virtual void setDiskImageFile(std::FILE *imageFile_, bool isReadOnly) = 0;
    /*!
     * Returns true if there is a disk image file opened.
     */
    virtual bool haveDisk() const = 0;
    /*!
     * Returns the current state of drive LEDs. Bit 0 is set if the red LED
     * is on, bit 1 is set if the green LED is on, and bit 2 is set if the
     * blue LED is on.
     */
    virtual uint8_t getLEDState() const = 0;
    /*!
     * Returns the current head position (track * 256 + sector), or 0xFFFF
     * if there is no disk. Bit 7 is set depending on which side is selected
     * (1581 only), and bit 15 is set if there are 80 tracks (i.e. emulating
     * the 1581).
     */
    virtual uint16_t getHeadPosition() const = 0;
  };

}       // namespace Plus4

#endif  // PLUS4EMU_P4FLOPPY_HPP

