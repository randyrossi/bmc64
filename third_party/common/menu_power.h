/*
 * menu_power.h
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

#ifndef RASPI_MENU_POWER_H
#define RASPI_MENU_POWER_H

#include "ui.h"

// The on-screen menu's Power folder (machine resets, Reboot BMC64, Shut
// down), and the safe restart and power off it shares with the web UI.

// Adds the "Power" folder.
void build_power_menu(struct menu_item *root);

// Write out the disk and tape images, then restart BMC64, or halt the Pi
// until power is cycled. Main loop only; neither returns.
void menu_power_reboot(void);
void menu_power_off(void);

#endif
