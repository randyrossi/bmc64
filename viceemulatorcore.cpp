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
#include "third_party/vice-3.2/src/main.h"
}

ViceEmulatorCore::ViceEmulatorCore (CMemorySystem *pMemorySystem) : 
   CMultiCoreSupport (pMemorySystem), launch_(false) {
}

ViceEmulatorCore::~ViceEmulatorCore (void) {
}

void ViceEmulatorCore::Run (unsigned nCore)
{
  assert (nCore > 0);
  if (nCore == 1) {
    printf ("Core 1 waiting for launch\n");
    bool waiting = true;
    while (waiting) {
      m_Lock.Acquire();
      if (launch_) waiting = false;
      circle_sleep(100);
      m_Lock.Release();
    }

    printf ("Starting emulator on core 1\n");
    int argc = 11;
    char *argv[] = {
      (char*)"vice",
      timing_option_,
      (char*)"-sounddev",
      (char*)"raspi",
      (char*)"-soundoutput",
      (char*)"1",
      (char*)"-soundsync",
      (char*)"0",
      (char*)"-refresh",
      (char*)"1",
      // Unless we disable the video cache, vsync is messed up
      (char*)"+VICIIvcache",
    };
    main_program(argc, argv);
  } else {
    printf ("Core %d idle\n", nCore);
    for (;;) { circle_sleep(1000000); }
  }
}

void ViceEmulatorCore::Launch() {
  m_Lock.Acquire();
  launch_ = true;
  m_Lock.Release();
}


void ViceEmulatorCore::SetTimingOption(char* timing_option) {
  strncpy(timing_option_, timing_option, 8);
}
