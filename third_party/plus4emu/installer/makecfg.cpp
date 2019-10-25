
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
#include "fileio.hpp"
#include "system.hpp"
#include "cfg_db.hpp"
#include "mkcfg_fl.hpp"
#include "guicolor.hpp"

#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>

#ifdef WIN32
#  include <windows.h>
#  include <direct.h>
#else
#  include <sys/types.h>
#  include <sys/stat.h>
#endif

static int keyboardMap_P4[256] = {
  0xFF08,     -1, 0xFF0D, 0xFF8D, 0xFFFF,     -1, 0xFFC1,     -1,
  0xFFBE,     -1, 0xFFBF,     -1, 0xFFC0,     -1, 0x002D,     -1,
  0x0033,     -1, 0x0077,     -1, 0x0061,     -1, 0x0034,     -1,
  0x007A,     -1, 0x0073,     -1, 0x0065,     -1, 0xFFE1, 0xFFE2,
  0x0035,     -1, 0x0072,     -1, 0x0064,     -1, 0x0036,     -1,
  0x0063,     -1, 0x0066,     -1, 0x0074,     -1, 0x0078,     -1,
  0x0037,     -1, 0x0079,     -1, 0x0067,     -1, 0x0038,     -1,
  0x0062,     -1, 0x0068,     -1, 0x0075,     -1, 0x0076,     -1,
  0x0039,     -1, 0x0069,     -1, 0x006A,     -1, 0x0030,     -1,
  0x006D,     -1, 0x006B,     -1, 0x006F,     -1, 0x006E,     -1,
  0xFF54,     -1, 0x0070,     -1, 0x006C,     -1, 0xFF52,     -1,
  0x002E,     -1, 0x003B,     -1, 0x005D, 0xFFAD, 0x002C,     -1,
  0xFF51,     -1, 0x005C, 0xFFAA, 0x0027,     -1, 0xFF53,     -1,
  0xFF1B, 0x0060, 0x003D,     -1, 0x005B,     -1, 0x002F,     -1,
  0x0031,     -1, 0xFF50,     -1, 0xFFE4,     -1, 0x0032,     -1,
  0x0020,     -1, 0xFFE3,     -1, 0x0071,     -1, 0xFF61, 0xFF09,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  0xFFB8, 0xC006, 0xFFB2, 0xC007, 0xFFB4, 0xC004, 0xFFB6, 0xC005,
      -1,     -1,     -1,     -1,     -1,     -1, 0xFFB0, 0xC011,
  0xFFAF, 0xC002, 0xFFB5, 0xC003, 0xFFB7, 0xC000, 0xFFB9, 0xC001,
      -1,     -1,     -1,     -1, 0xFFAB, 0xC010,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1
};

#ifndef WIN32

static int keyboardMap_P4_HU[256] = {
  0xFF08,     -1, 0xFF0D, 0xFF8D, 0xFFFF,     -1, 0xFFC1,     -1,
  0xFFBE,     -1, 0xFFBF,     -1, 0xFFC0,     -1, 0x00FC,     -1,
  0x0033,     -1, 0x0077,     -1, 0x0061,     -1, 0x0034,     -1,
  0x0079,     -1, 0x0073,     -1, 0x0065,     -1, 0xFFE1, 0xFFE2,
  0x0035,     -1, 0x0072,     -1, 0x0064,     -1, 0x0036,     -1,
  0x0063,     -1, 0x0066,     -1, 0x0074,     -1, 0x0078,     -1,
  0x0037,     -1, 0x007A,     -1, 0x0067,     -1, 0x0038,     -1,
  0x0062,     -1, 0x0068,     -1, 0x0075,     -1, 0x0076,     -1,
  0x0039,     -1, 0x0069,     -1, 0x006A,     -1, 0x00F6,     -1,
  0x006D,     -1, 0x006B,     -1, 0x006F,     -1, 0x006E,     -1,
  0xFF54,     -1, 0x0070,     -1, 0x006C,     -1, 0xFF52,     -1,
  0x002E,     -1, 0x00E9,     -1, 0x00FA, 0xFFAD, 0x002C,     -1,
  0xFF51,     -1, 0x01FB, 0xFFAA, 0x00E1,     -1, 0xFF53,     -1,
  0xFF1B, 0x0030, 0x00F3,     -1, 0x01F5,     -1, 0x002D,     -1,
  0x0031,     -1, 0xFF50,     -1, 0xFFE4,     -1, 0x0032,     -1,
  0x0020,     -1, 0xFFE3,     -1, 0x0071,     -1, 0xFF61, 0xFF09,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  0xFFB8, 0xC006, 0xFFB2, 0xC007, 0xFFB4, 0xC004, 0xFFB6, 0xC005,
      -1,     -1,     -1,     -1,     -1,     -1, 0xFFB0, 0xC011,
  0xFFAF, 0xC002, 0xFFB5, 0xC003, 0xFFB7, 0xC000, 0xFFB9, 0xC001,
      -1,     -1,     -1,     -1, 0xFFAB, 0xC010,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1
};

