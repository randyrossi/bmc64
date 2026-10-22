//
// fan.h
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
// The Pi 5's fan header (the Active Cooler or a case fan), driven from the
// SoC's temperature with the same steps as Raspberry Pi OS. Called from
// the board code (board.cpp).
//

#ifndef BMC64_PI45_FAN_H_
#define BMC64_PI45_FAN_H_

#if RASPPI >= 5

// Core 0, often: starts the fan's PWM the first time, then reads the
// temperature once a second and sets the fan's speed.
void Pi45FanPoll(void);

#endif

#endif
