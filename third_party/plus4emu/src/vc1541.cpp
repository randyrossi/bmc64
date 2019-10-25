
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
#include "cpu.hpp"
#include "via6522.hpp"
#include "serial.hpp"
#include "p4floppy.hpp"
#include "d64image.hpp"
#include "vc1541.hpp"

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

  VC1541::M7501_::M7501_(VC1541& vc1541_)
    : M7501(),
      vc1541(vc1541_)
  {
    // initialize memory map
    setMemoryCallbackUserData((void *) &vc1541_);
    for (uint16_t i = 0x0000; i <= 0x7FFF; i++) {
      switch (i & 0x1C00) {
      case 0x0000:
      case 0x0400:
        if (i < 0x0800) {
          setMemoryReadCallback(i, &VC1541::readMemory_RAM_0000_07FF);
          setMemoryWriteCallback(i, &VC1541::writeMemory_RAM_0000_07FF);
        }
        else {
          setMemoryReadCallback(i, &VC1541::readMemory_RAM);
          setMemoryWriteCallback(i, &VC1541::writeMemory_RAM);
        }
        break;
      case 0x1800:
        setMemoryReadCallback(i, &VC1541::readMemory_VIA1);
        setMemoryWriteCallback(i, &VC1541::writeMemory_VIA1);
        break;
      case 0x1C00:
        setMemoryReadCallback(i, &VC1541::readMemory_VIA2);
        setMemoryWriteCallback(i, &VC1541::writeMemory_VIA2);
        break;
      default:
        setMemoryReadCallback(i, &VC1541::readMemory_Dummy);
        setMemoryWriteCallback(i, &VC1541::writeMemory_Dummy);
        break;
      }
    }
    for (uint32_t i = 0x8000; i <= 0xFFFF; i++) {
      setMemoryReadCallback(uint16_t(i), &VC1541::readMemory_Dummy);
      setMemoryWriteCallback(uint16_t(i), &VC1541::writeMemory_Dummy);
    }
  }

  VC1541::M7501_::~M7501_()
  {
  }

  void VC1541::M7501_::breakPointCallback(int type,
                                          uint16_t addr, uint8_t value)
  {
    vc1541.breakPointCallback(vc1541.breakPointCallbackUserData,
                              (vc1541.deviceNumber & 3) + 1, type, addr, value);
  }

  VC1541::VIA6522_::VIA6522_(VC1541& vc1541_)
    : VIA6522(),
      vc1541(vc1541_),
      interruptFlag(false)
  {
  }

  VC1541::VIA6522_::~VIA6522_()
  {
  }

  void VC1541::VIA6522_::irqStateChangeCallback(bool newState)
  {
    interruptFlag = newState;
    vc1541.cpu.interruptRequest(vc1541.via1.interruptFlag
                                | vc1541.via2.interruptFlag);
  }

  // --------------------------------------------------------------------------

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_RAM_0000_07FF(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = vc1541.memory_ram[addr];
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_RAM(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = vc1541.memory_ram[addr & 0x07FF];
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_Dummy(
      void *userData, uint16_t addr)
  {
    (void) addr;
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_VIA1(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    uint8_t serialBusInput = uint8_t((vc1541.serialBus.getDATA() & 0x01)
                                     | (vc1541.serialBus.getCLK() & 0x04)
                                     | (vc1541.serialBus.getATN() & 0x80));
    vc1541.via1.setPortB(serialBusInput ^ vc1541.via1PortBInput);
    vc1541.dataBusState = vc1541.via1.readRegister(addr & 0x000F);
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_VIA2(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = vc1541.via2.readRegister(addr & 0x000F);
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_ROM_8000_BFFF(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = vc1541.memory_rom[addr | 0x4000];
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM2 uint8_t VC1541::readMemory_ROM_C000_FFFF(
      void *userData, uint16_t addr)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = vc1541.memory_rom[addr];
    return vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM3 void VC1541::writeMemory_RAM_0000_07FF(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = value & 0xFF;
    vc1541.memory_ram[addr] = vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM3 void VC1541::writeMemory_RAM(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = value & 0xFF;
    vc1541.memory_ram[addr & 0x07FF] = vc1541.dataBusState;
  }

  PLUS4EMU_REGPARM3 void VC1541::writeMemory_Dummy(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = value & 0xFF;
  }

  PLUS4EMU_REGPARM3 void VC1541::writeMemory_VIA1(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = value & 0xFF;
    addr = addr & 0x000F;
    vc1541.via1.writeRegister(addr, vc1541.dataBusState);
    vc1541.via1PortBOutputChangeFlag = true;
  }

  PLUS4EMU_REGPARM3 void VC1541::writeMemory_VIA2(
      void *userData, uint16_t addr, uint8_t value)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.dataBusState = value & 0xFF;
    addr = addr & 0x000F;
    vc1541.via2.writeRegister(addr, vc1541.dataBusState);
  }

  // --------------------------------------------------------------------------

  bool VC1541::updateMotors()
  {
    int     prvTrackPosFrac = currentTrackFrac;
    // 16 * (65536 / 128) cycles / 1000000 Hz = ~8.2 ms seek time
    currentTrackFrac = currentTrackFrac + (steppingDirection * 128);
    currentTrackFrac = currentTrackFrac & (~(int(127)));
    if (((currentTrackFrac ^ prvTrackPosFrac) & 0xC000) == 0x4000) {
      if (steppingDirection > 0)
        currentTrackStepperMotorPhase = (currentTrackStepperMotorPhase + 1) & 3;
      else
        currentTrackStepperMotorPhase = (currentTrackStepperMotorPhase + 3) & 3;
    }
    uint8_t stepperMotorPhase = via2.getPortB() & 3;
    switch ((stepperMotorPhase - currentTrackStepperMotorPhase) & 3) {
    case 1:
      steppingDirection = 1;    // stepping in
      break;
    case 3:
      steppingDirection = -1;   // stepping out
      break;
    default:                    // not stepping
      if (!(currentTrackFrac & 0x4000))
        steppingDirection = (!(currentTrackFrac & 0x7FFF) ? 0 : -1);
      else
        steppingDirection = 1;
      break;
    }
    if (currentTrackFrac <= -65536 || currentTrackFrac >= 65536) {
      // done stepping one track
      // FIXME: should report errors ?
      (void) setCurrentTrack(currentTrack + (currentTrackFrac > 0 ? 1 : -1));
    }
    if (diskChangeCnt) {
      diskChangeCnt--;
      if (diskChangeCnt == 0) {
        // write protect sense input is inverted for 0.25 seconds after
        // disk change
        via2PortBInput = uint8_t(writeProtectFlag ? (via2PortBInput & 0xEF)
                                                    : (via2PortBInput | 0x10));
        via2.setPortB(via2PortBInput);
        spindleMotorSpeed = 0;
      }
      return false;
    }
    // update spindle motor speed
    // spin up/down time is 16 * (65536 / 4) cycles / 1000000 Hz = ~262 ms
    if (!(via2.getPortB() & 0x04)) {
      spindleMotorSpeed = spindleMotorSpeed - 4;
      spindleMotorSpeed = (spindleMotorSpeed >= 0 ? spindleMotorSpeed : 0);
    }
    else {
      spindleMotorSpeed = spindleMotorSpeed + 4;
      spindleMotorSpeed =
          (spindleMotorSpeed < 65536 ? spindleMotorSpeed : 65536);
    }
    // return true if data can be read/written by the head
    return (currentTrackFrac == 0 && spindleMotorSpeed == 65536 &&
            (currentTrack >= 1 && currentTrack <= nTracks));
  }

  bool VC1541::setCurrentTrack(int trackNum)
  {
    currentTrackFrac = 0;
    int     oldTrackNum = currentTrack;
    bool    retval = D64Image::setCurrentTrack(trackNum);
    if (currentTrack != oldTrackNum) {
      // recalculate head position
      headPosition = (headPosition * trackSizeTable[trackNum])
                     / trackSizeTable[currentTrack];
    }
    return retval;
  }

  void VC1541::updateHead()
  {
    bool    syncFlag = false;
    if (via2.getCB2()) {
      // read mode
      uint8_t readByte = 0x00;
      if (headLoadedFlag) {
        readByte = trackBuffer_GCR[headPosition];
        if (readByte == 0xFF)
          syncFlag = prvByteWasFF;
      }
      prvByteWasFF = (readByte == 0xFF);
      via2.setPortA(readByte);
    }
    else {
      // write mode
      via2.setPortA(0xFF);
      if (headLoadedFlag && !writeProtectFlag) {
        trackDirtyFlag = true;
        trackBuffer_GCR[headPosition] = via2.getPortA();
      }
      prvByteWasFF = false;
    }
    via2PortBInput = uint8_t(syncFlag ? (via2PortBInput & 0x7F)
                                        : (via2PortBInput | 0x80));
    via2.setPortB(via2PortBInput);
    // set byte ready flag
    if (via2.getCA2() && !syncFlag) {
      cpu.setOverflowFlag();
      via2.setCA1(false);
    }
    // update head position
    if (spindleMotorSpeed >= 32768) {
      headPosition = headPosition + 1;
      if (headPosition >= trackSizeTable[currentTrack])
        headPosition = 0;
    }
  }

  // --------------------------------------------------------------------------

  VC1541::VC1541(SerialBus& serialBus_, int driveNum_)
    : FloppyDrive(serialBus_, driveNum_),
      D64Image(),
      cpu(*this),
      via1(*this),
      via2(*this),
      memory_rom((uint8_t *) 0),
      serialBusDelay(715827882U),
      deviceNumber(uint8_t(driveNum_)),
      dataBusState(0x00),
      via1PortBInput(0xFF),
      via1PortBOutputChangeFlag(true),
      halfCycleFlag(false),
      headLoadedFlag(false),
      prvByteWasFF(false),
      via2PortBInput(0xEF),
      motorUpdateCnt(0),
      shiftRegisterBitCnt(0),
      shiftRegisterBitCntFrac(0),
      headPosition(0),
      currentTrackFrac(0),
      steppingDirection(0),
      currentTrackStepperMotorPhase(0),
      spindleMotorSpeed(0),
      diskChangeCnt(15625),
      breakPointCallback(&defaultBreakPointCallback),
      breakPointCallbackUserData((void *) 0)
  {
    // clear RAM
    for (int i = 0; i < 2048; i++)
      memory_ram[i] = 0x00;
    // set device number
    via1PortBInput = uint8_t(0x9F | ((deviceNumber & 0x03) << 5));
    via1.setPortB(via1PortBInput);
    via1.setPortA(0xFE);
    via1.setCA1(!(serialBus.getATN()));
    via2.setPortB(0xEF);
    this->reset();
  }

  VC1541::~VC1541()
  {
  }

  void VC1541::setROMImage(int n, const uint8_t *romData_)
  {
    if (n == 2) {
      if (romData_ != (uint8_t *) 0 &&
          cpu.getMemoryReadCallback(0xC000) == &readMemory_Dummy) {
        for (uint32_t i = 0x8000U; i <= 0xBFFFU; i++)
          cpu.setMemoryReadCallback(uint16_t(i), &readMemory_ROM_8000_BFFF);
        for (uint32_t i = 0xC000U; i <= 0xFFFFU; i++)
          cpu.setMemoryReadCallback(uint16_t(i), &readMemory_ROM_C000_FFFF);
      }
      else if (romData_ == (uint8_t *) 0 &&
               cpu.getMemoryReadCallback(0xC000) != &readMemory_Dummy) {
        for (uint32_t i = 0x8000U; i <= 0xFFFFU; i++)
          cpu.setMemoryReadCallback(uint16_t(i), &readMemory_Dummy);
      }
      if (romData_ != (uint8_t *) 0)
        romData_ = romData_ - 0xC000;
      memory_rom = romData_;
    }
  }

  void VC1541::setDiskImageFile(std::FILE *imageFile_, bool isReadOnly)
  {
    headLoadedFlag = false;
    prvByteWasFF = false;
    spindleMotorSpeed = 0;
    diskChangeCnt = 15625;
    currentTrackStepperMotorPhase = 0;
    (void) setCurrentTrack(18);         // FIXME: should report errors ?
    via2PortBInput &= uint8_t(0xEF);
    via2.setPortB(via2PortBInput);
    D64Image::setImageFile(imageFile_, isReadOnly);
    // invert write protect sense input for 0.25 seconds so that the DOS can
    // detect the disk change
    via2PortBInput = uint8_t(writeProtectFlag ? (via2PortBInput | 0x10)
                                                : (via2PortBInput & 0xEF));
    via2.setPortB(via2PortBInput);
  }

  bool VC1541::haveDisk() const
  {
    return (imageFile != (std::FILE *) 0);
  }

  PLUS4EMU_REGPARM1 void VC1541::updateSerialBus()
  {
    via1PortBOutputChangeFlag = false;
    uint8_t via1PortBOutput = via1.getPortB();
    uint8_t atnAck_ = via1PortBOutput ^ ~(uint8_t(serialBus.getATN()));
    atnAck_ = uint8_t((atnAck_ & 0x10) | (via1PortBOutput & 0x02));
    serialBus.setCLKAndDATA(deviceNumber,
                            !(via1PortBOutput & 0x08), !(atnAck_));
  }

  PLUS4EMU_INLINE void VC1541::updateDrive()
  {
    int8_t  currentTrackSpeed = int8_t(trackSpeedTable[currentTrack]);
    shiftRegisterBitCntFrac += currentTrackSpeed;
    motorUpdateCnt -= currentTrackSpeed;
    if (motorUpdateCnt < 0) {
      motorUpdateCnt += 64;
      headLoadedFlag = updateMotors();
    }
    if (PLUS4EMU_UNLIKELY(shiftRegisterBitCnt >= 7)) {
      shiftRegisterBitCnt = 0;
      // read/write next byte
      updateHead();
    }
    else if (PLUS4EMU_UNLIKELY(++shiftRegisterBitCnt == 1)) {
      // clear byte ready flag
      via2.setCA1(true);
    }
  }

  PLUS4EMU_REGPARM1 void VC1541::processCallback(void *userData)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.timeRemaining += vc1541.serialBus.timesliceLength;
    if (PLUS4EMU_UNLIKELY(vc1541.timeRemaining < 0))
      return;
    do {
      vc1541.timeRemaining -= (int64_t(1) << 32);
      if (PLUS4EMU_UNLIKELY(vc1541.via1PortBOutputChangeFlag))
        vc1541.updateSerialBus();
      vc1541.via1.runOneCycle();
      vc1541.via2.runOneCycle();
      vc1541.cpu.runOneCycle_RDYHigh();
      vc1541.shiftRegisterBitCntFrac -= 4;
      if (PLUS4EMU_UNLIKELY(vc1541.shiftRegisterBitCntFrac < 0))
        vc1541.updateDrive();
    } while (PLUS4EMU_UNLIKELY(vc1541.timeRemaining >= 0));
  }

  PLUS4EMU_REGPARM1 void VC1541::processCallbackHighAccuracy(void *userData)
  {
    VC1541& vc1541 = *(reinterpret_cast<VC1541 *>(userData));
    vc1541.timeRemaining += (vc1541.serialBus.timesliceLength >> 1);
    if (PLUS4EMU_UNLIKELY(vc1541.timeRemaining < 0))
      return;
    do {
      if (!vc1541.halfCycleFlag) {
        // delay serial port output by ~833.3 ns
        vc1541.halfCycleFlag = true;
        if (PLUS4EMU_UNLIKELY(vc1541.via1PortBOutputChangeFlag))
          vc1541.updateSerialBus();
      }
      if (vc1541.timeRemaining >= int64_t(vc1541.serialBusDelay)) {
        vc1541.via1.runOneCycle();
        vc1541.via2.runOneCycle();
        vc1541.cpu.runOneCycle_RDYHigh();
        vc1541.shiftRegisterBitCntFrac -= 4;
        if (PLUS4EMU_UNLIKELY(vc1541.shiftRegisterBitCntFrac < 0))
          vc1541.updateDrive();
        vc1541.halfCycleFlag = false;
        vc1541.timeRemaining -= (int64_t(1) << 32);
      }
      else
        break;
    } while (PLUS4EMU_UNLIKELY(vc1541.timeRemaining >= 0));
  }

  SerialDevice::ProcessCallbackPtr VC1541::getProcessCallback()
  {
    return &processCallback;
  }

  SerialDevice::ProcessCallbackPtr VC1541::getHighAccuracyProcessCallback()
  {
    return &processCallbackHighAccuracy;
  }

  void VC1541::atnStateChangeCallback(bool newState)
  {
    via1PortBOutputChangeFlag = true;
    via1.setCA1(!newState);
  }

  void VC1541::reset()
  {
    (void) flushTrack();        // FIXME: should report errors ?
    via1.reset();
    via2.reset();
    cpu.reset();
    via1.setPortA(0xFE);
    via1PortBInput = uint8_t(0x9F | ((deviceNumber & 0x03) << 5));
    via1PortBOutputChangeFlag = true;
    via1.setPortB(via1PortBInput);
  }

  M7501 * VC1541::getCPU()
  {
    return (&cpu);
  }

  const M7501 * VC1541::getCPU() const
  {
    return (&cpu);
  }

  void VC1541::setBreakPointCallback(void (*breakPointCallback_)(
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

  uint8_t VC1541::readMemoryDebug(uint16_t addr) const
  {
    if (addr < 0x8000) {
      switch (addr & 0x1C00) {
      case 0x0000:
      case 0x0400:
        return memory_ram[addr & 0x07FF];
      case 0x1800:
        return via1.readRegisterDebug(addr & 0x000F);
      case 0x1C00:
        return via2.readRegisterDebug(addr & 0x000F);
      }
    }
    else if (cpu.getMemoryReadCallback(0xC000) != &readMemory_Dummy)
      return memory_rom[(addr & 0x3FFF) | 0xC000];
    return uint8_t(0xFF);
  }

  void VC1541::writeMemoryDebug(uint16_t addr, uint8_t value)
  {
    if (addr < 0x8000) {
      switch (addr & 0x1C00) {
      case 0x0000:
      case 0x0400:
        memory_ram[addr & 0x07FF] = value;
        break;
      case 0x1800:
        via1.writeRegister(addr & 0x000F, value);
        via1PortBOutputChangeFlag = true;
        break;
      case 0x1C00:
        via2.writeRegister(addr & 0x000F, value);
        break;
      }
    }
  }

  uint8_t VC1541::getLEDState() const
  {
    return uint8_t((via2.getPortB() & 0x08) >> 3);
  }

  uint16_t VC1541::getHeadPosition() const
  {
    if (!imageFile)
      return 0xFFFF;
    uint16_t  retval = uint16_t((currentTrack & 0x7F) << 8);
    // FIXME: this is not accurate
    retval |= uint16_t((headPosition / 367) & 0x7F);
    return retval;
  }

  void VC1541::setSerialBusDelayOffset(int n)
  {
    n = (n > -100 ? (n < 100 ? n : 100) : -100);
    serialBusDelay = uint32_t(715827882 - (n * 4294967));
  }

  void VC1541::saveState(Plus4Emu::File::Buffer& buf)
  {
    // TODO: implement this
    (void) buf;
  }

  void VC1541::saveState(Plus4Emu::File& f)
  {
    // TODO: implement this
    (void) f;
  }

  void VC1541::loadState(Plus4Emu::File::Buffer& buf)
  {
    // TODO: implement this
    (void) buf;
  }

  void VC1541::registerChunkTypes(Plus4Emu::File& f)
  {
    // TODO: implement this
    (void) f;
  }

}       // namespace Plus4

