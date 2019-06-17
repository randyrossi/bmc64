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

  virtual bool Initialize(void) {
    if (!mSerial.Initialize(115200)) {
      return false;
    }

    // Initialize our replacement newlib stdio. Give it
    // a pointer to our serial device so we can use printf
    // to serial as soon as possible.
    CGlueStdioInit(&mSerial);

    if (!mInterrupt.Initialize()) {
      return false;
    }

    return true;
  }

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
        mGPIOManager(&mInterrupt) {}

  virtual bool Initialize(void) {
    if (!ViceApp::Initialize()) {
      return false;
    }

    if (!mScreen.Initialize()) {
      return false;
    }

    if (!mLogger.Initialize(&mScreen)) {
      return false;
    }

    if (!mEmulatorCore.Initialize()) {
      return false;
    }

    if (!mTimer.Initialize()) {
      return false;
    }

    if (!mGPIOManager.Initialize()) {
      return false;
    }

    SetupGPIO();

    return true;
  }

protected:
  void SetupGPIO() {
    // PA - Set to output-low for when scanning each
    // row. Otherwise set to input-pullup.
    // Note: Lines 0 and 7 are swapped. The order here is
    // from keyboard connector pins 20 down to 13.

    // Connector Pin 20 - PA7
    gpioPins[7] =
        new CGPIOPin(26, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 19 - PA1
    gpioPins[1] =
        new CGPIOPin(20, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 18 - PA2
    gpioPins[2] =
        new CGPIOPin(19, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 17 - PA3
    gpioPins[3] =
        new CGPIOPin(16, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 16 - PA4
    gpioPins[4] =
        new CGPIOPin(13, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 15 - PA5
    gpioPins[5] =
        new CGPIOPin(6, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 14 - PA6
    gpioPins[6] =
        new CGPIOPin(12, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 13 - PA0
    gpioPins[0] =
        new CGPIOPin(5, GPIOModeInputPullUp, &mGPIOManager);

    // PB - Always input-pullup for read during kbd scan or joy port 1
    // Note: Lines 3 and 7 are swapped. The order here is from
    // keyboard connector pins 12 down to 5

    // Connector Pin 12 - PB 0
    gpioPins[8] =
        new CGPIOPin(8, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 11 - PB 1
    gpioPins[9] =
        new CGPIOPin(25, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 10 - PB 2
    gpioPins[10] =
        new CGPIOPin(24, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 9 - PB 7
    gpioPins[15] =
        new CGPIOPin(22, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 8 - PB 4
    gpioPins[12] =
        new CGPIOPin(23, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 7 - PB 5
    gpioPins[13] =
        new CGPIOPin(27, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 6 - PB 6
    gpioPins[14] =
        new CGPIOPin(17, GPIOModeInputPullUp, &mGPIOManager);
    // Connector Pin 5 - PB 3
    gpioPins[11] =
        new CGPIOPin(18, GPIOModeInputPullUp, &mGPIOManager);

    // A few more special pins
    gpioPins[GPIO_KBD_RESTORE_INDEX] =
        new CGPIOPin(GPIO_KBD_RESTORE, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_JS1_SELECT_INDEX] =
        new CGPIOPin(GPIO_JS1_SELECT, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_JS2_SELECT_INDEX] =
        new CGPIOPin(GPIO_JS2_SELECT, GPIOModeInputPullUp, &mGPIOManager);

    // Convenience arrays for joysticks
    joystickPins1[JOY_UP] = gpioPins[GPIO_JOY_1_UP_INDEX];
    joystickPins1[JOY_DOWN] = gpioPins[GPIO_JOY_1_DOWN_INDEX];
    joystickPins1[JOY_LEFT] = gpioPins[GPIO_JOY_1_LEFT_INDEX];
    joystickPins1[JOY_RIGHT] = gpioPins[GPIO_JOY_1_RIGHT_INDEX];
    joystickPins1[JOY_FIRE] = gpioPins[GPIO_JOY_1_FIRE_INDEX];

    joystickPins2[JOY_UP] = gpioPins[GPIO_JOY_2_UP_INDEX];
    joystickPins2[JOY_DOWN] = gpioPins[GPIO_JOY_2_DOWN_INDEX];
    joystickPins2[JOY_LEFT] = gpioPins[GPIO_JOY_2_LEFT_INDEX];
    joystickPins2[JOY_RIGHT] = gpioPins[GPIO_JOY_2_RIGHT_INDEX];
    joystickPins2[JOY_FIRE] = gpioPins[GPIO_JOY_2_FIRE_INDEX];

    noPCBJoystickPins1[JOY_UP] = gpioPins[GPIO_NOPCB_JOY_1_UP_INDEX];
    noPCBJoystickPins1[JOY_DOWN] = gpioPins[GPIO_NOPCB_JOY_1_DOWN_INDEX];
    noPCBJoystickPins1[JOY_LEFT] = gpioPins[GPIO_NOPCB_JOY_1_LEFT_INDEX];
    noPCBJoystickPins1[JOY_RIGHT] = gpioPins[GPIO_NOPCB_JOY_1_RIGHT_INDEX];
    noPCBJoystickPins1[JOY_FIRE] = gpioPins[GPIO_NOPCB_JOY_1_FIRE_INDEX];

    noPCBJoystickPins2[JOY_UP] = gpioPins[GPIO_NOPCB_JOY_2_UP_INDEX];
    noPCBJoystickPins2[JOY_DOWN] = gpioPins[GPIO_NOPCB_JOY_2_DOWN_INDEX];
    noPCBJoystickPins2[JOY_LEFT] = gpioPins[GPIO_NOPCB_JOY_2_LEFT_INDEX];
    noPCBJoystickPins2[JOY_RIGHT] = gpioPins[GPIO_NOPCB_JOY_2_RIGHT_INDEX];
    noPCBJoystickPins2[JOY_FIRE] = gpioPins[GPIO_NOPCB_JOY_2_FIRE_INDEX];
  }

  ViceEmulatorCore mEmulatorCore;
  CViceScreenDevice mScreen;
  CTimer mTimer;
  CLogger mLogger;
  CGPIOManager mGPIOManager;

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

  virtual bool Initialize(void) {
    if (!ViceScreenApp::Initialize()) {
      return false;
    }

    if (!mEMMC.Initialize()) {
      return false;
    }

    int partition = mViceOptions.GetDiskPartition();
    int ss = 0;
    if (partition > 4) {
      // User is forcing a start sector by specifying
      // a partition above 4. Tell glue code partition
      // is 5 and this will set the start sector to what
      // they provided when the disk is mounted.
      ss = partition;
      partition = 5;
    }

    // When mounting, fatfs gets ":" appended.  But StdioInit
    // does not.
    const char *volumeName = mViceOptions.GetDiskVolume();
    char fatFsVol[VOLUME_NAME_LEN];
    strncpy(fatFsVol, volumeName, VOLUME_NAME_LEN - 2);
    strcat(fatFsVol, ":");

    CGlueStdioSetPartitionForVolume(volumeName, partition, ss);

    if (f_mount(&mFileSystem, fatFsVol, 1) != FR_OK) {
      mLogger.Write(GetKernelName(), LogError, "Cannot mount partition: %s",
                    fatFsVol);
      return false;
    }

    InitBootStat();

    // Now that emmc is initialized, launch
    // the emulator main loop on CORE 1 before DWHCI.
    char timing_option[8];
    int timing_int = mViceOptions.GetMachineTiming();
    if (timing_int == MACHINE_TIMING_NTSC_HDMI ||
        timing_int == MACHINE_TIMING_NTSC_COMPOSITE) {
      strcpy(timing_option, "-ntsc");
    } else {
      strcpy(timing_option, "-pal");
    }

    mEmulatorCore.LaunchEmulator(timing_option);

    // This takes 1.5 seconds to init.
    if (!mDWHCI.Initialize()) {
      return false;
    }

    if (!mConsole.Initialize()) {
      return false;
    }

    return true;
  }

  virtual void Cleanup(void) {
    // When mounting, fatfs gets ":" appended.  But StdioInit
    // does not.
    const char *volumeName = mViceOptions.GetDiskVolume();
    char fatFsVol[VOLUME_NAME_LEN];
    strncpy(fatFsVol, volumeName, VOLUME_NAME_LEN - 2);
    strcat(fatFsVol, ":");

    if (f_mount(0, fatFsVol, 0) != FR_OK) {
      mLogger.Write(GetKernelName(), LogError, "Cannot unmount drive");
    }
    ViceScreenApp::Cleanup();
  }

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
