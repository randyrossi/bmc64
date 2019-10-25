
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
#include "serial.hpp"
#include "printer.hpp"
#include "cpu.hpp"
#include "via6522.hpp"
#include "riot6532.hpp"
#include "vc1526.hpp"
#include "system.hpp"

static void defaultBreakPointCallback(void *userData,
                                      int debugContext_, int type,
                                      uint16_t addr, uint8_t value)
{
  (void) userData;
  (void) debugContext_;
  (void) type;
  (void) addr;
  (void) value;
}

namespace Plus4 {

  VC1526::M6504_::M6504_(VC1526& vc1526_)
    : M7501(),
      vc1526(vc1526_)
  {
  }

  VC1526::M6504_::~M6504_()
  {
  }

  void VC1526::M6504_::breakPointCallback(int type,
                                          uint16_t addr, uint8_t value)
  {
    vc1526.breakPointCallback(vc1526.breakPointCallbackUserData,
                              5, type, addr, value);
  }

  VC1526::VIA6522_::VIA6522_(VC1526& vc1526_)
    : VIA6522(),
      vc1526(vc1526_),
      interruptFlag(false)
  {
  }

  VC1526::VIA6522_::~VIA6522_()
  {
  }

  void VC1526::VIA6522_::irqStateChangeCallback(bool newState)
  {
    interruptFlag = newState;
    vc1526.cpu.interruptRequest(vc1526.via.getInterruptFlag()
                                | vc1526.riot1.isInterruptRequest()
                                | vc1526.riot2.isInterruptRequest());
  }

  VC1526::RIOT6532_::RIOT6532_(VC1526& vc1526_)
    : RIOT6532(),
      vc1526(vc1526_)
  {
  }

  VC1526::RIOT6532_::~RIOT6532_()
  {
  }

  void VC1526::RIOT6532_::irqStateChangeCallback(bool newState)
  {
    (void) newState;
    vc1526.cpu.interruptRequest(vc1526.via.getInterruptFlag()
                                | vc1526.riot1.isInterruptRequest()
                                | vc1526.riot2.isInterruptRequest());
  }

  // --------------------------------------------------------------------------

  PLUS4EMU_REGPARM2 uint8_t VC1526::readRIOT1RAM(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    return vc1526.riot1.readMemory(addr);
  }

  PLUS4EMU_REGPARM3 void VC1526::writeRIOT1RAM(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.riot1.writeMemory(addr, value);
  }

  PLUS4EMU_REGPARM2 uint8_t VC1526::readRIOT2RAM(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    return vc1526.riot2.readMemory(addr);
  }

  PLUS4EMU_REGPARM3 void VC1526::writeRIOT2RAM(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.riot2.writeMemory(addr, value);
  }

  PLUS4EMU_REGPARM2 uint8_t VC1526::readMemoryROM(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    if (vc1526.memory_rom != (uint8_t *) 0)
      return vc1526.memory_rom[addr & 0x1FFF];
    return 0xFF;
  }

  PLUS4EMU_REGPARM3 void VC1526::writeMemoryDummy(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) userData;
    (void) addr;
    (void) value;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1526::readVIARegister(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    return vc1526.via.readRegister(addr);
  }

  PLUS4EMU_REGPARM3 void VC1526::writeVIARegister(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.via.writeRegister(addr, value);
  }

  PLUS4EMU_REGPARM2 uint8_t VC1526::readRIOT1Register(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    uint8_t riot1PortAInput = vc1526.riot1.getPortAInput() & 0x7C;
    riot1PortAInput = riot1PortAInput
                      | uint8_t(((vc1526.serialBus.getATN() & 0x01)
                                 | (vc1526.serialBus.getCLK() & 0x02)
                                 | (vc1526.serialBus.getDATA() & 0x80)) ^ 0x83);
    vc1526.riot1.setPortA(riot1PortAInput);
    return vc1526.riot1.readRegister(addr);
  }

