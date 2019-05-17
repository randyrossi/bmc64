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

#define NUM_GPIO_PINS 19

// Keyboard matrix port B pins - for PCB
#define GPIO_KBD_PB_0 17
#define GPIO_KBD_PB_1 27
#define GPIO_KBD_PB_2 22
#define GPIO_KBD_PB_3 5
#define GPIO_KBD_PB_4 6
#define GPIO_KBD_PB_5 13
#define GPIO_KBD_PB_6 19
#define GPIO_KBD_PB_7 26

// Keyboard matrix port A pins - for PCB
#define GPIO_KBD_PA_0 21
#define GPIO_KBD_PA_1 20
#define GPIO_KBD_PA_2 16
#define GPIO_KBD_PA_3 12
#define GPIO_KBD_PA_4 7
#define GPIO_KBD_PA_5 8
#define GPIO_KBD_PA_6 25
#define GPIO_KBD_PA_7 24

// Joystick select pins for PCB method
#define GPIO_JS1_SELECT 23
#define GPIO_JS2_SELECT 18

// Restore key pin
#define GPIO_KBD_RESTORE 4

// These are the indices of each pin in our gpio pin array
#define GPIO_KBD_PA_0_INDEX 7
#define GPIO_KBD_PA_1_INDEX 1
#define GPIO_KBD_PA_2_INDEX 2
#define GPIO_KBD_PA_3_INDEX 3
#define GPIO_KBD_PA_4_INDEX 4
#define GPIO_KBD_PA_5_INDEX 5
#define GPIO_KBD_PA_6_INDEX 6
#define GPIO_KBD_PA_7_INDEX 0
#define GPIO_KBD_PB_0_INDEX 8
#define GPIO_KBD_PB_1_INDEX 9
#define GPIO_KBD_PB_2_INDEX 10
#define GPIO_KBD_PB_3_INDEX 15
#define GPIO_KBD_PB_4_INDEX 12
#define GPIO_KBD_PB_5_INDEX 13
#define GPIO_KBD_PB_6_INDEX 14
#define GPIO_KBD_PB_7_INDEX 11
#define GPIO_KBD_RESTORE_INDEX 16
#define GPIO_JS1_SELECT_INDEX 17
#define GPIO_JS2_SELECT_INDEX 18

// Make sure these always ref the right PIN even if the indices
// above are changed.
#define GPIO_JOY_UP_INDEX GPIO_KBD_PB_0_INDEX // must match GPIO 17
#define GPIO_JOY_DOWN_INDEX GPIO_KBD_PB_1_INDEX // must match GPIO 27
#define GPIO_JOY_LEFT_INDEX GPIO_KBD_PB_2_INDEX // must match GPIO 22
#define GPIO_JOY_RIGHT_INDEX GPIO_KBD_PB_3_INDEX // must match GPIO 5 
#define GPIO_JOY_FIRE_INDEX GPIO_KBD_PB_4_INDEX // must match GPIO 6 

// These are for the non-PBC way of doing joysticks
#define GPIO_OLD_JOY_1_UP_INDEX GPIO_KBD_PB_0_INDEX      // Pin 1 - 17
#define GPIO_OLD_JOY_1_DOWN_INDEX GPIO_JS2_SELECT_INDEX  // Pin 2 - 18
#define GPIO_OLD_JOY_1_LEFT_INDEX GPIO_KBD_PB_1_INDEX    // Pin 3 - 27
#define GPIO_OLD_JOY_1_RIGHT_INDEX GPIO_KBD_PB_2_INDEX   // Pin 4 - 22
#define GPIO_OLD_JOY_1_FIRE_INDEX GPIO_JS1_SELECT_INDEX  // Pin 6 - 23

#define GPIO_OLD_JOY_2_UP_INDEX GPIO_KBD_PB_3_INDEX    // Pin 1 - 5
#define GPIO_OLD_JOY_2_DOWN_INDEX GPIO_KBD_PB_4_INDEX  // Pin 2 - 6
#define GPIO_OLD_JOY_2_LEFT_INDEX GPIO_KBD_PA_3_INDEX  // Pin 3 - 12
#define GPIO_OLD_JOY_2_RIGHT_INDEX GPIO_KBD_PB_5_INDEX // Pin 4 - 13
#define GPIO_OLD_JOY_2_FIRE_INDEX GPIO_KBD_PB_6_INDEX  // Pin 6 - 19

