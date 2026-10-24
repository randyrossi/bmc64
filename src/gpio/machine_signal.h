//
// machine_signal.h
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

#ifndef BMC64_GPIO_MACHINE_SIGNAL_H
#define BMC64_GPIO_MACHINE_SIGNAL_H

#include <circle/gpiomanager.h>
#include <circle/gpiopin.h>

// GPIO machine signal: drives GPIO 9, 10 and 11 as bits 0, 1 and 2 of a
// value worked out by third_party/common/machine_signal.c. Only the pins are
// handled here; what the bits mean is up to machine_signal.c.

#define MACHINE_SIGNAL_NUM_PINS 3

class CMachineSignal {
public:
  CMachineSignal(void);

  // GPIO 9 and 10 are shared with the rest of the GPIO code (gpioPins[]);
  // GPIO 11 has no pin object until it is first driven.
  void Init(CGPIOPin *pin9, CGPIOPin *pin10, CGPIOManager *manager);

  // Drive the pins with BITS, or return them to input-pullup.
  void Set(bool enabled, unsigned bits);

  // Drive the pins again after something else changed their mode, such as
  // circle_reset_gpio() making every pin an input.
  void Apply(void);

  // True while GPIO 9 and 10 are outputs, so they must not be read.
  bool IsDriving(void) const;

private:
  CGPIOPin *mPins[MACHINE_SIGNAL_NUM_PINS];
  CGPIOManager *mManager;
  bool mEnabled;
  unsigned mBits;
};

#endif