  PLUS4EMU_REGPARM3 void VC1526::writeRIOT1Register(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.riot1.writeRegister(addr, value);
    vc1526.riot1PortAOutputChangeFlag = true;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1526::readRIOT2Register(
      void *userData, uint16_t addr)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    return vc1526.riot2.readRegister(addr);
  }

  PLUS4EMU_REGPARM3 void VC1526::writeRIOT2Register(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.riot2.writeRegister(addr, value);
  }

  // --------------------------------------------------------------------------

  VC1526::VC1526(SerialBus& serialBus_, int devNum_)
    : Printer(serialBus_, devNum_),
      cpu(*this),
      via(*this),
      riot1(*this),
      riot2(*this),
      memory_rom((uint8_t *) 0),
      deviceNumber(devNum_ & 7),
      updatePinCnt(0),
      updateMotorCnt(0),
      headPosX(marginLeft),
      headPosY(pixelToYPos(marginTop)),
      motorXPhase(0),
      prvMotorXPhase(0),
      motorXCnt(0),
      motorYPhase(0),
      prvMotorYPhase(0),
      motorYCnt(0),
      riot1PortAOutputChangeFlag(true),
      pinState(0x00),
      prvPinState(0x00),
      changeFlag(true),
      pageBuf((uint8_t *) 0),
      breakPointCallback(&defaultBreakPointCallback),
      breakPointCallbackUserData((void *) 0),
      outFileASCIIMode(false),
      outFile((std::FILE *) 0)
  {
    pageBuf = new uint8_t[pageWidth * pageHeight];
    for (size_t i = 0; i < size_t(pageWidth * pageHeight); i++)
      pageBuf[i] = 0xFF;        // clear to white
    cpu.setMemoryCallbackUserData((void *) this);
    for (uint16_t i = 0x0000; i <= 0x1FFF; i++) {
      M7501MemoryReadCallback   readCallback_ = &readMemoryROM;
      M7501MemoryWriteCallback  writeCallback_ = &writeMemoryDummy;
      if (i < 0x0200) {
        if (!(i & 0x0080)) {
          readCallback_ = &readRIOT1RAM;
          writeCallback_ = &writeRIOT1RAM;
        }
        else {
          readCallback_ = &readRIOT2RAM;
          writeCallback_ = &writeRIOT2RAM;
        }
      }
      else if (i < 0x0400) {
        if (i & 0x0040) {
          readCallback_ = &readVIARegister;
          writeCallback_ = &writeVIARegister;
        }
        else if (!(i & 0x0080)) {
          readCallback_ = &readRIOT1Register;
          writeCallback_ = &writeRIOT1Register;
        }
        else {
          readCallback_ = &readRIOT2Register;
          writeCallback_ = &writeRIOT2Register;
        }
      }
      for (uint32_t j = 0x0000U; j <= 0xE000U; j += 0x2000U) {
        cpu.setMemoryReadCallback(i | uint16_t(j), readCallback_);
        cpu.setMemoryWriteCallback(i | uint16_t(j), writeCallback_);
      }
    }
    this->reset();
  }

  VC1526::~VC1526()
  {
    if (outFile) {
      // FIXME: errors are ignored here
      if (outFileASCIIMode)
        std::fputc('\n', outFile);
      std::fflush(outFile);
      std::fclose(outFile);
    }
    delete[] pageBuf;
  }

  void VC1526::setROMImage(int n, const uint8_t *romData_)
  {
    if (n == 4)
      memory_rom = romData_;
  }

  void VC1526::updatePins()
  {
    pinState = riot2.getPortB() ^ 0xFF;
    pinState = pinState & (((via.getPortB() & 0x01) - 1) & 0xFF);
    if (pinState != prvPinState) {
      uint8_t tmp = (pinState ^ prvPinState) & pinState;
      if (tmp != 0x00 && headPosX >= 0 && headPosX < pageWidth) {
        changeFlag = true;
        int     headPosYInt = yPosToPixel(headPosY);
        for (int i = 0; i < 8; i++) {
          if (tmp & 0x80) {
            int     y = headPosYInt + i;
            if (y >= 0 && y < pageHeight) {
              int     n = (y * pageWidth) + headPosX;
              pageBuf[n] = (pageBuf[n] >> 3) + (pageBuf[n] >> 4);
            }
          }
          tmp = tmp << 1;
        }
      }
      prvPinState = pinState;
    }
  }

