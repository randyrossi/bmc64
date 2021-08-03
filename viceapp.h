//
// viceapp.h
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _viceapp_h
#define _viceapp_h

#include "viceoptions.h"
#include <SDCard/emmc.h>
#include <circle/actled.h>
#include <circle/devicenameservice.h>
#include <circle/exceptionhandler.h>
#include <circle/gpiomanager.h>
#include <circle/gpiopin.h>
#include <circle/input/console.h>
#include <circle/interrupt.h>
#include <circle/koptions.h>
#include <circle/logger.h>
#include <circle/sched/scheduler.h>
#include <circle/machineinfo.h>
#include <circle/memory.h>
#include <circle/net/netsubsystem.h>
#include <circle/nulldevice.h>
#include <circle/serial.h>
#include <circle/timer.h>
#include <circle/usb/usbhcidevice.h>
#include <ff.h>

#include <circle_glue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(RASPI_PLUS4EMU)
#include "plus4emulatorcore.h"
#else
#include "viceemulatorcore.h"
#endif

// GPIO  JSFUNC   KEYFUNC  KEYCON   gpioPins Index
//
// 04             RESTORE  KDB3     16
//
// 26    J2_FIRE  PA7      KBD20    7
// 20    J2_UP    PA1      KBD19    1
// 19    J2_DOWN  PA2      KBD18    2
// 16    J2_LEFT  PA3      KBD17    3
// 13    J2_RIGHT PA4      KBD16    4
// 06             PA5      KBD15    5
// 12             PA6      KBD14    6
// 05             PA0      KDB13    0
// 21    J2_SEL                     18
//
// 08             PB0      KBD12    8
// 25             PB1      KBD11    9
// 24             PB2      KBD10   10
// 22    J1_FIRE  PB7      KBD9    15
// 23    J1_UP    PB4      KBD8    12
// 27    J1_DOWN  PB5      KBD7    13
// 17    J1_LEFT  PB6      KBD6    14
// 18    J1_RIGHT PB3      KBD5    11
// 07    J1_SEL                    17
//
// 14 TXD0
// 15 RXD0
//
// 02 I2C (Currently unused by BMC64)
// 03 I2C (Currently unused by BMC64)
//
// 10 SPI (Currently unused by BMC64)
// 09 SPI (Currently unused by BMC64)
// 11 SPI (Currently unused by BMC64)

// Joystick select pins for config 1.
#define GPIO_JS1_SELECT  7
#define GPIO_JS2_SELECT  21

// Restore key pin.
#define GPIO_KBD_RESTORE 4

// Keyboard pins PA0-7 (Pins 20-13) are indices 0-7 (but PA lines 0,7 swapped)
// Keyboard pins PB0-7 (Pins 12-5 ) are indices 8-15 (but PB lines 3,7 swapped)

// These are indices within the master gpio array for some
// special pins we need to address.
#define GPIO_CONFIG_1_JOY_1_UP_INDEX     12  // GPIO 23
#define GPIO_CONFIG_1_JOY_1_DOWN_INDEX   13  // GPIO 27
#define GPIO_CONFIG_1_JOY_1_LEFT_INDEX   14  // GPIO 17
#define GPIO_CONFIG_1_JOY_1_RIGHT_INDEX  11  // GPIO 18
#define GPIO_CONFIG_1_JOY_1_FIRE_INDEX   15  // GPIO 22

#define GPIO_CONFIG_1_JOY_2_UP_INDEX     1   // GPIO 20
#define GPIO_CONFIG_1_JOY_2_DOWN_INDEX   2   // GPIO 19
#define GPIO_CONFIG_1_JOY_2_LEFT_INDEX   3   // GPIO 16
#define GPIO_CONFIG_1_JOY_2_RIGHT_INDEX  4   // GPIO 13
#define GPIO_CONFIG_1_JOY_2_FIRE_INDEX   7   // GPIO 26

#define GPIO_CONFIG_0_JOY_1_UP_INDEX     14  // GPIO 17
#define GPIO_CONFIG_0_JOY_1_DOWN_INDEX   11  // GPIO 18
#define GPIO_CONFIG_0_JOY_1_LEFT_INDEX   13  // GPIO 27
#define GPIO_CONFIG_0_JOY_1_RIGHT_INDEX  15  // GPIO 22
#define GPIO_CONFIG_0_JOY_1_FIRE_INDEX   12  // GPIO 23

#define GPIO_CONFIG_0_JOY_2_UP_INDEX      0  // GPIO 5
#define GPIO_CONFIG_0_JOY_2_DOWN_INDEX    5  // GPIO 6
#define GPIO_CONFIG_0_JOY_2_LEFT_INDEX    6  // GPIO 12
#define GPIO_CONFIG_0_JOY_2_RIGHT_INDEX   4  // GPIO 13
#define GPIO_CONFIG_0_JOY_2_FIRE_INDEX    2  // GPIO 19

