//
// webui_machine.cpp
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
#elif defined(RASPI_PLUS4)
const char *const kRunTypes[] = {
    "d64", "d71", "d81", "g64", "x64", "t64", "tap", "prg", "p00", 0,
};
#elif defined(RASPI_PLUS4EMU)
// plus4emu's Autostart only loads a program into memory.
const char *const kRunTypes[] = {
    "prg", "p00", 0,
};
#endif

// The listing editor only knows C64 BASIC V2 so far.
const WebUiMachine kMachine = {
#if defined(RASPI_C64)
    "C64", "/settings.txt", kRunTypes, "v2",
#elif defined(RASPI_C128)
    "C128", "/settings-c128.txt", kRunTypes, "",
#elif defined(RASPI_PLUS4)
    "Plus4", "/settings-plus4.txt", kRunTypes, "",
#elif defined(RASPI_PLUS4EMU)
    "Plus4Emu", "/settings-plus4emu.txt", kRunTypes, "",
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
