
// plus4emu -- portable Commodore Plus/4 emulator
// Copyright (C) 2003-2018 Istvan Varga <istvanv@users.sourceforge.net>
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
#include "display.hpp"
#include "soundio.hpp"
#include "plus4vm.hpp"
#include "disasm.hpp"
#include "videorec.hpp"

#include <cmath>
#include <vector>
#include <typeinfo>

#include "resid/sid.hpp"
#include "p4floppy.hpp"
#include "mps801.hpp"
#include "vc1526.hpp"
#include "vc1541.hpp"
#include "vc1551.hpp"
#include "vc1581.hpp"
#include "iecdrive.hpp"
#include "system.hpp"
#include "charconv.hpp"

static void writeDemoTimeCnt(Plus4Emu::File::Buffer& buf, uint64_t n)
{
  uint64_t  mask = uint64_t(0x7F) << 49;
  uint8_t   rshift = 49;
  while (rshift != 0 && !(n & mask)) {
    mask >>= 7;
    rshift -= 7;
  }
  while (rshift != 0) {
    buf.writeByte(uint8_t((n & mask) >> rshift) | 0x80);
    mask >>= 7;
    rshift -= 7;
  }
  buf.writeByte(uint8_t(n) & 0x7F);
}

static uint64_t readDemoTimeCnt(Plus4Emu::File::Buffer& buf)
{
  uint64_t  n = 0U;
  uint8_t   i = 8, c;
  do {
    c = buf.readByte();
    n = (n << 7) | uint64_t(c & 0x7F);
    i--;
  } while ((c & 0x80) != 0 && i != 0);
  return n;
}

namespace Plus4 {

  Plus4VM::TED7360_::TED7360_(Plus4VM& vm_)
    : TED7360(),
      vm(vm_),
      serialPort()
  {
    setMemoryReadCallback(0x0001, memoryRead0001Callback);
    setMemoryWriteCallback(0x0001, memoryWrite0001Callback);
    for (uint16_t i = 0xFD00; i <= 0xFD0F; i++) {
      setMemoryReadCallback(i, &aciaRegisterRead);
      setMemoryWriteCallback(i, &aciaRegisterWrite);
    }
    for (uint16_t i = 0x00; i <= 0x1F; i++) {
      setMemoryReadCallback(uint16_t(0xFD40) + i, &sidRegisterRead);
      setMemoryWriteCallback(uint16_t(0xFD40) + i, &sidRegisterWrite);
      setMemoryReadCallback(uint16_t(0xFE80) + i, &sidRegisterRead);
      setMemoryWriteCallback(uint16_t(0xFE80) + i, &sidRegisterWrite);
    }
    for (uint16_t i = 0xFEC0; i <= 0xFEFF; i++) {
      setMemoryReadCallback(i, &parallelIECRead);
      setMemoryWriteCallback(i, &parallelIECWrite);
    }
  }

  Plus4VM::TED7360_::~TED7360_()
  {
  }

  void Plus4VM::TED7360_::playSample(int16_t sampleValue)
  {
    int32_t tmp = vm.soundOutputAccumulator;
    if (tmp != 0) {
      vm.soundOutputAccumulator = 0;
      tmp = (tmp >= -1048576 ? (tmp < 1048576 ? tmp : 1048576) : -1048576);
      tmp = int32_t((uint32_t(tmp * vm.sidOutputVolume)
                     + uint32_t(0x80004000UL)) >> 15) - int32_t(65536);
    }
    vm.soundOutputSignal = tmp + int32_t(sampleValue);
    vm.sendMonoAudioOutput(vm.soundOutputSignal);
  }

  void Plus4VM::TED7360_::videoOutputCallback(const uint8_t *buf, size_t nBytes)
  {
    if (vm.getIsDisplayEnabled())
      vm.display.sendVideoOutput(buf, nBytes);
  }

  void Plus4VM::TED7360_::ntscModeChangeCallback(bool isNTSC_)
  {
    vm.updateTimingParameters(isNTSC_);
  }

  void Plus4VM::TED7360_::breakPointCallback(int type,
                                             uint16_t addr, uint8_t value)
  {
    vm.breakPointCallback(vm.breakPointCallbackUserData, 0, type, addr, value);
  }

  void Plus4VM::TED7360_::reset(bool cold_reset)
  {
    if (cold_reset) {
      serialPort.removeDevices(0xFFFF);
      serialPort.setATN(true);
      for (int i = 4; i < 12; i++) {
        if (vm.serialDevices[i] != (SerialDevice *) 0)
          vm.serialDevices[i]->atnStateChangeCallback(true);
      }
    }
    TED7360::reset(cold_reset);
  }

  PLUS4EMU_REGPARM2 uint8_t Plus4VM::TED7360_::sidRegisterRead(
      void *userData, uint16_t addr)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    if (ted.vm.sidEnabled) {
      uint8_t regNum = uint8_t(addr & 0x001F);
      if (ted.vm.digiBlasterEnabled && regNum >= 0x1E) {
        if (regNum == 0x1E) {
          ted.dataBusState = ted.vm.digiBlasterOutput;
        }
        else if (!(ted.vm.isRecordingDemo | ted.vm.isPlayingDemo)) {
          int     tmp = ted.vm.soundOutputSignal;
          tmp += 32768;
          tmp = (tmp >= 0 ? (tmp < 65536 ? tmp : 65535) : 0);
          ted.dataBusState = uint8_t(tmp >> 8);
        }
        else {
          ted.dataBusState = 0x80;
        }
      }
      else
        ted.dataBusState = uint8_t(ted.vm.sid_->read(regNum));
    }
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM3 void Plus4VM::TED7360_::sidRegisterWrite(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    ted.dataBusState = value;
    if (PLUS4EMU_UNLIKELY(!ted.vm.sidEnabled)) {
      ted.vm.sidEnabled = true;
      if (!(ted.vm.sidFlags & 4))
        ted.setCallback(&SID::clockCallback, ted.vm.sid_, 1);
      else
        ted.setCallback(&(ted.vm.sidCallbackC64), &(ted.vm), 1);
    }
    uint8_t regNum = uint8_t(addr & 0x001F);
    if (regNum == 0x1E) {
      ted.vm.digiBlasterOutput = value;
      if (ted.vm.digiBlasterEnabled)
        ted.vm.sid_->input((int(value) << 8) - 32768);
    }
    ted.vm.sid_->write(regNum, value);
  }

  PLUS4EMU_REGPARM3 void Plus4VM::TED7360_::sidRegisterWriteC64(
      void *userData, uint16_t addr, uint8_t value)
  {
    // SID register write at $D400-$D41F, also store the data in RAM
    TED7360::write_memory_C000_to_FCFF(userData, addr, value);
    Plus4VM::TED7360_::sidRegisterWrite(userData, addr, value);
  }

  PLUS4EMU_REGPARM2 uint8_t Plus4VM::TED7360_::parallelIECRead(
      void *userData, uint16_t addr)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
#if 0
    if (!(addr & 0x0007)) {
      // work around kernal parallel interface test bug
      ted.dataBusState |= uint8_t(0x02);
    }
#endif
    if (!(ted.vm.isRecordingDemo | ted.vm.isPlayingDemo)) {
      if (ted.vm.drive8Is1551) {
        SerialDevice  *p = ted.vm.serialDevices[8];
        if (p != (SerialDevice *) 0) {
          VC1551& vc1551 = *(reinterpret_cast<VC1551 *>(p));
          if (vc1551.parallelIECRead(addr, ted.dataBusState))
            return ted.dataBusState;
        }
        else if (addr >= 0xFEE0) {
          (void) ted.vm.iecDrive8->parallelIECRead(addr, ted.dataBusState);
          return ted.dataBusState;
        }
      }
      if (ted.vm.drive9Is1551) {
        SerialDevice  *p = ted.vm.serialDevices[9];
        if (p != (SerialDevice *) 0) {
          VC1551& vc1551 = *(reinterpret_cast<VC1551 *>(p));
          if (vc1551.parallelIECRead(addr, ted.dataBusState))
            return ted.dataBusState;
        }
        else if (addr < 0xFEE0) {
          (void) ted.vm.iecDrive9->parallelIECRead(addr, ted.dataBusState);
          return ted.dataBusState;
        }
      }
    }
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM3 void Plus4VM::TED7360_::parallelIECWrite(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    ted.dataBusState = value;
    if (!(ted.vm.isRecordingDemo | ted.vm.isPlayingDemo)) {
      if (ted.vm.drive8Is1551) {
        SerialDevice  *p = ted.vm.serialDevices[8];
        if (p != (SerialDevice *) 0) {
          VC1551& vc1551 = *(reinterpret_cast<VC1551 *>(p));
          (void) vc1551.parallelIECWrite(addr, ted.dataBusState);
        }
        else if (addr >= 0xFEE0)
          (void) ted.vm.iecDrive8->parallelIECWrite(addr, ted.dataBusState);
      }
      if (ted.vm.drive9Is1551) {
        SerialDevice  *p = ted.vm.serialDevices[9];
        if (p != (SerialDevice *) 0) {
          VC1551& vc1551 = *(reinterpret_cast<VC1551 *>(p));
          (void) vc1551.parallelIECWrite(addr, ted.dataBusState);
        }
        else if (addr < 0xFEE0)
          (void) ted.vm.iecDrive9->parallelIECWrite(addr, ted.dataBusState);
      }
    }
  }

  PLUS4EMU_REGPARM2 uint8_t Plus4VM::TED7360_::memoryRead0001Callback(
      void *userData, uint16_t addr)
  {
    (void) addr;
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    uint8_t tmp = ted.ioPortRead();
    uint8_t mask_ = ted.ioRegister_0000;
    uint8_t nmask_ = mask_ ^ uint8_t(0xFF);
    if (!(ted.vm.isRecordingDemo | ted.vm.isPlayingDemo)) {
      tmp &= (ted.serialPort.getCLK() | uint8_t(0xBF));
      tmp &= (ted.serialPort.getDATA() | uint8_t(0x7F));
    }
    else {
      uint8_t tmp2 = ted.ioRegister_0001 | nmask_;
      tmp2 = ((tmp2 & uint8_t(0x01)) << 7) | ((tmp2 & uint8_t(0x02)) << 5);
      tmp &= (tmp2 ^ uint8_t(0xFF));
    }
    tmp = (tmp & nmask_) | (ted.ioRegister_0001 & mask_);
    return tmp;
  }

  PLUS4EMU_REGPARM3 void Plus4VM::TED7360_::memoryWrite0001Callback(
      void *userData, uint16_t addr, uint8_t value)
  {
    (void) addr;
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    ted.ioRegister_0001 = value;
    uint8_t tmp = value | (ted.ioRegister_0000 ^ uint8_t(0xFF));
    uint8_t tmp2 = tmp ^ uint8_t(0xFF);
    tmp |= uint8_t(((tmp2 & 0x80) >> 7) | ((tmp2 & 0x40) >> 5));
    // FIXME: tape output should also be affected by other devices on the
    // serial bus
    ted.ioPortWrite(tmp);
    if (!(ted.vm.isRecordingDemo | ted.vm.isPlayingDemo)) {
      ted.serialPort.setDATA(0, !(tmp & uint8_t(0x01)));
      ted.serialPort.setCLK(0, !(tmp & uint8_t(0x02)));
      bool    atnState = !(tmp & uint8_t(0x04));
      if (atnState != bool(ted.serialPort.getATN())) {
        ted.serialPort.setATN(atnState);
        if (ted.vm.serialDevices[4] != (SerialDevice *) 0)
          ted.vm.serialDevices[4]->atnStateChangeCallback(atnState);
        for (int i = 8; i < 12; i++) {
          if (ted.vm.serialDevices[i] != (SerialDevice *) 0)
            ted.vm.serialDevices[i]->atnStateChangeCallback(atnState);
        }
      }
    }
  }

