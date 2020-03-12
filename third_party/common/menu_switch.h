/*
 * menu_switch.h
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

#ifndef RASPI_MENU_SWITCH_H
#define RASPI_MENU_SWITCH_H

#include "emux_api.h"

#define CONFIG_TXT_LINE_LEN 512
#define KEY_LEN 32
#define VALUE_LEN 96
#define DESC_LEN 64

typedef enum {
  BMC64_VIDEO_STANDARD_UNKNOWN,
  BMC64_VIDEO_STANDARD_NTSC,
  BMC64_VIDEO_STANDARD_PAL,
} BMC64VideoStandard;

typedef enum {
  BMC64_VIDEO_OUT_UNKNOWN,
  BMC64_VIDEO_OUT_HDMI,
  BMC64_VIDEO_OUT_COMPOSITE,
  BMC64_VIDEO_OUT_DPI,
} BMC64VideoOut;

struct machine_option {
   char key[KEY_LEN];
   char value[VALUE_LEN];
   struct machine_option *next;
};

struct machine_entry {
   int id;
   BMC64MachineClass class;
   BMC64VideoStandard video_standard;   
   BMC64VideoOut video_out;
   char desc[DESC_LEN];
   struct machine_option *options;
   struct machine_entry *next;
};

int load_machines(struct machine_entry** head);
void free_machines(struct machine_entry* head);
int switch_apply_files(struct machine_entry* ptr);

// Apply a safe HDMI video mode
void switch_safe();
#endif
