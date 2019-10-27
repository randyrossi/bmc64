
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
#include "cfg_db.hpp"
#include "emucfg.hpp"
#include "system.hpp"

template <typename T>
static void configChangeCallback(void *userData,
                                 const std::string& name, T value)
{
  (void) name;
  (void) value;
  *((bool *) userData) = true;
}

template <typename T>
static void defineConfigurationVariable(
    Plus4Emu::ConfigurationDB& db, const std::string& name,
    T& value, T defaultValue, bool& changeFlag,
    double minVal, double maxVal, double step = 0.0)
{
  value = defaultValue;
  changeFlag = true;
  db.createKey(name, value);
  db[name].setRange(minVal, maxVal, step);
  db[name].setCallback(&(configChangeCallback<T>), &changeFlag, true);
}

static void defineConfigurationVariable(
    Plus4Emu::ConfigurationDB& db, const std::string& name,
    bool& value, bool defaultValue, bool& changeFlag)
{
  value = defaultValue;
  changeFlag = true;
  db.createKey(name, value);
  db[name].setCallback(&(configChangeCallback<bool>), &changeFlag, true);
}

static void defineConfigurationVariable(
    Plus4Emu::ConfigurationDB& db, const std::string& name,
    std::string& value, const std::string& defaultValue, bool& changeFlag)
{
  value = defaultValue;
  changeFlag = true;
  db.createKey(name, value);
  db[name].setStripString(true);
  db[name].setCallback(&(configChangeCallback<const std::string&>),
                       &changeFlag, true);
}

static void defaultErrorCallback(void *userData, const char *msg)
{
  (void) userData;
  throw Plus4Emu::Exception(msg);
}

namespace Plus4Emu {