  PLUS4EMU_REGPARM2 uint8_t Plus4VM::TED7360_::aciaRegisterRead(
      void *userData, uint16_t addr)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    if (ted.vm.aciaEnabled) {
      // FIXME: this breaks the 'const'-ness of TED7360::readMemoryCPU(),
      // and may possibly clear the interrupt request on saving PRG files
      ted.dataBusState = ted.vm.acia_.readRegister(addr);
      if ((addr & 0x0003) == 0x0001) {
        // interrupt request is cleared on reading the status register
        ted.interruptRequest(bool(ted.tedRegisters[0x09]
                                  & ted.tedRegisters[0x0A]));
      }
    }
    return ted.dataBusState;
  }

  PLUS4EMU_REGPARM3 void Plus4VM::TED7360_::aciaRegisterWrite(
      void *userData, uint16_t addr, uint8_t value)
  {
    TED7360_& ted = *(reinterpret_cast<TED7360_ *>(userData));
    ted.dataBusState = value;
    if (ted.vm.aciaEnabled) {
      ted.vm.acia_.writeRegister(addr, value);
      if (ted.vm.acia_.isEnabled())
        ted.vm.setEnableACIACallback(true);
    }
  }

  void Plus4VM::TED7360_::setEnableC64CompatibleSID(bool isEnabled)
  {
    PLUS4EMU_REGPARM3 void (*func)(void *, uint16_t, uint8_t) =
        (isEnabled ? &sidRegisterWriteC64 : &write_memory_C000_to_FCFF);
    for (uint16_t i = 0xD400; i <= 0xD41F; i++)
      setMemoryWriteCallback(i, func);
  }

  // --------------------------------------------------------------------------

  void Plus4VM::stopDemoPlayback()
  {
    if (isPlayingDemo) {
      isPlayingDemo = false;
      ted->setCallback(&demoPlayCallback, this, 0);
      demoTimeCnt = 0U;
      demoBuffer.clear();
      // tape button state sensing is disabled while recording or playing demo
      ted->setTapeButtonState(!isRecordingDemo && getTapeButtonState() != 0);
      // clear keyboard state at end of playback
      for (int i = 0; i < 128; i++)
        ted->setKeyState(i, false);
    }
  }

  void Plus4VM::stopDemoRecording(bool writeFile_)
  {
    if (isRecordingDemo) {
      isRecordingDemo = false;
      ted->setCallback(&demoRecordCallback, this, 0);
    }
    // tape button state sensing is disabled while recording or playing demo
    ted->setTapeButtonState(!isPlayingDemo && getTapeButtonState() != 0);
    if (writeFile_ && demoFile != (Plus4Emu::File *) 0) {
      // if file object is still open:
      try {
        // put end of demo event
        writeDemoTimeCnt(demoBuffer, demoTimeCnt);
        demoTimeCnt = 0U;
        demoBuffer.writeByte(0x00);
        demoBuffer.writeByte(0x00);
        demoFile->addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_PLUS4_DEMO,
                           demoBuffer);
      }
      catch (...) {
        demoFile = (Plus4Emu::File *) 0;
        demoTimeCnt = 0U;
        demoBuffer.clear();
        throw;
      }
      demoFile = (Plus4Emu::File *) 0;
      demoTimeCnt = 0U;
      demoBuffer.clear();
    }
  }

  void Plus4VM::updateTimingParameters(bool ntscMode_)
  {
    size_t  singleClockFreq = tedInputClockFrequency;
    if (!ntscMode_)
      singleClockFreq = ((singleClockFreq + 40) / 80) << 2;
    else
      singleClockFreq = ((singleClockFreq + 32) / 64) << 2;
    tedTimesliceLength = int64_t(((uint64_t(1000000) << 32)
                                  + (singleClockFreq >> 1))
                                 / singleClockFreq);
    ted->serialPort.timesliceLength = tedTimesliceLength;
    size_t  freqMult = cpuClockFrequency;
    if (freqMult > 1000)
      freqMult = (freqMult + singleClockFreq) / (singleClockFreq << 1);
    freqMult = (freqMult > 1 ? (freqMult < 100 ? freqMult : 100) : 1);
    ted->setCPUClockMultiplier(freqMult);
    if ((singleClockFreq >> 2) != soundClockFrequency) {
      soundClockFrequency = singleClockFreq >> 2;
      setAudioConverterSampleRate(float(long(soundClockFrequency)));
      if (videoCapture)
        videoCapture->setClockFrequency(soundClockFrequency << 3);
    }
  }

  void Plus4VM::addFloppyCallback(int n)
  {
    n = (n & 3) | 8;
    SerialDevice::ProcessCallbackPtr  func;
    void    *userData = serialDevices[n]->getProcessCallbackUserData();
    if (is1541HighAccuracy) {
      func = serialDevices[n]->getHighAccuracyProcessCallback();
      if (func) {
        ted->setCallback(func, userData, 3);
        return;
      }
    }
    func = serialDevices[n]->getProcessCallback();
    if (func)
      ted->setCallback(func, userData, 1);
  }

  void Plus4VM::removeFloppyCallback(int n)
  {
    n = (n & 3) | 8;
    SerialDevice::ProcessCallbackPtr  func;
    void    *userData = serialDevices[n]->getProcessCallbackUserData();
    func = serialDevices[n]->getProcessCallback();
    if (func)
      ted->setCallback(func, userData, 0);
    func = serialDevices[n]->getHighAccuracyProcessCallback();
    if (func)
      ted->setCallback(func, userData, 0);
  }

  void Plus4VM::resetACIA()
  {
    acia_.reset();
    aciaTimeRemaining = int64_t(0);
    setEnableACIACallback(true);
  }

  M7501 * Plus4VM::getDebugCPU()
  {
    if (currentDebugContext == 0)
      return ted;
    int     n = (currentDebugContext <= 4 ?
                 (currentDebugContext + 7) : printerDeviceNumber);
    if (serialDevices[n] != (SerialDevice *) 0)
      return serialDevices[n]->getCPU();
    return (M7501 *) 0;
  }

  const M7501 * Plus4VM::getDebugCPU() const
  {
    if (currentDebugContext == 0)
      return ted;
    int     n = (currentDebugContext <= 4 ?
                 (currentDebugContext + 7) : printerDeviceNumber);
    if (serialDevices[n] != (SerialDevice *) 0)
      return serialDevices[n]->getCPU();
    return (M7501 *) 0;
  }

  PLUS4EMU_REGPARM1 void Plus4VM::tapeCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    vm.tapeTimeRemaining += vm.tedTimesliceLength;
    if (vm.tapeTimeRemaining >= 0) {
      // assume tape sample rate < single clock frequency
      int64_t timesliceLength = vm.tapeTimesliceLength;
      if (timesliceLength <= 0)
        timesliceLength = vm.tedTimesliceLength << 2;
      vm.tapeTimeRemaining -= timesliceLength;
      vm.setTapeMotorState(vm.ted->getTapeMotorState());
      bool    tedTapeOutput = vm.ted->getTapeOutput();
      bool    tedTapeInput = (vm.runTape(tedTapeOutput ? 1 : 0) > 0);
      vm.ted->setTapeInput(tedTapeInput);
      int     tapeButtonState = vm.getTapeButtonState();
      bool    tapeFeedback = ((tapeButtonState == 1 && tedTapeInput) ||
                              (tapeButtonState == 2 && tedTapeOutput));
      vm.tapeFeedbackSignal = (tapeFeedback ?
                               vm.tapeFeedbackMult : int32_t(0));
    }
    vm.soundOutputAccumulator += vm.tapeFeedbackSignal;
  }

  PLUS4EMU_REGPARM1 void Plus4VM::sidCallbackC64(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    // FIXME: the accuracy and sound quality of this solution could be improved
    if (PLUS4EMU_UNLIKELY(!(--(vm.sidCycleCnt)))) {
      // on every 9th TED single clock cycle (7th if NTSC),
      // run the SID emulation twice and average the outputs
      vm.sidCycleCnt = (uint8_t(vm.tedInputClockFrequency >> 24) << 1) + 7;
      vm.soundOutputAccumulator = (vm.soundOutputAccumulator << 1) + 0x40000001;
      SID::clockCallback(vm.sid_);
      SID::clockCallback(vm.sid_);
      vm.soundOutputAccumulator = (vm.soundOutputAccumulator >> 1) - 0x20000000;
      return;
    }
    SID::clockCallback(vm.sid_);
  }

  PLUS4EMU_REGPARM1 void Plus4VM::demoPlayCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    while (!vm.demoTimeCnt) {
      if (vm.haveTape() &&
          vm.getIsTapeMotorOn() && vm.getTapeButtonState() != 0)
        vm.stopDemoPlayback();
      try {
        uint8_t evtType = vm.demoBuffer.readByte();
        uint8_t evtBytes = vm.demoBuffer.readByte();
        uint8_t evtData = 0;
        while (evtBytes) {
          evtData = vm.demoBuffer.readByte();
          evtBytes--;
        }
        switch (evtType) {
        case 0x00:
          vm.stopDemoPlayback();
          break;
        case 0x01:
          vm.ted->setKeyState(evtData, true);
          break;
        case 0x02:
          vm.ted->setKeyState(evtData, false);
          break;
        }
        vm.demoTimeCnt = readDemoTimeCnt(vm.demoBuffer);
      }
      catch (...) {
        vm.stopDemoPlayback();
      }
      if (!vm.isPlayingDemo) {
        vm.demoBuffer.clear();
        vm.demoTimeCnt = 0U;
        break;
      }
    }
    if (vm.demoTimeCnt)
      vm.demoTimeCnt--;
  }

  PLUS4EMU_REGPARM1 void Plus4VM::demoRecordCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    vm.demoTimeCnt++;
  }

  PLUS4EMU_REGPARM1 void Plus4VM::videoBreakPointCheckCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    TED7360_& ted_ = *(vm.ted);
    if (vm.videoBreakPoints) {
      uint16_t  n = (ted_.getVideoPositionY() << 7)
                    | uint16_t(ted_.getVideoPositionX() >> 1);
      if (vm.videoBreakPoints[n] != 0) {
        if (int(vm.videoBreakPoints[n])
            > ted_.getBreakPointPriorityThreshold()) {
          // correct video position for FF1E read delay
          if ((n & 0x007F) != 0)
            n--;
          else
            n = n | 113;
          vm.breakPointCallback(vm.breakPointCallbackUserData, 0, 4, n, 0x00);
        }
      }
    }
  }

  PLUS4EMU_REGPARM1 void Plus4VM::lightPenCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    TED7360_& ted_ = *(vm.ted);
    if (vm.lightPenPositionX >= 0 && vm.lightPenPositionY >= 0) {
      if (ted_.checkLightPen(vm.lightPenPositionX, vm.lightPenPositionY)) {
        vm.lightPenCycleCounter = 150;
        vm.setKeyboardState(86, true);
      }
    }
    if (vm.lightPenCycleCounter) {
      vm.lightPenCycleCounter--;
      if (!vm.lightPenCycleCounter)
        vm.setKeyboardState(86, false);
    }
  }

  PLUS4EMU_REGPARM1 void Plus4VM::videoCaptureCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    vm.videoCapture->runOneCycle(vm.ted->getVideoOutput(),
                                 vm.soundOutputSignal);
  }

  PLUS4EMU_REGPARM1 void Plus4VM::aciaCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    vm.aciaTimeRemaining += (vm.tedTimesliceLength >> 1);
    while (vm.aciaTimeRemaining > 0) {
      // clock frequency is 1.8432 MHz
      vm.aciaTimeRemaining -= int64_t(2330168889UL);
      vm.acia_.runOneCycle();
    }
    if (vm.acia_.getInterruptFlag())
      vm.ted->interruptRequest(true);
    else if (!vm.acia_.isEnabled())
      vm.setEnableACIACallback(false);
  }

  PLUS4EMU_REGPARM1 void Plus4VM::pasteTextCallback(void *userData)
  {
    Plus4VM&  vm = *(reinterpret_cast<Plus4VM *>(userData));
    vm.pasteTextCycleCnt--;
    if (vm.pasteTextCycleCnt >= 0)
      return;
    vm.pasteTextCycleCnt = 10000;
    if (vm.isRecordingDemo | vm.isPlayingDemo) {
      vm.removePasteTextCallback();
      return;
    }
    if (vm.ted->readMemoryCPU(0x00EF) != 0 ||
        vm.ted->readMemoryCPU(0x055D) != 0 ||
        !vm.checkEditorMode()) {
      // wait if not all characters are consumed yet, or not in editor mode
      vm.pasteTextWaitCnt++;
      if (vm.pasteTextWaitCnt >= (vm.pasteTextBufferPos > 0 ? 448 : 96)) {
        // time out after about 5 seconds (or 1 second for the first character)
        vm.removePasteTextCallback();
      }
      return;
    }
    vm.pasteTextWaitCnt = 0;
    if (vm.pasteTextCursorPositionX >= 0 && vm.pasteTextCursorPositionY >= 0) {
      vm.setCursorPosition_(vm.pasteTextCursorPositionX,
                            vm.pasteTextCursorPositionY);
    }
    vm.pasteTextCursorPositionX = -1;
    vm.pasteTextCursorPositionY = -1;
    if (vm.pasteTextBuffer == (char *) 0) {
      vm.removePasteTextCallback();     // no text to paste
      return;
    }
    int     charCnt = 0;
    uint8_t c = '\0';
    do {
      c = Plus4Emu::utf8ToPETSCII(vm.pasteTextBuffer, vm.pasteTextBufferPos,
                                  bool(vm.ted->readMemoryCPU(0xFF13) & 0x04));
      if (c == '\0') {
        vm.ted->writeMemoryCPU(0x00EF, uint8_t(charCnt));
        vm.removePasteTextCallback();   // done pasting all characters
        return;
      }
      if (c == char(0xFF)) {
        // ignore invalid characters
        continue;
      }
      // convert tabs to spaces
      if (c == '\t')
        c = ' ';
      // store characters in keyboard buffer
      vm.ted->writeMemoryCPU(uint16_t(0x0527 + charCnt), uint8_t(c));
      charCnt++;
      // until the buffer is full or end of line
    } while (charCnt < 10 && c != '\r');
    vm.ted->writeMemoryCPU(0x00EF, uint8_t(charCnt));
  }

  void Plus4VM::removePasteTextCallback()
  {
    ted->setCallback(&pasteTextCallback, (void *) this, 0);
    pasteTextCycleCnt = 0;
    pasteTextWaitCnt = 0;
    pasteTextCursorPositionX = -1;
    pasteTextCursorPositionY = -1;
    pasteTextBufferPos = 0;
    if (pasteTextBuffer != (char *) 0) {
      delete[] pasteTextBuffer;
      pasteTextBuffer = (char *) 0;
    }
  }

  bool Plus4VM::checkEditorMode() const
  {
    M7501Registers  r;
    ted->getRegisters(r);
    if (!(r.reg_PC >= 0xD90A && r.reg_PC <= 0xD911 &&
          ted->getMemoryPage(3) == 0x01)) {
      // if not waiting for input, return
      return false;
    }
    // some additional consistency checks:
    int     cursorPositionL = ted->readMemoryCPU(0xFF0D);
    int     cursorPositionH = ted->readMemoryCPU(0xFF0C) & 0x03;
    int     cursorPosition = (cursorPositionH << 8) | cursorPositionL;
    if (cursorPosition >= 0x03E8)
      return false;             // cursor not visible
    int     cursorLine = cursorPosition / 40;
    int     cursorColumn = cursorPosition % 40;
    int     cursorLineStart = cursorLine * 40;
    int     cursorLineStartL = cursorLineStart & 0xFF;
    int     cursorLineStartH = cursorLineStart >> 8;
    if (!(int(ted->readMemoryCPU(0x00C8)) == cursorLineStartL &&
          int(ted->readMemoryCPU(0x00C9)) == (cursorLineStartH | 0x0C) &&
          int(ted->readMemoryCPU(0x00CA)) == cursorColumn &&
          int(ted->readMemoryCPU(0x00CD)) == cursorLine &&
          int(ted->readMemoryCPU(0x00EA)) == cursorLineStartL &&
          int(ted->readMemoryCPU(0x00EB)) == (cursorLineStartH | 0x08))) {
      // TED cursor position is not consistent with zeropage variables
      return false;
    }
    if (cursorLine > int(ted->readMemoryCPU(0x07E5)) ||
        cursorLine < int(ted->readMemoryCPU(0x07E6)) ||
        ted->readMemoryCPU(0x07E5) < ted->readMemoryCPU(0x07E6) ||
        cursorColumn < int(ted->readMemoryCPU(0x07E7)) ||
        cursorColumn > int(ted->readMemoryCPU(0x07E8)) ||
        ted->readMemoryCPU(0x07E7) > ted->readMemoryCPU(0x07E8)) {
      // cursor position is out of range
      return false;
    }
    return true;
  }

  void Plus4VM::setCursorPosition_(int xPos, int yPos)
  {
    if (isRecordingDemo | isPlayingDemo)
      return;
    if (xPos < 0 || xPos > 65535 || yPos < 0 || yPos > 65535)
      return;
    if (!checkEditorMode())
      return;
    int     xc = ((xPos * 384) >> 16) + 424;
    if (xc >= 456)
      xc -= 456;
    if (xc < 0 || xc >= 320)
      return;
    int     yc = (yPos * 288) >> 16;
    if (!ted->getIsNTSCMode()) {
      yc = yc + 275;
      if (yc >= 312)
        yc -= 312;
    }
    else {
      yc = yc + 225;
      if (yc >= 262)
        yc -= 262;
    }
    if (yc < 4 || yc >= 204)
      return;
    xc = xc / 8;
    yc = (yc - 4) / 8;
    if (yc > int(ted->readMemoryCPU(0x07E5)) ||
        yc < int(ted->readMemoryCPU(0x07E6)) ||
        xc < int(ted->readMemoryCPU(0x07E7)) ||
        xc > int(ted->readMemoryCPU(0x07E8))) {
      return;
    }
    uint16_t  savedColorAddr = 0;
    {
      M7501Registers  r;
      ted->getRegisters(r);
      savedColorAddr = uint16_t(0x0100 | ((r.reg_SP + 1) & 0xFF));
      r.reg_YR = uint8_t(xc);
      ted->setRegisters(r);
    }
    ted->writeMemoryCPU(uint16_t(0x0800)
                        | uint16_t(ted->readMemoryCPU(0xFF0D))
                        | (uint16_t(ted->readMemoryCPU(0xFF0C) & 0x03) << 8),
                        ted->readMemoryCPU(savedColorAddr));
    int     cursorLineStart = yc * 40;
    int     cursorLineStartL = cursorLineStart & 0xFF;
    int     cursorLineStartH = cursorLineStart >> 8;
    int     cursorPosition = (yc * 40) + xc;
    ted->writeMemoryCPU(0x00C4, uint8_t(0x80));
    ted->writeMemoryCPU(0x00C8, uint8_t(cursorLineStartL));
    ted->writeMemoryCPU(0x00C9, uint8_t(cursorLineStartH | 0x0C));
    ted->writeMemoryCPU(0x00CA, uint8_t(xc));
    ted->writeMemoryCPU(0x00CD, uint8_t(yc));
    ted->writeMemoryCPU(0x00EA, uint8_t(cursorLineStartL));
    ted->writeMemoryCPU(0x00EB, uint8_t(cursorLineStartH | 0x08));
    ted->writeMemoryCPU(0xFF0C, uint8_t(cursorPosition >> 8));
    ted->writeMemoryCPU(0xFF0D, uint8_t(cursorPosition & 0xFF));
    ted->writeMemoryCPU(savedColorAddr,
                        ted->readMemoryCPU(uint16_t(0x0800 | cursorPosition)));
    ted->writeMemoryCPU(uint16_t(0x0800 | cursorPosition),
                        ted->readMemoryCPU(0x053B));
  }

  // --------------------------------------------------------------------------

  Plus4VM::Plus4VM(Plus4Emu::VideoDisplay& display_,
                   Plus4Emu::AudioOutput& audioOutput_)
    : VirtualMachine(display_, audioOutput_),
      ted((TED7360_ *) 0),
      cpuClockFrequency(1),
      tedInputClockFrequency(17734475),
      soundClockFrequency(0),
      tedTimesliceLength(0),
      tedTimeRemaining(0),
      tapeTimesliceLength(0),
      tapeTimeRemaining(0),
      demoFile((Plus4Emu::File *) 0),
      demoBuffer(),
      isRecordingDemo(false),
      isPlayingDemo(false),
      snapshotLoadFlag(false),
      tapeCallbackFlag(false),
      demoTimeCnt(0U),
      sid_((SID *) 0),
      soundOutputAccumulator(0),
      soundOutputSignal(0),
      sidOutputVolume(1051),
      sidEnabled(false),
      digiBlasterEnabled(false),
      digiBlasterOutput(0x80),
      sidCycleCnt(4),
      sidFlags(0),
      is1541HighAccuracy(true),
      serialBusDelayOffset(0),
      floppyROM_1541((uint8_t *) 0),
      floppyROM_1551((uint8_t *) 0),
      floppyROM_1581_0((uint8_t *) 0),
      floppyROM_1581_1((uint8_t *) 0),
      printerROM_MPS801((uint8_t *) 0),
      printerROM_1526((uint8_t *) 0),
      videoBreakPointCnt(0),
      videoBreakPoints((uint8_t *) 0),
      tapeFeedbackSignal(0),
      tapeFeedbackMult(0),
      lightPenPositionX(-1),
      lightPenPositionY(-1),
      lightPenCycleCounter(0),
      printerOutputChangedFlag(true),
      printerFormFeedOn(false),
      videoCaptureNTSCMode(false),
      tapeFeedbackLevel(0),
      videoCapture((Plus4Emu::VideoCapture *) 0),
      acia_(),
      aciaTimeRemaining(0),
      aciaEnabled(false),
      aciaCallbackFlag(false),
      drive8Is1551(false),
      drive9Is1551(false),
      iecDrive8((ParallelIECDrive *) 0),
      iecDrive9((ParallelIECDrive *) 0),
      pasteTextCycleCnt(0),
      pasteTextWaitCnt(0),
      pasteTextCursorPositionX(-1),
      pasteTextCursorPositionY(-1),
      pasteTextBufferPos(0),
      pasteTextBuffer((char *) 0)
  {
    for (int i = 0; i < 12; i++)
      serialDevices[i] = (SerialDevice *) 0;
    sid_ = new SID(soundOutputAccumulator);
    try {
      sid_->set_chip_model(MOS8580);
      sid_->enable_external_filter(false);
      sid_->reset();
      iecDrive8 = new ParallelIECDrive(8);
      iecDrive9 = new ParallelIECDrive(9);
      ted = new TED7360_(*this);
      updateTimingParameters(false);
      // reset
      ted->reset(true);
      // use PLUS/4 colormap
      Plus4Emu::VideoDisplay::DisplayParameters
          dp(display.getDisplayParameters());
      dp.indexToYUVFunc = &TED7360::convertPixelToYUV;
      display.setDisplayParameters(dp);
    }
    catch (...) {
      if (ted)
        delete ted;
      if (iecDrive8)
        delete iecDrive8;
      if (iecDrive9)
        delete iecDrive9;
      delete sid_;
      throw;
    }
  }

  Plus4VM::~Plus4VM()
  {
    if (videoCapture) {
      delete videoCapture;
      videoCapture = (Plus4Emu::VideoCapture *) 0;
    }
    try {
      // FIXME: cannot handle errors here
      stopDemo();
    }
    catch (...) {
    }
    removePasteTextCallback();
    for (int i = 0; i < 12; i++) {
      if (serialDevices[i] != (SerialDevice *) 0) {
        delete serialDevices[i];
        serialDevices[i] = (SerialDevice *) 0;
      }
    }
    if (floppyROM_1541)
      delete[] floppyROM_1541;
    if (floppyROM_1551)
      delete[] floppyROM_1551;
    if (floppyROM_1581_0)
      delete[] floppyROM_1581_0;
    if (floppyROM_1581_1)
      delete[] floppyROM_1581_1;
    if (printerROM_MPS801)
      delete[] printerROM_MPS801;
    if (printerROM_1526)
      delete[] printerROM_1526;
    delete ted;
    delete iecDrive8;
    delete iecDrive9;
    delete sid_;
    if (videoBreakPoints)
      delete[] videoBreakPoints;
  }

  void Plus4VM::run(size_t microseconds)
  {
    Plus4Emu::VirtualMachine::run(microseconds);
    if (snapshotLoadFlag) {
      snapshotLoadFlag = false;
      // if just loaded a snapshot, and not playing a demo,
      // clear keyboard state
      if (!isPlayingDemo) {
        for (int i = 0; i < 128; i++)
          ted->setKeyState(i, false);
      }
    }
    bool    newTapeCallbackFlag = (getTapeButtonState() != 0);
    if (newTapeCallbackFlag != tapeCallbackFlag) {
      tapeCallbackFlag = newTapeCallbackFlag;
      if (!tapeCallbackFlag) {
        ted->setTapeInput(false);
        tapeFeedbackSignal = 0;
      }
      ted->setCallback(&tapeCallback, this, (tapeCallbackFlag ? 1 : 0));
    }
    tedTimeRemaining = tedTimeRemaining + (int64_t(microseconds) << 32);
    int32_t tedCycles = int32_t(double(tedTimeRemaining)
                                * double(int32_t(tedInputClockFrequency))
                                * (1.0 / 4294967296000000.0));
    if (tedCycles >= 0) {
      tedCycles = tedCycles - ted->run(tedCycles);
      tedTimeRemaining = tedTimeRemaining
                         - int64_t(double(tedCycles) * 4294967296000000.0
                                   / double(int32_t(tedInputClockFrequency)));
    }
  }

  void Plus4VM::reset(bool isColdReset)
  {
    stopDemoPlayback();         // TODO: should be recorded as an event ?
    stopDemoRecording(false);
    removePasteTextCallback();
    ted->reset(isColdReset);
    setTapeMotorState(false);
    sid_->reset();
    digiBlasterOutput = 0x80;
    sid_->input(0);
    if (isColdReset) {
      sidEnabled = false;
      ted->setCallback(&SID::clockCallback, sid_, 0);
      ted->setCallback(&sidCallbackC64, this, 0);
      disableUnusedFloppyDrives();
    }
    resetFloppyDrive(-1);
    if (serialDevices[printerDeviceNumber] != (SerialDevice *) 0)
      serialDevices[printerDeviceNumber]->reset();
    resetACIA();
  }

  void Plus4VM::resetMemoryConfiguration(size_t memSize, uint64_t ramPattern)
  {
    try {
      stopDemo();
      // delete all ROM segments
      for (uint8_t n = 0; n < 8; n++)
        loadROMSegment(n, (char *) 0, 0);
      for (int i = 4; i < 12; i++) {
        if (serialDevices[i] != (SerialDevice *) 0) {
          for (int n = 0; n < 5; n++)
            serialDevices[i]->setROMImage(n, (uint8_t *) 0);
        }
      }
      if (floppyROM_1541) {
        delete[] floppyROM_1541;
        floppyROM_1541 = (uint8_t *) 0;
      }
      if (floppyROM_1551) {
        delete[] floppyROM_1551;
        floppyROM_1551 = (uint8_t *) 0;
      }
      if (floppyROM_1581_0) {
        delete[] floppyROM_1581_0;
        floppyROM_1581_0 = (uint8_t *) 0;
      }
      if (floppyROM_1581_1) {
        delete[] floppyROM_1581_1;
        floppyROM_1581_1 = (uint8_t *) 0;
      }
      if (printerROM_MPS801) {
        delete[] printerROM_MPS801;
        printerROM_MPS801 = (uint8_t *) 0;
      }
      if (printerROM_1526) {
        delete[] printerROM_1526;
        printerROM_1526 = (uint8_t *) 0;
      }
      // set new RAM size
      ted->setRAMSize(memSize, ramPattern);
    }
    catch (...) {
      try {
        this->reset(true);
      }
      catch (...) {
      }
      throw;
    }
    // cold reset
    this->reset(true);
  }

  void Plus4VM::loadROMSegment(uint8_t n, const char *fileName, size_t offs)
  {
    stopDemo();
    int     floppyROMSegment = -1;
    uint8_t **floppyROMPtr = (uint8_t **) 0;
    size_t  nBytes = 16384;
    if (n >= 8) {
      switch (n) {
      case 0x0A:
        floppyROMSegment = 5;
        floppyROMPtr = &printerROM_MPS801;
        nBytes = 4096;
        break;
      case 0x0C:
        floppyROMSegment = 4;
        floppyROMPtr = &printerROM_1526;
        nBytes = 8192;
        break;
      case 0x10:
        floppyROMSegment = 2;
        floppyROMPtr = &floppyROM_1541;
        break;
      case 0x20:
        floppyROMSegment = 3;
        floppyROMPtr = &floppyROM_1551;
        break;
      case 0x30:
        floppyROMSegment = 0;
        floppyROMPtr = &floppyROM_1581_0;
        break;
      case 0x31:
        floppyROMSegment = 1;
        floppyROMPtr = &floppyROM_1581_1;
        break;
      default:
        return;
      }
    }
    // clear segment first
    if (floppyROMSegment < 0) {
      ted->loadROM(int(n >> 1), int(n & 1) << 14, 0, (uint8_t *) 0);
    }
    else {
      for (int i = 4; i < 12; i++) {
        if (serialDevices[i] != (SerialDevice *) 0)
          serialDevices[i]->setROMImage(floppyROMSegment, (uint8_t *) 0);
      }
    }
    if (fileName == (char *) 0 || fileName[0] == '\0') {
      // empty file name: delete segment
      return;
    }
    // load file into memory
    std::vector<uint8_t>  buf;
    buf.resize(nBytes);
    std::FILE   *f = Plus4Emu::fileOpen(fileName, "rb");
    if (!f)
      throw Plus4Emu::Exception("cannot open ROM file");
    std::fseek(f, 0L, SEEK_END);
    if (ftell(f) < long(offs + nBytes)) {
      std::fclose(f);
      throw Plus4Emu::Exception("ROM file is shorter than expected");
    }
    std::fseek(f, long(offs), SEEK_SET);
    std::fread(&(buf.front()), 1, nBytes, f);
    std::fclose(f);
    if (floppyROMSegment < 0) {
      ted->loadROM(int(n) >> 1, int(n & 1) << 14, int(nBytes), &(buf.front()));
    }
    else {
      if ((*floppyROMPtr) == (uint8_t *) 0)
        (*floppyROMPtr) = new uint8_t[nBytes];
      for (size_t i = 0; i < nBytes; i++)
        (*floppyROMPtr)[i] = buf[i];
      for (int i = 4; i < 12; i++) {
        if (serialDevices[i] != (SerialDevice *) 0)
          serialDevices[i]->setROMImage(floppyROMSegment, (*floppyROMPtr));
      }
    }
  }

  void Plus4VM::setCPUFrequency(size_t freq_)
  {
    size_t  freq = (freq_ > 1 ? (freq_ < 150000000 ? freq_ : 150000000) : 1);
    if (freq == cpuClockFrequency)
      return;
    stopDemoPlayback();         // changing configuration implies stopping
    stopDemoRecording(false);   // any demo playback or recording
    cpuClockFrequency = freq;
    updateTimingParameters(ted->getIsNTSCMode());
  }

  void Plus4VM::setVideoFrequency(size_t freq_)
  {
    size_t  freq = (freq_ > 7159090 ? (freq_ < 35468950 ? freq_ : 35468950)
                                      : 7159090);
    if (freq == tedInputClockFrequency)
      return;
    stopDemoPlayback();         // changing configuration implies stopping
    stopDemoRecording(false);   // any demo playback or recording
    tedInputClockFrequency = freq;
    updateTimingParameters(ted->getIsNTSCMode());
  }

  void Plus4VM::setEnableACIAEmulation(bool isEnabled)
  {
    if (isEnabled != aciaEnabled) {
      stopDemoPlayback();
      stopDemoRecording(false);
      aciaEnabled = isEnabled;
      resetACIA();
    }
  }

  void Plus4VM::setSIDConfiguration(uint8_t sidFlags_, bool enableDigiBlaster,
                                    int outputVolume)
  {
    sidFlags_ = sidFlags_ & 7;
    if (sidFlags_ != sidFlags) {
      uint8_t changeMask = sidFlags_ ^ sidFlags;
      sidFlags = sidFlags_;
      if (sidFlags_ & 1)
        sid_->set_chip_model(MOS6581);
      else
        sid_->set_chip_model(MOS8580);
      if (changeMask & 6) {
        stopDemoPlayback();
        stopDemoRecording(false);
        if (changeMask & 2)
          ted->setEnableC64CompatibleSID(bool(sidFlags_ & 2));
        if (sidEnabled && (changeMask & 4) != 0) {
          ted->setCallback(&SID::clockCallback, sid_, int(!(sidFlags_ & 4)));
          ted->setCallback(&sidCallbackC64, this, int(bool(sidFlags_ & 4)));
        }
      }
    }
    digiBlasterEnabled = enableDigiBlaster;
    sid_->set_voice_mask(enableDigiBlaster ? 0x0F : 0x07);
    if (enableDigiBlaster)
      sid_->input((int(digiBlasterOutput) << 8) - 32768);
    else
      sid_->input(0);
    int32_t newSIDOutputVolume =
        int32_t(std::pow(10.0, double(outputVolume) * 0.05)
                * (65536.0 * 3.0 / 187.0) + 0.5);
    if (newSIDOutputVolume != sidOutputVolume) {
      // adjust tape feedback level so that only the SID output level changes
      sidOutputVolume = newSIDOutputVolume;
      setTapeFeedbackLevel(tapeFeedbackLevel);
    }
  }

  void Plus4VM::disableSIDEmulation()
  {
    if (sidEnabled) {
      stopDemoPlayback();
      stopDemoRecording(false);
      sid_->reset();
      digiBlasterOutput = 0x80;
      sid_->input(0);
      sidEnabled = false;
      ted->setCallback(&SID::clockCallback, sid_, 0);
      ted->setCallback(&sidCallbackC64, this, 0);
    }
  }

  void Plus4VM::setKeyboardState(int keyCode, bool isPressed)
  {
    if (!isPlayingDemo)
      ted->setKeyState(keyCode, isPressed);
    if (isRecordingDemo) {
      if (haveTape() && getIsTapeMotorOn() && getTapeButtonState() != 0) {
        stopDemoRecording(false);
        return;
      }
      writeDemoTimeCnt(demoBuffer, demoTimeCnt);
      demoTimeCnt = 0U;
      demoBuffer.writeByte(isPressed ? 0x01 : 0x02);
      demoBuffer.writeByte(0x01);
      demoBuffer.writeByte(uint8_t(keyCode & 0x7F));
    }
  }

  void Plus4VM::setLightPenPosition(int xPos, int yPos)
  {
    if (xPos < 0 || xPos > 65535 || yPos < 0 || yPos > 65535) {
      // disable light pen
      if (lightPenCycleCounter) {
        setKeyboardState(86, false);
        lightPenCycleCounter = 0;
      }
      if (lightPenPositionX >= 0 || lightPenPositionY >= 0) {
        ted->setCallback(&lightPenCallback, this, 0);
        lightPenPositionX = -1;
        lightPenPositionY = -1;
      }
    }
    else {
      if (lightPenPositionX < 0 || lightPenPositionY < 0)
        ted->setCallback(&lightPenCallback, this, 3);
      lightPenPositionX = ((xPos * 384) >> 16) + 424;
      if (lightPenPositionX >= 456)
        lightPenPositionX -= 456;
      if (!ted->getIsNTSCMode()) {
        lightPenPositionY = ((yPos * 288) >> 16) + 275;
        if (lightPenPositionY >= 312)
          lightPenPositionY -= 312;
      }
      else {
        lightPenPositionY = ((yPos * 288) >> 16) + 225;
        if (lightPenPositionY >= 262)
          lightPenPositionY -= 262;
      }
    }
  }

  void Plus4VM::setCursorPosition(int xPos, int yPos)
  {
    if (xPos < 0 || xPos > 65535 || yPos < 0 || yPos > 65535)
      return;
    if (isRecordingDemo | isPlayingDemo)
      return;
    removePasteTextCallback();
    pasteTextCursorPositionX = xPos;
    pasteTextCursorPositionY = yPos;
    ted->setCallback(&pasteTextCallback, (void *) this, 1);
  }

  std::string Plus4VM::copyText(int xPos, int yPos) const
  {
    std::string s;
    if (yPos >= 100) {
      yPos = (yPos * 288) >> 16;
      if (!ted->getIsNTSCMode()) {
        yPos = yPos + 275;
        if (yPos >= 312)
          yPos -= 312;
      }
      else {
        yPos = yPos + 225;
        if (yPos >= 262)
          yPos -= 262;
      }
      if (yPos < 4 || yPos >= 204)
        return s;
      yPos = (yPos - 4) / 8;
    }
    else if (yPos == -1) {
      yPos = ted->readMemoryCPU(0x00CD);
    }
    if (yPos >= 25)
      return s;
    if (yPos < 0)
      xPos = -2;
    if (xPos >= 100) {
      xPos = ((xPos * 384) >> 16) + 424;
      if (xPos >= 456)
        xPos -= 456;
      if (xPos < 0 || xPos >= 320)
        return s;
      xPos = xPos / 8;
    }
    else if (xPos == -1) {
      xPos = ted->readMemoryCPU(0x00CA);
    }
    if (xPos >= 40)
      return s;
    bool    lineContTable[26];
    for (int i = 0; i < 25; i++) {
      lineContTable[i] =
          bool(ted->readMemoryCPU(0x07EE + (i >> 3)) & (0x80 >> (i & 7)));
    }
    lineContTable[25] = false;
    int     firstLine = 0;
    int     lastLine = 24;
    if (yPos >= 0) {
      int     y = yPos;
      while (y > 0 && lineContTable[y]) {
        y--;
        if (xPos >= 0)
          xPos = xPos + 40;
      }
      firstLine = y;
      while (lineContTable[y + 1])
        y++;
      lastLine = y;
    }
    std::string tmpBuf;
    for (int y = firstLine; y <= lastLine; y++) {
      tmpBuf.clear();
      for (int x = 0; x < 40; x++) {
        Plus4Emu::screenCharToUTF8(tmpBuf,
                                   ted->readMemoryCPU(0x0C00 | ((y * 40) + x)),
                                   bool(ted->readMemoryCPU(0xFF13) & 0x04));
      }
      if (xPos < 0) {
        if (yPos >= 0 && !lineContTable[y]) {
          // strip leading spaces if this is a first line
          size_t  n = 0;
          for ( ; n < tmpBuf.length() && tmpBuf[n] == ' '; n++)
            ;
          if (n)
            tmpBuf.erase(0, n);
        }
        if (!lineContTable[y + 1]) {
          // strip trailing spaces if this is a last line,
          // and append a newline character
          size_t  n = tmpBuf.length();
          for ( ; n > 0 && tmpBuf[n - 1] == ' '; n--)
            ;
          if (n < tmpBuf.length())
            tmpBuf.resize(n);
          if (s.empty() && tmpBuf.empty())      // skip any leading empty lines
            continue;
          tmpBuf += '\n';
        }
      }
      // append to output buffer
      s += tmpBuf;
    }
    // remove trailing newline characters
    while (s.length() > 0 && s[s.length() - 1] == '\n')
      s.resize(s.length() - 1);
    if (xPos >= 0) {
      // if only a word is copied:
      int     startPos = xPos;
      int     endPos = xPos;
      while (startPos >= 0) {
        char    c = s[startPos];
        if ((unsigned char) c <= (unsigned char) ' ' || c == '[' || c == ']' ||
            (c >= '&' && c <= '-') || c == '/' || (c >= ':' && c <= '?')) {
          break;
        }
        startPos--;
      }
      if (startPos < xPos)
        startPos++;
      while (size_t(endPos) < s.length()) {
        char    c = s[endPos];
        if ((unsigned char) c <= (unsigned char) ' ' || c == '[' || c == ']' ||
            (c >= '&' && c <= '-') || c == '/' || (c >= ':' && c <= '?')) {
          if (endPos == xPos)
            endPos++;
          break;
        }
        endPos++;
      }
      if ((endPos - startPos) == 1 && s[startPos] == ' ')
        s.clear();      // do not copy a single space character
      else
        s = s.substr(size_t(startPos), size_t(endPos - startPos));
    }
    return s;
  }

  void Plus4VM::pasteText(const char *s, int xPos, int yPos)
  {
    if (s == (char *) 0 || s[0] == '\0')
      return;           // nothing to paste
    if (isRecordingDemo | isPlayingDemo)
      return;
    removePasteTextCallback();
    size_t  lenp1 = std::strlen(s) + 1;
    pasteTextBuffer = new char[lenp1];
    for (size_t i = 0; i < lenp1; i++)
      pasteTextBuffer[i] = s[i];
    pasteTextCursorPositionX = xPos;
    pasteTextCursorPositionY = yPos;
    ted->setCallback(&pasteTextCallback, (void *) this, 1);
  }

  void Plus4VM::setPrinterType(int n)
  {
    n = ((n >= 0 && n <= 3) ? n : 0);
    // TODO: allow setting device number ?
    SerialDevice*&  printerDevice = serialDevices[printerDeviceNumber];
    int     prvPrinterType = 0;
    if (printerDevice != (SerialDevice *) 0)
      prvPrinterType = (typeid(*printerDevice) == typeid(VC1526) ? 2 : 1);
    if (n >= 2 && prvPrinterType >= 2) {
      // set 1526/MPS-802 printer 1525 mode only
      reinterpret_cast<VC1526 *>(printerDevice)->setEnable1525Mode(n == 3);
      return;
    }
    if (n == prvPrinterType)
      return;                   // printer type is not changed, nothing to do
    if (prvPrinterType != 0) {
      // delete previous printer object
      printerOutputChangedFlag = true;
      Printer *printer_ = reinterpret_cast<Printer *>(printerDevice);
      ted->setCallback(printer_->getProcessCallback(),
                       printer_->getProcessCallbackUserData(), 0);
      delete printerDevice;
      printerDevice = (SerialDevice *) 0;
      ted->serialPort.removeDevice(printerDeviceNumber);
    }
    // create new printer object:
    if (n == 1) {               // MPS-801 (IEC level emulation only)
      MPS801  *printer_ = new MPS801(ted->serialPort, printerDeviceNumber);
      printerDevice = printer_;
      printer_->setROMImage(5, printerROM_MPS801);
      ted->setCallback(printer_->getProcessCallback(),
                       printer_->getProcessCallbackUserData(), 1);
    }
    else if (n >= 2) {          // 1526/MPS-802
      VC1526  *printer_ = new VC1526(ted->serialPort, printerDeviceNumber);
      printerDevice = printer_;
      printer_->setROMImage(4, printerROM_1526);
      printer_->setEnable1525Mode(n == 3);
      printer_->setFormFeedOn(printerFormFeedOn);
      ted->setCallback(printer_->getProcessCallback(),
                       printer_->getProcessCallbackUserData(), 1);
      printer_->setBreakPointCallback(breakPointCallback,
                                      breakPointCallbackUserData);
      M7501   *p = printer_->getCPU();
      if (p) {
        p->setBreakPointPriorityThreshold(
            ted->getBreakPointPriorityThreshold());
        p->setBreakOnInvalidOpcode(ted->getIsBreakOnInvalidOpcode());
      }
    }
  }

  void Plus4VM::getPrinterOutput(const uint8_t*& buf_, int& w_, int& h_) const
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_) {
      buf_ = printer_->getPageData();
      w_ = printer_->getPageWidth();
      h_ = printer_->getPageHeight();
    }
    else {
      buf_ = (uint8_t *) 0;
      w_ = 0;
      h_ = 0;
    }
  }

  void Plus4VM::clearPrinterOutput()
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_)
      printer_->clearPage();
  }

  uint8_t Plus4VM::getPrinterLEDState() const
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_)
      return printer_->getLEDState();
    return 0x00;
  }

  void Plus4VM::getPrinterHeadPosition(int& xPos, int& yPos)
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_) {
      printer_->getHeadPosition(xPos, yPos);
      return;
    }
    xPos = -1;
    yPos = -1;
  }

  bool Plus4VM::getIsPrinterOutputChanged() const
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_)
      return printer_->getIsOutputChanged();
    return printerOutputChangedFlag;
  }

  void Plus4VM::clearPrinterOutputChangedFlag()
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (printer_)
      printer_->clearOutputChangedFlag();
    printerOutputChangedFlag = false;
  }

  void Plus4VM::setPrinterFormFeedOn(bool isEnabled)
  {
    printerFormFeedOn = isEnabled;
    SerialDevice  *printerDevice = serialDevices[printerDeviceNumber];
    if (printerDevice) {
      if (typeid(*printerDevice) == typeid(VC1526))
        reinterpret_cast<VC1526 *>(printerDevice)->setFormFeedOn(isEnabled);
    }
  }

  void Plus4VM::setPrinterTextOutputFile(const char *fileName, bool asciiMode)
  {
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (!printer_) {
      if (fileName != (char *) 0 && fileName[0] != '\0') {
        throw Plus4Emu::Exception("cannot set printer output file "
                                  "- printer emulation is not enabled");
      }
    }
    else {
      printer_->setTextOutputFile(fileName, asciiMode);
    }
  }

  void Plus4VM::getVMStatus(VMStatus& vmStatus_)
  {
    vmStatus_.tapeReadOnly = getIsTapeReadOnly();
    vmStatus_.tapePosition = getTapePosition();
    vmStatus_.tapeLength = getTapeLength();
    vmStatus_.tapeSampleRate = getTapeSampleRate();
    vmStatus_.tapeSampleSize = getTapeSampleSize();
    uint64_t  h = 0UL;
    uint32_t  n = 0U;
    for (int i = 11; i >= 8; i--) {
      n = n << 8;
      h = h << 16;
      if (serialDevices[i] != (SerialDevice *) 0) {
        FloppyDrive&  floppyDrive =
            *(reinterpret_cast<FloppyDrive *>(serialDevices[i]));
        n |= uint32_t(floppyDrive.getLEDState() & 0xFF);
        h |= uint64_t(floppyDrive.getHeadPosition() ^ 0xFFFF);
      }
      else if (i == 8) {
        if (drive8Is1551)
          n |= uint32_t(iecDrive8->getLEDState() & 0xFF);
      }
      else if (i == 9) {
        if (drive9Is1551)
          n |= uint32_t(iecDrive9->getLEDState() & 0xFF);
      }
    }
    vmStatus_.floppyDriveLEDState = n;
    vmStatus_.floppyDriveHeadPositions = (~h);
    Printer *printer_ =
        reinterpret_cast<Printer *>(serialDevices[printerDeviceNumber]);
    if (!printer_) {
      vmStatus_.printerHeadPositionX = -1;
      vmStatus_.printerHeadPositionY = -1;
      vmStatus_.printerOutputChanged = printerOutputChangedFlag;
      vmStatus_.printerLEDState = 0x00;
    }
    else {
      printer_->getHeadPosition(vmStatus_.printerHeadPositionX,
                                vmStatus_.printerHeadPositionY);
      vmStatus_.printerOutputChanged = printer_->getIsOutputChanged();
      vmStatus_.printerLEDState = printer_->getLEDState();
    }
    vmStatus_.isPlayingDemo = isPlayingDemo;
    if (demoFile != (Plus4Emu::File *) 0 && !isRecordingDemo)
      stopDemoRecording(true);
    vmStatus_.isRecordingDemo = isRecordingDemo;
  }

  void Plus4VM::openVideoCapture(
      int frameRate_,
      bool yuvFormat_,
      void (*errorCallback_)(void *userData, const char *msg),
      void (*fileNameCallback_)(void *userData, std::string& fileName),
      void *userData_)
  {
    if (!videoCapture) {
      if (yuvFormat_) {
        videoCapture =
            new Plus4Emu::VideoCapture_YV12(&TED7360::convertPixelToYUV,
                                            frameRate_);
      }
      else {
        videoCapture =
            new Plus4Emu::VideoCapture_RLE8(&TED7360::convertPixelToYUV,
                                            frameRate_);
      }
      videoCapture->setClockFrequency(soundClockFrequency << 3);
      ted->setCallback(&videoCaptureCallback, this, 3);
    }
    videoCapture->setErrorCallback(errorCallback_, userData_);
    videoCapture->setFileNameCallback(fileNameCallback_, userData_);
    videoCapture->setNTSCMode(videoCaptureNTSCMode);
  }

  void Plus4VM::setVideoCaptureFile(const std::string& fileName_)
  {
    if (!videoCapture) {
      throw Plus4Emu::Exception("internal error: "
                                "video capture object does not exist");
    }
    videoCapture->openFile(fileName_.c_str());
  }

  void Plus4VM::setVideoCaptureNTSCMode(bool ntscMode)
  {
    videoCaptureNTSCMode = ntscMode;
    if (videoCapture)
      videoCapture->setNTSCMode(ntscMode);
  }

  void Plus4VM::closeVideoCapture()
  {
    if (videoCapture) {
      ted->setCallback(&videoCaptureCallback, this, 0);
      delete videoCapture;
      videoCapture = (Plus4Emu::VideoCapture *) 0;
    }
  }

  void Plus4VM::setDiskImageFile(int n, const std::string& fileName_,
                                 int driveType)
  {
    if (n < 0 || n > 3)
      throw Plus4Emu::Exception("invalid floppy drive number");
    if (driveType < 0 || driveType > 1)
      throw Plus4Emu::Exception("invalid floppy drive type");
    n = n + 8;
    std::FILE *imageFile = (std::FILE *) 0;
    try {
      int     oldDriveType = -1;
      if (serialDevices[n] != (SerialDevice *) 0) {
        if (typeid(*(serialDevices[n])) == typeid(VC1541))
          oldDriveType = 0;
        else if (typeid(*(serialDevices[n])) == typeid(VC1551))
          oldDriveType = 1;
        else if (typeid(*(serialDevices[n])) == typeid(VC1581))
          oldDriveType = 4;
      }
      int     newDriveType = driveType;
      bool    isReadOnly = false;
      if (fileName_.length() > 0) {
        // insert or replace disk
        bool    isD64 = false;
        bool    isD81 = true;
        int     fileType = 1;
        imageFile = Plus4Emu::openPlus4ImageFile(fileName_.c_str(),
                                                 fileType, isReadOnly);
        if (!imageFile)
          throw Plus4Emu::Exception("error opening disk image file");
        // find out file type
        if (std::fseek(imageFile, 0L, SEEK_END) >= 0) {
          long    fSize = std::ftell(imageFile);
          long    nSectors = fSize / 256L;
          if ((nSectors * 256L) != fSize) {
            // allow error info (one byte per sector at the end of the file)
            nSectors = fSize / 257L;
            if ((nSectors * 257L) != fSize)
              nSectors = 0L;
          }
          nSectors -= 683L;
          // allow any number of D64 tracks from 35 to 42
          isD64 = (nSectors >= 0L && nSectors <= 119L &&
                   ((nSectors / 17L) * 17L) == nSectors);
          isD81 = (fSize == (80L * 2L * 10L * 512L));
        }
        if (!isD64 && !isD81)
          throw Plus4Emu::Exception("disk image is not a D64 or D81 file");
        newDriveType = (isD64 ? newDriveType : 4);
        if (newDriveType == 1) {
          if (n >= 10) {
            throw Plus4Emu::Exception("1551 emulation is only allowed "
                                      "for unit 8 and unit 9");
          }
        }
      }
      else {
        // remove disk
        if (oldDriveType < 0 || (oldDriveType != 1 && newDriveType != 1))
          newDriveType = oldDriveType;
        else if (newDriveType != oldDriveType)
          newDriveType = -1;
      }
      if (oldDriveType >= 0 && newDriveType != oldDriveType) {
        // need to change drive type
        removeFloppyCallback(n);
        delete serialDevices[n];
        serialDevices[n] = (SerialDevice *) 0;
        ted->serialPort.removeDevice(n);
      }
      if (serialDevices[n] == (SerialDevice *) 0 && newDriveType >= 0) {
        if (n == 8)
          iecDrive8->reset();
        else if (n == 9)
          iecDrive9->reset();
        FloppyDrive *floppyDrive = (FloppyDrive *) 0;
        switch (newDriveType) {
        case 0:
          {
            VC1541  *floppyDrive_ = new VC1541(ted->serialPort, n);
            serialDevices[n] = floppyDrive_;
            floppyDrive_->setSerialBusDelayOffset(int(serialBusDelayOffset));
            floppyDrive = floppyDrive_;
            floppyDrive->setROMImage(2, floppyROM_1541);
          }
          break;
        case 1:
          floppyDrive = new VC1551(ted->serialPort, n);
          serialDevices[n] = floppyDrive;
          floppyDrive->setROMImage(3, floppyROM_1551);
          break;
        case 4:
          floppyDrive = new VC1581(ted->serialPort, n);
          serialDevices[n] = floppyDrive;
          floppyDrive->setROMImage(0, floppyROM_1581_0);
          floppyDrive->setROMImage(1, floppyROM_1581_1);
          break;
        }
        addFloppyCallback(n);
        floppyDrive->setBreakPointCallback(breakPointCallback,
                                           breakPointCallbackUserData);
        M7501   *p = floppyDrive->getCPU();
        if (p) {
          p->setBreakPointPriorityThreshold(
              ted->getBreakPointPriorityThreshold());
          p->setBreakOnInvalidOpcode(ted->getIsBreakOnInvalidOpcode());
        }
      }
      if (serialDevices[n] != (SerialDevice *) 0) {
        reinterpret_cast<FloppyDrive *>(serialDevices[n])->setDiskImageFile(
            imageFile, isReadOnly);
      }
    }
    catch (...) {
      if (imageFile)
        std::fclose(imageFile);
      if (serialDevices[8] != (SerialDevice *) 0)
        drive8Is1551 = (typeid(*(serialDevices[8])) == typeid(VC1551));
      else if (n == 8)
        drive8Is1551 = (driveType == 1);
      if (serialDevices[9] != (SerialDevice *) 0)
        drive9Is1551 = (typeid(*(serialDevices[9])) == typeid(VC1551));
      else if (n == 9)
        drive9Is1551 = (driveType == 1);
      throw;
    }
    if (serialDevices[8] != (SerialDevice *) 0)
      drive8Is1551 = (typeid(*(serialDevices[8])) == typeid(VC1551));
    else if (n == 8)
      drive8Is1551 = (driveType == 1);
    if (serialDevices[9] != (SerialDevice *) 0)
      drive9Is1551 = (typeid(*(serialDevices[9])) == typeid(VC1551));
    else if (n == 9)
      drive9Is1551 = (driveType == 1);
  }

  uint32_t Plus4VM::getFloppyDriveLEDState() const
  {
    uint32_t  n = 0U;
    for (int i = 11; i >= 8; i--) {
      n = n << 8;
      if (serialDevices[i] != (SerialDevice *) 0) {
        FloppyDrive&  floppyDrive =
            *(reinterpret_cast<FloppyDrive *>(serialDevices[i]));
        n |= uint32_t(floppyDrive.getLEDState() & 0xFF);
      }
      else if (i == 8) {
        if (drive8Is1551)
          n |= uint32_t(iecDrive8->getLEDState() & 0xFF);
      }
      else if (i == 9) {
        if (drive9Is1551)
          n |= uint32_t(iecDrive9->getLEDState() & 0xFF);
      }
    }
    return n;
  }

  uint64_t Plus4VM::getFloppyDriveHeadPositions() const
  {
    uint64_t  h = 0UL;
    for (int i = 11; i >= 8; i--) {
      h = h << 16;
      if (serialDevices[i] != (SerialDevice *) 0) {
        FloppyDrive&  floppyDrive =
            *(reinterpret_cast<FloppyDrive *>(serialDevices[i]));
        h |= uint64_t(floppyDrive.getHeadPosition() ^ 0xFFFF);
      }
    }
    return (~h);
  }

  void Plus4VM::setFloppyDriveHighAccuracy(bool isEnabled)
  {
    if (isEnabled == is1541HighAccuracy)
      return;
    is1541HighAccuracy = isEnabled;
    for (int i = 8; i < 12; i++) {
      if (serialDevices[i] != (SerialDevice *) 0) {
        removeFloppyCallback(i);
        addFloppyCallback(i);
      }
    }
  }

  void Plus4VM::setSerialBusDelayOffset(int n)
  {
    n = (n > -100 ? (n < 100 ? n : 100) : -100);
    if (n != int(serialBusDelayOffset)) {
      for (int i = 8; i < 12; i++) {
        if (serialDevices[i] != (SerialDevice *) 0) {
          if (typeid(*(serialDevices[i])) == typeid(VC1541)) {
            reinterpret_cast<VC1541 *>(
                serialDevices[i])->setSerialBusDelayOffset(n);
          }
        }
      }
      serialBusDelayOffset = int16_t(n);
    }
  }

  void Plus4VM::disableUnusedFloppyDrives()
  {
    for (int i = 8; i < 12; i++) {
      if (serialDevices[i] != (SerialDevice *) 0) {
        FloppyDrive&  floppyDrive =
            *(reinterpret_cast<FloppyDrive *>(serialDevices[i]));
        if (!floppyDrive.haveDisk()) {
          // "garbage collect" unused floppy drives to improve performance
          removeFloppyCallback(i);
          delete serialDevices[i];
          serialDevices[i] = (SerialDevice *) 0;
          ted->serialPort.removeDevice(i);
        }
      }
    }
  }

  void Plus4VM::resetFloppyDrive(int n)
  {
    if (n < 0) {
      for (int i = 0; i < 4; i++)
        resetFloppyDrive(i);
      return;
    }
    n = (n & 3) | 8;
    if (serialDevices[n] != (SerialDevice *) 0)
      serialDevices[n]->reset();
    if (n == 8)
      iecDrive8->reset();
    else if (n == 9)
      iecDrive9->reset();
  }

  void Plus4VM::setIECDriveReadOnlyMode(bool isReadOnly)
  {
    iecDrive8->setReadOnlyMode(isReadOnly);
    iecDrive9->setReadOnlyMode(isReadOnly);
  }

  void Plus4VM::setWorkingDirectory(const std::string& dirName_)
  {
    iecDrive8->setWorkingDirectory(dirName_);
    iecDrive9->setWorkingDirectory(dirName_);
    VirtualMachine::setWorkingDirectory(dirName_);
  }

  void Plus4VM::setTapeFileName(const std::string& fileName)
  {
    Plus4Emu::VirtualMachine::setTapeFileName(fileName);
    setTapeMotorState(ted->getTapeMotorState());
    if (haveTape()) {
      int64_t tmp = getTapeSampleRate();
      tapeTimesliceLength = ((int64_t(1000000) << 32) + (tmp >> 1)) / tmp;
    }
    else
      tapeTimesliceLength = 0;
    tapeTimeRemaining = 0;
  }

  void Plus4VM::setTapeFeedbackLevel(int n)
  {
    n = (n >= -10 ? (n <= 10 ? n : 10) : -10);
    tapeFeedbackLevel = int8_t(n);
    if (n > 0) {
      tapeFeedbackMult = int32_t(std::pow(2.0, double(n) * 0.5)
                                 * 5632.0 * 1117.0 / double(sidOutputVolume)
                                 + 0.5);
    }
    else if (n < 0) {
      tapeFeedbackMult = int32_t(std::pow(2.0, double(n) * -0.5)
                                 * -5632.0 * 1117.0 / double(sidOutputVolume)
                                 - 0.5);
    }
    else {
      tapeFeedbackMult = 0;
    }
  }

  void Plus4VM::tapePlay()
  {
    Plus4Emu::VirtualMachine::tapePlay();
    // tape button state sensing is disabled while recording or playing demo
    ted->setTapeButtonState(!(isRecordingDemo || isPlayingDemo) &&
                            getTapeButtonState() != 0);
    if (haveTape() && getIsTapeMotorOn() && getTapeButtonState() != 0)
      stopDemo();
  }

  void Plus4VM::tapeRecord()
  {
    Plus4Emu::VirtualMachine::tapeRecord();
    // tape button state sensing is disabled while recording or playing demo
    ted->setTapeButtonState(!(isRecordingDemo || isPlayingDemo) &&
                            getTapeButtonState() != 0);
    if (haveTape() && getIsTapeMotorOn() && getTapeButtonState() != 0)
      stopDemo();
  }

  void Plus4VM::tapeStop()
  {
    Plus4Emu::VirtualMachine::tapeStop();
    ted->setTapeButtonState(false);
  }

  void Plus4VM::setDebugContext(int n)
  {
    currentDebugContext = (n >= 0 ? (n <= 5 ? n : 5) : 0);
    // disable single stepping in other debug contexts
    for (int i = 0; i < 6; i++) {
      if (i != currentDebugContext) {
        M7501   *p = (M7501 *) 0;
        if (i == 0) {
          p = ted;
        }
        else {
          int     tmp = (i <= 4 ? (i + 7) : printerDeviceNumber);
          if (serialDevices[tmp] != (SerialDevice *) 0)
            p = serialDevices[tmp]->getCPU();
        }
        if (p)
          p->setSingleStepMode(0);
      }
    }
  }

  void Plus4VM::setBreakPoint(int bpType, uint16_t bpAddr, int bpPriority)
  {
    bpPriority = (bpPriority < 3 ? bpPriority : 3);
    if (bpType == 4) {
      if (currentDebugContext != 0) {
        throw Plus4Emu::Exception("video breakpoints can only be set "
                                  "for the main CPU");
      }
      // correct video position for FF1E read delay
      uint16_t  addrX = (bpAddr + 1) & 0x7F;
      bpAddr = bpAddr & 0xFF80;
      if (addrX != 114)
        bpAddr = bpAddr | addrX;
      if (bpPriority >= 0) {
        if (videoBreakPointCnt == 0) {
          if (!videoBreakPoints) {
            videoBreakPoints = new uint8_t[65536];
            for (size_t j = 0; j <= 0xFFFF; j++)
              videoBreakPoints[j] = 0;
          }
          ted->setCallback(&videoBreakPointCheckCallback, this, 3);
        }
        if (!videoBreakPoints[bpAddr])
          videoBreakPointCnt++;
        if (bpPriority >= int(videoBreakPoints[bpAddr]))
          videoBreakPoints[bpAddr] = uint8_t(bpPriority + 1);
      }
      else if (videoBreakPoints) {
        if (videoBreakPoints[bpAddr]) {
          videoBreakPoints[bpAddr] = 0;
          videoBreakPointCnt--;
          if (!videoBreakPointCnt)
            ted->setCallback(&videoBreakPointCheckCallback, this, 0);
        }
      }
      return;
    }
    M7501   *p = getDebugCPU();
    if (!p)
      return;
    if (bpType < 0 || bpType > 6) {
      bpType = 0;
    }
    else {
      bpType = ((bpType == 0 || bpType == 3) ?
                7 : (bpType == 6 ? 4 : (bpType == 5 ? 8 : bpType)));
    }
    p->setBreakPoint(bpType, bpAddr, bpPriority);
  }

  void Plus4VM::setBreakPoints(const Plus4Emu::BreakPointList& bpList)
  {
    for (size_t i = 0; i < bpList.getBreakPointCnt(); i++) {
      const Plus4Emu::BreakPoint& bp = bpList.getBreakPoint(i);
      setBreakPoint(bp.type(), bp.addr(), bp.priority());
    }
  }

  void Plus4VM::clearBreakPoints()
  {
    M7501   *p = getDebugCPU();
    if (p)
      p->clearBreakPoints();
    if (currentDebugContext == 0 && videoBreakPoints) {
      ted->setCallback(&videoBreakPointCheckCallback, this, 0);
      videoBreakPointCnt = 0;
      delete[] videoBreakPoints;
      videoBreakPoints = (uint8_t *) 0;
    }
  }

  void Plus4VM::setBreakPointPriorityThreshold(int n)
  {
    ted->setBreakPointPriorityThreshold(n);
    for (int i = 0; i < 5; i++) {
      M7501   *p = (M7501 *) 0;
      int     tmp = (i < 4 ? (i + 8) : printerDeviceNumber);
      if (serialDevices[tmp] != (SerialDevice *) 0)
        p = serialDevices[tmp]->getCPU();
      if (p)
        p->setBreakPointPriorityThreshold(n);
    }
  }

  void Plus4VM::setSingleStepMode(int mode_)
  {
    M7501   *p = getDebugCPU();
    if (p)
      p->setSingleStepMode(mode_);
  }

  void Plus4VM::setSingleStepModeNextAddress(int32_t addr)
  {
    M7501   *p = getDebugCPU();
    if (p)
      p->setSingleStepModeNextAddress(addr);
  }

  void Plus4VM::setBreakOnInvalidOpcode(bool isEnabled)
  {
    ted->setBreakOnInvalidOpcode(isEnabled);
    for (int i = 0; i < 5; i++) {
      M7501   *p = (M7501 *) 0;
      int     tmp = (i < 4 ? (i + 8) : printerDeviceNumber);
      if (serialDevices[tmp] != (SerialDevice *) 0)
        p = serialDevices[tmp]->getCPU();
      if (p)
        p->setBreakOnInvalidOpcode(isEnabled);
    }
  }

  void Plus4VM::setBreakPointCallback(void (*breakPointCallback_)(
                                          void *userData,
                                          int debugContext_, int type,
                                          uint16_t addr, uint8_t value),
                                      void *userData_)
  {
    VirtualMachine::setBreakPointCallback(breakPointCallback_, userData_);
    for (int i = 0; i < 5; i++) {
      int     tmp = (i < 4 ? (i + 8) : printerDeviceNumber);
      if (serialDevices[tmp] != (SerialDevice *) 0) {
        serialDevices[tmp]->setBreakPointCallback(breakPointCallback_,
                                                  userData_);
      }
    }
  }

  uint8_t Plus4VM::getMemoryPage(int n) const
  {
    if (currentDebugContext == 0) {
      return ted->getMemoryPage(n);
    }
    else {
      int     tmp = (currentDebugContext <= 4 ?
                     (currentDebugContext + 7) : printerDeviceNumber);
      if (serialDevices[tmp] != (SerialDevice *) 0) {
        // serial devices are mapped to segments 40..6F
        return uint8_t((n & 3) | (tmp << 2) | 0x40);
      }
    }
    return uint8_t(0x7F);
  }

  uint8_t Plus4VM::readMemory(uint32_t addr, bool isCPUAddress) const
  {
    if (isCPUAddress) {
      if (currentDebugContext == 0) {
        return readMemory((addr & 0xFFFFU) | 0x00100000U, false);
      }
      else {
        int     tmp = (currentDebugContext <= 4 ?
                       (currentDebugContext + 7) : printerDeviceNumber);
        if (serialDevices[tmp] != (SerialDevice *) 0)
          return serialDevices[tmp]->readMemoryDebug(uint16_t(addr & 0xFFFFU));
      }
    }
    else {
      uint8_t segment = uint8_t((addr >> 14) & 0xFF);
      switch (segment) {
      case 0x0A:
        if (printerROM_MPS801)
          return printerROM_MPS801[addr & 0x0FFFU];
        break;
      case 0x0C:
        if (printerROM_1526)
          return printerROM_1526[addr & 0x1FFFU];
        break;
      case 0x10:
        if (floppyROM_1541)
          return floppyROM_1541[addr & 0x3FFFU];
        break;
      case 0x20:
        if (floppyROM_1551)
          return floppyROM_1551[addr & 0x3FFFU];
        break;
      case 0x30:
        if (floppyROM_1581_0)
          return floppyROM_1581_0[addr & 0x3FFFU];
        break;
      case 0x31:
        if (floppyROM_1581_1)
          return floppyROM_1581_1[addr & 0x3FFFU];
        break;
      case 0x40:
      case 0x41:
      case 0x42:
      case 0x43:
        switch (addr & 0xFFF0U) {
        case 0xFD40U:
        case 0xFD50U:
        case 0xFE80U:
        case 0xFE90U:
          return sid_->readDebug(addr);
        case 0xFD00U:
          if (aciaEnabled)
            return acia_.readRegisterDebug(uint16_t(addr & 0xFFFFU));
        }
        return ted->readMemoryCPU(uint16_t(addr & 0xFFFFU));
      case 0x50:
      case 0x51:
      case 0x52:
      case 0x53:
      case 0x54:
      case 0x55:
      case 0x56:
      case 0x57:
      case 0x60:
      case 0x61:
      case 0x62:
      case 0x63:
      case 0x64:
      case 0x65:
      case 0x66:
      case 0x67:
      case 0x68:
      case 0x69:
      case 0x6A:
      case 0x6B:
      case 0x6C:
      case 0x6D:
      case 0x6E:
      case 0x6F:
        {
          const SerialDevice  *p = serialDevices[(segment >> 2) & 15];
          if (p)
            return p->readMemoryDebug(uint16_t(addr & 0xFFFFU));
        }
        break;
      default:
        return ted->readMemoryRaw(addr & uint32_t(0x003FFFFF));
      }
    }
    return uint8_t(0xFF);
  }

  void Plus4VM::writeMemory(uint32_t addr, uint8_t value, bool isCPUAddress)
  {
    if (isRecordingDemo || isPlayingDemo) {
      stopDemoPlayback();
      stopDemoRecording(false);
    }
    if (isCPUAddress) {
      if (currentDebugContext == 0) {
        ted->writeMemoryCPU(uint16_t(addr & 0xFFFFU), value);
      }
      else {
        int     tmp = (currentDebugContext <= 4 ?
                       (currentDebugContext + 7) : printerDeviceNumber);
        if (serialDevices[tmp] != (SerialDevice *) 0)
          serialDevices[tmp]->writeMemoryDebug(uint16_t(addr & 0xFFFFU), value);
      }
    }
    else {
      if (addr >= 0x00200000U) {
        ted->writeMemoryRaw(addr & uint32_t(0x003FFFFF), value);
      }
      else {
        uint32_t  tmp = (addr >> 16) & 0x3FU;
        switch (tmp) {
        case 0x10U:
          ted->writeMemoryCPU(uint16_t(addr & 0xFFFFU), value);
          break;
        case 0x14U:
        case 0x15U:
        case 0x18U:
        case 0x19U:
        case 0x1AU:
        case 0x1BU:
          {
            SerialDevice  *p = serialDevices[tmp & 15U];
            if (p)
              p->writeMemoryDebug(uint16_t(addr & 0xFFFFU), value);
          }
          break;
        }
      }
    }
  }

  uint16_t Plus4VM::getProgramCounter() const
  {
    const M7501 *p = getDebugCPU();
    if (p) {
      M7501Registers  r;
      p->getRegisters(r);
      return r.reg_PC;
    }
    return uint16_t(0xFFFF);
  }

  uint16_t Plus4VM::getStackPointer() const
  {
    const M7501 *p = getDebugCPU();
    if (p) {
      M7501Registers  r;
      p->getRegisters(r);
      return (uint16_t(0x0100)
              | uint16_t((r.reg_SP + uint8_t(1)) & uint8_t(0xFF)));
    }
    return uint16_t(0xFFFF);
  }

  void Plus4VM::listCPURegisters(std::string& buf) const
  {
    char        tmpBuf[96];
    const M7501 *p = getDebugCPU();
    if (p) {
      M7501Registers  r;
      p->getRegisters(r);
      std::sprintf(&(tmpBuf[0]),
                   " PC  SR AC XR YR SP\n"
                   "%04X %02X %02X %02X %02X %02X",
                   (unsigned int) r.reg_PC, (unsigned int) r.reg_SR,
                   (unsigned int) r.reg_AC, (unsigned int) r.reg_XR,
                   (unsigned int) r.reg_YR, (unsigned int) r.reg_SP);
    }
    else {
      std::sprintf(&(tmpBuf[0]),
                   " PC  SR AC XR YR SP\n"
                   "FFFF FF FF FF FF FF");
    }
    buf = &(tmpBuf[0]);
  }

  uint32_t Plus4VM::disassembleInstruction(std::string& buf,
                                           uint32_t addr, bool isCPUAddress,
                                           int32_t offs) const
  {
    return M7501Disassembler::disassembleInstruction(buf, (*this),
                                                     addr, isCPUAddress, offs);
  }

  void Plus4VM::getVideoPosition(int& xPos, int& yPos) const
  {
    xPos = int(ted->getVideoPositionX()) << 1;
    yPos = int(ted->getVideoPositionY());
  }

  void Plus4VM::setCPURegisters(const M7501Registers& r)
  {
    if (isRecordingDemo | isPlayingDemo) {
      if (currentDebugContext == 0) {
        stopDemoPlayback();
        stopDemoRecording(false);
      }
    }
    M7501   *p = getDebugCPU();
    if (p)
      p->setRegisters(r);
  }

  void Plus4VM::getCPURegisters(M7501Registers& r) const
  {
    const M7501 *p = getDebugCPU();
    if (p) {
      p->getRegisters(r);
    }
    else {
      r.reg_PC = 0xFFFF;
      r.reg_SR = 0xFF;
      r.reg_AC = 0xFF;
      r.reg_XR = 0xFF;
      r.reg_YR = 0xFF;
      r.reg_SP = 0xFF;
    }
  }

  void Plus4VM::saveState(Plus4Emu::File& f)
  {
    ted->saveState(f);
    sid_->saveState(f);
    {
      Plus4Emu::File::Buffer  buf;
      buf.setPosition(0);
      buf.writeUInt32(0x01000004);      // version number
      buf.writeUInt32(uint32_t(cpuClockFrequency));
      buf.writeUInt32(uint32_t(tedInputClockFrequency));
      buf.writeUInt32(uint32_t(soundClockFrequency));
      buf.writeBoolean(sidEnabled);
      buf.writeByte(sidFlags);
      buf.writeByte(sidCycleCnt);
      buf.writeBoolean(digiBlasterEnabled);
      buf.writeByte(digiBlasterOutput);
      buf.writeBoolean(aciaEnabled);
      buf.writeBoolean(aciaCallbackFlag);
      buf.writeInt64(aciaTimeRemaining);
      uint8_t tmpBuf[32];
      if (acia_.getSnapshotSize() > 32)
        throw Plus4Emu::Exception("internal error: snapshot buffer overflow");
      acia_.saveSnapshot(&(tmpBuf[0]));
      for (size_t i = 0; i < acia_.getSnapshotSize(); i++)
        buf.writeByte(tmpBuf[i]);
      f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_P4VM_STATE, buf);
    }
  }

  void Plus4VM::saveMachineConfiguration(Plus4Emu::File& f)
  {
    Plus4Emu::File::Buffer  buf;
    buf.setPosition(0);
    buf.writeUInt32(0x01000000);        // version number
    buf.writeUInt32(uint32_t(cpuClockFrequency));
    buf.writeUInt32(uint32_t(tedInputClockFrequency));
    buf.writeUInt32(uint32_t(soundClockFrequency));
    f.addChunk(Plus4Emu::File::PLUS4EMU_CHUNKTYPE_P4VM_CONFIG, buf);
  }

  void Plus4VM::saveProgram(Plus4Emu::File& f)
  {
    ted->saveProgram(f);
  }

  void Plus4VM::saveProgram(const char *fileName)
  {
    ted->saveProgram(fileName);
  }

  void Plus4VM::loadProgram(const char *fileName)
  {
    ted->loadProgram(fileName);
  }

  void Plus4VM::recordDemo(Plus4Emu::File& f)
  {
    // turn off tape motor, stop any previous demo recording or playback,
    // and reset keyboard state
    ted->setTapeMotorState(false);
    ted->setTapeInput(false);
    setTapeMotorState(false);
    ted->serialPort.removeDevices(0xFFFE);
    stopDemo();
    for (int i = 0; i < 128; i++)
      ted->setKeyState(i, false);
    // save full snapshot, including timing and clock frequency settings
    saveMachineConfiguration(f);
    saveState(f);
    demoBuffer.clear();
    demoBuffer.writeUInt32(0x0001020B); // version 1.2.11
    demoFile = &f;
    isRecordingDemo = true;
    ted->setCallback(&demoRecordCallback, this, 1);
    demoTimeCnt = 0U;
    // tape button state sensing is disabled while recording or playing demo
    ted->setTapeButtonState(false);
  }

  void Plus4VM::stopDemo()
  {
    stopDemoPlayback();
    stopDemoRecording(true);
  }

  bool Plus4VM::getIsRecordingDemo()
  {
    if (demoFile != (Plus4Emu::File *) 0 && !isRecordingDemo)
      stopDemoRecording(true);
    return isRecordingDemo;
  }

  bool Plus4VM::getIsPlayingDemo() const
  {
    return isPlayingDemo;
  }

  // --------------------------------------------------------------------------

  void Plus4VM::loadState(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (!(version >= 0x01000000 && version <= 0x01000004)) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible plus4 snapshot format");
    }
    ted->setTapeMotorState(false);
    setTapeMotorState(false);
    stopDemo();
    removePasteTextCallback();
    snapshotLoadFlag = true;
    disableUnusedFloppyDrives();
    resetFloppyDrive(-1);
    try {
      uint32_t  tmpCPUClockFrequency = buf.readUInt32();
      uint32_t  tmpTEDInputClockFrequency = buf.readUInt32();
      uint32_t  tmpSoundClockFrequency = buf.readUInt32();
      (void) tmpCPUClockFrequency;
      (void) tmpTEDInputClockFrequency;
      (void) tmpSoundClockFrequency;
      sidEnabled = buf.readBoolean();
      sidCycleCnt = 4;
      if (version != 0x01000000) {
        if (version < 0x01000004) {
          sidFlags = uint8_t(buf.readBoolean());
        }
        else {
          sidFlags = buf.readByte() & 7;
          sidCycleCnt = ((buf.readByte() - 1) & 15) + 1;
        }
        digiBlasterEnabled = buf.readBoolean();
        digiBlasterOutput = buf.readByte();
      }
      else {
        sidFlags = 0;
        digiBlasterEnabled = false;
        digiBlasterOutput = 0x80;
      }
      if (sidFlags & 1)
        sid_->set_chip_model(MOS6581);
      else
        sid_->set_chip_model(MOS8580);
      ted->setEnableC64CompatibleSID(bool(sidFlags & 2));
      if (digiBlasterEnabled)
        sid_->input((int(digiBlasterOutput) << 8) - 32768);
      else
        sid_->input(0);
      ted->setCallback(&SID::clockCallback, sid_,
                       int(sidEnabled) & int(!(sidFlags & 4)));
      ted->setCallback(&sidCallbackC64, this,
                       int(sidEnabled) & int(bool(sidFlags & 4)));
      aciaEnabled = (ted->getRAMSize() >= 64);
      resetACIA();
      if (version >= 0x01000002) {
        if (version >= 0x01000003) {
          aciaEnabled = buf.readBoolean();
          setEnableACIACallback(buf.readBoolean());
        }
        aciaTimeRemaining = buf.readInt64();
        uint8_t tmpBuf[32];
        if (acia_.getSnapshotSize() > 32)
          throw Plus4Emu::Exception("internal error: snapshot buffer overflow");
        for (size_t i = 0; i < acia_.getSnapshotSize(); i++)
          tmpBuf[i] = buf.readByte();
        if (aciaEnabled)
          acia_.loadSnapshot(&(tmpBuf[0]));
      }
      if (buf.getPosition() != buf.getDataSize())
        throw Plus4Emu::Exception("trailing garbage at end of "
                                  "plus4 snapshot data");
    }
    catch (...) {
      this->reset(true);
      throw;
    }
  }

  void Plus4VM::loadMachineConfiguration(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
    if (version != 0x01000000) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible plus4 "
                                "machine configuration format");
    }
    try {
      uint32_t  tmpCPUClockFrequency = buf.readUInt32();
      uint32_t  tmpTEDInputClockFrequency = buf.readUInt32();
      uint32_t  tmpSoundClockFrequency = buf.readUInt32();
      (void) tmpSoundClockFrequency;
      setCPUFrequency(tmpCPUClockFrequency);
      setVideoFrequency(tmpTEDInputClockFrequency);
      if (buf.getPosition() != buf.getDataSize())
        throw Plus4Emu::Exception("trailing garbage at end of "
                                  "plus4 machine configuration data");
    }
    catch (...) {
      this->reset(true);
      throw;
    }
  }

  void Plus4VM::loadDemo(Plus4Emu::File::Buffer& buf)
  {
    buf.setPosition(0);
    // check version number
    unsigned int  version = buf.readUInt32();
#if 0
    if (version != 0x0001020B) {
      buf.setPosition(buf.getDataSize());
      throw Plus4Emu::Exception("incompatible plus4 demo format");
    }
#endif
    (void) version;
    // turn off tape motor, stop any previous demo recording or playback,
    // and reset keyboard state
    ted->setTapeMotorState(false);
    ted->setTapeInput(false);
    setTapeMotorState(false);
    ted->serialPort.removeDevices(0xFFFE);
    stopDemo();
    for (int i = 0; i < 128; i++)
      ted->setKeyState(i, false);
    // initialize time counter with first delta time
    demoTimeCnt = readDemoTimeCnt(buf);
    isPlayingDemo = true;
    ted->setCallback(&demoPlayCallback, this, 1);
    // tape button state sensing is disabled while recording or playing demo
    ted->setTapeButtonState(false);
    // copy any remaining demo data to local buffer
    demoBuffer.clear();
    demoBuffer.writeData(buf.getData() + buf.getPosition(),
                         buf.getDataSize() - buf.getPosition());
    demoBuffer.setPosition(0);
  }

  class ChunkType_Plus4VMConfig : public Plus4Emu::File::ChunkTypeHandler {
   private:
    Plus4VM&  ref;
   public:
    ChunkType_Plus4VMConfig(Plus4VM& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_Plus4VMConfig()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_P4VM_CONFIG;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadMachineConfiguration(buf);
    }
  };

  class ChunkType_Plus4VMSnapshot : public Plus4Emu::File::ChunkTypeHandler {
   private:
    Plus4VM&  ref;
   public:
    ChunkType_Plus4VMSnapshot(Plus4VM& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_Plus4VMSnapshot()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_P4VM_STATE;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadState(buf);
    }
  };

  class ChunkType_Plus4DemoStream : public Plus4Emu::File::ChunkTypeHandler {
   private:
    Plus4VM&  ref;
   public:
    ChunkType_Plus4DemoStream(Plus4VM& ref_)
      : Plus4Emu::File::ChunkTypeHandler(),
        ref(ref_)
    {
    }
    virtual ~ChunkType_Plus4DemoStream()
    {
    }
    virtual Plus4Emu::File::ChunkType getChunkType() const
    {
      return Plus4Emu::File::PLUS4EMU_CHUNKTYPE_PLUS4_DEMO;
    }
    virtual void processChunk(Plus4Emu::File::Buffer& buf)
    {
      ref.loadDemo(buf);
    }
  };

  void Plus4VM::registerChunkTypes(Plus4Emu::File& f)
  {
    ChunkType_Plus4VMConfig   *p1 = (ChunkType_Plus4VMConfig *) 0;
    ChunkType_Plus4VMSnapshot *p2 = (ChunkType_Plus4VMSnapshot *) 0;
    ChunkType_Plus4DemoStream *p3 = (ChunkType_Plus4DemoStream *) 0;

    try {
      p1 = new ChunkType_Plus4VMConfig(*this);
      f.registerChunkType(p1);
      p1 = (ChunkType_Plus4VMConfig *) 0;
      p2 = new ChunkType_Plus4VMSnapshot(*this);
      f.registerChunkType(p2);
      p2 = (ChunkType_Plus4VMSnapshot *) 0;
      p3 = new ChunkType_Plus4DemoStream(*this);
      f.registerChunkType(p3);
      p3 = (ChunkType_Plus4DemoStream *) 0;
    }
    catch (...) {
      if (p1)
        delete p1;
      if (p2)
        delete p2;
      if (p3)
        delete p3;
      throw;
    }
    ted->registerChunkTypes(f);
    sid_->registerChunkType(f);
  }

}       // namespace Plus4