#else   // WIN32

static int keyboardMap_P4_HU[256] = {
  0xFF08,     -1, 0xFF0D, 0xFF8D, 0xFFFF,     -1, 0xFFC1,     -1,
  0xFFBE,     -1, 0xFFBF,     -1, 0xFFC0,     -1, 0x002F,     -1,
  0x0033,     -1, 0x0077,     -1, 0x0061,     -1, 0x0034,     -1,
  0x0079,     -1, 0x0073,     -1, 0x0065,     -1, 0xFFE1, 0xFFE2,
  0x0035,     -1, 0x0072,     -1, 0x0064,     -1, 0x0036,     -1,
  0x0063,     -1, 0x0066,     -1, 0x0074,     -1, 0x0078,     -1,
  0x0037,     -1, 0x007A,     -1, 0x0067,     -1, 0x0038,     -1,
  0x0062,     -1, 0x0068,     -1, 0x0075,     -1, 0x0076,     -1,
  0x0039,     -1, 0x0069,     -1, 0x006A,     -1, 0x0060,     -1,
  0x006D,     -1, 0x006B,     -1, 0x006F,     -1, 0x006E,     -1,
  0xFF54,     -1, 0x0070,     -1, 0x006C,     -1, 0xFF52,     -1,
  0x002E,     -1, 0x003B,     -1, 0x005D, 0xFFAD, 0x002C,     -1,
  0xFF51,     -1, 0x005C, 0xFFAA, 0x0027,     -1, 0xFF53,     -1,
  0xFF1B, 0x0030, 0x003D,     -1, 0x005B,     -1, 0x002D,     -1,
  0x0031,     -1, 0xFF50,     -1, 0xFFE4,     -1, 0x0032,     -1,
  0x0020,     -1, 0xFFE3,     -1, 0x0071,     -1, 0xFF61, 0xFF09,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
  0xFFB8, 0xC006, 0xFFB2, 0xC007, 0xFFB4, 0xC004, 0xFFB6, 0xC005,
      -1,     -1,     -1,     -1,     -1,     -1, 0xFFB0, 0xC011,
  0xFFAF, 0xC002, 0xFFB5, 0xC003, 0xFFB7, 0xC000, 0xFFB9, 0xC001,
      -1,     -1,     -1,     -1, 0xFFAB, 0xC010,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1,
      -1,     -1,     -1,     -1,     -1,     -1,     -1,     -1
};

#endif  // WIN32

#include "shaders.hpp"

static const char *keyboardConfigFileNames[8] = {
  (char *) 0,                   // 0
  (char *) 0,                   // 1
  (char *) 0,                   // 2
  (char *) 0,                   // 3
  "P4_Keyboard_US.cfg",         // 4
  "P4_Keyboard_HU.cfg",         // 5
  (char *) 0,                   // 6
  (char *) 0                    // 7
};