  EmulatorConfiguration::EmulatorConfiguration(VirtualMachine& vm__,
                                               VideoDisplay& videoDisplay_,
                                               AudioOutput& audioOutput_)
    : vm_(vm__),
      videoDisplay(videoDisplay_),
      audioOutput(audioOutput_),
      errorCallback(&defaultErrorCallback),
      errorCallbackUserData((void *) 0)
  {
    defineConfigurationVariable(*this, "vm.cpuClockFrequency",
                                vm.cpuClockFrequency, 1U,
                                vmConfigurationChanged, 1.0, 150000000.0);
    defineConfigurationVariable(*this, "vm.videoClockFrequency",
                                vm.videoClockFrequency, 17734475U,
                                vmConfigurationChanged, 7159090.0, 35468950.0);
    defineConfigurationVariable(*this, "vm.speedPercentage",
                                vm.speedPercentage, 100U,
                                soundSettingsChanged, 0.0, 1000.0);
    defineConfigurationVariable(*this, "vm.serialBusDelayOffset",
                                vm.serialBusDelayOffset, int(0),
                                vmConfigurationChanged, -100.0, 100.0);
    defineConfigurationVariable(*this, "vm.sidOutputVolume",
                                vm.sidOutputVolume, int(0),
                                vmConfigurationChanged, -8.0, 2.0);
    defineConfigurationVariable(*this, "vm.processPriority",
                                vm.processPriority, int(0),
                                vmProcessPriorityChanged,
#ifdef WIN32
                                -2.0, 3.0
#else
                                // TODO: implement process priority setting
                                // on non-Windows platforms
                                0.0, 0.0
#endif
                                );
    defineConfigurationVariable(*this, "vm.enableACIA",
                                vm.enableACIA, false,
                                vmConfigurationChanged);
    defineConfigurationVariable(*this, "vm.sidModel6581",
                                vm.sidModel6581, false,
                                vmConfigurationChanged);
    defineConfigurationVariable(*this, "vm.sidDigiBlaster",
                                vm.sidDigiBlaster, false,
                                vmConfigurationChanged);
    defineConfigurationVariable(*this, "vm.sidRegistersC64",
                                vm.sidRegistersC64, false,
                                vmConfigurationChanged);
    defineConfigurationVariable(*this, "vm.sidClockC64",
                                vm.sidClockC64, false,
                                vmConfigurationChanged);
    // ----------------
    defineConfigurationVariable(*this, "memory.ram.size",
                                memory.ram.size, 64,
                                memoryConfigurationChanged, 16.0, 1024.0, 16.0);
    (*this)["memory.ram.size"].setRequirePowerOfTwo(true);
    defineConfigurationVariable(
        *this, "memory.ram.startupPattern", memory.ram.startupPattern,
        std::string(VirtualMachine::defaultRAMPatternString),
        memoryConfigurationChanged);
    for (size_t i = 0; i < 50; i++) {
      if (i >= 8 && i < 48 && (i != 10 && i != 12 && i != 16 && i != 32))
        continue;
      char  tmpBuf[24];
      char  *s = &(tmpBuf[0]);
      std::sprintf(s, "memory.rom.%02X.file", (unsigned int) i);
      defineConfigurationVariable(*this, std::string(s),
                                  memory.rom[i].file, std::string(""),
                                  memoryConfigurationChanged);
      std::sprintf(s, "memory.rom.%02X.offset", (unsigned int) i);
      defineConfigurationVariable(*this, std::string(s),
                                  memory.rom[i].offset, int(0),
                                  memoryConfigurationChanged, 0.0, 16760832.0);
    }
    // ----------------
    defineConfigurationVariable(*this, "display.enabled",
                                display.enabled, true,
                                displaySettingsChanged);
    defineConfigurationVariable(*this, "display.ntscMode",
                                display.ntscMode, false,
                                displaySettingsChanged);
    defineConfigurationVariable(*this, "display.bufferingMode",
                                display.bufferingMode, int(0),
                                displaySettingsChanged, 0.0, 2.0);
    defineConfigurationVariable(*this, "display.quality",
                                display.displayQuality, int(1),
                                displaySettingsChanged, 0.0, 3.0);
    defineConfigurationVariable(*this, "display.brightness",
                                display.brightness, 0.0f,
                                displaySettingsChanged, -0.5, 0.5);
    defineConfigurationVariable(*this, "display.contrast",
                                display.contrast, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.gamma",
                                display.gamma, 1.0f,
                                displaySettingsChanged, 0.25, 4.0);
    defineConfigurationVariable(*this, "display.hueShift",
                                display.hueShift, 0.0f,
                                displaySettingsChanged, -180.0, 180.0);
    defineConfigurationVariable(*this, "display.saturation",
                                display.saturation, 1.0f,
                                displaySettingsChanged, 0.0, 2.0);
    defineConfigurationVariable(*this, "display.red.brightness",
                                display.redBrightness, 0.0f,
                                displaySettingsChanged, -0.5, 0.5);
    defineConfigurationVariable(*this, "display.red.contrast",
                                display.redContrast, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.red.gamma",
                                display.redGamma, 1.0f,
                                displaySettingsChanged, 0.25, 4.0);
    defineConfigurationVariable(*this, "display.green.brightness",
                                display.greenBrightness, 0.0f,
                                displaySettingsChanged, -0.5, 0.5);
    defineConfigurationVariable(*this, "display.green.contrast",
                                display.greenContrast, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.green.gamma",
                                display.greenGamma, 1.0f,
                                displaySettingsChanged, 0.25, 4.0);
    defineConfigurationVariable(*this, "display.blue.brightness",
                                display.blueBrightness, 0.0f,
                                displaySettingsChanged, -0.5, 0.5);
    defineConfigurationVariable(*this, "display.blue.contrast",
                                display.blueContrast, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.blue.gamma",
                                display.blueGamma, 1.0f,
                                displaySettingsChanged, 0.25, 4.0);
    defineConfigurationVariable(*this, "display.palPhaseError",
                                display.palPhaseError, 8.0f,
                                displaySettingsChanged, -30.0, 30.0);
    defineConfigurationVariable(*this, "display.lineShade",
                                display.lineShade, 0.8f,
                                displaySettingsChanged, 0.0, 1.0);
    defineConfigurationVariable(*this, "display.blendScale",
                                display.blendScale, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.motionBlur",
                                display.motionBlur, 0.25f,
                                displaySettingsChanged, 0.0, 0.95);
    defineConfigurationVariable(*this, "display.width",
                                display.width, 768,
                                displaySettingsChanged, 384.0, 1536.0);
    defineConfigurationVariable(*this, "display.height",
                                display.height, 576,
                                displaySettingsChanged, 288.0, 1152.0);
    defineConfigurationVariable(*this, "display.pixelAspectRatio",
                                display.pixelAspectRatio, 1.0f,
                                displaySettingsChanged, 0.5, 2.0);
    defineConfigurationVariable(*this, "display.shaderSourcePAL",
                                display.shaderSourcePAL, std::string(""),
                                displaySettingsChanged);
    defineConfigurationVariable(*this, "display.shaderSourceNTSC",
                                display.shaderSourceNTSC, std::string(""),
                                displaySettingsChanged);
    // ----------------
    defineConfigurationVariable(*this, "sound.enabled",
                                sound.enabled, true,
                                soundSettingsChanged);
    defineConfigurationVariable(*this, "sound.highQuality",
                                sound.highQuality, true,
                                soundSettingsChanged);
    defineConfigurationVariable(*this, "sound.device",
                                sound.device, int(0),
                                soundSettingsChanged, -1.0, 1000.0);
    defineConfigurationVariable(*this, "sound.sampleRate",
                                sound.sampleRate, 48000.0f,
                                soundSettingsChanged, 11025.0, 96000.0);
    defineConfigurationVariable(*this, "sound.latency",
                                sound.latency, 0.07f,
                                soundSettingsChanged, 0.005, 0.5);
    defineConfigurationVariable(*this, "sound.hwPeriods",
                                sound.hwPeriods, int(16),
                                soundSettingsChanged, 2.0, 16.0);
    defineConfigurationVariable(*this, "sound.swPeriods",
                                sound.swPeriods, int(3),
                                soundSettingsChanged, 1.0, 16.0);
    defineConfigurationVariable(*this, "sound.file",
                                sound.file, std::string(""),
                                soundSettingsChanged);
    defineConfigurationVariable(*this, "sound.volume",
                                sound.volume, 0.7943f,
                                soundSettingsChanged, 0.01, 1.0);
    defineConfigurationVariable(*this, "sound.dcBlockFilter1Freq",
                                sound.dcBlockFilter1Freq, 5.0f,
                                soundSettingsChanged, 1.0, 1000.0);
    defineConfigurationVariable(*this, "sound.dcBlockFilter2Freq",
                                sound.dcBlockFilter2Freq, 15.0f,
                                soundSettingsChanged, 1.0, 1000.0);
    defineConfigurationVariable(*this, "sound.equalizer.mode",
                                sound.equalizer.mode, int(2),
                                soundSettingsChanged, -1.0, 2.0);
    defineConfigurationVariable(*this, "sound.equalizer.frequency",
                                sound.equalizer.frequency, 15000.0f,
                                soundSettingsChanged, 1.0, 100000.0);
    defineConfigurationVariable(*this, "sound.equalizer.level",
                                sound.equalizer.level, 0.5f,
                                soundSettingsChanged, 0.0001, 100.0);
    defineConfigurationVariable(*this, "sound.equalizer.q",
                                sound.equalizer.q, 0.5f,
                                soundSettingsChanged, 0.001, 100.0);
    // ----------------
    for (int i = 0; i < 128; i++) {
      char  tmpBuf[16];
      char  *s = &(tmpBuf[0]);
      std::sprintf(s, "keyboard.%02X.x", (unsigned int) i);
      for (int j = 0; j < 2; j++) {
        s[12] = '0' + char(j);
        defineConfigurationVariable(*this, std::string(s),
                                    keyboard[i][j], int(-1),
                                    keyboardMapChanged, -1.0, 65535.0);
      }
    }
    // ----------------
    joystickSettingsChanged = true;
    joystick.registerConfigurationVariables(*this);
    (*this)["joystick.enableJoystick"].setCallback(
        &configChangeCallback<bool>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.enablePWM"].setCallback(
        &configChangeCallback<bool>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.enableAutoFire"].setCallback(
        &configChangeCallback<bool>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.axisThreshold"].setCallback(
        &configChangeCallback<float>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.pwmFrequency"].setCallback(
        &configChangeCallback<float>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.autoFireFrequency"].setCallback(
        &configChangeCallback<float>, (void *) &joystickSettingsChanged, true);
    (*this)["joystick.autoFirePulseWidth"].setCallback(
        &configChangeCallback<float>, (void *) &joystickSettingsChanged, true);
    // ----------------
    for (int i = 0; i < 4; i++) {
      FloppyDriveSettings *floppy_ = (FloppyDriveSettings *) 0;
      bool                *floppyChanged_ = (bool *) 0;
      switch (i) {
      case 0:
        floppy_ = &(floppy.a);
        floppyChanged_ = &floppyAChanged;
        break;
      case 1:
        floppy_ = &(floppy.b);
        floppyChanged_ = &floppyBChanged;
        break;
      case 2:
        floppy_ = &(floppy.c);
        floppyChanged_ = &floppyCChanged;
        break;
      case 3:
        floppy_ = &(floppy.d);
        floppyChanged_ = &floppyDChanged;
        break;
      }
      char  tmpBuf[32];
      char  *s = &(tmpBuf[0]);
      std::sprintf(s, "floppy.%c.imageFile", int('a') + i);
      defineConfigurationVariable(*this, std::string(s),
                                  floppy_->imageFile, std::string(""),
                                  *floppyChanged_);
      std::sprintf(s, "floppy.%c.driveType", int('a') + i);
      defineConfigurationVariable(*this, std::string(s),
                                  floppy_->driveType, int(i == 1 ? 1 : 0),
                                  *floppyChanged_, 0.0, 1.0);
    }
    defineConfigurationVariable(*this, "floppy.highTimingAccuracy",
                                floppy.highTimingAccuracy, true,
                                floppyTimingAccuracyChanged);
    // ----------------
    defineConfigurationVariable(*this, "tape.imageFile",
                                tape.imageFile, std::string(""),
                                tapeFileChanged);
    defineConfigurationVariable(*this, "tape.defaultSampleRate",
                                tape.defaultSampleRate, int(24000),
                                tapeSettingsChanged,
                                10000.0, 120000.0);
    defineConfigurationVariable(*this, "tape.feedbackLevel",
                                tape.feedbackLevel, int(0),
                                tapeSettingsChanged,
                                -10.0, 10.0);
    defineConfigurationVariable(*this, "tape.soundFileChannel",
                                tape.soundFileChannel, int(0),
                                tapeSoundFileSettingsChanged,
                                0.0, 15.0);
    defineConfigurationVariable(*this, "tape.invertSoundFileSignal",
                                tape.invertSoundFileSignal, false,
                                tapeSoundFileSettingsChanged);
    defineConfigurationVariable(*this, "tape.enableSoundFileFilter",
                                tape.enableSoundFileFilter, false,
                                tapeSoundFileSettingsChanged);
    defineConfigurationVariable(*this, "tape.soundFileFilterMinFreq",
                                tape.soundFileFilterMinFreq, 500.0f,
                                tapeSoundFileSettingsChanged,
                                0.0, 2000.0);
    defineConfigurationVariable(*this, "tape.soundFileFilterMaxFreq",
                                tape.soundFileFilterMaxFreq, 5000.0f,
                                tapeSoundFileSettingsChanged,
                                1000.0, 20000.0);
    // ----------------
    defineConfigurationVariable(*this, "fileio.iecDriveReadOnlyMode",
                                fileio.iecDriveReadOnlyMode, true,
                                fileioSettingsChanged);
    defineConfigurationVariable(*this, "fileio.workingDirectory",
                                fileio.workingDirectory, std::string("."),
                                fileioSettingsChanged);
    // ----------------
    defineConfigurationVariable(*this, "debug.bpPriorityThreshold",
                                debug.bpPriorityThreshold, int(0),
                                debugSettingsChanged, 0.0, 4.0);
    defineConfigurationVariable(*this, "debug.breakOnInvalidOpcode",
                                debug.breakOnInvalidOpcode, false,
                                debugSettingsChanged);
    // ----------------
    defineConfigurationVariable(*this, "videoCapture.frameRate",
                                videoCapture.frameRate, int(30),
                                videoCaptureSettingsChanged, 24.0, 60.0);
    defineConfigurationVariable(*this, "videoCapture.yuvFormat",
                                videoCapture.yuvFormat, true,
                                videoCaptureSettingsChanged);
    // ----------------
    // videoCaptureSettingsChanged is used only as a dummy variable here
    defineConfigurationVariable(*this, "compressFiles",
                                compressFiles, true,
                                videoCaptureSettingsChanged);
  }

