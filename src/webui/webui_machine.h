//
// webui_machine.h
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
// What the web UI can do on the machine this build emulates: its name,
// the file types Autostart accepts, the BASIC dialect the listing editor
// understands, and Main's settings file. GET /api/status reports these as
// "caps" so the page needs no machine knowledge of its own.
//

#ifndef _webui_machine_h
#define _webui_machine_h

#include <circle/types.h>

struct WebUiMachine {
  // Machine name as profiles use it ("C64", "C128", "VIC20", "Plus4",
  // "Pet").
  const char *name;
  // Main's usual settings file.
  const char *settings;
  // Lowercase extensions Autostart accepts, ending with a null.
  const char *const *run_types;
  // BASIC dialect of the listing editor ("v2"), or "" for none.
  const char *basic;
};

const WebUiMachine *WebUiMachineGet(void);

// TRUE if Autostart accepts files with this extension (any case).
boolean WebUiMachineCanRun(const char *ext);

// Writes the "caps" JSON object for /api/status. Returns its length, or
// -1 if it doesn't fit.
int WebUiMachineCapsJson(char *out, unsigned size);

#endif
