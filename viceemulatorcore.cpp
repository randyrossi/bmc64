//
// viceemulatorcore.cpp
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

#include "viceemulatorcore.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "third_party/vice-3.3/src/main.h"
}

#include "third_party/vice-3.3/src/resid/filter.h"

ViceEmulatorCore::ViceEmulatorCore(CMemorySystem *pMemorySystem) :
#ifdef ARM_ALLOW_MULTI_CORE
       CMultiCoreSupport(pMemorySystem),
#endif
       launch_(false) {}

ViceEmulatorCore::~ViceEmulatorCore(void) {}

void ViceEmulatorCore::RunMainVice(bool wait) {
  if (wait) {
     printf("Core waiting for launch\n");
     bool waiting = true;
     while (waiting) {
       m_Lock.Acquire();
       if (launch_)
         waiting = false;
       m_Lock.Release();
     }
  }

  // Call Vice's main_program

  // Use -soundsync 0 option for 'flexible'
  // sound sync.

  // Use -refresh 1 option to turn off the 'auto'
  // refresh which screws up badly after some time.
  // The vertical blank really messes up vice's
  // algorithm that decides to skip frames. Might
  // want to go back to using the open gl hook.
  // See arch/raspi/videoarch.c

  printf("Starting emulator main loop\n");

#if defined(RASPI_C64) || defined(RASPI_C128)
  int argc = 11;
  char *argv[] = {
      (char *)"vice", timing_option_, (char *)"-sounddev", (char *)"raspi",
      (char *)"-soundoutput", (char *)"1", (char *)"-soundsync", (char *)"0",
      (char *)"-refresh", (char *)"1",
      // Unless we disable the video cache, vsync is messed up
      (char *)"+VICIIvcache",
  };
#elif defined(RASPI_VIC20)
  int argc = 11;
  char *argv[] = {
      (char *)"vice", timing_option_, (char *)"-sounddev", (char *)"raspi",
      (char *)"-soundoutput", (char *)"1", (char *)"-soundsync", (char *)"0",
      (char *)"-refresh", (char *)"1",
      // Unless we disable the video cache, vsync is messed up
      (char *)"+VICvcache",
  };
#else
#error "RASPI_[model] NOT DEFINED"
#endif

  main_program(argc, argv);
  main_exit();
}

// Initializing the filters for each SID model takes quite a bit.
// This method instantiates a modified Filter object in ReSid.  The
// modified version lets us initialize both SIDs in parallel on seperate
// cores.  This saves a lot of boot time since when VICE eventually gets
// around to initializing the filters, they are already done and opening
// ReSid is very quick at that point.  See resid/filter.cc for
// modifications done for BMC64.
void ViceEmulatorCore::ComputeResidFilter(int model) { reSID::Filter f(model); }

void ViceEmulatorCore::Run(unsigned nCore) {
  assert(nCore > 0);
  switch (nCore) {
  case 1:
    RunMainVice(true);
    break;
  case 2:
    // Core 2 will initialize 6581 filter data. Then sleep.
    ComputeResidFilter(0);
    break;
  case 3:
    // Core 3 will initialize 8580 filter data. Then sleep.
    ComputeResidFilter(1);
    break;
  }

#ifdef ARM_ALLOW_MULTI_CORE
  printf("Core %d idle\n", nCore);
  asm("dsb\n\t"
      "1: wfi\n\t"
      "b 1b\n\t");
#endif
}

void ViceEmulatorCore::LaunchEmulator(char *timing_option) {
  strncpy(timing_option_, timing_option, 8);
#ifdef ARM_ALLOW_MULTI_CORE
  m_Lock.Acquire();
  launch_ = true;
  m_Lock.Release();
#else
  RunMainVice(false);
#endif
}
