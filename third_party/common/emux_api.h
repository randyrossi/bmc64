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

#include <stdio.h>
#include <stdint.h>

#include "ui.h"

extern const uint8_t ascii_to_petscii[256];

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

typedef enum {
  Setting_C128ColumnKey,
  Setting_Datasette,
  Setting_DatasetteResetWithCPU,
  Setting_DriveNParallelCable,
  Setting_DriveNType,
  Setting_DriveSoundEmulation,
  Setting_DriveSoundEmulationVolume,
  Setting_FileSystemDeviceN,
  Setting_IECDeviceN,
  Setting_Mouse,
  Setting_RAMBlock0, // Vic20
  Setting_RAMBlock1, // Vic20
  Setting_RAMBlock2, // Vic20
  Setting_RAMBlock3, // Vic20
  Setting_RAMBlock5, // Vic20
  Setting_SidEngine, // Vice
  Setting_SidFilters, // Vice
  Setting_SidModel,
  Setting_SidResidSampling, // Vice
  Setting_SidWriteAccess, // Plus4Emu
  Setting_SidDigiblaster, // Plus4Emu
  Setting_WarpMode,
} IntSetting;

typedef enum {
  Setting_FSDeviceNDir,
} StringSetting;

// Types of queued joystick events for calls into emulator API
#define PENDING_EMU_JOY_TYPE_ABSOLUTE 0
#define PENDING_EMU_JOY_TYPE_AND 1
#define PENDING_EMU_JOY_TYPE_OR 2

struct pending_emu_key_s {
  int head;
  int tail;
  long key[16];
  int pressed[16];
};

struct pending_emu_joy_s {
  int head;
  int tail;
  int value[128];
  int port[128];
  int type[128];
  int device[128];
};

extern struct pending_emu_key_s pending_emu_key;
extern struct pending_emu_joy_s pending_emu_joy;

typedef char*(*fullpath_func)(DirType dir_type, char *name);

struct CanvasState {
  //struct video_canvas_s *canvas;
  int palette_index;
  // Just the gfx area (no border)
  int gfx_w;
  int gfx_h;
  // How much border is available
  int max_border_w;
  int max_border_h;
  // How much of the border we want to see
  int border_w;
  int border_h;

  int src_off_x;
  int src_off_y;
  // The total visiible pixels in each dimension
  int vis_w;
  int vis_h;
  // For our src region
  int top;
  int left;

  // Where does the status overlay show up?
  int overlay_x;
  int overlay_y;

  int extra_offscreen_border_left;
  int first_displayed_line;
};

// One struct for each display (can be 2 for C128)
extern struct CanvasState canvas_state[2];

struct vkbd_key {
  int x;
  int y;
  int w;
  int h;
  int row;
  int col;
  int layout_row;
  int layout_col;
  int toggle;
  int code;
  int shift_code;
  int comm_code;
  int up;
  int down;
  int left;
  int right;
  int state;
};

// special code values for keys
#define VKBD_KEY_HOME -2
#define VKBD_DEL -3
#define VKBD_F1 -4
#define VKBD_F3 -6
#define VKBD_F5 -8
#define VKBD_F7 -10
#define VKBD_CNTRL -12
#define VKBD_RESTORE -13
#define VKBD_RUNSTOP -14
#define VKBD_SHIFTLOCK -15
#define VKBD_RETURN -16
#define VKBD_COMMODORE -17
#define VKBD_LSHIFT -18
#define VKBD_RSHIFT -19
#define VKBD_CURSDOWN -20
#define VKBD_CURSRIGHT -21
#define VKBD_SPACE -22
#define VKBD_CLR -23
#define VKBD_INS -24
#define VKBD_ESC -25
#define VKBD_CURSUP -26
#define VKBD_CURSLEFT -27

typedef struct vkbd_key* vkbd_key_array;

// NOTE: For Plus4 vic == ted
extern int emux_machine_class;
extern int vic_showing;
extern int vdc_showing;
extern int vic_enabled;
extern int vdc_enabled;
extern int vdc_canvas_index;
extern int vic_canvas_index;

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

void emux_video_color_setting_changed(int display_num);

void emux_set_color_brightness(int display_num, int value);
void emux_set_color_contrast(int display_num, int value);
void emux_set_color_gamma(int display_num, int value);
void emux_set_color_tint(int display_num, int value);

int emux_get_color_brightness(int display_num);
int emux_get_color_contrast(int display_num);
int emux_get_color_gamma(int display_num);
int emux_get_color_tint(int display_num);

void emux_set_video_cache(int value);
void emux_set_hw_scale(int value);

void emux_cartridge_trigger_freeze(void);
struct menu_item* emux_add_palette_options(int menu_id,
                                           struct menu_item* parent);
void emux_add_machine_options(struct menu_item* parent);
struct menu_item* emux_add_cartridge_options(struct menu_item* parent);

void emux_set_warp(int warp);

void emux_apply_video_adjustments(int layer, int hcenter, int vcenter,
                                  double hborder, double vborder,
                                  double aspect, double lpad, double rpad,
                                  double tpad, double bpad, int zlayer);

// Select a palette index for the given display number.
// (For indexed displays only)
void emux_change_palette(int display_num, int palette_index);

// Set joystick latch value for port and device.
// Safe to call from ISR
// type is PENDING_EMU_JOY_*
// device is JOYDEV_*
void emux_joy_interrupt(int type, int port, int device, int value);

// Set key latch value for a keycode
// Safe to call from ISR
void emux_key_interrupt(long key, int pressed);
void emux_key_interrupt_locked(long key, int pressed);

vkbd_key_array emux_get_vkbd(void);
int emux_get_vkbd_width(void);
int emux_get_vkbd_height(void);
int emux_get_vkbd_size(void);

// Handles one of the MENU_*_FILE rom change events from the menu
void emux_handle_rom_change(struct menu_item* item, fullpath_func fullpath);

// Sets the directory for IEC emulation.
void emux_set_iec_dir(int unit, char* dir);

// Set a setting integer value.
void emux_set_int(IntSetting setting, int value);
void emux_set_int_1(IntSetting setting, int value, int param);
void emux_get_int(IntSetting setting, int* dest);
void emux_get_int_1(IntSetting setting, int* dest, int param);
void emux_get_string_1(StringSetting setting, const char** dest, int param);

// Save settings
int emux_save_settings(void);

void emux_ensure_video(void);

// VICE specific cart attach func.
void emux_vice_attach_cart(int menu_id, char* filename);
void emux_vice_easy_flash(void);

void emu_pause_trap(uint16_t addr, void *data);

// Return 1 to indicate item was handled, 0 otherwise
int emux_handle_menu_change(struct menu_item* item);

void emux_handle_load_setting(char *name, int value, char* value_str);
void emux_handle_save_settings(FILE *fp);

#endif
