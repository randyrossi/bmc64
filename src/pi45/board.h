//
// board.h
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
// What the Pi 4 and 5 need beyond the display and sound, called from
// BMC64's kernel (src/kernel.cpp) and power code (src/power/power.cpp).
//

#ifndef BMC64_PI45_BOARD_H_
#define BMC64_PI45_BOARD_H_

// Core 0, often: on the Pi 5 the power button (the Pi 500's power key),
// and the Pi 400's power key, power off as the menu does; on the Pi 5 the
// fan (fan.h).
void Pi45BoardPoll(void);

// Each USB keyboard report's keys: the Pi 400's keyboard sends its power
// key (Fn+F10) as Keyboard Power, 0x66.
void Pi45BoardKeys(const unsigned char keys[6]);

// Before restarting or powering off: blanks the screen and tells the
// firmware.
void Pi45BoardBeforeRestart(void);

// Runs the display at the emulated machine's frame rate: its
// CYCLES_PER_SECOND over the cycles in its PAL or NTSC frame.
void Pi45BoardMatchRate(int cycles_per_second, bool ntsc);

// BMC64's integer scaling (scaling_params in cmdline.txt, made for one
// screen size) for the screen in use: the same whole-number scale of
// FBW x FBH, as large as fits (4x of 240 lines at 1080p, for 3x at 720p);
// other sizes in proportion to the screen config.txt asks for (1280x684
// for 720p is 1920x1026 at 1080p).
void Pi45BoardScalingParams(int *fbw, int *fbh, int *sx, int *sy);

#endif