static const char *shaderSourceFiles[] = {
  "pal.glsl",   shaderSourcePAL,
  "ntsc.glsl",  shaderSourceNTSC,
  (char *) 0
};

static void setKeyboardConfiguration(Plus4Emu::ConfigurationDB& config, int n)
{
  int     *keyboardMapPtr = &(keyboardMap_P4[0]);
  switch (n) {
  case 4:                               // Plus/4
    keyboardMapPtr = &(keyboardMap_P4[0]);
    break;
  case 5:                               // Plus/4 (HU)
    keyboardMapPtr = &(keyboardMap_P4_HU[0]);
    break;
  }
  for (unsigned int i = 0U; i < 256U; i++) {
    char    tmpBuf[16];
    std::sprintf(&(tmpBuf[0]), "keyboard.%02X.%X", (i >> 1), (i & 1U));
    config.createKey(std::string(&(tmpBuf[0])), keyboardMapPtr[i]);
  }
}

class Plus4EmuMachineConfiguration {
 private:
  struct {
    unsigned int  cpuClockFrequency;
    unsigned int  videoClockFrequency;
    int           serialBusDelayOffset;
    bool          enableACIA;
  } vm;
  struct {
    struct {
      int       size;
    } ram;
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
  } memory;
  struct {
    bool    ntscMode;
  } display;
 public:
  Plus4EmuMachineConfiguration(Plus4Emu::ConfigurationDB& config, int n,
                               const std::string& romDirectory);
  ~Plus4EmuMachineConfiguration()
  {
  }
};

static const char *machineConfigFileNames[16] = {
  "P4_16k_PAL.cfg",                     // 0
  "P4_16k_NTSC.cfg",                    // 1
  "P4_64k_PAL.cfg",                     // 2
  "P4_64k_NTSC.cfg",                    // 3
  "P4_16k_PAL_3PLUS1.cfg",              // 4
  "P4_16k_NTSC_3PLUS1.cfg",             // 5
  "P4_64k_PAL_3PLUS1.cfg",              // 6
  "P4_64k_NTSC_3PLUS1.cfg",             // 7
  "P4_16k_PAL_TapeTurbo.cfg",           // 8
  "P4_16k_NTSC_TapeTurbo.cfg",          // 9
  "P4_64k_PAL_TapeTurbo.cfg",           // 10
  "P4_64k_NTSC_TapeTurbo.cfg",          // 11
  "P4_16k_PAL_3PLUS1_TapeTurbo.cfg",    // 12
  "P4_16k_NTSC_3PLUS1_TapeTurbo.cfg",   // 13
  "P4_64k_PAL_3PLUS1_TapeTurbo.cfg",    // 14
  "P4_64k_NTSC_3PLUS1_TapeTurbo.cfg"    // 15
};

