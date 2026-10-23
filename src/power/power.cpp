//
// power.cpp
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Restarts or powers off the Raspberry Pi (circle_reboot and
// circle_power_off in third_party/common/circle.h). The callers write out
// the disk images first (menu_power.c); this writes out the log file and
// hands over to the hardware.
//

#include "../viceapp.h"
#if RASPPI >= 4
#include "../pi45/board.h"
#endif

#include <circle/bcmwatchdog.h>
#include <circle/logger.h>
#include <circle/startup.h>

#define POWER_LOG "power"

extern "C" void circle_reboot(void) {
  CLogger::Get()->Write(POWER_LOG, LogNotice, "Rebooting");
  ViceCloseLogFile();
#if RASPPI >= 4
  Pi45BoardBeforeRestart();
#endif
  reboot();
}

extern "C" void circle_power_off(void) {
  CLogger::Get()->Write(POWER_LOG, LogNotice, "Powering off");
  ViceCloseLogFile();
  // The Pi 0-3 cannot switch its own power off. Restarting into partition
  // 63 makes the firmware halt instead of booting, as Linux's poweroff
  // does: the board idles at low power with the display off until power
  // is cycled.
#if RASPPI >= 4
  // The Pi 4 B halts (waking on GPIO3), the Pi 400 switches off, and the
  // Pi 5, 500 and 500+ halt until their power button or key is pressed.
  Pi45BoardBeforeRestart();
#endif
  CBcmWatchdog watchdog;
  watchdog.Restart(CBcmWatchdog::PartitionHalt);
}
