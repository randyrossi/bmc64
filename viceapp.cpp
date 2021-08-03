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

#include "fbl.h"

#if defined(RASPI_C64)
#include "bootstat_c64.h"
#elif defined(RASPI_C128)
#include "bootstat_c128.h"
#elif defined(RASPI_VIC20)
#include "bootstat_vic20.h"
#elif defined(RASPI_PLUS4)
#include "bootstat_plus4.h"
#elif defined(RASPI_PLUS4EMU)
#include "bootstat_plus4emu.h"
#elif defined(RASPI_PET)
#include "bootstat_pet.h"
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

int ViceApp::circle_get_machine_timing() {
  // See circle.h for valid values
  return mViceOptions.GetMachineTiming();
}

#if defined(RASPI_PLUS4) | defined(RASPI_PLUS4EMU)
int ViceApp::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI || timing == MACHINE_TIMING_NTSC_DPI) {
    // 60hz
    return 1792080;
  } else if (circle_get_machine_timing() == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1793672;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1778400;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1781245;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1778400;
  }
}
#elif defined(RASPI_VIC20)
int ViceApp::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1017900;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1018804;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1107600;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1109372;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1017900;
  }
}
#elif defined(RASPI_C64) | defined(RASPI_C128)
int ViceApp::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1025700;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1026611;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 982800;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 984404;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 982800;
  }
}
#elif defined(RASPI_PET)
int ViceApp::circle_cycles_per_second() {
  int timing = circle_get_machine_timing();
  if (timing == MACHINE_TIMING_NTSC_HDMI) {
    // 60hz
    return 1013760;
  } else if (timing == MACHINE_TIMING_NTSC_COMPOSITE) {
    // Actual C64's NTSC Composite frequency is 59.826 but the Pi's vertical
    // sync frequency on composite is 60.053. See c64.h for how this is
    // calculated. This keeps audio buffer to a minimum using ReSid.
    return 1014661;
  } else if (timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI || timing == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else if (timing == MACHINE_TIMING_PAL_HDMI) {
    // 50hz
    return 1001600;
  } else if (timing == MACHINE_TIMING_PAL_COMPOSITE) {
    // Actual C64's PAL Composite frequency is 50.125 but the Pi's vertical
    // sync frequency on composite is 50.0816. See c64.h for how this is
    // calculated.  This keep audio buffer to a minimum using ReSid.
    return 1003202;
  } else if (timing == MACHINE_TIMING_PAL_CUSTOM_HDMI || timing == MACHINE_TIMING_PAL_CUSTOM_DPI) {
    return mViceOptions.GetCyclesPerSecond();
  } else {
    return 1000000;
  }
}
#else
  #error Unknown RASPI_ variant
#endif

//
// ViceScreenApp impl
//

bool ViceScreenApp::Initialize(void) {
  if (!ViceApp::Initialize()) {
    return false;
  }

  if (mViceOptions.SerialEnabled()) {
     if (!mLogger.Initialize(&mSerial)) {
        return false;
     }
  } else {
     if (!mLogger.Initialize(&mNullDevice)) {
        return false;
     }
  }

  if (!mEmulatorCore->Init(&mViceOptions)) {
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

  FrameBufferLayer::Initialize();

  return true;
}

// Setup GPIO pins for scanning keyboard, button or joysticks.
void ViceScreenApp::SetupGPIOForInput() {
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

  gpioPins[NO_FIXED_PURPOSE_1_INDEX] =
      new CGPIOPin(2, GPIOModeInputPullUp, &mGPIOManager);
  gpioPins[NO_FIXED_PURPOSE_2_INDEX] =
      new CGPIOPin(3, GPIOModeInputPullUp, &mGPIOManager);
  gpioPins[NO_FIXED_PURPOSE_3_INDEX] =
      new CGPIOPin(9, GPIOModeInputPullUp, &mGPIOManager);
  gpioPins[NO_FIXED_PURPOSE_4_INDEX] =
      new CGPIOPin(10, GPIOModeInputPullUp, &mGPIOManager);

  // Convenience arrays for joysticks
  config_1_joystickPins1[JOY_UP] = gpioPins[GPIO_CONFIG_1_JOY_1_UP_INDEX];
  config_1_joystickPins1[JOY_DOWN] = gpioPins[GPIO_CONFIG_1_JOY_1_DOWN_INDEX];
  config_1_joystickPins1[JOY_LEFT] = gpioPins[GPIO_CONFIG_1_JOY_1_LEFT_INDEX];
  config_1_joystickPins1[JOY_RIGHT] = gpioPins[GPIO_CONFIG_1_JOY_1_RIGHT_INDEX];
  config_1_joystickPins1[JOY_FIRE] = gpioPins[GPIO_CONFIG_1_JOY_1_FIRE_INDEX];

  config_1_joystickPins2[JOY_UP] = gpioPins[GPIO_CONFIG_1_JOY_2_UP_INDEX];
  config_1_joystickPins2[JOY_DOWN] = gpioPins[GPIO_CONFIG_1_JOY_2_DOWN_INDEX];
  config_1_joystickPins2[JOY_LEFT] = gpioPins[GPIO_CONFIG_1_JOY_2_LEFT_INDEX];
  config_1_joystickPins2[JOY_RIGHT] = gpioPins[GPIO_CONFIG_1_JOY_2_RIGHT_INDEX];
  config_1_joystickPins2[JOY_FIRE] = gpioPins[GPIO_CONFIG_1_JOY_2_FIRE_INDEX];

  config_0_joystickPins1[JOY_UP] = gpioPins[GPIO_CONFIG_0_JOY_1_UP_INDEX];
  config_0_joystickPins1[JOY_DOWN] = gpioPins[GPIO_CONFIG_0_JOY_1_DOWN_INDEX];
  config_0_joystickPins1[JOY_LEFT] = gpioPins[GPIO_CONFIG_0_JOY_1_LEFT_INDEX];
  config_0_joystickPins1[JOY_RIGHT] = gpioPins[GPIO_CONFIG_0_JOY_1_RIGHT_INDEX];
  config_0_joystickPins1[JOY_FIRE] = gpioPins[GPIO_CONFIG_0_JOY_1_FIRE_INDEX];

  config_0_joystickPins2[JOY_UP] = gpioPins[GPIO_CONFIG_0_JOY_2_UP_INDEX];
  config_0_joystickPins2[JOY_DOWN] = gpioPins[GPIO_CONFIG_0_JOY_2_DOWN_INDEX];
  config_0_joystickPins2[JOY_LEFT] = gpioPins[GPIO_CONFIG_0_JOY_2_LEFT_INDEX];
  config_0_joystickPins2[JOY_RIGHT] = gpioPins[GPIO_CONFIG_0_JOY_2_RIGHT_INDEX];
  config_0_joystickPins2[JOY_FIRE] = gpioPins[GPIO_CONFIG_0_JOY_2_FIRE_INDEX];

  config_2_joystickPins[JOY_UP] = gpioPins[GPIO_CONFIG_2_WAVESHARE_UP_INDEX];
  config_2_joystickPins[JOY_DOWN] = gpioPins[GPIO_CONFIG_2_WAVESHARE_DOWN_INDEX];
  config_2_joystickPins[JOY_LEFT] = gpioPins[GPIO_CONFIG_2_WAVESHARE_LEFT_INDEX];
  config_2_joystickPins[JOY_RIGHT] = gpioPins[GPIO_CONFIG_2_WAVESHARE_RIGHT_INDEX];
  config_2_joystickPins[JOY_FIRE] = gpioPins[GPIO_CONFIG_2_WAVESHARE_B_INDEX];
  config_2_joystickPins[JOY_POTX] = gpioPins[GPIO_CONFIG_2_WAVESHARE_A_INDEX];
  config_2_joystickPins[JOY_POTY] = gpioPins[GPIO_CONFIG_2_WAVESHARE_Y_INDEX];

  config_3_joystickPins1[JOY_UP] = gpioPins[GPIO_CONFIG_3_JOY_1_UP_INDEX];
  config_3_joystickPins1[JOY_DOWN] = gpioPins[GPIO_CONFIG_3_JOY_1_DOWN_INDEX];
  config_3_joystickPins1[JOY_LEFT] = gpioPins[GPIO_CONFIG_3_JOY_1_LEFT_INDEX];
  config_3_joystickPins1[JOY_RIGHT] = gpioPins[GPIO_CONFIG_3_JOY_1_RIGHT_INDEX];
  config_3_joystickPins1[JOY_FIRE] = gpioPins[GPIO_CONFIG_3_JOY_1_FIRE_INDEX];

  config_3_joystickPins2[JOY_UP] = gpioPins[GPIO_CONFIG_3_JOY_2_UP_INDEX];
  config_3_joystickPins2[JOY_DOWN] = gpioPins[GPIO_CONFIG_3_JOY_2_DOWN_INDEX];
  config_3_joystickPins2[JOY_LEFT] = gpioPins[GPIO_CONFIG_3_JOY_2_LEFT_INDEX];
  config_3_joystickPins2[JOY_RIGHT] = gpioPins[GPIO_CONFIG_3_JOY_2_RIGHT_INDEX];
  config_3_joystickPins2[JOY_FIRE] = gpioPins[GPIO_CONFIG_3_JOY_2_FIRE_INDEX];

  config_3_userportPins[USERPORT_PB0] = gpioPins[GPIO_CONFIG_3_USERPORT_PB0_INDEX];
  config_3_userportPins[USERPORT_PB1] = gpioPins[GPIO_CONFIG_3_USERPORT_PB1_INDEX];
  config_3_userportPins[USERPORT_PB2] = gpioPins[GPIO_CONFIG_3_USERPORT_PB2_INDEX];
  config_3_userportPins[USERPORT_PB3] = gpioPins[GPIO_CONFIG_3_USERPORT_PB3_INDEX];
  config_3_userportPins[USERPORT_PB4] = gpioPins[GPIO_CONFIG_3_USERPORT_PB4_INDEX];
  config_3_userportPins[USERPORT_PB5] = gpioPins[GPIO_CONFIG_3_USERPORT_PB5_INDEX];
  config_3_userportPins[USERPORT_PB6] = gpioPins[GPIO_CONFIG_3_USERPORT_PB6_INDEX];
  config_3_userportPins[USERPORT_PB7] = gpioPins[GPIO_CONFIG_3_USERPORT_PB7_INDEX];
}

// Setup GPIO pins for DPI
void ViceScreenApp::SetupGPIOForDPI() {
  for (int i=0; i< 28; i++) {
    DPIPins[i] =
      new CGPIOPin(i, GPIOModeAlternateFunction2, &mGPIOManager);
  }
}

void ViceScreenApp::SetupGPIO() {
  if (mViceOptions.DPIEnabled()) {
     SetupGPIOForDPI();
  } else {
     SetupGPIOForInput();
  }
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
#elif defined(RASPI_PLUS4)
  fp = fopen("/PLUS4/bootstat.txt", "r");
#elif defined(RASPI_PLUS4EMU)
  fp = NULL;
#elif defined(RASPI_PET)
  fp = fopen("/PET/bootstat.txt", "r");
#else
  #error Unknown RASPI_ variant
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
      if (strcmp(file, "d1541II") == 0) {
        // Ignore legacy d1541II faking found file without a fully
        // qualified path.
        printf("Ignoring d1541II in bootstat.txt\n");
        continue;
      }
      mBootStatWhat[num] = BOOTSTAT_WHAT_STAT;
    } else if (strcmp(what, "fail") == 0) {
      if (strcmp(file,"rpi_pos.vkm") == 0) {
        // Ignore legacy mistake blocking rpi_pos.vkm
        printf("Ignoring rpi_pos.vkm in bootstat.txt\n");
        continue;
      }
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

  if (f_mount(&mFileSystemSD, fatFsVol, 1) != FR_OK) {
    mLogger.Write(GetKernelName(), LogError, "Cannot mount partition: %s",
                  fatFsVol);
    return false;
  }

  InitBootStat();

  // Now that emmc is initialized, launch
  // the emulator main loop on CORE 1 before USBHCII.
  int timing_int = mViceOptions.GetMachineTiming();
  if (timing_int == MACHINE_TIMING_NTSC_HDMI ||
      timing_int == MACHINE_TIMING_NTSC_CUSTOM_HDMI ||
      timing_int == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing_int == MACHINE_TIMING_NTSC_DPI ||
      timing_int == MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    strcpy(mTimingOption, "-ntsc");
  } else {
    strcpy(mTimingOption, "-pal");
  }

#ifdef ARM_ALLOW_MULTI_CORE
  mEmulatorCore->LaunchEmulator(mTimingOption);
#endif

  // This takes 1.5 seconds to init.
  if (!mUSBHCII.Initialize()) {
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

void ViceStdioApp::circle_find_usb(int (*usb)[3]) {
  CDevice* usb1 = CDeviceNameService::Get()->GetDevice ("umsd1", TRUE);
  (*usb)[0] = usb1 ? 1 : 0;
  CDevice* usb2 = CDeviceNameService::Get()->GetDevice ("umsd2", TRUE);
  (*usb)[1] = usb2 ? 1 : 0;
  CDevice* usb3 = CDeviceNameService::Get()->GetDevice ("umsd3", TRUE);
  (*usb)[2] = usb3 ? 1 : 0;
}

int ViceStdioApp::circle_mount_usb(int usb) {
  int status;
  switch (usb) {
     case 0:
       status = f_mount(&mFileSystemUSB1, "USB:", 1);
       break;
     case 1:
       status = f_mount(&mFileSystemUSB1, "USB2:", 1);
       break;
     case 2:
       status = f_mount(&mFileSystemUSB1, "USB3:", 1);
       break;
     default: return 0;
  }

  if (status != FR_OK) {
    mLogger.Write(GetKernelName(), LogError, "Cannot mount usb %d", usb);
    return 0;
  }

  return 1;
}

int ViceStdioApp::circle_unmount_usb(int usb) {
  int status;
  switch (usb) {
     case 0:
       status = f_mount(0, "USB:", 1);
       break;
     case 1:
       status = f_mount(0, "USB2:", 1);
       break;
     case 2:
       status = f_mount(0, "USB3:", 1);
       break;
     default: return 0;
  }

  if (status != FR_OK) {
    mLogger.Write(GetKernelName(), LogError, "Cannot unmount usb %d", usb);
    return 0;
  }

  return 1;
}
