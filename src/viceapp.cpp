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
#include "vice_network.h"
#include "network_time_sync.h"
#include "../third_party/common/circle.h"
#include "../third_party/common/io_stats.h"
#include "../third_party/common/menu_logging.h"
#include "fbl.h"
#include <circle/bcm2835.h>
#include <circle/bcmpropertytags.h>
#include <circle/memio.h>

static const unsigned int WIFI_SCAN_DURATION_US = 4000000;
static const unsigned int WIFI_CONNECT_TIMEOUT_US = 30000000;
static CLoggingDevice *gLoggingDevice = nullptr;

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

void ViceApp::DrainLogging() {
  if (gLoggingDevice != nullptr) {
    gLoggingDevice->Drain();
  }
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
    strncpy(mBootStatFile[num], file, MAX_BOOTSTAT_FLEN - 1);
    mBootStatFile[num][MAX_BOOTSTAT_FLEN - 1] = '\0';
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

void ViceStdioApp::LoadNetworkDevice() {
#if defined(RASPI_C64) || defined(RASPI_C128)
  const char *settings_path;
#if defined(RASPI_C64)
  settings_path = "/settings.txt";
#elif defined(RASPI_C128)
  settings_path = "/settings-c128.txt";
#endif

  FILE *settings = fopen(settings_path, "r");
  if (settings == nullptr) {
    return;
  }

  char line[64];
  while (fgets(line, sizeof(line), settings) != nullptr) {
    int network_device;
    if (sscanf(line, "network_device=%d", &network_device) == 1 &&
        network_device >= 0 && network_device <= 2) {
      mNetworkDevice = network_device;
      continue;
    }

    int timezone_offset_minutes;
    if (sscanf(line, "timezone_offset_minutes=%d",
               &timezone_offset_minutes) == 1 &&
        timezone_offset_minutes >= -12 * 60 &&
        timezone_offset_minutes <= 14 * 60) {
      mTimezoneOffsetMinutes = timezone_offset_minutes;
    }
  }
  fclose(settings);
#endif
}
void ViceStdioApp::InitializeNetwork() {
#if defined(RASPI_C64) || defined(RASPI_C128)
  if (mNetworkDevice == 0) {
    SetNetworkStatus(CIRCLE_NETWORK_DISABLED);
    mLogger.Write(GetKernelName(), LogNotice, "Networking not enabled");
    return;
  }

  if (mNetworkDevice == 1 &&
      ViceNetworkHasOnboardEthernet(mMachineInfo.GetMachineModel())) {
    SetNetworkStatus(CIRCLE_NETWORK_ETHERNET_INITIALIZING);
    mNet = new CNetSubSystem(0, 0, 0, 0, "bmc64", NetDeviceTypeEthernet);
    if (!mNet->Initialize(FALSE)) {
      SetNetworkStatus(CIRCLE_NETWORK_ETHERNET_INIT_FAILED);
      mLogger.Write(GetKernelName(), LogError,
                    "Cannot initialize Ethernet network stack");
      delete mNet;
      mNet = nullptr;
    } else {
      ViceNetworkSetSubsystem(mNet);
      StartNetworkTimeSync(mNet);
      SetNetworkStatus(CIRCLE_NETWORK_ETHERNET_WAITING_FOR_DHCP);
      mLogger.Write(GetKernelName(), LogNotice, "Networking: Ethernet initialized");
    }
    return;
  }

  if (mNetworkDevice == 1) {
    mLogger.Write(GetKernelName(), LogNotice,
                  "Ethernet selected, but this Raspberry Pi has no onboard Ethernet; disabling networking");
    mNetworkDevice = 0;
    SetNetworkStatus(CIRCLE_NETWORK_DISABLED);
    return;
  }

  if (!ViceNetworkHasOnboardWifi(mMachineInfo.GetMachineModel())) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_UNAVAILABLE);
    mLogger.Write(GetKernelName(), LogError,
                  "Wi-Fi selected, but this Raspberry Pi has no onboard WLAN");
    return;
  }

  CString firmwarePath;
  CString configPath;
  firmwarePath.Format("%s:/firmware/", mViceOptions.GetDiskVolume());
  configPath.Format("%s:/wpa_supplicant.conf", mViceOptions.GetDiskVolume());

  if (!ViceNetworkHasWifiFirmware((const char *)firmwarePath)) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_FIRMWARE_MISSING);
    mLogger.Write(GetKernelName(), LogError,
                  "Wi-Fi firmware is missing from %s",
                  (const char *)firmwarePath);
    return;
  }

  FIL configFile;
  if (f_open(&configFile, (const char *)configPath, FA_READ) != FR_OK) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_CONFIG_MISSING);
    mLogger.Write(GetKernelName(), LogError,
                  "Wi-Fi enabled but WPA config is missing: %s",
                  (const char *)configPath);
    return;
  }
  f_close(&configFile);

  SetNetworkStatus(CIRCLE_NETWORK_WIFI_DEVICE_INITIALIZING);
  mWLAN = new CBcm4343Device((const char *)firmwarePath);
  if (!mWLAN->Initialize()) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_DEVICE_INIT_FAILED);
    mLogger.Write(GetKernelName(), LogError, "Cannot initialize WLAN");
    delete mWLAN;
    mWLAN = nullptr;
    return;
  }

  mNet = new CNetSubSystem(0, 0, 0, 0, "bmc64", NetDeviceTypeWLAN);
  if (!mNet->Initialize(FALSE)) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_NETWORK_INIT_FAILED);
    mLogger.Write(GetKernelName(), LogError, "Cannot initialize WLAN network stack");
    delete mNet;
    mNet = nullptr;
    delete mWLAN;
    mWLAN = nullptr;
    return;
  }
  ViceNetworkSetSubsystem(mNet);
  StartNetworkTimeSync(mNet);
  SetNetworkStatus(CIRCLE_NETWORK_WIFI_WPA_INITIALIZING);
  mLogger.Write(GetKernelName(), LogNotice, "Networking: Wi-Fi initialized");

  mWPASupplicant = new CWPASupplicant((const char *)configPath);
  if (!mWPASupplicant->Initialize()) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_WPA_INIT_FAILED);
    mLogger.Write(GetKernelName(), LogError, "Cannot initialize WPA supplicant");
    delete mWPASupplicant;
    mWPASupplicant = nullptr;
  } else {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_CONNECTING);
  }
