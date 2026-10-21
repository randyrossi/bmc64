//
// vchiqdevice.h
//
// The Pi 4 and 5 builds don't use the VideoCore's services (VCHIQ): the
// display and sound have their own drivers (src/av/). This stands in for
// Circle's addon/vc4/vchiq/vchiqdevice.h (found first on the include path,
// src/pi45/pi45.mk), so the code holding a CVCHIQDevice stays as it is.
//
// This file is part of BMC64's Raspberry Pi 4 and 5 support, from BMA800XL.
// See LICENSE for licensing details.
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
