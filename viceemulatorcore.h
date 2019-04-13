//
// viceemulatorcore.h
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

#ifndef viceemualtorcore_h
#define viceemulatorcore_h

#include <circle/memory.h>
#include <circle/multicore.h>
#include <circle/spinlock.h>

extern "C" {
#include "third_party/vice-3.3/src/arch/raspi/circle.h"
}

class ViceEmulatorCore : public CMultiCoreSupport
{
public:
	ViceEmulatorCore (CMemorySystem *pMemorySystem);
	~ViceEmulatorCore (void);

	void Run (unsigned nCore) override;

        void LaunchEmulator(char* timing_option);

private:
	bool launch_;
	char timing_option_[8];
	CSpinLock m_Lock;

	void RunMainVice();
	void ComputeResidFilter(int model);
};

#endif