  void VC1526::updateMotors()
  {
    uint8_t viaPortAOutput = via.getPortA();
    int     newMotorXPhase = motorXPhase;
    if (riot2.getPortA() & 0x02) {
      switch (viaPortAOutput & 0x0F) {
      case 0x05:
        newMotorXPhase = 0;
        break;
      case 0x06:
        newMotorXPhase = 1;
        break;
      case 0x0A:
        newMotorXPhase = 2;
        break;
      case 0x09:
        newMotorXPhase = 3;
        break;
      }
    }
    int     newMotorYPhase = motorYPhase;
    if (riot2.getPortA() & 0x01) {
      switch (viaPortAOutput & 0xF0) {
      case 0x90:
        newMotorYPhase = 0;
        break;
      case 0xA0:
        newMotorYPhase = 1;
        break;
      case 0x60:
        newMotorYPhase = 2;
        break;
      case 0x50:
        newMotorYPhase = 3;
        break;
      }
    }
    if (newMotorXPhase != motorXPhase) {
      motorXPhase = newMotorXPhase;
      if (motorXPhase != prvMotorXPhase)
        motorXCnt = 3;
    }
    if (newMotorYPhase != motorYPhase) {
      motorYPhase = newMotorYPhase;
      if (motorYPhase != prvMotorYPhase)
        motorYCnt = 3;
    }
    if (motorXCnt) {
      motorXCnt--;
      if (!motorXCnt) {
        switch ((motorXPhase - prvMotorXPhase) & 3) {
        case 1:
          headPosX = (headPosX < (pageWidth - 1) ? (headPosX + 1) : headPosX);
          break;
        case 3:
          headPosX = (headPosX > 0 ? (headPosX - 1) : headPosX);
          break;
        }
        prvMotorXPhase = motorXPhase;
      }
    }
    if (motorYCnt) {
      motorYCnt--;
      if (!motorYCnt) {
        switch ((motorYPhase - prvMotorYPhase) & 3) {
        case 1:
          headPosY = (headPosY < pixelToYPos(pageHeight - 1) ?
                      (headPosY + 1) : headPosY);
          break;
        case 3:
          headPosY = (headPosY > 0 ? (headPosY - 1) : headPosY);
          break;
        }
        prvMotorYPhase = motorYPhase;
      }
    }
    if (motorXCnt != 0) {
      via.setCA1(true);
      riot2.setPortA(riot2.getPortAInput() | 0x40);
    }
    else {
      via.setCA1(false);
      riot2.setPortA(riot2.getPortAInput() & 0xBF);
    }
    if (headPosX <= marginLeft) {
      via.setCA2(false);
      riot2.setPortA(riot2.getPortAInput() & 0x7F);
    }
    else {
      via.setCA2(true);
      riot2.setPortA(riot2.getPortAInput() | 0x80);
    }
    if (headPosY < pixelToYPos(pageHeight - marginBottom))
      riot2.setPortA(riot2.getPortAInput() | 0x04);
    else
      riot2.setPortA(riot2.getPortAInput() & 0xFB);
  }

