
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

#ifndef PLUS4EMU_EMUCFG_HPP
#define PLUS4EMU_EMUCFG_HPP

#include "plus4emu.hpp"
#include "cfg_db.hpp"
#include "display.hpp"
#include "joystick.hpp"
#include "soundio.hpp"
#include "vm.hpp"

#include <map>

namespace Plus4Emu {

  class EmulatorConfiguration : public ConfigurationDB {
   private:
    VirtualMachine& vm_;
    VideoDisplay&   videoDisplay;
    AudioOutput&    audioOutput;
    std::map< int, int >  keyboardMap;
    void            (*errorCallback)(void *, const char *);
    void            *errorCallbackUserData;
   public:
    struct {
      unsigned int  cpuClockFrequency;
      unsigned int  videoClockFrequency;
      unsigned int  speedPercentage;    // NOTE: this uses soundSettingsChanged
      int           serialBusDelayOffset;
      int           sidOutputVolume;
      int           processPriority;    // uses vmProcessPriorityChanged
      bool          enableACIA;
      bool          sidModel6581;
      bool          sidDigiBlaster;
      bool          sidRegistersC64;
      bool          sidClockC64;
    } vm;
    bool          vmConfigurationChanged;
    bool          vmProcessPriorityChanged;
    // --------
    struct MemoryRAMConfiguration_ {
      int         size;
      std::string startupPattern;
      MemoryRAMConfiguration_()
        : size(64),
          startupPattern(std::string(VirtualMachine::defaultRAMPatternString))
      {
      }
    };
    struct MemoryConfiguration_ {
      MemoryRAMConfiguration_   ram;
      struct ROMSegmentConfig {
        std::string file;
        int         offset;
        ROMSegmentConfig()
          : file(""),
            offset(0)
        {
        }
      };
      // ROM files can be loaded to segments 0x00 to 0x07, 0x0A, 0x0C, 0x10,
      // 0x20, 0x30, and 0x31
      ROMSegmentConfig  rom[50];
    };
    MemoryConfiguration_  memory;
    bool          memoryConfigurationChanged;
    // --------
    struct DisplayParameters_ : public VideoDisplay::DisplayParameters {
      bool        enabled;
      int         width;
      int         height;
    };
    DisplayParameters_  display;
    bool          displaySettingsChanged;
    // --------
    struct SoundConfiguration_ {
      bool        enabled;
      bool        highQuality;
      int         device;
      float       sampleRate;
      float       latency;
      int         hwPeriods;
      int         swPeriods;
      std::string file;
      float       volume;
      float       dcBlockFilter1Freq;
      float       dcBlockFilter2Freq;
      struct {
        int       mode;
        float     frequency;
        float     level;
        float     q;
      } equalizer;
    };
    SoundConfiguration_   sound;
    bool          soundSettingsChanged;
    // --------
    int           keyboard[128][2];
    bool          keyboardMapChanged;
    // --------
    JoystickInput::JoystickConfiguration  joystick;
    bool          joystickSettingsChanged;
    // --------
    struct FloppyDriveSettings {
      std::string imageFile;
      int         driveType;
      FloppyDriveSettings()
        : imageFile(""),
          driveType(0)
      {
      }
    };
    struct FloppyConfiguration_ {
      FloppyDriveSettings a;
      FloppyDriveSettings b;
      FloppyDriveSettings c;
      FloppyDriveSettings d;
      bool        highTimingAccuracy;
    };
    FloppyConfiguration_  floppy;
    bool          floppyAChanged;
    bool          floppyBChanged;
    bool          floppyCChanged;
    bool          floppyDChanged;
    bool          floppyTimingAccuracyChanged;
    // --------
    struct TapeConfiguration_ {
      std::string imageFile;
      int         defaultSampleRate;
      int         feedbackLevel;
      int         soundFileChannel;
      bool        invertSoundFileSignal;
      bool        enableSoundFileFilter;
      float       soundFileFilterMinFreq;
      float       soundFileFilterMaxFreq;
    };
    TapeConfiguration_    tape;
    bool          tapeFileChanged;
    bool          tapeSettingsChanged;
    bool          tapeSoundFileSettingsChanged;
    // --------
    struct FileIOConfiguration_ {
      std::string workingDirectory;
      bool        iecDriveReadOnlyMode;
    };
    FileIOConfiguration_  fileio;
    bool          fileioSettingsChanged;
    // --------
    struct {
      int         bpPriorityThreshold;
      bool        breakOnInvalidOpcode;
    } debug;
    bool          debugSettingsChanged;
    // --------
    struct {
      int         frameRate;
      bool        yuvFormat;
    } videoCapture;
    bool          videoCaptureSettingsChanged;
    // ----------------
    bool          compressFiles;
    // ----------------
    EmulatorConfiguration(VirtualMachine& vm__,
                          VideoDisplay& videoDisplay_,
                          AudioOutput& audioOutput_);
    virtual ~EmulatorConfiguration();
    void applySettings();
    int convertKeyCode(int keyCode);
    void setErrorCallback(void (*func)(void *userData, const char *msg),
                          void *userData_);
  };

}       // namespace Plus4Emu

#endif  // PLUS4EMU_EMUCFG_HPP

