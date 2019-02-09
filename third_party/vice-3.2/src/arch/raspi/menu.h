/*
 * menu.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "ui.h"

#ifndef VICE_MENU_H
#define VICE_MENU_H

#define MENU_ABOUT 0
#define MENU_LICENSE 1

#define MENU_ATTACH_DISK_8 2
#define MENU_ATTACH_DISK_9 3
#define MENU_ATTACH_DISK_10 4
#define MENU_ATTACH_DISK_11 5

#define MENU_DETACH_DISK_8 6
#define MENU_DETACH_DISK_9 7
#define MENU_DETACH_DISK_10 8
#define MENU_DETACH_DISK_11 9

#define MENU_SOFT_RESET 10
#define MENU_HARD_RESET 11

#define MENU_ATTACH_CART 12
#define MENU_ATTACH_CART_8K 13
#define MENU_ATTACH_CART_16K 14
#define MENU_ATTACH_CART_ULTIMAX 15
#define MENU_DETACH_CART 16

#define MENU_ATTACH_TAPE 17
#define MENU_DETACH_TAPE 18

#define MENU_DISK_FILE 19
#define MENU_TAPE_FILE 20
#define MENU_CART_FILE 21
#define MENU_CART_8K_FILE 22
#define MENU_CART_16K_FILE 23
#define MENU_CART_ULTIMAX_FILE 24

#define MENU_TEXT 25

#define MENU_SWAP_JOYSTICKS 26

#define MENU_JOYSTICK_PORT_1 27
#define MENU_JOYSTICK_PORT_2 28

#define MENU_CONFIG_GP_1 29
#define MENU_CONFIG_GP_2 30

#define MENU_WARP_MODE 31

#define MENU_AUTOSTART 32
#define MENU_AUTOSTART_FILE 33

#define MENU_IECDEVICE_8 34
#define MENU_IECDEVICE_9 35
#define MENU_IECDEVICE_10 36
#define MENU_IECDEVICE_11 37

#define MENU_COLOR_PALETTE 38

#define MENU_USB_0_PREF 39
#define MENU_USB_1_PREF 40

#define MENU_SAVE_SETTINGS 41
#define MENU_KEYBOARD_TYPE 42

#define MENU_TAPE_START 43
#define MENU_TAPE_STOP 44
#define MENU_TAPE_REWIND 45
#define MENU_TAPE_RESET 46

#define MENU_DRIVE_SOUND_EMULATION 47
#define MENU_DRIVE_SOUND_EMULATION_VOLUME 48

#define KEYBOARD_TYPE_US 0
#define KEYBOARD_TYPE_UK 1

// Called at initialzation
void build_menu(struct menu_item* root);

// Called when menu is up to handle gamepad configuration
void menu_gamepad_event(int device, int button, int dpad);

int menu_get_keyboard_type(void);

#endif
