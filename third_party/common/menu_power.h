/*
 * menu_power.h
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
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
