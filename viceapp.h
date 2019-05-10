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

#include <circle/actled.h>
#include <circle/devicenameservice.h>
#include <circle/memory.h>
#include <circle/nulldevice.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/koptions.h>
#include <circle/gpiopin.h>
#include <circle/gpiomanager.h>
#include "viceoptions.h"
#include "vicescreen.h"
#include <circle/serial.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/dwhcidevice.h>
#include <SDCard/emmc.h>
#include <ff.h>
#include <circle/input/console.h>
#include <circle/net/netsubsystem.h>

#include <circle_glue.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "viceemulatorcore.h"

#define GPIO_JOY_1_UP 17     // Pin 1
#define GPIO_JOY_1_DOWN 18   // Pin 2
#define GPIO_JOY_1_LEFT 27   // Pin 3
#define GPIO_JOY_1_RIGHT 22  // Pin 4
#define GPIO_JOY_1_FIRE 23   // Pin 6

#define GPIO_JOY_2_UP 5      // Pin 1
#define GPIO_JOY_2_DOWN 6    // Pin 2
#define GPIO_JOY_2_LEFT 12   // Pin 3
#define GPIO_JOY_2_RIGHT 13  // Pin 4
#define GPIO_JOY_2_FIRE 19   // Pin 6

#define NUM_GPIO_PINS 18

// Special functions
#define GPIO_MENU 16

#define GPIO_KBD_PB_0 17     // Same as GPIO_JOY_1_UP
#define GPIO_KBD_PB_1 18     // Same as GPIO_JOY_1_DOWN
#define GPIO_KBD_PB_2 27     // Same as GPIO_JOY_1_LEFT
#define GPIO_KBD_PB_3 22     // Same as GPIO_JOY_1_RIGHT
#define GPIO_KBD_PB_4 23     // Same as GPIO_JOY_1_FIRE
#define GPIO_KBD_PB_5 24
#define GPIO_KBD_PB_6 25
#define GPIO_KBD_PB_7 8

#define GPIO_KBD_PA_0 5      // Same as GPIO_JOY_2_UP
#define GPIO_KBD_PA_1 6      // Same as GPIO_JOY_2_DOWN
#define GPIO_KBD_PA_2 12     // Same as GPIO_JOY_2_LEFT
#define GPIO_KBD_PA_3 13     // Same as GPIO_JOY_2_RIGHT
#define GPIO_KBD_PA_4 19     // Same as GPIO_JOY_2_FIRE
#define GPIO_KBD_PA_5 26
#define GPIO_KBD_PA_6 20
#define GPIO_KBD_PA_7 21
#define GPIO_KBD_RESTORE 04

// These are the indices of each pin in our gpio pin array
#define GPIO_KBD_PA_0_INDEX 0
#define GPIO_KBD_PA_1_INDEX 1
#define GPIO_KBD_PA_2_INDEX 2
#define GPIO_KBD_PA_3_INDEX 3
#define GPIO_KBD_PA_4_INDEX 4
#define GPIO_KBD_PA_5_INDEX 5
#define GPIO_KBD_PA_6_INDEX 6
#define GPIO_KBD_PA_7_INDEX 7
#define GPIO_KBD_PB_0_INDEX 8
#define GPIO_KBD_PB_1_INDEX 9
#define GPIO_KBD_PB_2_INDEX 10
#define GPIO_KBD_PB_3_INDEX 11
#define GPIO_KBD_PB_4_INDEX 12
#define GPIO_KBD_PB_5_INDEX 13
#define GPIO_KBD_PB_6_INDEX 14
#define GPIO_KBD_PB_7_INDEX 15
#define GPIO_KBD_RESTORE_INDEX 16
#define GPIO_MENU_INDEX 17

// Used as indices into the hardwires joystick arrays
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

extern "C" { void circle_fs_ready(); }

class ViceApp
{
public:
        enum TShutdownMode
        {
                ShutdownNone,
                ShutdownHalt,
                ShutdownReboot
        };

        ViceApp (const char *kernel) :
                FromKernel (kernel)
        {
        }

        virtual ~ViceApp (void)
        {
        }

        virtual bool Initialize (void)
        {
                if (!mSerial.Initialize (115200))
                {
                        return false;
                }

                // Initialize our replacement newlib stdio. Give it
                // a pointer to our serial device so we can use printf
                // to serial as soon as possible.
                CGlueStdioInit (&mSerial);

                if (!mInterrupt.Initialize ()) {
			return false;
		}

		return true;
        }

        virtual void Cleanup (void)
        {
        }

        virtual TShutdownMode Run (void) = 0;

        const char *GetKernelName(void) const
        {
                return FromKernel;
        }

private:
        char const *FromKernel;

protected:
        CActLED            mActLED;
        ViceOptions        mViceOptions;
        CKernelOptions     mOptions;
        CSerialDevice      mSerial;
        CMemorySystem      mMemory;
        CDeviceNameService mDeviceNameService;
        CNullDevice        mNullDevice;
        CExceptionHandler  mExceptionHandler;
        CInterruptSystem   mInterrupt;
};

