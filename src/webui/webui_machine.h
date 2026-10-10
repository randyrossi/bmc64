//
// webui_machine.h
//
// What the web UI can do on the machine this build emulates: its name,
// the file types Autostart accepts, the BASIC dialect the listing editor
// understands, and Main's settings file. GET /api/status reports these as
// "caps" so the page needs no machine knowledge of its own.
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

#ifndef _webui_machine_h
#define _webui_machine_h

#include <circle/types.h>

struct WebUiMachine {
  // Machine name as profiles use it ("C64", "C128", "VIC20", "Plus4").
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
