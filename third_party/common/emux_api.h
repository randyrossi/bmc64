/*
 * emu_api.h - emulator specific API functions
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

// API between common code and emulator specific impl

#ifndef RASPI_EMUX_API_H_
#define RASPI_EMUX_API_H_

#include <stdint.h>

#include "ui.h"

typedef enum {
  BMC64_MACHINE_CLASS_UNKNOWN,
  BMC64_MACHINE_CLASS_VIC20,
  BMC64_MACHINE_CLASS_C64,
  BMC64_MACHINE_CLASS_C128,
  BMC64_MACHINE_CLASS_PLUS4,
  BMC64_MACHINE_CLASS_PLUS4EMU,
} BMC64MachineClass;

typedef enum {
  EMUX_TAPE_STOP,
  EMUX_TAPE_PLAY,
  EMUX_TAPE_RECORD,
  EMUX_TAPE_REWIND,
  EMUX_TAPE_FASTFORWARD,
  EMUX_TAPE_RESET,
  EMUX_TAPE_ZERO,
} EmuxTapeCmd;

// For file type dialogs. Determines what dir we start in. Used
// as index into default_dir_names and current_dir_names in menu.c
#define NUM_DIR_TYPES 7
typedef enum {
   DIR_ROOT,
   DIR_DISKS,
   DIR_TAPES,
   DIR_CARTS,
   DIR_SNAPS,
   DIR_ROMS,
   DIR_IEC,
} DirType;

typedef char*(*fullpath_func)(DirType dir_type, char *name);

extern int emux_machine_class;

// Pause emulator main loop and run our ui loop. 
void emux_trap_main_loop_ui(void);

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data);

// Press/release key by row/col in keyboard matrix.
void emux_kbd_set_latch_keyarr(int row, int col, int value);

// Attach a disk image to a drive
// Return negative on error.
int emux_attach_disk_image(int unit, char *filename);

// Detach a disk drive image or mounted file system.
void emux_detach_disk(int unit);

// Attach a tape image
// Return negative on error.
int emux_attach_tape_image(char *filename);

// Detach a tape image.
void emux_detach_tape(void);

// Attach a cart image
// Return negative on error.
int emux_attach_cart(int bank, char *filename);

// Make current attached cartridge the default
void emux_set_cart_default(void);

// Detach a cart image
void emux_detach_cart(int bank);

// Reset machine
void emux_reset(int isSoft);

// Save state of machine
int emux_save_state(char *filename);

// Restore state of machine
int emux_load_state(char *filename);

// Change tape drive state
int emux_tape_control(int cmd);

// Enable the cartridge OSD popup
void emux_show_cart_osd_menu(void);

// Caluculate cycles per second timing for this machine
unsigned long emux_calculate_timing(double fps);

// Peripheral display status updates
void emux_enable_drive_status(int state, int *drive_led_color);
void emux_display_drive_led(int drive, unsigned int pwm1, unsigned int pwm2);
void emux_display_tape_counter(int counter);
void emux_display_tape_control_status(int control);
void emux_display_tape_motor_status(int motor);

// Autostart a file
int emux_autostart_file(char* filename);

// Show change model menu
void emux_drive_change_model(int unit);

// Add the parallel cable option to the menu container (if applicable)
void emux_add_parallel_cable_option(struct menu_item* parent,
                                    int id, int drive);

// Create an empty disk image
void emux_create_disk(struct menu_item* item, fullpath_func fullpath);

// Set the joy port device to JOYDEV_*
void emux_set_joy_port_device(int port_num, int dev_id);

void emux_set_joy_pot_x(int value);
void emux_set_joy_pot_y(int value);

void emux_add_sound_options(struct menu_item* parent);
void emux_load_sound_options(void);

// VICE specific cart attach func.
void emux_vice_attach_cart(int menu_id, char* filename);
void emux_vice_easy_flash(void);

#endif