// Used as indices into the joystickPins arrays
#define JOY_UP 0
#define JOY_DOWN 1
#define JOY_LEFT 2
#define JOY_RIGHT 3
#define JOY_FIRE 4

// For debouncing logic
#define BTN_PRESS 1
#define BTN_RELEASE 2
#define BTN_UP 3
#define BTN_DOWN 4

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
    gpioPins[GPIO_KBD_PA_0_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_0, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_1_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_1, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_2_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_2, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_3_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_3, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_4_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_4, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_5_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_5, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_6_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_6, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PA_7_INDEX] =
        new CGPIOPin(GPIO_KBD_PA_7, GPIOModeInputPullUp, &mGPIOManager);

    gpioPins[GPIO_KBD_PB_0_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_0, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_1_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_1, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_2_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_2, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_3_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_3, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_4_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_4, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_5_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_5, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_6_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_6, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_KBD_PB_7_INDEX] =
        new CGPIOPin(GPIO_KBD_PB_7, GPIOModeInputPullUp, &mGPIOManager);

    gpioPins[GPIO_KBD_RESTORE_INDEX] =
        new CGPIOPin(GPIO_KBD_RESTORE, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_JS1_SELECT_INDEX] =
        new CGPIOPin(GPIO_JS1_SELECT, GPIOModeInputPullUp, &mGPIOManager);
    gpioPins[GPIO_JS2_SELECT_INDEX] =
        new CGPIOPin(GPIO_JS2_SELECT, GPIOModeInputPullUp, &mGPIOManager);

    joystickPins[JOY_UP] = gpioPins[GPIO_JOY_UP_INDEX];
    joystickPins[JOY_DOWN] = gpioPins[GPIO_JOY_DOWN_INDEX];
    joystickPins[JOY_LEFT] = gpioPins[GPIO_JOY_LEFT_INDEX];
    joystickPins[JOY_RIGHT] = gpioPins[GPIO_JOY_RIGHT_INDEX];
    joystickPins[JOY_FIRE] = gpioPins[GPIO_JOY_FIRE_INDEX];

    oldJoystickPins1[JOY_UP] = gpioPins[GPIO_OLD_JOY_1_UP_INDEX];
    oldJoystickPins1[JOY_DOWN] = gpioPins[GPIO_OLD_JOY_1_DOWN_INDEX];
    oldJoystickPins1[JOY_LEFT] = gpioPins[GPIO_OLD_JOY_1_LEFT_INDEX];
    oldJoystickPins1[JOY_RIGHT] = gpioPins[GPIO_OLD_JOY_1_RIGHT_INDEX];
    oldJoystickPins1[JOY_FIRE] = gpioPins[GPIO_OLD_JOY_1_FIRE_INDEX];

    oldJoystickPins2[JOY_UP] = gpioPins[GPIO_OLD_JOY_2_UP_INDEX];
    oldJoystickPins2[JOY_DOWN] = gpioPins[GPIO_OLD_JOY_2_DOWN_INDEX];
    oldJoystickPins2[JOY_LEFT] = gpioPins[GPIO_OLD_JOY_2_LEFT_INDEX];
    oldJoystickPins2[JOY_RIGHT] = gpioPins[GPIO_OLD_JOY_2_RIGHT_INDEX];
    oldJoystickPins2[JOY_FIRE] = gpioPins[GPIO_OLD_JOY_2_FIRE_INDEX];
  }

  ViceEmulatorCore mEmulatorCore;
  CViceScreenDevice mScreen;
  CTimer mTimer;
  CLogger mLogger;
  CGPIOManager mGPIOManager;

  CGPIOPin *oldJoystickPins1[5];
  CGPIOPin *oldJoystickPins2[5];
  CGPIOPin *joystickPins[5];
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
