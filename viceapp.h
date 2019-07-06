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
#include "vicescreen.h"
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
#include <circle/memory.h>
#include <circle/net/netsubsystem.h>
#include <circle/nulldevice.h>
#include <circle/serial.h>
#include <circle/timer.h>
#include <circle/usb/dwhcidevice.h>
#include <ff.h>

#include <circle_glue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "viceemulatorcore.h"

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

#define NUM_GPIO_PINS 19

// Joystick select pins.
#define GPIO_JS1_SELECT  7
#define GPIO_JS2_SELECT  21

// Restore key pin.
#define GPIO_KBD_RESTORE 4

// Keyboard pins PA0-7 (Pins 20-13) are indices 0-7 (but PA lines 0,7 swapped)
// Keyboard pins PB0-7 (Pins 12-5 ) are indices 8-15 (but PB lines 3,7 swapped)

// These are indices within the master gpio array for some
// special pins we need to address. 
#define GPIO_JOY_1_UP_INDEX     12
#define GPIO_JOY_1_DOWN_INDEX   13
#define GPIO_JOY_1_LEFT_INDEX   14
#define GPIO_JOY_1_RIGHT_INDEX  11
#define GPIO_JOY_1_FIRE_INDEX   15

#define GPIO_JOY_2_UP_INDEX     1
#define GPIO_JOY_2_DOWN_INDEX   2
#define GPIO_JOY_2_LEFT_INDEX   3
#define GPIO_JOY_2_RIGHT_INDEX  4
#define GPIO_JOY_2_FIRE_INDEX   7

#define GPIO_NOPCB_JOY_1_UP_INDEX     14  // GPIO 17
#define GPIO_NOPCB_JOY_1_DOWN_INDEX   11  // GPIO 18
#define GPIO_NOPCB_JOY_1_LEFT_INDEX   13  // GPIO 27
#define GPIO_NOPCB_JOY_1_RIGHT_INDEX  15  // GPIO 22
#define GPIO_NOPCB_JOY_1_FIRE_INDEX   12  // GPIO 23

#define GPIO_NOPCB_JOY_2_UP_INDEX      0  // GPIO 5
#define GPIO_NOPCB_JOY_2_DOWN_INDEX    5  // GPIO 6
#define GPIO_NOPCB_JOY_2_LEFT_INDEX    6  // GPIO 12
#define GPIO_NOPCB_JOY_2_RIGHT_INDEX   4  // GPIO 13
#define GPIO_NOPCB_JOY_2_FIRE_INDEX    2  // GPIO 19

#define GPIO_KBD_RESTORE_INDEX 16
#define GPIO_JS1_SELECT_INDEX  17
#define GPIO_JS2_SELECT_INDEX  18

// Used as indices into the joystickPins arrays
#define JOY_UP    0
#define JOY_DOWN  1
#define JOY_LEFT  2
#define JOY_RIGHT 3
#define JOY_FIRE  4

// For debouncing logic
#define BTN_PRESS   1
#define BTN_RELEASE 2
#define BTN_UP      3
#define BTN_DOWN    4

// Deprecated GPIO menu pin. Can't be used with PCB.
#define GPIO_MENU_INDEX  3     // GPIO 16

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
      : ViceApp(kernel), mEmulatorCore(&mMemory),
        mScreen(mViceOptions.GetCanvasWidth(), mViceOptions.GetCanvasHeight()),
        mTimer(&mInterrupt), mLogger(mOptions.GetLogLevel(), &mTimer),
        mGPIOManager(&mInterrupt), mVCHIQ(&mMemory, &mInterrupt) {}

  virtual bool Initialize(void);

protected:
  void SetupGPIO();

  ViceEmulatorCore mEmulatorCore;
  CViceScreenDevice mScreen;
  CTimer mTimer;
  CLogger mLogger;
  CScheduler mScheduler;
  CGPIOManager mGPIOManager;
  CVCHIQDevice mVCHIQ;

  CGPIOPin *joystickPins1[5];
  CGPIOPin *joystickPins2[5];
  CGPIOPin *noPCBJoystickPins1[5];
  CGPIOPin *noPCBJoystickPins2[5];
  CGPIOPin *gpioPins[NUM_GPIO_PINS];
};

class ViceStdioApp : public ViceScreenApp {
public:
  ViceStdioApp(const char *kernel)
      : ViceScreenApp(kernel), mDWHCI(&mInterrupt, &mTimer),
        mEMMC(&mInterrupt, &mTimer, &mActLED), mConsole(&mScreen) {}

  virtual bool Initialize(void);
  virtual void Cleanup(void);

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

  CDWHCIDevice mDWHCI;
  CEMMCDevice mEMMC;
  FATFS mFileSystem;
  CConsole mConsole;

  int mBootStatWhat[MAX_BOOTSTAT_LINES];
  char *mBootStatFile[MAX_BOOTSTAT_LINES];
  int mBootStatSize[MAX_BOOTSTAT_LINES];
};

#endif