Plus4EmuMachineConfiguration::Plus4EmuMachineConfiguration(
    Plus4Emu::ConfigurationDB& config, int n, const std::string& romDirectory)
{
  vm.cpuClockFrequency = 1U;
  vm.videoClockFrequency = ((n & 1) == 0 ? 17734475U : 14318180U);
  vm.serialBusDelayOffset = 0;
  vm.enableACIA = bool(n & 2);
  memory.ram.size = ((n & 2) == 0 ? 16 : 64);
  memory.rom[0x00].file = romDirectory + "p4_basic.rom";
  if ((n & 1) == 0) {
    display.ntscMode = false;
    memory.rom[0x01].file = romDirectory + "p4kernal.rom";
  }
  else {
    display.ntscMode = true;
    memory.rom[0x01].file = romDirectory + "p4_ntsc.rom";
  }
  if ((n & 4) != 0) {
    memory.rom[0x02].file = romDirectory + "3plus1.rom";
    memory.rom[0x03].file = romDirectory + "3plus1.rom";
    memory.rom[0x03].offset = 16384;
  }
  if (n >= 8)
    memory.rom[0x06].file = romDirectory + "p4fileio.rom";
  memory.rom[0x0A].file = romDirectory + "mps801.rom";
  memory.rom[0x0C].file = romDirectory + "1526_07c.rom";
  memory.rom[0x10].file = romDirectory + "dos1541.rom";
  memory.rom[0x20].file = romDirectory + "dos1551.rom";
  memory.rom[0x30].file = romDirectory + "dos1581.rom";
  memory.rom[0x31].file = romDirectory + "dos1581.rom";
  memory.rom[0x31].offset = 16384;
  config.createKey("vm.cpuClockFrequency", vm.cpuClockFrequency);
  config.createKey("vm.videoClockFrequency", vm.videoClockFrequency);
  config.createKey("vm.serialBusDelayOffset", vm.serialBusDelayOffset);
  config.createKey("vm.enableACIA", vm.enableACIA);
  config.createKey("memory.ram.size", memory.ram.size);
  config.createKey("memory.rom.00.file", memory.rom[0x00].file);
  config.createKey("memory.rom.00.offset", memory.rom[0x00].offset);
  config.createKey("memory.rom.01.file", memory.rom[0x01].file);
  config.createKey("memory.rom.01.offset", memory.rom[0x01].offset);
  config.createKey("memory.rom.02.file", memory.rom[0x02].file);
  config.createKey("memory.rom.02.offset", memory.rom[0x02].offset);
  config.createKey("memory.rom.03.file", memory.rom[0x03].file);
  config.createKey("memory.rom.03.offset", memory.rom[0x03].offset);
  config.createKey("memory.rom.04.file", memory.rom[0x04].file);
  config.createKey("memory.rom.04.offset", memory.rom[0x04].offset);
  config.createKey("memory.rom.05.file", memory.rom[0x05].file);
  config.createKey("memory.rom.05.offset", memory.rom[0x05].offset);
  config.createKey("memory.rom.06.file", memory.rom[0x06].file);
  config.createKey("memory.rom.06.offset", memory.rom[0x06].offset);
  config.createKey("memory.rom.07.file", memory.rom[0x07].file);
  config.createKey("memory.rom.07.offset", memory.rom[0x07].offset);
  config.createKey("memory.rom.0A.file", memory.rom[0x0A].file);
  config.createKey("memory.rom.0A.offset", memory.rom[0x0A].offset);
  config.createKey("memory.rom.0C.file", memory.rom[0x0C].file);
  config.createKey("memory.rom.0C.offset", memory.rom[0x0C].offset);
  config.createKey("memory.rom.10.file", memory.rom[0x10].file);
  config.createKey("memory.rom.10.offset", memory.rom[0x10].offset);
  config.createKey("memory.rom.20.file", memory.rom[0x20].file);
  config.createKey("memory.rom.20.offset", memory.rom[0x20].offset);
  config.createKey("memory.rom.30.file", memory.rom[0x30].file);
  config.createKey("memory.rom.30.offset", memory.rom[0x30].offset);
  config.createKey("memory.rom.31.file", memory.rom[0x31].file);
  config.createKey("memory.rom.31.offset", memory.rom[0x31].offset);
  config.createKey("display.ntscMode", display.ntscMode);
}

class Plus4EmuDisplaySndConfiguration {
 private:
    struct {
      int         quality;
    } display;
    struct {
      float       latency;
      int         hwPeriods;
    } sound;
 public:
  Plus4EmuDisplaySndConfiguration(Plus4Emu::ConfigurationDB& config)
  {
    display.quality = 1;
    sound.latency = 0.07;
    sound.hwPeriods = 16;
    config.createKey("display.quality", display.quality);
    config.createKey("sound.latency", sound.latency);
    config.createKey("sound.hwPeriods", sound.hwPeriods);
  }
};

