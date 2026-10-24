/*
 * machine_signal.h
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef RASPI_MACHINE_SIGNAL_H
#define RASPI_MACHINE_SIGNAL_H

// GPIO machine signal (enable_gpio_machine_signal in cmdline.txt): GPIO 9,
// 10 and 11 tell the outside world which machine is running, either as a
// 3-bit machine code for a microcontroller or as the colour of an RGB LED.
// The pins are driven by src/gpio/machine_signal.cpp.

// Work out the value for the running machine and drive the pins. Call once
// the emulator is set up, and when the PET model changes.
void machine_signal_update(void);

// 1 while GPIO 9 and 10 are driven, so Custom GPIO can't use them.
int machine_signal_reserved(void);

#endif
