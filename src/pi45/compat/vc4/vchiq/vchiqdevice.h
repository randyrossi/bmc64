//
// vchiqdevice.h
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
// The Pi 4 and 5 builds don't use the VideoCore's services (VCHIQ): the
// display and sound have their own drivers (src/av/). This stands in for
// Circle's addon/vc4/vchiq/vchiqdevice.h (found first on the include path,
// src/pi45/pi45.mk), so the code holding a CVCHIQDevice stays as it is.
//

#ifndef BMC64_PI45_COMPAT_VCHIQDEVICE_H_
#define BMC64_PI45_COMPAT_VCHIQDEVICE_H_

#include <circle/interrupt.h>
#include <circle/memory.h>
#include <circle/types.h>

class CVCHIQDevice {
public:
  CVCHIQDevice(CMemorySystem *pMemory, CInterruptSystem *pInterrupt) {
    (void)pMemory;
    (void)pInterrupt;
  }
  boolean Initialize(void) { return TRUE; }
};

#endif