class Plus4EmuGUIConfiguration {
 private:
  struct {
    std::string snapshotDirectory;
    std::string demoDirectory;
    std::string soundFileDirectory;
    std::string configDirectory;
    std::string loadFileDirectory;
    std::string tapeImageDirectory;
    std::string diskImageDirectory;
    std::string romImageDirectory;
    std::string prgFileDirectory;
    std::string debuggerDirectory;
    std::string screenshotDirectory;
  } gui;
 public:
  Plus4EmuGUIConfiguration(Plus4Emu::ConfigurationDB& config,
                           const std::string& installDirectory)
  {
    gui.snapshotDirectory = ".";
    gui.demoDirectory = installDirectory + "demo";
    gui.soundFileDirectory = ".";
    gui.configDirectory = installDirectory + "config";
    gui.loadFileDirectory = ".";
    gui.tapeImageDirectory = installDirectory + "tape";
    gui.diskImageDirectory = installDirectory + "disk";
    gui.romImageDirectory = installDirectory + "roms";
    gui.prgFileDirectory = installDirectory + "progs";
    gui.debuggerDirectory = ".";
    gui.screenshotDirectory = ".";
    config.createKey("gui.snapshotDirectory", gui.snapshotDirectory);
    config.createKey("gui.demoDirectory", gui.demoDirectory);
    config.createKey("gui.soundFileDirectory", gui.soundFileDirectory);
    config.createKey("gui.configDirectory", gui.configDirectory);
    config.createKey("gui.loadFileDirectory", gui.loadFileDirectory);
    config.createKey("gui.tapeImageDirectory", gui.tapeImageDirectory);
    config.createKey("gui.diskImageDirectory", gui.diskImageDirectory);
    config.createKey("gui.romImageDirectory", gui.romImageDirectory);
    config.createKey("gui.prgFileDirectory", gui.prgFileDirectory);
    config.createKey("gui.debuggerDirectory", gui.debuggerDirectory);
    config.createKey("gui.screenshotDirectory", gui.screenshotDirectory);
  }
};

// ----------------------------------------------------------------------------

static void saveConfigurationFile(Plus4Emu::ConfigurationDB& config,
                                  const std::string& dirName,
                                  const char *fileName,
                                  Plus4EmuConfigInstallerGUI& gui)
{
  try {
    std::string fullName = dirName;
    fullName += fileName;
#ifdef WIN32
    try
#endif
    {
      config.saveState(fullName.c_str(), false);
    }
#ifdef WIN32
    catch (Plus4Emu::Exception& e) {
      if (std::strncmp(e.what(), "error opening ", 14) == 0) {
        // hack to work around errors due to lack of write access to
        // Program Files if makecfg is run as a normal user; if the
        // file already exists, then the error is ignored
        std::FILE *f = Plus4Emu::fileOpen(fullName.c_str(), "rb");
        if (!f)
          throw;
        else
          std::fclose(f);
      }
      else {
        throw;
      }
    }
#endif
  }
  catch (std::exception& e) {
    gui.errorMessage(e.what());
  }
}

int main(int argc, char **argv)
{
  Fl::lock();
#ifndef WIN32
  Plus4Emu::setGUIColorScheme(0);
#else
  Plus4Emu::setGUIColorScheme(1);
#endif
  bool    forceInstallFlag = false;
  std::string installDirectory = "";
  {
    int     i = 0;
    while (++i < argc) {
      if (argv[i][0] == '-') {
        if (argv[i][1] == '-' && argv[i][2] == '\0')
          break;
        if (argv[i][1] == 'f' && argv[i][2] == '\0') {
          forceInstallFlag = true;
          continue;
        }
      }
      installDirectory = argv[i];
    }
    if (i < (argc - 1))
      installDirectory = argv[argc - 1];
  }
#ifndef WIN32
  if (installDirectory.length() == 0 && forceInstallFlag)
    installDirectory = Plus4Emu::getPlus4EmuHomeDirectory();
#endif
  if (installDirectory.length() == 0) {
    std::string tmp = "";
#ifndef WIN32
    tmp = Plus4Emu::getPlus4EmuHomeDirectory();
#else
    {
      // try to get installation directory from registry
      wchar_t installDir[256];
      wchar_t valueName = wchar_t(0);
      HKEY    regKey = 0;
      DWORD   regType = 0;
      DWORD   bufSize = DWORD(sizeof(wchar_t) * 256);
      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                       "Software\\plus4emu\\InstallDirectory", 0,
                       KEY_QUERY_VALUE | KEY_WOW64_32KEY, &regKey)
          == ERROR_SUCCESS) {
        if (RegQueryValueExW(regKey, LPCWSTR(&valueName), (LPDWORD) 0, &regType,
                             (LPBYTE) installDir, &bufSize)
            == ERROR_SUCCESS && regType == REG_SZ && bufSize < 256) {
          installDir[bufSize] = wchar_t(0);
          Plus4Emu::convertToUTF8(tmp, &(installDir[0]));
        }
        RegCloseKey(regKey);
      }
    }
