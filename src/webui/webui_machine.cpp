//
// webui_machine.cpp
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

#include "webui.h"

#if BMC64_WEBUI

#include "webui_machine.h"

#include <stdio.h>

namespace {

#if defined(RASPI_C64) || defined(RASPI_C128)
// A .crt cartridge image is accepted too: VICE's autostart_autodetect
// attaches it on the C64 and C128 only.
const char *const kRunTypes[] = {
    "d64", "d71", "d81", "d82", "g64", "x64", "t64", "tap", "prg", "p00",
    "crt", 0,
};
#elif defined(RASPI_VIC20) || defined(RASPI_PLUS4)
const char *const kRunTypes[] = {
    "d64", "d71", "d81", "g64", "x64", "t64", "tap", "prg", "p00", 0,
};
#elif defined(RASPI_PLUS4EMU)
// plus4emu's Autostart only loads a program into memory.
const char *const kRunTypes[] = {
    "prg", "p00", 0,
};
#elif defined(RASPI_PET)
// The PET has no Autostart in its menu, so the web UI offers none either.
const char *const kRunTypes[] = {
    0,
};
#endif

// The listing editor only knows C64 BASIC V2 so far.
const WebUiMachine kMachine = {
#if defined(RASPI_C64)
    "C64", "/settings.txt", kRunTypes, "v2",
#elif defined(RASPI_C128)
    "C128", "/settings-c128.txt", kRunTypes, "",
#elif defined(RASPI_VIC20)
    "VIC20", "/settings-vic20.txt", kRunTypes, "",
#elif defined(RASPI_PLUS4)
    "Plus4", "/settings-plus4.txt", kRunTypes, "",
#elif defined(RASPI_PLUS4EMU)
    "Plus4Emu", "/settings-plus4emu.txt", kRunTypes, "",
#elif defined(RASPI_PET)
    "Pet", "/settings-pet.txt", kRunTypes, "",
#endif
};

char Lower(char c) {
  return (c >= 'A' && c <= 'Z') ? (char) (c - 'A' + 'a') : c;
}

}  // namespace

const WebUiMachine *WebUiMachineGet(void) {
  return &kMachine;
}

boolean WebUiMachineCanRun(const char *ext) {
  for (const char *const *type = kMachine.run_types; *type != 0; type++) {
    const char *a = ext;
    const char *b = *type;
    while (*a != '\0' && Lower(*a) == *b) {
      a++;
      b++;
    }
    if (*a == '\0' && *b == '\0') return TRUE;
  }
  return FALSE;
}

int WebUiMachineCapsJson(char *out, unsigned size) {
  char run[128];
  unsigned used = 0;
  for (const char *const *type = kMachine.run_types; *type != 0; type++) {
    int n = snprintf(run + used, sizeof(run) - used, "%s\"%s\"",
                     used == 0 ? "" : ",", *type);
    if (n < 0 || (unsigned) n >= sizeof(run) - used) return -1;
    used += (unsigned) n;
  }
  run[used] = '\0';

  int length = snprintf(out, size,
                        "{\"settings\":\"%s\",\"run\":[%s],\"basic\":\"%s\"}",
                        kMachine.settings, run, kMachine.basic);
  if (length < 0 || (unsigned) length >= size) return -1;
  return length;
}

#endif  // BMC64_WEBUI
