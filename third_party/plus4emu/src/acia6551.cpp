
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2007 Istvan Varga <istvanv@users.sourceforge.net>
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

#include "plus4emu.hpp"
#include "acia6551.hpp"

static const uint32_t aciaBaudRateTable[16] = {
     16U, 36864U, 24576U, 16769U,  13704U, 12288U,  6144U,  3072U,
   1536U,  1024U,   768U,   512U,    384U,   256U,   192U,    96U
};

namespace Plus4 {

  ACIA6551::ACIA6551()
  {
    this->reset();
  }

  ACIA6551::~ACIA6551()
  {
  }

  void ACIA6551::runHalfBit()
  {
    halfBitFlag = !halfBitFlag;
    cyclesRemaining = (cyclesPerBit + uint32_t(halfBitFlag)) >> 1;
    if (transmitContinuousMark && (statusRegister & 0x10) == 0x00)
      transmitState = 23;
    switch (transmitState) {
    case 0:                             // start bit
      transmitState++;
      break;
    case 1:
      switch (controlRegister & 0x60) {
      case 0x00:                // 8 data bits
        transmitState = 2;
        break;
      case 0x20:                // 7 data bits
        transmitState = 4;
        break;
      case 0x40:                // 6 data bits
        transmitState = 6;
        break;
      case 0x60:                // 5 data bits
        transmitState = 8;
        break;
      }
      break;
    case 2:                             // data bits
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      transmitState++;
      break;
    case 17:
      if (commandRegister & 0x20)
        transmitState++;
      else
        transmitState = 20;     // skip parity bit
      break;
    case 18:                            // parity bit
    case 19:
      transmitState++;
      break;
    case 20:                            // stop bit(s)
      {
        uint8_t tmp =
            uint8_t((controlRegister & 0xE0) | ((commandRegister & 0x20) >> 1));
        if ((tmp & 0x80) == 0x00 || tmp == 0x90)
          transmitState = 23;   // one stop bit
        else if (tmp == 0xE0)
          transmitState = 22;   // 1.5 stop bits
        else
          transmitState = 21;   // 2 stop bits
      }
      break;
    case 21:
    case 22:
      transmitState++;
      break;
    case 23:                            // end of stop bit
      transmitState = 0;
      transmitContinuousMark = bool(statusRegister & 0x10);
      statusRegister |= uint8_t(0x10);
      if ((commandRegister & 0x0C) == 0x04)
        statusRegister |= uint8_t(0x80);
      break;
    case 24:                            // restart with delay
      transmitState++;
      break;
    default:
      transmitState = 0;
      break;
    }
    if (controlRegister & 0x10) {
      // using baud rate as receiver clock
      switch (receiveState) {
      case 0:                           // start bit
        receiveState++;
        break;
      case 1:
        switch (controlRegister & 0x60) {
        case 0x00:              // 8 data bits
          receiveState = 2;
          break;
        case 0x20:              // 7 data bits
          receiveState = 4;
          break;
        case 0x40:              // 6 data bits
          receiveState = 6;
          break;
        case 0x60:              // 5 data bits
          receiveState = 8;
          break;
        }
        break;
      case 2:                           // data bits
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
      case 11:
      case 12:
      case 13:
      case 14:
      case 15:
      case 16:
        receiveState++;
        break;
      case 17:
        if (commandRegister & 0x20)
          receiveState++;
        else
          receiveState = 20;    // skip parity bit
        break;
      case 18:                          // parity bit
        receiveState++;
        break;
      case 19:
        if ((commandRegister & 0xC0) == 0x00) {
          // parity error
          statusRegister |= uint8_t(0x01);
        }
        receiveState++;
        break;
      case 20:                          // stop bit(s)
        {
          uint8_t tmp = uint8_t((controlRegister & 0xE0)
                                | ((commandRegister & 0x20) >> 1));
          if ((tmp & 0x80) == 0x00 || tmp == 0x90) {
            if (!(statusRegister & 0x02)) {
              // receive data register full, framing error
              statusRegister |= uint8_t(0x0A);
              if (!(commandRegister & 0x02))
                statusRegister |= uint8_t(0x80);
            }
            receiveState = 23;  // one stop bit
          }
          else if (tmp == 0xE0)
            receiveState = 22;  // 1.5 stop bits
          else
            receiveState = 21;  // 2 stop bits
        }
        break;
      case 21:
        receiveState++;
        break;
      case 22:
        if (!(statusRegister & 0x02)) {
          // receive data register full, framing error
          statusRegister |= uint8_t(0x0A);
          if (!(commandRegister & 0x02))
            statusRegister |= uint8_t(0x80);
        }
        receiveState++;
        break;
      case 23:                          // end of stop bit
        receiveState = 0;
        break;
      case 24:                          // restart with delay
        receiveState++;
        break;
      default:
        receiveState = 0;
        break;
      }
    }
  }