#define GPIO_KBD_RESTORE_INDEX            16 // GPIO 4
#define GPIO_JS1_SELECT_INDEX             17 // GPIO 7
#define GPIO_JS2_SELECT_INDEX             18 // GPIO 21

#define NO_FIXED_PURPOSE_1_INDEX          19 // GPIO 2
#define NO_FIXED_PURPOSE_2_INDEX          20 // GPIO 3
#define NO_FIXED_PURPOSE_3_INDEX          21 // GPIO 9
#define NO_FIXED_PURPOSE_4_INDEX          22 // GPIO 10

// Used as indices into the joystickPins arrays
#define JOY_UP    0
#define JOY_DOWN  1
#define JOY_LEFT  2
#define JOY_RIGHT 3
#define JOY_FIRE  4
#define JOY_POTX  5
#define JOY_POTY  6

// For debouncing logic
#define BTN_PRESS   1
#define BTN_RELEASE 2
#define BTN_UP      3
#define BTN_DOWN    4

// Nav buttons only for config 0
#define GPIO_CONFIG_0_MENU_INDEX       3   // GPIO 16
#define GPIO_CONFIG_0_MENU_BACK_INDEX  16  // GPIO 4
#define GPIO_CONFIG_0_MENU_UP_INDEX    9   // GPIO 25
#define GPIO_CONFIG_0_MENU_DOWN_INDEX  8   // GPIO 8
#define GPIO_CONFIG_0_MENU_LEFT_INDEX  1   // GPIO 20
#define GPIO_CONFIG_0_MENU_RIGHT_INDEX 18  // GPIO 21
#define GPIO_CONFIG_0_MENU_ENTER_INDEX 10  // GPIO 24
#define GPIO_CONFIG_0_MENU_VKBD_INDEX  7   // GPIO 26

// Buttons for Waveshare HAT
// up, down, left, right, start, select, a,  b,  tr, y,  x,  tl
// 5,  6,    13,  19,    21,    4,      26, 12, 23, 20, 16, 18
#define GPIO_CONFIG_2_WAVESHARE_START_INDEX  18 // GPIO 21
#define GPIO_CONFIG_2_WAVESHARE_SELECT_INDEX 16 // GPIO 4
#define GPIO_CONFIG_2_WAVESHARE_UP_INDEX     0  // GPIO 5
#define GPIO_CONFIG_2_WAVESHARE_DOWN_INDEX   5  // GPIO 6
#define GPIO_CONFIG_2_WAVESHARE_LEFT_INDEX   4  // GPIO 13
#define GPIO_CONFIG_2_WAVESHARE_RIGHT_INDEX  2  // GPIO 19
#define GPIO_CONFIG_2_WAVESHARE_A_INDEX      7  // GPIO 26
#define GPIO_CONFIG_2_WAVESHARE_B_INDEX      6  // GPIO 12
#define GPIO_CONFIG_2_WAVESHARE_TR_INDEX     12 // GPIO 23
#define GPIO_CONFIG_2_WAVESHARE_Y_INDEX      1  // GPIO 20
#define GPIO_CONFIG_2_WAVESHARE_X_INDEX      3  // GPIO 16
#define GPIO_CONFIG_2_WAVESHARE_TL_INDEX     11 // GPIO 18

// Config for userport CIA2 port B access
#define GPIO_CONFIG_3_JOY_1_UP_INDEX     14  // GPIO 17
#define GPIO_CONFIG_3_JOY_1_DOWN_INDEX   11  // GPIO 18
#define GPIO_CONFIG_3_JOY_1_LEFT_INDEX   13  // GPIO 27
#define GPIO_CONFIG_3_JOY_1_RIGHT_INDEX  15  // GPIO 22
#define GPIO_CONFIG_3_JOY_1_FIRE_INDEX   12  // GPIO 23

#define GPIO_CONFIG_3_JOY_2_UP_INDEX      0  // GPIO 5
#define GPIO_CONFIG_3_JOY_2_DOWN_INDEX    5  // GPIO 6
#define GPIO_CONFIG_3_JOY_2_LEFT_INDEX    6  // GPIO 12
#define GPIO_CONFIG_3_JOY_2_RIGHT_INDEX   4  // GPIO 13
#define GPIO_CONFIG_3_JOY_2_FIRE_INDEX    2  // GPIO 19

