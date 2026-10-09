//
// webui.h
//
// Optional LAN web UI for BMC64. Serves a small single-page app that
// shows status and can reboot the machine. All web UI code lives under
// src/webui/ and is only active on the builds BMC64_WEBUI names (C64,
// C128, Plus/4, Plus4Emu); it is a no-op elsewhere.
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

#ifndef _webui_h
#define _webui_h

// The machine builds the web UI runs on (see webui_machine.cpp).
#if defined(RASPI_C64) || defined(RASPI_C128) || defined(RASPI_PLUS4) || \
    defined(RASPI_PLUS4EMU)
#define BMC64_WEBUI 1
#else
#define BMC64_WEBUI 0
#endif

class CNetSubSystem;

// Start the web UI HTTP server as a Circle scheduler task (runs on the
// kernel/networking core, never the emulator core). Call once, after the
// network subsystem has been created. Safe to call with a null pointer
// and safe to call more than once; only the first call has an effect.
//
// pin: optional HTTP Basic Auth PIN. When null or empty the server is
// open; otherwise every request must carry Authorization: Basic
// base64(<anything>:<pin>).
void WebUiStart(CNetSubSystem *network, const char *pin);

#endif