#endif
}

void ViceStdioApp::SetNetworkStatus(int status) {
  if (mNetworkStatus == status) {
    return;
  }
  mNetworkStatus = status;
  ViceNetworkNotifyStatusChanged();
}

int ViceStdioApp::GetNetworkStatus(void) const {
  if (mNet != nullptr && mNet->IsRunning()) {
    CString address;
    mNet->GetConfig()->GetIPAddress()->Format(&address);
    if (strcmp((const char *)address, "0.0.0.0") != 0) {
      return mNetworkDevice == 1 ? CIRCLE_NETWORK_ETHERNET_CONNECTED
                                 : CIRCLE_NETWORK_WIFI_CONNECTED;
    }
  }

  return mNetworkStatus;
}

int ViceStdioApp::WifiIsRunning(void) const {
#if defined(RASPI_C64) || defined(RASPI_C128)
  return ViceNetworkHasOnboardWifi(mMachineInfo.GetMachineModel()) && mWLAN != nullptr;
#else
  return 0;
#endif
}

int ViceStdioApp::ConnectWifi(void) {
#if defined(RASPI_C64) || defined(RASPI_C128)
  if (!ViceNetworkHasOnboardWifi(mMachineInfo.GetMachineModel())) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_UNAVAILABLE);
    return 0;
  }

  if (mWLAN == nullptr) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_DEVICE_NOT_INITIALIZED);
    mLogger.Write(GetKernelName(), LogError,
                  "Wi-Fi connection requested without WLAN");
    return 0;
  }

  CString config_path;
  config_path.Format("%s:/wpa_supplicant.conf", mViceOptions.GetDiskVolume());
  SetNetworkStatus(CIRCLE_NETWORK_WIFI_CONNECTING);
  mLogger.Write(GetKernelName(), LogNotice,
                "Wi-Fi reconnecting with %s", (const char *)config_path);
  delete mWPASupplicant;
  mWPASupplicant = new CWPASupplicant((const char *)config_path);
  if (!mWPASupplicant->Initialize()) {
    SetNetworkStatus(CIRCLE_NETWORK_WIFI_WPA_INIT_FAILED);
    mLogger.Write(GetKernelName(), LogError,
                  "Cannot restart WPA supplicant");
    delete mWPASupplicant;
    mWPASupplicant = nullptr;
    return 0;
  }

  unsigned int connect_started_at = CTimer::GetClockTicks();
  while (!CWPASupplicant::IsConnected()) {
    if ((unsigned int)(CTimer::GetClockTicks() - connect_started_at) >=
        WIFI_CONNECT_TIMEOUT_US) {
      mLogger.Write(GetKernelName(), LogError,
                    "Wi-Fi connection timed out after %u seconds",
                    WIFI_CONNECT_TIMEOUT_US / 1000000);
      SetNetworkStatus(CIRCLE_NETWORK_WIFI_CONNECTION_TIMEOUT);
      return 0;
    }
    CScheduler::Get()->MsSleep(100);
  }
  mLogger.Write(GetKernelName(), LogNotice, "Wi-Fi connected");
  SetNetworkStatus(CIRCLE_NETWORK_WIFI_CONNECTED);
  return 1;
