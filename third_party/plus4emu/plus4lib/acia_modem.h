//
// acia_modem.h
//
// Connects BMC64's network modem (src/bmcmodem.cpp) to plus4emu's built-in
// 6551 ACIA at $FD00.
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

#ifndef PLUS4EMU_ACIA_MODEM_H
#define PLUS4EMU_ACIA_MODEM_H

#include "plus4emu.h"

#ifdef __cplusplus
extern "C" {
#endif

// Enables the ACIA and puts the modem on its lines. Call once, on the
// emulator core, before the first reset.
void acia_modem_attach(Plus4VM *vm);

#ifdef __cplusplus
}
#endif

#endif
