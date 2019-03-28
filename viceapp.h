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
#include <circle/nulldevice.h>
#include <circle/exceptionhandler.h>
#include <circle/interrupt.h>
#include <circle/koptions.h>
#include "viceoptions.h"
#include "vicescreen.h"
#include <circle/serial.h>
#include <circle/timer.h>
#include <circle/logger.h>
#include <circle/usb/dwhcidevice.h>
#include <SDCard/emmc.h>
#include <ff.h>
#include <circle/input/console.h>
#include <circle/sched/scheduler.h>
#include <circle/net/netsubsystem.h>

#include <circle_glue.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

                return mInterrupt.Initialize ();
        }

        virtual void Cleanup (void)
        {
        }

        virtual TShutdownMode Run (void) = 0;

        const char *GetKernelName(void) const
        {
                return FromKernel;
        }

protected:
        CActLED            mActLED;
        ViceOptions        mViceOptions;
        CKernelOptions     mOptions;
        CSerialDevice      mSerial;
        CDeviceNameService mDeviceNameService;
        CNullDevice        mNullDevice;
        CExceptionHandler  mExceptionHandler;
        CInterruptSystem   mInterrupt;

private:
        char const *FromKernel;
};

class ViceScreenApp : public ViceApp
{
public:
        ViceScreenApp(const char *kernel)
                : ViceApp (kernel),
                  mScreen(mViceOptions.GetCanvasWidth(),
			mViceOptions.GetCanvasHeight()),
                  mTimer (&mInterrupt),
                  mLogger (mOptions.GetLogLevel(), &mTimer)
        {
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

                return mTimer.Initialize ();
        }

protected:
        CViceScreenDevice mScreen;
        CTimer          mTimer;
        CLogger         mLogger;
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

		// When mounting, fatfs gets ":" appended.  But StdioInit
		// does not.
                const char *volumeName = mViceOptions.GetDiskVolume ();
		char fatFsVol[VOLUME_NAME_LEN];
		strncpy (fatFsVol, volumeName, VOLUME_NAME_LEN-2);
		strcat (fatFsVol, ":");

                CGlueStdioSetPartitionForVolume (volumeName, partition);

                if (f_mount (&mFileSystem, fatFsVol, 1) != FR_OK) {
                        mLogger.Write (GetKernelName (), LogError,
                            "Cannot mount partition: %s", fatFsVol);
                        return false;
                }

                if (!mDWHCI.Initialize ())
                {
                        return false;
                }

                if (!mConsole.Initialize ())
                {
                        return false;
                }

                // Initialize our replacement newlib stdio
                CGlueStdioInit ();

                InitBootStat();

                mLogger.Write (GetKernelName (), LogNotice, "Compile time: " __DATE__ " " __TIME__);

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