  PLUS4EMU_REGPARM1 void VC1526::processCallback(void *userData)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    vc1526.timeRemaining += vc1526.serialBus.timesliceLength;
    while (vc1526.timeRemaining >= 0) {
      vc1526.timeRemaining -= (int64_t(1) << 32);
      if (vc1526.riot1PortAOutputChangeFlag) {
        vc1526.riot1PortAOutputChangeFlag = false;
        bool    dataOut = !(vc1526.riot1.getPortA() & 0x40);
        bool    atnAck = !(vc1526.riot1.getPortA() & 0x20);
        atnAck = atnAck && !(vc1526.serialBus.getATN());
        vc1526.serialBus.setDATA(vc1526.deviceNumber, dataOut && !atnAck);
        uint8_t riot1PortAInput = vc1526.riot1.getPortAInput() | 0x80;
        riot1PortAInput = riot1PortAInput ^ (vc1526.serialBus.getDATA() & 0x80);
        vc1526.riot1.setPortA(riot1PortAInput);
      }
      vc1526.via.runOneCycle();
      vc1526.cpu.runOneCycle_RDYHigh();
      vc1526.riot1.runOneCycle();
      vc1526.riot2.runOneCycle();
      if (!vc1526.updatePinCnt) {
        vc1526.updatePinCnt = vc1526.updatePinReload;
        if (vc1526.updateMotorCnt <= 0) {
          vc1526.updateMotorCnt =
              vc1526.updateMotorCnt + vc1526.updateMotorReload;
          vc1526.updateMotors();
        }
        vc1526.updateMotorCnt = vc1526.updateMotorCnt - vc1526.updatePinReload;
        vc1526.updatePins();
      }
      vc1526.updatePinCnt--;
    }
  }

  SerialDevice::ProcessCallbackPtr VC1526::getProcessCallback()
  {
    return &processCallback;
  }

  void VC1526::atnStateChangeCallback(bool newState)
  {
    (void) newState;
    riot1PortAOutputChangeFlag = true;
  }

  const uint8_t * VC1526::getPageData() const
  {
    return pageBuf;
  }

  int VC1526::getPageWidth() const
  {
    return pageWidth;
  }

  int VC1526::getPageHeight() const
  {
    return pageHeight;
  }

  void VC1526::clearPage()
  {
    changeFlag = true;
    for (size_t i = 0; i < size_t(pageWidth * pageHeight); i++)
      pageBuf[i] = 0xFF;        // clear to white
    headPosY = pixelToYPos(marginTop);
  }

  uint8_t VC1526::getLEDState()
  {
    return uint8_t(((riot2.getPortA() ^ 0xFF) & 0x20) >> 5);
  }

  void VC1526::getHeadPosition(int& xPos, int& yPos)
  {
    xPos = headPosX;
    yPos = yPosToPixel(headPosY);
  }

  bool VC1526::getIsOutputChanged() const
  {
    return changeFlag;
  }

  void VC1526::clearOutputChangedFlag()
  {
    changeFlag = false;
  }

  void VC1526::setEnable1525Mode(bool isEnabled)
  {
    if (isEnabled)
      riot1.setPortB(riot1.getPortBInput() & 0x7F);
    else
      riot1.setPortB(riot1.getPortBInput() | 0x80);
  }

  void VC1526::setFormFeedOn(bool isEnabled)
  {
    if (isEnabled)
      riot2.setPortA(riot2.getPortAInput() & 0xF7);
    else
      riot2.setPortA(riot2.getPortAInput() | 0x08);
  }

  void VC1526::setTextOutputFile(const char *fileName, bool asciiMode)
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
        // reset memory write callbacks
        for (uint16_t i = 0x0080; i <= 0x00FF; i++) {
          cpu.setMemoryWriteCallback(i, &writeRIOT2RAM);
          cpu.setMemoryWriteCallback(i | uint16_t(0x0100), &writeRIOT2RAM);
        }
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
    // set memory write callbacks
    for (uint16_t i = 0x0080; i <= 0x00FF; i++) {
      cpu.setMemoryWriteCallback(i, &writeRIOT2RAMAndFile);
      cpu.setMemoryWriteCallback(i | uint16_t(0x0100), &writeRIOT2RAMAndFile);
    }
  }

  void VC1526::reset()
  {
    via.reset();
    riot1.reset();
    riot2.reset();
    cpu.reset();
    riot1.setPortB((riot1.getPortBInput() & 0xF8) | uint8_t(deviceNumber & 3));
    riot1PortAOutputChangeFlag = true;
  }

  M7501 * VC1526::getCPU()
  {
    return (&cpu);
  }

  const M7501 * VC1526::getCPU() const
  {
    return (&cpu);
  }

  void VC1526::setBreakPointCallback(void (*breakPointCallback_)(
                                         void *userData,
                                         int debugContext_, int type,
                                         uint16_t addr, uint8_t value),
                                     void *userData_)
  {
    if (breakPointCallback_)
      breakPointCallback = breakPointCallback_;
    else
      breakPointCallback = &defaultBreakPointCallback;
    breakPointCallbackUserData = userData_;
  }

  uint8_t VC1526::readMemoryDebug(uint16_t addr) const
  {
    if ((addr & 0x1FFF) < 0x0400) {
      switch (addr & 0x02C0) {
      case 0x0000:
      case 0x0040:
        return riot1.readMemory(addr);
      case 0x0080:
      case 0x00C0:
        return riot2.readMemory(addr);
      case 0x0200:
        return riot1.readRegisterDebug(addr);
      case 0x0240:
        return via.readRegisterDebug(addr);
      case 0x0280:
        return riot2.readRegisterDebug(addr);
      case 0x02C0:
        return via.readRegisterDebug(addr);
      }
    }
    if (memory_rom)
      return memory_rom[addr & 0x1FFF];
    return uint8_t(0xFF);
  }

  void VC1526::writeMemoryDebug(uint16_t addr, uint8_t value)
  {
    if ((addr & 0x1FFF) >= 0x0400)
      return;
    switch (addr & 0x02C0) {
    case 0x0000:
    case 0x0040:
      riot1.writeMemory(addr, value);
      break;
    case 0x0080:
    case 0x00C0:
      riot2.writeMemory(addr, value);
      break;
    case 0x0200:
      riot1.writeRegister(addr, value);
      riot1PortAOutputChangeFlag = true;
      break;
    case 0x0240:
      via.writeRegister(addr, value);
      break;
    case 0x0280:
      riot2.writeRegister(addr, value);
      break;
    case 0x02C0:
      via.writeRegister(addr, value);
      break;
    }
  }

  PLUS4EMU_REGPARM3 void VC1526::writeRIOT2RAMAndFile(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1526& vc1526 = *(reinterpret_cast<VC1526 *>(userData));
    addr = addr & 0x007F;
    vc1526.riot2.writeMemory(addr, value);
    if (!(addr >= 0x0013 && addr <= 0x0067))
      return;
    if (value != vc1526.riot1.readMemory(0x0064))
      return;
    {
      M7501Registers  r;
      vc1526.cpu.getRegisters(r);
      if (!(r.reg_PC >= 0xF4BE && r.reg_PC <= 0xF4C5))
        return;
    }
    if (!vc1526.outFile) {
      // file is closed: reset memory write callbacks, and return
      for (uint16_t i = 0x0080; i <= 0x00FF; i++) {
        vc1526.cpu.setMemoryWriteCallback(i, &VC1526::writeRIOT2RAM);
        vc1526.cpu.setMemoryWriteCallback(i | uint16_t(0x0100),
                                          &VC1526::writeRIOT2RAM);
      }
      return;
    }
    if (vc1526.outFileASCIIMode) {
      // convert character to ASCII format
      if ((value >= 0x41 && value <= 0x5A) || (value >= 0x61 && value <= 0x7A))
        value = value ^ 0x20;   // swap upper/lower case
      else if (value == 0x0A || value == 0x0D)
        value = 0x0A;           // CR -> LF
      else if (value == 0x09 || value == 0xA0)
        value = 0x20;           // tab -> space
      else if (value < 0x20 || (value >= 0x80 && value <= 0x9F))
        return;                 // strip any other non-printable characters
      else if (!(value >= 0x20 && value <= 0x5F && value != 0x5C))
        value = 0x5F;           // replace invalid characters with underscores
    }
    // FIXME: errors are ignored here
    std::fputc(value, vc1526.outFile);
  }

}       // namespace Plus4