#else
  SetNetworkStatus(CIRCLE_NETWORK_WIFI_UNSUPPORTED);
  mLogger.Write(GetKernelName(), LogError,
                "Wi-Fi connection is not supported by this build");
  return 0;
#endif
}

int ViceStdioApp::ScanWifiAccessPoints(struct wifi_access_point *access_points,
                                       unsigned int max_access_points) {
#if defined(RASPI_C64) || defined(RASPI_C128)
  if (!ViceNetworkHasOnboardWifi(mMachineInfo.GetMachineModel())) {
    return 0;
  }

  if (mWLAN == nullptr) {
    mLogger.Write(GetKernelName(), LogNotice,
                  "Wi-Fi scan unavailable until Wi-Fi is selected and rebooted");
    return 0;
  }

  uint8_t buffer[FRAME_BUFFER_SIZE];
  unsigned int result_length;
  while (mWLAN->ReceiveScanResult(buffer, &result_length)) {
  }

  if (!mWLAN->Control("escan %u", 3)) {
    mLogger.Write(GetKernelName(), LogError, "Cannot start Wi-Fi scan");
    return 0;
  }

  unsigned int count = 0;
  unsigned int result_messages = 0;
  unsigned int scan_started_at = CTimer::GetClockTicks();
  do {
    count = ViceNetworkCollectWifiScanResults(mWLAN, access_points, max_access_points,
                                   count, &result_messages);
    CScheduler::Get()->MsSleep(100);
  } while ((unsigned int)(CTimer::GetClockTicks() - scan_started_at) <
           WIFI_SCAN_DURATION_US);

  mWLAN->Control("escan 0");
  count = ViceNetworkCollectWifiScanResults(mWLAN, access_points, max_access_points,
                                 count, &result_messages);
  mLogger.Write(GetKernelName(), LogNotice,
                "Wi-Fi scan received %u results, found %u access points",
                result_messages, count);
  return count;
#else
  (void)access_points;
  (void)max_access_points;
  return 0;
#endif
}
bool ViceStdioApp::Initialize(void) {
  ViceNetworkSetStdioApp(this);
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

  // Log the negotiated SD bus clock. The SDHOST clock divider (SDCDIV, 11 bits,
  // holds divisor - 2) is programmed by Circle/firmware from the core clock;
  // high-speed (SDR25) mode runs the bus near 50 MHz, the default near 25 MHz.
  // Reads one documented BCM2835 register - no card I/O, no Circle patch.
#if RASPPI <= 3
  {
    unsigned cdiv = read32(ARM_SDHOST_BASE + 0x0c) & 0x7ff;
    unsigned sd_khz =
        mMachineInfo.GetClockRate(CLOCK_ID_CORE) / ((cdiv + 2) * 1000);
    mLogger.Write(GetKernelName(), LogNotice, "SD bus clock ~%u kHz (%s)",
                  sd_khz, sd_khz >= 40000 ? "high-speed mode" : "default speed");
  }
#endif

  if (mViceOptions.FileLoggingEnabled()) {
    CGlueStdioInit(&mSerial);
    mLogger.SetNewTarget(&mSerial);
    gLoggingDevice = new CLoggingDevice;
    gLoggingDevice->Initialize(&mSerial);
    if (!gLoggingDevice->OpenFile()) {
      mLogger.Write(GetKernelName(), LogError, "Cannot open /bmc64.log");
    } else {
      char logBuffer[1024];
      int logBytes;
      while ((logBytes = mLogger.Read(logBuffer, sizeof logBuffer)) > 0) {
        gLoggingDevice->Write(logBuffer, logBytes);
      }

      CGlueStdioInit(gLoggingDevice);
      mLogger.SetNewTarget(gLoggingDevice);
    }
  }

  InitBootStat();
  LoadNetworkDevice();
  if (!ConfigureSystemTimeZone(mTimezoneOffsetMinutes)) {
    mLogger.Write(GetKernelName(), LogWarning, "Cannot configure timezone");
  }
  InitializeNetwork();

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
#ifdef BMC64_IO_STATS
  // Emit the final I/O counters while the log file is still open.
  io_stats_dump("shutdown");
#endif

  ViceNetworkSetStdioApp(nullptr);
  delete mWPASupplicant;
  ViceNetworkSetSubsystem(nullptr);
  delete mNet;
  delete mWLAN;
  if (gLoggingDevice != nullptr) {
    gLoggingDevice->CloseFile();
    delete gLoggingDevice;
    gLoggingDevice = nullptr;
  }

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