  uint8_t ACIA6551::readRegister(uint16_t addr)
  {
    switch (addr & 0x0003) {
    case 0x0000:                        // read receive data register
      statusRegister &= uint8_t(0xF7);
      return receiveDataRegister;
    case 0x0001:                        // read status register
      {
        uint8_t retval = statusRegister;
        statusRegister &= uint8_t(0x7F);
        return retval;
      }
      break;
    case 0x0002:                        // read command register
      return commandRegister;
    case 0x0003:                        // read control register
      return controlRegister;
    }
    return 0x00;        // not reached
  }

  uint8_t ACIA6551::readRegisterDebug(uint16_t addr) const
  {
    switch (addr & 0x0003) {
    case 0x0000:                        // read receive data register
      return receiveDataRegister;
    case 0x0001:                        // read status register
      return statusRegister;
    case 0x0002:                        // read command register
      return commandRegister;
    case 0x0003:                        // read control register
      return controlRegister;
    }
    return 0x00;        // not reached
  }

  void ACIA6551::writeRegister(uint16_t addr, uint8_t value)
  {
    switch (addr & 0x0003) {
    case 0x0000:                        // write transmit data register
      transmitDataRegister = value;
      statusRegister &= uint8_t(0xEF);
      break;
    case 0x0001:                        // program reset
      statusRegister &= uint8_t(0xFB);
      commandRegister &= uint8_t(0xE0);
      break;
    case 0x0002:                        // write command register
      commandRegister = value;
      break;
    case 0x0003:                        // write control register
      controlRegister = value;
      cyclesPerBit = aciaBaudRateTable[value & 0x0F];
      break;
    }
  }

  void ACIA6551::reset()
  {
    // hardware reset
    transmitDataRegister = 0x00;
    receiveDataRegister = 0x00;
    statusRegister = 0x10;
    commandRegister = 0x00;
    controlRegister = 0x00;
    cyclesRemaining = aciaBaudRateTable[0] >> 1;
    cyclesPerBit = aciaBaudRateTable[0];
    transmitState = 25;
    receiveState = 25;
    transmitContinuousMark = true;
    halfBitFlag = false;
  }

  void ACIA6551::saveSnapshot(uint8_t *buf)
  {
    buf[0] = transmitDataRegister;
    buf[1] = receiveDataRegister;
    buf[2] = statusRegister;
    buf[3] = commandRegister;
    buf[4] = controlRegister;
    buf[5] = uint8_t((cyclesRemaining >> 8) & 0xFFU);
    buf[6] = uint8_t(cyclesRemaining & 0xFFU);
    buf[7] = uint8_t(transmitState);
    buf[8] = uint8_t(receiveState);
    buf[9] = uint8_t(transmitContinuousMark);
    buf[10] = uint8_t(halfBitFlag);
  }

  void ACIA6551::loadSnapshot(const uint8_t *buf)
  {
    transmitDataRegister = buf[0];
    receiveDataRegister = 0x00;
    statusRegister = uint8_t(buf[2] & 0x9F);
    commandRegister = buf[3];
    controlRegister = buf[4];
    cyclesPerBit = aciaBaudRateTable[controlRegister & 0x0F];
    cyclesRemaining = uint32_t(buf[5]) << 8;
    cyclesRemaining |= uint32_t(buf[6]);
    if (cyclesRemaining == 0U || cyclesRemaining > aciaBaudRateTable[1])
      cyclesRemaining = cyclesPerBit >> 1;
    transmitState = int(buf[7]);
    receiveState = int(buf[8]);
    if (transmitState < 0 || transmitState > 25) {
      transmitState = 25;
      transmitContinuousMark = true;
    }
    else
      transmitContinuousMark = bool(buf[9]);
    if (receiveState < 0 || receiveState > 25)
      receiveState = 25;
    halfBitFlag = bool(buf[10]);
  }

}       // namespace Plus4

