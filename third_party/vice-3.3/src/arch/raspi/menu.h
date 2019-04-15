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
#define MENU_TAPE_FASTFWD 46
#define MENU_TAPE_RESET 47
#define MENU_TAPE_RESET_COUNTER 48
#define MENU_TAPE_RECORD 49

#define MENU_DRIVE_SOUND_EMULATION 50
#define MENU_DRIVE_SOUND_EMULATION_VOLUME 51

#define MENU_CONFIGURE_USB_0 52
#define MENU_CONFIGURE_USB_1 53
#define MENU_USB_0_X_AXIS 54
#define MENU_USB_0_Y_AXIS 55
#define MENU_USB_1_X_AXIS 56
#define MENU_USB_1_Y_AXIS 57
#define MENU_USB_0_WATCH_RAW 58
#define MENU_USB_1_WATCH_RAW 59

#define MENU_USB_0_BTN_ASSIGN 60
#define MENU_USB_1_BTN_ASSIGN 61

#define MENU_KEYBOARD_MENU_ALT_F12 62
#define MENU_SID_ENGINE 63
#define MENU_SID_MODEL 64
#define MENU_SID_FILTER 65

#define MENU_ERROR_DIALOG 66
#define MENU_INFO_DIALOG 67

#define MENU_SAVE_SNAP 68
#define MENU_SAVE_SNAP_FILE 69
#define MENU_LOAD_SNAP 70
#define MENU_LOAD_SNAP_FILE 71

#define MENU_DEMO_MODE 72
#define MENU_OVERLAY 73

#define MENU_DRIVE_TYPE_8 74
#define MENU_DRIVE_TYPE_9 75
#define MENU_DRIVE_TYPE_10 76
#define MENU_DRIVE_TYPE_11 77
#define MENU_TAPE_RESET_WITH_MACHINE 78

#define MENU_COLOR_BRIGHTNESS 80
#define MENU_COLOR_CONTRAST 81
#define MENU_COLOR_GAMMA 82
#define MENU_COLOR_TINT 83
#define MENU_COLOR_RESET 84

#define MENU_DRIVE_SELECT 85
#define MENU_DRIVE_CHANGE_MODEL_8 90
#define MENU_DRIVE_CHANGE_MODEL_9 91
#define MENU_DRIVE_CHANGE_MODEL_10 92
#define MENU_DRIVE_CHANGE_MODEL_11 93

#define MENU_SUB_NONE 0
#define MENU_SUB_PICK_FILE 1
#define MENU_SUB_UP_DIR 2
#define MENU_SUB_ENTER_DIR 3

#define KEYBOARD_TYPE_US 0
#define KEYBOARD_TYPE_UK 1

#define MENU_SID_ENGINE_FAST 0
#define MENU_SID_ENGINE_RESID 1

#define MENU_SID_MODEL_6581 0
#define MENU_SID_MODEL_8580 1

#define ALT_F12_DISABLED 0
#define ALT_F12_COMMODOREF7 1

#define OVERLAY_NEVER 0
#define OVERLAY_ALWAYS 1
#define OVERLAY_ON_ACTIVITY 2

// Called at initialzation
void build_menu(struct menu_item* root);

int menu_get_keyboard_type(void);
int menu_alt_f12(void);
void menu_swap_joysticks(void);
int overlay_enabled(void);
int overlay_forced(void);

void menu_about_to_activate(void);
void menu_about_to_deactivate(void);

#endif
