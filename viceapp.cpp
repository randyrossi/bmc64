//
// viceapp.cpp
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

#include "viceapp.h"

#include "fb2.h"

#if defined(RASPI_C64)
int dflt_bootStatNum = 19;

int dflt_bootStatWhat[] = {
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",           "basic",   "chargen",    "d1541II",  "rpi_pos.vkm",
    "fliplist-C64.vfl", "mps803",  "mps803.vpl", "nl10-cbm", "1520.vpl",
    "dos1540",          "dos1570", "dos2000",
    "dos4000",          "dos2031", "dos2040",    "dos3040",  "dos4040",
    "dos1001",
};
int dflt_bootStatSize[] = {8192, 8192, 4096, 16384, 0, 0, 0, 0, 0, 0, 0,
                           0,    0,    0,    0,     0, 0, 0, 0};
#elif defined(RASPI_C128)
int dflt_bootStatNum = 21;

int dflt_bootStatWhat[] = {
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",     "kernal64", "basic64",    "basichi", "basiclo",
    "chargen",    "d1541II",  "rpi_pos.vkm",
    "fliplist-C128.vfl", "mps803",  "mps803.vpl", "nl10-cbm", "1520.vpl",
    "dos1540",          "dos1570", "dos2000",
    "dos4000",          "dos2031", "dos2040",    "dos3040",  "dos4040",
    "dos1001",
};
int dflt_bootStatSize[] = {16384, 8192, 8192, 16384, 16384,
                           4096, 16384, 0, 0, 0, 0, 0, 0, 0,
                           0,    0,    0,    0,     0, 0, 0, 0};

#elif defined(RASPI_VIC20)
int dflt_bootStatNum = 19;

int dflt_bootStatWhat[] = {
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_STAT,
    BOOTSTAT_WHAT_STAT, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL, BOOTSTAT_WHAT_FAIL,
    BOOTSTAT_WHAT_FAIL,
};

const char *dflt_bootStatFile[] = {
    "kernal",           "basic",   "chargen",    "d1541II",  "rpi_pos.vkm",
    "fliplist-C64.vfl", "mps803",  "mps803.vpl", "nl10-cbm", "1520.vpl",
    "dos1540",          "dos1570", "dos2000",
    "dos4000",          "dos2031", "dos2040",    "dos3040",  "dos4040",
    "dos1001",
};
int dflt_bootStatSize[] = {8192, 8192, 4096, 16384, 0, 0, 0, 0, 0, 0, 0,
                           0,    0,    0,    0,     0, 0, 0, 0};
#else
  #error Unknown RASPI_ variant
#endif

//
// ViceApp impl
//

bool ViceApp::Initialize(void) {
  if (!mSerial.Initialize(115200)) {
    return false;
  }

  // Initialize our replacement newlib stdio. Give it
  // a pointer to our serial device so we can use printf
  // to serial as soon as possible.
  CGlueStdioInit(mViceOptions.SerialEnabled() ? &mSerial : nullptr);

  if (!mInterrupt.Initialize()) {
    return false;
  }

  return true;
}

//
// ViceScreenApp impl
//

bool ViceScreenApp::Initialize(void) {
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

  if (!mVCHIQ.Initialize()) {
    return false;
  }

  SetupGPIO();

  FrameBuffer2::Initialize();

  return true;
}

void ViceScreenApp::SetupGPIO() {
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

//
// ViceStdioApp impl
//

void ViceStdioApp::InitBootStat() {
  FILE *fp;
#if defined(RASPI_C64)
  fp = fopen("/C64/bootstat.txt", "r");
#elif defined(RASPI_C128)
  fp = fopen("/C128/bootstat.txt", "r");
#elif defined(RASPI_VIC20)
  fp = fopen("/VIC20/bootstat.txt", "r");
#endif

  if (fp == NULL) {
    printf("Could not find bootstat. Using default list.\n");

    CGlueStdioInitBootStat(dflt_bootStatNum, dflt_bootStatWhat,
                           dflt_bootStatFile, dflt_bootStatSize);

    return;
  }

  char line[80];
  int num = 0;
  while (fgets(line, 79, fp)) {
    if (feof(fp))
      break;
    if (strlen(line) == 0)
      continue;
    if (line[0] == '#')
      continue;
    char *what = strtok(line, ",");
    if (what == NULL)
      continue;
    char *file = strtok(NULL, ",");
    if (file == NULL)
      continue;
    char *size = strtok(NULL, ",");
    if (size == NULL)
      continue;
    if (size[strlen(size) - 1] == '\n') {
      size[strlen(size) - 1] = '\0';
    }

    if (num >= MAX_BOOTSTAT_LINES) {
      printf("Warning: bootstat.txt too long, max %d entries\n",
             MAX_BOOTSTAT_LINES);
      break;
    }

    if (strcmp(what, "stat") == 0) {
      mBootStatWhat[num] = BOOTSTAT_WHAT_STAT;
    } else if (strcmp(what, "fail") == 0) {
      mBootStatWhat[num] = BOOTSTAT_WHAT_FAIL;
    } else {
      printf("Ignoring unknown bootstat.txt '%s'\n", what);
      continue;
    }

    // These never get freed...
    mBootStatFile[num] = (char *)malloc(MAX_BOOTSTAT_FLEN);
    strncpy(mBootStatFile[num], file, MAX_BOOTSTAT_FLEN);
    mBootStatSize[num] = atoi(size);

    num++;
  }

  fclose(fp);

  CGlueStdioInitBootStat(num, mBootStatWhat, (const char **)mBootStatFile,
                         mBootStatSize);
}

void ViceStdioApp::DisableBootStat() {
  CGlueStdioInitBootStat(0, nullptr, nullptr, nullptr);
}

bool ViceStdioApp::Initialize(void) {
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

void ViceStdioApp::Cleanup(void) {
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
