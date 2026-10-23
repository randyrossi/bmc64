//
// update_boot.c
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
// The two entry points the rest of BMC64 calls (update.h).
//

#include "update.h"

#include <stddef.h>

#include "update_apply.h"
#include "../sdcard/sd_fs.h"
#include "update_host.h"
#include "update_view.h"

static int update_pending;

void update_boot_check(void) {
  if (ua_resume() > 0) {
    // Files were moved into place; start again on the new version.
    uh_log("rebooting into the updated version");
    uh_reboot();
  }
  update_pending = sd_stat(UA_ZIP, NULL, NULL) == 0;
  if (update_pending) {
    uh_log(UA_ZIP " found");
  }
}

void update_run_if_pending(void) {
  if (!update_pending) {
    return;
  }
  update_pending = 0;
  uh_take_screen(1);
  uv_run();
  uh_take_screen(0);
}
