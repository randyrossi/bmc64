//
// power.cpp
//
// Restarts or powers off the Raspberry Pi (circle_reboot and
// circle_power_off in third_party/common/circle.h). The callers write out
// the disk images first (menu_power.c); this writes out the log file and
// hands over to the hardware.
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

#include "../viceapp.h"

#include <circle/bcmwatchdog.h>
#include <circle/logger.h>
#include <circle/startup.h>

#define POWER_LOG "power"

extern "C" void circle_reboot(void) {
  CLogger::Get()->Write(POWER_LOG, LogNotice, "Rebooting");
  ViceCloseLogFile();
  reboot();
}

extern "C" void circle_power_off(void) {
  CLogger::Get()->Write(POWER_LOG, LogNotice, "Powering off");
  ViceCloseLogFile();
  // The Pi 0-3 cannot switch its own power off. Restarting into partition
  // 63 makes the firmware halt instead of booting, as Linux's poweroff
  // does: the board idles at low power with the display off until power
  // is cycled.
  CBcmWatchdog watchdog;
  watchdog.Restart(CBcmWatchdog::PartitionHalt);
}