#define GPIO_CONFIG_3_USERPORT_PB0_INDEX 16 // GPIO 4
#define GPIO_CONFIG_3_USERPORT_PB1_INDEX 10 // GPIO 24
#define GPIO_CONFIG_3_USERPORT_PB2_INDEX 9  // GPIO 25
#define GPIO_CONFIG_3_USERPORT_PB3_INDEX 8  // GPIO 8
#define GPIO_CONFIG_3_USERPORT_PB4_INDEX 3  // GPIO 16
#define GPIO_CONFIG_3_USERPORT_PB5_INDEX 7  // GPIO 26
#define GPIO_CONFIG_3_USERPORT_PB6_INDEX 1  // GPIO 20
#define GPIO_CONFIG_3_USERPORT_PB7_INDEX 18 // GPIO 21

// Used as indices into the userportPins array
#define USERPORT_PB0 0
#define USERPORT_PB1 1
#define USERPORT_PB2 2
#define USERPORT_PB3 3
#define USERPORT_PB4 4
#define USERPORT_PB5 5
#define USERPORT_PB6 6
#define USERPORT_PB7 7

extern "C" {
void circle_fs_ready();
}

class ViceApp {
public:
  enum TShutdownMode { ShutdownNone, ShutdownHalt, ShutdownReboot };

  ViceApp(const char *kernel) : FromKernel(kernel) {}

  virtual ~ViceApp(void) {}

  virtual bool Initialize(void);
  virtual void Cleanup(void) {}
  virtual TShutdownMode Run(void) = 0;
  const char *GetKernelName(void) const { return FromKernel; }

  int circle_get_machine_timing();
  int circle_cycles_per_second();

private:
  char const *FromKernel;

protected:
  CActLED mActLED;
  ViceOptions mViceOptions;
  CKernelOptions mOptions;
  CSerialDevice mSerial;
  CMemorySystem mMemory;
  CDeviceNameService mDeviceNameService;
  CNullDevice mNullDevice;
  CExceptionHandler mExceptionHandler;
  CInterruptSystem mInterrupt;
};

class ViceScreenApp : public ViceApp {
public:
  ViceScreenApp(const char *kernel)
      : ViceApp(kernel),
        mTimer(&mInterrupt),
        mLogger(mOptions.GetLogLevel(), &mTimer),
        mGPIOManager(&mInterrupt), mVCHIQ(&mMemory, &mInterrupt) {
#if defined(RASPI_PLUS4EMU)
     mEmulatorCore = new Plus4EmulatorCore(&mMemory, circle_cycles_per_second());
#else
     mEmulatorCore = new ViceEmulatorCore(&mMemory, circle_cycles_per_second());
#endif
  }

  virtual bool Initialize(void);

protected:

  EmulatorCore *mEmulatorCore;
  CTimer mTimer;
  CLogger mLogger;
  CScheduler mScheduler;
  CGPIOManager mGPIOManager;
  CVCHIQDevice mVCHIQ;
  CMachineInfo mMachineInfo;

  CGPIOPin *config_0_joystickPins1[5];
  CGPIOPin *config_0_joystickPins2[5];

  CGPIOPin *config_1_joystickPins1[5];
  CGPIOPin *config_1_joystickPins2[5];

  CGPIOPin *config_2_joystickPins[7]; // potx and poty included

  CGPIOPin *config_3_joystickPins1[5];
  CGPIOPin *config_3_joystickPins2[5];
  CGPIOPin *config_3_userportPins[8];

  CGPIOPin *gpioPins[NUM_GPIO_PINS];
  CGPIOPin *DPIPins[28];

private:
  void SetupGPIOForInput();
  void SetupGPIOForDPI();
  void SetupGPIO();
};

class ViceStdioApp : public ViceScreenApp {
public:
  ViceStdioApp(const char *kernel)
      : ViceScreenApp(kernel), mUSBHCII(&mInterrupt, &mTimer),
        mEMMC(&mInterrupt, &mTimer, &mActLED)
        {}

  virtual bool Initialize(void);
  virtual void Cleanup(void);

  void circle_find_usb(int (*usb)[3]);
  int circle_mount_usb(int usb);
  int circle_unmount_usb(int usb);

private:
  // Must be called after fatfs/stdio has been initialized
  // This routine loads the bootstat.txt file and passes the
  // information along to stdio so it can bypass the disk
  // to answer questions about a set of known files. This speeds
  // up boot time.
  void InitBootStat();

protected:
  // Called after VICE has completed booting so we no longer
  // fast fail or fast stat anything.
  void DisableBootStat();

  CUSBHCIDevice mUSBHCII;
  CEMMCDevice mEMMC;
  FATFS mFileSystemSD;
  FATFS mFileSystemUSB1;
  FATFS mFileSystemUSB2;
  FATFS mFileSystemUSB3;

  int mBootStatWhat[MAX_BOOTSTAT_LINES];
  char *mBootStatFile[MAX_BOOTSTAT_LINES];
  int mBootStatSize[MAX_BOOTSTAT_LINES];
  char mTimingOption[8];
};

#endif