  EmulatorConfiguration::~EmulatorConfiguration()
  {
    keyboardMap.clear();
  }

  void EmulatorConfiguration::applySettings()
  {
    if (vmConfigurationChanged) {
      // assume none of these will throw exceptions
      vm_.setCPUFrequency(vm.cpuClockFrequency);
      vm_.setVideoFrequency(vm.videoClockFrequency);
      vm_.setSerialBusDelayOffset(vm.serialBusDelayOffset);
      vm_.setEnableACIAEmulation(vm.enableACIA);
      vm_.setSIDConfiguration(uint8_t(vm.sidModel6581)
                              | (uint8_t(vm.sidRegistersC64) << 1)
                              | (uint8_t(vm.sidClockC64) << 2),
                              vm.sidDigiBlaster, vm.sidOutputVolume);
      vmConfigurationChanged = false;
    }
    if (vmProcessPriorityChanged) {
      try {
        setProcessPriority(vm.processPriority);
      }
      catch (std::exception& e) {
        errorCallback(errorCallbackUserData, e.what());
      }
      vmProcessPriorityChanged = false;
    }
    if (memoryConfigurationChanged) {
      uint64_t  ramPattern = 0UL;
      if (memory.ram.startupPattern.length() > 0) {
        try {
          if (memory.ram.startupPattern.length() > 12)
            throw Exception("invalid RAM startup pattern string");
          for (size_t i = 0; i < memory.ram.startupPattern.length(); i++) {
            char    c = memory.ram.startupPattern[i];
            ramPattern = ramPattern << 4;
            if (c >= '0' && c <= '9')
              ramPattern |= uint64_t(c - '0');
            else if (c >= 'A' && c <= 'F')
              ramPattern |= uint64_t((c - 'A') + 10);
            else if (c >= 'a' && c <= 'f')
              ramPattern |= uint64_t((c - 'a') + 10);
            else
              throw Exception("invalid RAM startup pattern string");
          }
        }
        catch (std::exception& e) {
          ramPattern = 0UL;
          memory.ram.startupPattern = "";
          errorCallback(errorCallbackUserData, e.what());
        }
      }
      vm_.resetMemoryConfiguration(memory.ram.size, ramPattern);
      for (size_t i = 0; i < 50; i++) {
        if (i >= 8 && i < 48 && (i != 10 && i != 12 && i != 16 && i != 32))
          continue;
        try {
          vm_.loadROMSegment(uint8_t(i),
                             memory.rom[i].file.c_str(),
                             size_t(memory.rom[i].offset));
        }
        catch (Exception& e) {
          memory.rom[i].file = "";
          memory.rom[i].offset = 0;
          vm_.loadROMSegment(uint8_t(i), "", 0);
          errorCallback(errorCallbackUserData, e.what());
        }
      }
      memoryConfigurationChanged = false;
    }
    if (displaySettingsChanged) {
      // assume that changing the display settings will not fail
      vm_.setEnableDisplay(display.enabled);
      vm_.setVideoCaptureNTSCMode(display.ntscMode);
      display.indexToYUVFunc =
          videoDisplay.getDisplayParameters().indexToYUVFunc;
      videoDisplay.setDisplayParameters(display);
      // NOTE: resolution changes are not handled here
      displaySettingsChanged = false;
    }
    if (soundSettingsChanged) {
      bool    soundEnableFlag = (sound.enabled && vm.speedPercentage == 100U);
      videoDisplay.limitFrameRate(vm.speedPercentage == 0U);
      vm_.setEnableAudioOutput(soundEnableFlag);
      if (!soundEnableFlag) {
        // close device if sound is disabled
        audioOutput.closeDevice();
      }
      else {
        try {
          audioOutput.setParameters(sound.device, sound.sampleRate,
                                    sound.latency, sound.hwPeriods,
                                    sound.swPeriods);
        }
        catch (Exception& e) {
          audioOutput.closeDevice();
          errorCallback(errorCallbackUserData, e.what());
        }
      }
      vm_.setAudioOutputHighQuality(sound.highQuality);
      vm_.setAudioOutputVolume(sound.volume);
      vm_.setAudioOutputFilters(sound.dcBlockFilter1Freq,
                                sound.dcBlockFilter2Freq);
      vm_.setAudioOutputEqualizer(sound.equalizer.mode,
                                  sound.equalizer.frequency,
                                  sound.equalizer.level, sound.equalizer.q);
      try {
        audioOutput.setOutputFile(sound.file);
      }
      catch (Exception& e) {
        sound.file = "";
        audioOutput.setOutputFile(sound.file);
        errorCallback(errorCallbackUserData, e.what());
      }
      soundSettingsChanged = false;
    }
    if (keyboardMapChanged) {
      keyboardMap.clear();
      for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 2; j++) {
          if (keyboard[i][j] >= 0)
            keyboardMap[keyboard[i][j]] = i;
        }
      }
      keyboardMapChanged = false;
    }
    if (floppyAChanged) {
      try {
        vm_.setDiskImageFile(0, floppy.a.imageFile, floppy.a.driveType);
      }
      catch (Exception& e) {
        floppy.a.imageFile = "";
        vm_.setDiskImageFile(0, floppy.a.imageFile, floppy.a.driveType);
        errorCallback(errorCallbackUserData, e.what());
      }
      floppyAChanged = false;
    }
    if (floppyBChanged) {
      try {
        vm_.setDiskImageFile(1, floppy.b.imageFile, floppy.b.driveType);
      }
      catch (Exception& e) {
        floppy.b.imageFile = "";
        vm_.setDiskImageFile(1, floppy.b.imageFile, floppy.b.driveType);
        errorCallback(errorCallbackUserData, e.what());
      }
      floppyBChanged = false;
    }
    if (floppyCChanged) {
      if (floppy.c.driveType == 1)
        floppy.c.driveType = 0;         // 1551 is not allowed for this drive
      try {
        vm_.setDiskImageFile(2, floppy.c.imageFile, floppy.c.driveType);
      }
      catch (Exception& e) {
        floppy.c.imageFile = "";
        vm_.setDiskImageFile(2, floppy.c.imageFile, floppy.c.driveType);
        errorCallback(errorCallbackUserData, e.what());
      }
      floppyCChanged = false;
    }
    if (floppyDChanged) {
      if (floppy.d.driveType == 1)
        floppy.d.driveType = 0;         // 1551 is not allowed for this drive
      try {
        vm_.setDiskImageFile(3, floppy.d.imageFile, floppy.d.driveType);
      }
      catch (Exception& e) {
        floppy.d.imageFile = "";
        vm_.setDiskImageFile(3, floppy.d.imageFile, floppy.d.driveType);
        errorCallback(errorCallbackUserData, e.what());
      }
      floppyDChanged = false;
    }
    if (floppyTimingAccuracyChanged) {
      vm_.setFloppyDriveHighAccuracy(floppy.highTimingAccuracy);
      floppyTimingAccuracyChanged = false;
    }
    if (tapeSettingsChanged) {
      vm_.setDefaultTapeSampleRate(tape.defaultSampleRate);
      vm_.setTapeFeedbackLevel(tape.feedbackLevel);
      tapeSettingsChanged = false;
    }
    if (tapeFileChanged) {
      try {
        vm_.setTapeFileName(tape.imageFile);
      }
      catch (Exception& e) {
        tape.imageFile = "";
        vm_.setTapeFileName(tape.imageFile);
        errorCallback(errorCallbackUserData, e.what());
      }
      tapeFileChanged = false;
    }
    if (tapeSoundFileSettingsChanged) {
      vm_.setTapeSoundFileParameters(tape.soundFileChannel,
                                     tape.invertSoundFileSignal,
                                     tape.enableSoundFileFilter,
                                     tape.soundFileFilterMinFreq,
                                     tape.soundFileFilterMaxFreq);
      tapeSoundFileSettingsChanged = false;
    }
    if (fileioSettingsChanged) {
      vm_.setIECDriveReadOnlyMode(fileio.iecDriveReadOnlyMode);
      try {
        vm_.setWorkingDirectory(fileio.workingDirectory);
      }
      catch (Exception& e) {
        vm_.setIECDriveReadOnlyMode(true);
        fileio.workingDirectory = ".";
        try {
          vm_.setWorkingDirectory(fileio.workingDirectory);
        }
        catch (Exception&) {
        }
        errorCallback(errorCallbackUserData, e.what());
      }
      fileioSettingsChanged = false;
    }
    if (debugSettingsChanged) {
      vm_.setBreakPointPriorityThreshold(debug.bpPriorityThreshold);
      vm_.setBreakOnInvalidOpcode(debug.breakOnInvalidOpcode);
      debugSettingsChanged = false;
    }
    if (videoCaptureSettingsChanged) {
      videoCaptureSettingsChanged = false;
    }
  }

  int EmulatorConfiguration::convertKeyCode(int keyCode)
  {
    std::map< int, int >::iterator  i;
    i = keyboardMap.find(keyCode);
    if (i == keyboardMap.end())
      return -1;
    return ((*i).second & 127);
  }

  void EmulatorConfiguration::setErrorCallback(void (*func)(void *,
                                                            const char *),
                                               void *userData_)
  {
    if (func)
      errorCallback = func;
    else
      errorCallback = &defaultErrorCallback;
    errorCallbackUserData = userData_;
  }

}       // namespace Plus4Emu