#endif
#if defined(__linux) || defined(__linux__)
    // use FLTK file chooser to work around bugs in the new 1.3.3 GTK chooser
    Fl_File_Chooser *w =
        new Fl_File_Chooser(
                tmp.c_str(), "*",
                Fl_File_Chooser::CREATE | Fl_File_Chooser::DIRECTORY,
                "Select installation directory for plus4emu data files");
    w->show();
    do {
      Fl::wait(0.05);
    } while (w->shown());
    if (w->value() && w->value()[0] != '\0')
      installDirectory = w->value();
    delete w;
#else
    Fl_Native_File_Chooser  *w = new Fl_Native_File_Chooser();
    w->type(Fl_Native_File_Chooser::BROWSE_SAVE_DIRECTORY);
    w->title("Select installation directory for plus4emu data files");
    w->filter("*");
    w->directory(tmp.c_str());
    if (w->show() == 0) {
      if (w->filename() != (char *) 0)
        installDirectory = w->filename();
    }
    delete w;
#endif
  }
  Plus4Emu::stripString(installDirectory);
  if (installDirectory.length() == 0)
    return -1;
#ifndef WIN32
  while (installDirectory[installDirectory.length() - 1] == '/' &&
         installDirectory.length() > 1) {
    installDirectory.resize(installDirectory.length() - 1);
  }
  {
    mkdir(installDirectory.c_str(), 0755);
    std::string tmp = installDirectory;
    if (tmp[tmp.length() - 1] != '/')
      tmp += '/';
    std::string tmp2 = tmp + "config";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "demo";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "disk";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "progs";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "roms";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "tape";
    mkdir(tmp2.c_str(), 0755);
    tmp2 = tmp + "config/shaders";
    mkdir(tmp2.c_str(), 0755);
  }
#else
  while ((installDirectory[installDirectory.length() - 1] == '/' ||
          installDirectory[installDirectory.length() - 1] == '\\') &&
         !(installDirectory.length() <= 1 ||
           (installDirectory.length() == 3 && installDirectory[1] == ':'))) {
    installDirectory.resize(installDirectory.length() - 1);
  }
  {
    Plus4Emu::mkdir_UTF8(installDirectory.c_str());
    std::string tmp = installDirectory;
    if (tmp[tmp.length() - 1] != '/' && tmp[tmp.length() - 1] != '\\')
      tmp += '\\';
    std::string tmp2 = tmp + "config";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "demo";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "disk";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "progs";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "roms";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "tape";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
    tmp2 = tmp + "config\\shaders";
    Plus4Emu::mkdir_UTF8(tmp2.c_str());
  }
#endif
#ifdef WIN32
  char    c = '\\';
#else
  char    c = '/';
