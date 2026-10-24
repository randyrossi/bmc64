//
// machine_signal.cpp
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

#include "machine_signal.h"

#define MACHINE_SIGNAL_PIN_11 11

CMachineSignal::CMachineSignal(void)
    : mManager(0), mEnabled(false), mBits(0) {
  for (int i = 0; i < MACHINE_SIGNAL_NUM_PINS; i++) {
    mPins[i] = 0;
  }
}

void CMachineSignal::Init(CGPIOPin *pin9, CGPIOPin *pin10,
                          CGPIOManager *manager) {
  mPins[0] = pin9;
  mPins[1] = pin10;
  mManager = manager;
}

void CMachineSignal::Set(bool enabled, unsigned bits) {
  mEnabled = enabled;
  mBits = bits;
  Apply();
}

void CMachineSignal::Apply(void) {
  if (mPins[0] == 0 || mPins[1] == 0) {
    // DPI uses the GPIO pins, so there are none to drive.
    return;
  }

  if (mEnabled && mPins[2] == 0) {
    mPins[2] = new CGPIOPin(MACHINE_SIGNAL_PIN_11, GPIOModeInputPullUp,
                            mManager);
  }

  for (int i = 0; i < MACHINE_SIGNAL_NUM_PINS; i++) {
    if (mPins[i] == 0) {
      continue;
    }
    if (mEnabled) {
      mPins[i]->SetMode(GPIOModeOutput);
      mPins[i]->Write(mBits & (1 << i) ? HIGH : LOW);
    } else {
      mPins[i]->SetMode(GPIOModeInputPullUp);
    }
  }
}

bool CMachineSignal::IsDriving(void) const {
  return mEnabled && mPins[0] != 0;
}
