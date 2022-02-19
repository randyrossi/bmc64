//
// plus4emulatorcore.cpp
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

#include "plus4emulatorcore.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "third_party/plus4emu/main.h"
}

#include "third_party/plus4emu/resid/filter.hpp"

Plus4EmulatorCore::Plus4EmulatorCore(CMemorySystem *pMemorySystem,
                                     int cyclesPerSecond) :
#ifdef ARM_ALLOW_MULTI_CORE
       CMultiCoreSupport(pMemorySystem),
#endif
       launch_(false), cyclesPerSecond_(cyclesPerSecond) {}

Plus4EmulatorCore::~Plus4EmulatorCore(void) {}

void Plus4EmulatorCore::RunMainPlus4(bool wait) {
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

  // Call Plus4Emu's main_program

  printf("Starting emulator main loop\n");
  int argc = 0;
  char *argv[] = {};
  emu_machine_init(m_options->GetRasterSkip(), false /* no vdc */);
  main_program(argc, argv);
  emu_exit();
}

// Initializing the filters for each SID model takes quite a bit.
// This method instantiates a modified Filter object in ReSid.  The
// modified version lets us initialize both SIDs in parallel on seperate
// cores.  This saves a lot of boot time since when PLUS4EMU eventually gets
// around to initializing the filters, they are already done and opening
// ReSid is very quick at that point.  See resid/filter.cc for
// modifications done for BMC64.
void Plus4EmulatorCore::ComputeResidFilter(int model) { Plus4::Filter f(model); }

void Plus4EmulatorCore::Run(unsigned nCore) {
  assert(nCore > 0);
  switch (nCore) {
  case 1:
    RunMainPlus4(true);
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

bool Plus4EmulatorCore::Init(ViceOptions* options) {
  m_options = options;
  return Initialize();
}

void Plus4EmulatorCore::LaunchEmulator(char *timing_option) {
  strncpy(timing_option_, timing_option, 8);
#ifdef ARM_ALLOW_MULTI_CORE
  m_Lock.Acquire();
  launch_ = true;
  m_Lock.Release();
#else
  RunMainPlus4(false);
#endif
}