#endif
  if (installDirectory[installDirectory.length() - 1] != c)
    installDirectory += c;
  std::string configDirectory = installDirectory + "config";
  configDirectory += c;
  std::string romDirectory = installDirectory + "roms";
  romDirectory += c;
  Plus4EmuConfigInstallerGUI  *gui = new Plus4EmuConfigInstallerGUI();
  Plus4Emu::setWindowIcon(gui->mainWindow, 11);
  Plus4Emu::setWindowIcon(gui->errorWindow, 12);
  if (!forceInstallFlag) {
    gui->mainWindow->show();
    do {
      Fl::wait(0.05);
    } while (gui->mainWindow->shown());
  }
  else
    gui->enableCfgInstall = true;
  try {
    Plus4Emu::ConfigurationDB     *config = (Plus4Emu::ConfigurationDB *) 0;
    Plus4EmuMachineConfiguration  *mCfg = (Plus4EmuMachineConfiguration *) 0;
    if (gui->enableCfgInstall) {
      Plus4EmuDisplaySndConfiguration   *dsCfg =
          (Plus4EmuDisplaySndConfiguration *) 0;
      config = new Plus4Emu::ConfigurationDB();
      {
        Plus4EmuGUIConfiguration  *gCfg =
            new Plus4EmuGUIConfiguration(*config, installDirectory);
        try {
          Plus4Emu::File  f;
          config->saveState(f);
          f.writeFile("gui_cfg.dat", true);
        }
        catch (std::exception& e) {
          gui->errorMessage(e.what());
        }
        delete gCfg;
      }
      delete config;
      config = new Plus4Emu::ConfigurationDB();
      mCfg = new Plus4EmuMachineConfiguration(*config, 6, romDirectory);
      dsCfg = new Plus4EmuDisplaySndConfiguration(*config);
      setKeyboardConfiguration(*config, (gui->keyboardMapHU ? 5 : 4));
      try {
        Plus4Emu::File  f;
        config->saveState(f);
        f.writeFile("plus4cfg.dat", true);
      }
      catch (std::exception& e) {
        gui->errorMessage(e.what());
      }
      delete config;
      delete mCfg;
      delete dsCfg;
      config = (Plus4Emu::ConfigurationDB *) 0;
      mCfg = (Plus4EmuMachineConfiguration *) 0;
    }
    for (int i = 0; i < 16; i++) {
      config = new Plus4Emu::ConfigurationDB();
      mCfg = new Plus4EmuMachineConfiguration(*config, i, romDirectory);
      saveConfigurationFile(*config,
                            configDirectory, machineConfigFileNames[i], *gui);
      delete config;
      delete mCfg;
      config = (Plus4Emu::ConfigurationDB *) 0;
      mCfg = (Plus4EmuMachineConfiguration *) 0;
    }
    for (int i = 0; i < 8; i++) {
      if (keyboardConfigFileNames[i] != (char *) 0) {
        config = new Plus4Emu::ConfigurationDB();
        setKeyboardConfiguration(*config, i);
        saveConfigurationFile(*config,
                              configDirectory, keyboardConfigFileNames[i],
                              *gui);
        delete config;
        config = (Plus4Emu::ConfigurationDB *) 0;
      }
    }
    for (int i = 0; shaderSourceFiles[i]; i = i + 2) {
      try {
        std::string fullName(configDirectory);
        fullName += "shaders";
        fullName += c;
        fullName += shaderSourceFiles[i];
        std::FILE *f = Plus4Emu::fileOpen(fullName.c_str(), "w");
        if (!f) {
#ifdef WIN32
          // hack to work around errors due to lack of write access to
          // Program Files if makecfg is run as a normal user; if the
          // file already exists, then the error is ignored
          f = Plus4Emu::fileOpen(fullName.c_str(), "rb");
          if (f) {
            std::fclose(f);
            continue;
          }
#endif
          throw Plus4Emu::Exception("error opening shader source file");
        }
        size_t  n = std::strlen(shaderSourceFiles[i + 1]);
        if (std::fwrite(shaderSourceFiles[i + 1], sizeof(char), n, f) != n ||
            std::fflush(f) != 0) {
          std::fclose(f);
          Plus4Emu::fileRemove(fullName.c_str());
          throw Plus4Emu::Exception("error writing shader source file");
        }
        std::fclose(f);
      }
      catch (std::exception& e) {
        gui->errorMessage(e.what());
      }
    }
  }
  catch (std::exception& e) {
    gui->errorMessage(e.what());
    delete gui;
    return -1;
  }
  delete gui;
  return 0;
}