class ViceScreenApp : public ViceApp
{
public:
        ViceScreenApp(const char *kernel)
                : ViceApp (kernel),
                  mEmulatorCore(&mMemory),
                  mScreen(mViceOptions.GetCanvasWidth(),
			  mViceOptions.GetCanvasHeight()),
                  mTimer (&mInterrupt),
                  mLogger (mOptions.GetLogLevel(), &mTimer),
                  mGPIOManager (&mInterrupt) {
        }

        virtual bool Initialize (void)
        {
                if (!ViceApp::Initialize ())
                {
                        return false;
                }

                if (!mScreen.Initialize ())
                {
                        return false;
                }

                if (!mLogger.Initialize (&mScreen))
                {
                        return false;
                }

		if (!mEmulatorCore.Initialize()) {
			return false;
		}

                if (!mTimer.Initialize ()) {
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
	   gpioPins[GPIO_MENU_INDEX] =
              new CGPIOPin(GPIO_MENU, GPIOModeInputPullUp, &mGPIOManager);

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

	   joystickPins1[JOY_UP] = gpioPins[GPIO_KBD_PB_0_INDEX];
	   joystickPins1[JOY_DOWN] = gpioPins[GPIO_KBD_PB_1_INDEX];
	   joystickPins1[JOY_LEFT] = gpioPins[GPIO_KBD_PB_2_INDEX];
	   joystickPins1[JOY_RIGHT] = gpioPins[GPIO_KBD_PB_3_INDEX];
	   joystickPins1[JOY_FIRE] = gpioPins[GPIO_KBD_PB_4_INDEX];
	   joystickPins2[JOY_UP] = gpioPins[GPIO_KBD_PA_0_INDEX];
	   joystickPins2[JOY_DOWN] = gpioPins[GPIO_KBD_PA_1_INDEX];
	   joystickPins2[JOY_LEFT] = gpioPins[GPIO_KBD_PA_2_INDEX];
	   joystickPins2[JOY_RIGHT] = gpioPins[GPIO_KBD_PA_3_INDEX];
	   joystickPins2[JOY_FIRE] = gpioPins[GPIO_KBD_PA_4_INDEX];

	   gpioPins[GPIO_KBD_RESTORE_INDEX] =
              new CGPIOPin(GPIO_KBD_RESTORE, GPIOModeInputPullUp, &mGPIOManager);
	}

        ViceEmulatorCore   mEmulatorCore;
        CViceScreenDevice  mScreen;
        CTimer             mTimer;
        CLogger            mLogger;
        CGPIOManager       mGPIOManager;

        CGPIOPin           *joystickPins1[5];
        CGPIOPin           *joystickPins2[5];
        CGPIOPin           *gpioPins[NUM_GPIO_PINS];
};

class ViceStdioApp: public ViceScreenApp
{
public:
        ViceStdioApp (const char *kernel)
                : ViceScreenApp (kernel),
                  mDWHCI (&mInterrupt, &mTimer),
                  mEMMC (&mInterrupt, &mTimer, &mActLED),
                  mConsole (&mScreen)
        {
        }

        virtual bool Initialize (void)
        {
                if (!ViceScreenApp::Initialize ())
                {
                        return false;
                }

                if (!mEMMC.Initialize ())
                {
                        return false;
                }

		int partition = mViceOptions.GetDiskPartition ();
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
                const char *volumeName = mViceOptions.GetDiskVolume ();
		char fatFsVol[VOLUME_NAME_LEN];
		strncpy (fatFsVol, volumeName, VOLUME_NAME_LEN-2);
		strcat (fatFsVol, ":");

                CGlueStdioSetPartitionForVolume (volumeName, partition, ss);

                if (f_mount (&mFileSystem, fatFsVol, 1) != FR_OK) {
                        mLogger.Write (GetKernelName (), LogError,
                            "Cannot mount partition: %s", fatFsVol);
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
                if (!mDWHCI.Initialize ())
                {
                        return false;
                }

                if (!mConsole.Initialize ())
                {
                        return false;
                }

                return true;
        }

        virtual void Cleanup (void)
        {
		// When mounting, fatfs gets ":" appended.  But StdioInit
		// does not.
                const char *volumeName = mViceOptions.GetDiskVolume ();
		char fatFsVol[VOLUME_NAME_LEN];
		strncpy (fatFsVol, volumeName, VOLUME_NAME_LEN-2);
		strcat (fatFsVol, ":");

                if (f_mount (0, fatFsVol, 0) != FR_OK) {
                        mLogger.Write (GetKernelName (), LogError,
                                "Cannot unmount drive");
                }
                ViceScreenApp::Cleanup ();
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

        CDWHCIDevice    mDWHCI;
        CEMMCDevice     mEMMC;
        FATFS           mFileSystem;
        CConsole        mConsole;

	int mBootStatWhat[MAX_BOOTSTAT_LINES];
	char *mBootStatFile[MAX_BOOTSTAT_LINES];
	int mBootStatSize[MAX_BOOTSTAT_LINES];
};

#endif

