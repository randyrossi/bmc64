/*
 * menu.c
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

#include "menu.h"

#include <math.h>
#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// RASPI Includes
#include "emux_api.h"
#include "demo.h"
#include "joy.h"
#include "kbd.h"
#include "text.h"
#include "menu_confirm_osd.h"
#include "menu_reset_osd.h"
#include "menu_tape_osd.h"
#include "menu_timing.h"
#include "menu_usb.h"
#include "menu_keyset.h"
#include "menu_switch.h"
#include "menu_gpio.h"
#include "overlay.h"
#include "raspi_util.h"
#include "ui.h"

extern void reboot(void);

#define VERSION_STRING "4.2"

#ifdef RASPI_LITE
#define VARIANT_STRING "-Lite"
#else
#define VARIANT_STRING ""
#endif

#define DEFAULT_VICII_H_STRETCH 1200
#define DEFAULT_VICII_V_STRETCH 1000

#define DEFAULT_VIC_H_STRETCH 1450
#define DEFAULT_VIC_V_STRETCH 1000

#define DEFAULT_VDC_H_STRETCH 1450
#define DEFAULT_VDC_V_STRETCH 1000

#define SWITCH_MSG "NOTE: For machines other than C64, " \
                   "the SDCard will only boot on the same Pi model the " \
                   "switch occurred from. To boot on a different Pi model, " \
                   "switch back to C64 first."

#define SWITCH_FAIL_MSG "Something went wrong. File a bug with the error " \
                        "code above. You may have to manually edit " \
                        "config.txt and/or cmdline.txt to restore boot."

// For filename filters
typedef enum {
   FILTER_NONE,
   FILTER_DISK,
   FILTER_CART,
   FILTER_TAPE,
   FILTER_SNAP,
   FILTER_DIRS,
   FILTER_PRGS,
} FileFilter;

// These can be saved
struct menu_item *port_1_menu_item;
struct menu_item *port_2_menu_item;
struct menu_item *port_3_menu_item;
struct menu_item *port_4_menu_item;
int usb_pref[MAX_USB_DEVICES];
int usb_x_axis[MAX_USB_DEVICES];
int usb_y_axis[MAX_USB_DEVICES];
float usb_x_thresh[MAX_USB_DEVICES];
float usb_y_thresh[MAX_USB_DEVICES];
int usb_button_assignments[MAX_USB_DEVICES][MAX_USB_BUTTONS];
int usb_button_bits[MAX_USB_BUTTONS]; // never change
long keyset_codes[2][7];
long key_bindings[6];

char attached_disk_name[4][MAX_STR_VAL_LEN];

// Lower byte is BTN_ASSIGN_ constant. Upper byte is port or other arg.
unsigned int gpio_bindings[NUM_GPIO_PINS];

struct menu_item *drive_sounds_item;
struct menu_item *drive_sounds_vol_item;
struct menu_item *hotkey_cf1_item;
struct menu_item *hotkey_cf3_item;
struct menu_item *hotkey_cf5_item;
struct menu_item *hotkey_cf7_item;
struct menu_item *hotkey_tf1_item;
struct menu_item *hotkey_tf3_item;
struct menu_item *hotkey_tf5_item;
struct menu_item *hotkey_tf7_item;
struct menu_item *volume_item;
struct menu_item *statusbar_item;
struct menu_item *statusbar_padding_item;
struct menu_item *tape_reset_with_machine_item;
struct menu_item *vkbd_transparency_item;

struct menu_item *palette_item[2];
struct menu_item *brightness_item[2];
struct menu_item *contrast_item[2];
struct menu_item *gamma_item[2];
struct menu_item *tint_item[2];
struct menu_item *saturation_item[2];

struct menu_item *warp_item;
struct menu_item *reset_confirm_item;
struct menu_item *gpio_config_item;
struct menu_item *active_display_item;

struct menu_item *use_scaling_params_item[2];

struct menu_item *h_center_item[2];
struct menu_item *v_center_item[2];
struct menu_item *h_border_item[2];
struct menu_item *v_border_item[2];
struct menu_item *h_stretch_item[2];
struct menu_item *v_stretch_item[2];
int h_integer_stretch[2];
int v_integer_stretch[2];
int use_h_integer_stretch[2];
int use_v_integer_stretch[2];

struct menu_item *pip_location_item;
struct menu_item *pip_swapped_item;

struct menu_item *c40_80_column_item;
struct menu_item *dir_convention_item;

struct menu_item *scaling_interp_item;

struct menu_item* s_enable_shader_item;
struct menu_item* s_curvature_item;
struct menu_item* s_curvature_x_item;
struct menu_item* s_curvature_y_item;
struct menu_item* s_mask_item;
struct menu_item* s_mask_brightness_item;
struct menu_item* s_gamma_item;
struct menu_item* s_scanlines_item;
struct menu_item* s_multisample_item;
struct menu_item* s_scanline_weight_item;
struct menu_item* s_scanline_gap_brightness_item;
struct menu_item* s_bloom_factor_item;
struct menu_item* s_input_gamma_item;
struct menu_item* s_output_gamma_item;
struct menu_item* s_sharper_item;

static int unit;
static int joyswap;
static int statusbar_forced;

// Held here, exported for menu_usb to read
int pot_x_high_value;
int pot_x_low_value;
int pot_y_high_value;
int pot_y_low_value;

// Property names for load/save files
static char usb_btn_name[MAX_USB_DEVICES][16];
static char usb_pref_name[MAX_USB_DEVICES][16];
static char usb_x_name[MAX_USB_DEVICES][16];
static char usb_y_name[MAX_USB_DEVICES][16];
static char usb_x_t_name[MAX_USB_DEVICES][16];
static char usb_y_t_name[MAX_USB_DEVICES][16];

const int num_disk_ext = 15;
static char disk_filt_ext[15][5] = {".d64", ".d67", ".d71", ".d80", ".d81",
                                    ".d82", ".d1m", ".d2m", ".d4m", ".g64",
                                    ".g71", ".g41", ".p64", ".x64", ".dhd"};

const int num_tape_ext = 2;
static char tape_filt_ext[2][5] = {".t64", ".tap"};

const int num_cart_ext = 2;
static char cart_filt_ext[2][5] = {".crt", ".bin"};

const int num_snap_ext = 1;
char snap_filt_ext[1][5];

const int num_prg_ext = 1;
const char prg_filt_ext[1][5] = {".prg"};

#define TEST_FILTER_MACRO(funcname, numvar, filtarray)                         \
  static int funcname(char *name) {                                            \
    int include = 0;                                                           \
    int len = strlen(name);                                                    \
    int i;                                                                     \
    for (i = 0; i < numvar; i++) {                                             \
      if (len > 4 && !strcasecmp(name + len - 4, filtarray[i])) {              \
        include = 1;                                                           \
        break;                                                                 \
      }                                                                        \
    }                                                                          \
    return include;                                                            \
  }

// What directories to initialize file search dialogs with for
// each type of file.
// TODO: Make these start dirs configurable.
static const char default_volume_name[8] = "SD:";
static const char default_dir_names[NUM_DIR_TYPES][16] = {
    "/", "/disks", "/tapes", "/carts", "/snapshots", "/roms", "/"};

// Keep track of the current volume
static char current_volume_name[8] = "";
// Keep track of current directory for each type of file.
static char current_dir_names[NUM_DIR_TYPES][256];
// Set to the sub dir name for this type.
static char machine_sub_dir[16];
// Keep track of last iec dirs for each drive
static char last_iec_dir[4][256];

static int usb1_mounted;
static int usb2_mounted;
static int usb3_mounted;

// Temp storage for full path name concatenations.
static char full_path_str[256];

// Keep track of last known position in the file list.
static int current_dir_pos[NUM_DIR_TYPES];

TEST_FILTER_MACRO(test_disk_name, num_disk_ext, disk_filt_ext);
TEST_FILTER_MACRO(test_tape_name, num_tape_ext, tape_filt_ext);
TEST_FILTER_MACRO(test_cart_name, num_cart_ext, cart_filt_ext);
TEST_FILTER_MACRO(test_snap_name, num_snap_ext, snap_filt_ext);
TEST_FILTER_MACRO(test_prg_name, num_prg_ext, prg_filt_ext);

static void rtrim(char *txt) {
  if (!txt) return;
  int p=strlen(txt)-1;
  while (isspace(txt[p])) { txt[p] = '\0'; p--; }
}

static char* ltrim(char *txt) {
  if (!txt) return NULL;
  int p=0;
  while (isspace(txt[p])) { p++; }
  return txt+p;
}

static void get_key_and_value(char *line, char **key, char **value) {
   for (int i=0;i<strlen(line);i++) {
      if (line[i] == '=') {
         line[i] = '\0';
         *key = ltrim(&line[0]);
         rtrim(*key);
         *value = ltrim(&line[i+1]);
         rtrim(*value);
         return;
      }
   }
   *key = 0;
   *value = 0;
}

static char *fullpath(DirType dir_type, char *name) {
  strcpy(full_path_str, current_volume_name);
  strcat(full_path_str, current_dir_names[dir_type]);
  // Put a trailing slash unless we are at the root
  if (current_dir_names[dir_type][strlen(
      current_dir_names[dir_type])-1] != '/'){
    strcat(full_path_str, "/");
  }
  strcat(full_path_str, name);
  return full_path_str;
}

// Remove one directory from the end of path
static void remove_dir(char *path) {
  int i;
  // Remove last directory from current_dir_names
  i = strlen(path) - 1;
  while (path[i] != '/' && i > 0)
    i--;
  path[i] = '\0';
  if (strlen(path) == 0) {
    strcpy(path, "/");
  }
}

// Clears the file menu and populates it with files.
static void list_files(struct menu_item *parent,
                       DirType dir_type, FileFilter filter,
                       int menu_id) {
  DIR *dp;
  struct dirent *ep;
  int i;
  int include;

  dp = opendir(fullpath(dir_type,""));
  if (dp == NULL) {
    // Machine dir may not be present. Try up one.
    remove_dir(current_dir_names[dir_type]);
    dp = opendir(fullpath(dir_type,""));
    if (dp == NULL) {
      // File dir may not be present. Try up one.
      remove_dir(current_dir_names[dir_type]);
      dp = opendir(fullpath(dir_type,""));
      if (dp == NULL) {
        return;
      }
    }
  }

  // Current directory item, also action to change disk drive
  struct menu_item* cur_dir = ui_menu_add_button(
     menu_id, parent, fullpath(dir_type,""));
  cur_dir->sub_id = MENU_SUB_SELECT_VOLUME;
  cur_dir->symbol = 31;  // left arrow
  ui_menu_add_divider(parent);

  // When we are picking dirs, include a button to select the current dir.
  if (filter == FILTER_DIRS) {
    struct menu_item *new_button =
         ui_menu_add_button(menu_id, parent, "(Use this dir)");
    new_button->sub_id = MENU_SUB_PICK_DIR;
    ui_menu_add_divider(parent);
  }

  // Put together a string that represents the root of this volume
  char current_root[16];
  strcpy (current_root, current_volume_name);
  strcat (current_root, "/");

  if (strcmp(fullpath(dir_type,""), current_root) != 0) {
    ui_menu_add_button(menu_id, parent, "..")->sub_id = MENU_SUB_UP_DIR;
  }

  // Make two buckets
  struct menu_item dirs_root;
  memset(&dirs_root, 0, sizeof(struct menu_item));
  dirs_root.type = FOLDER;
  dirs_root.is_expanded = 1;
  dirs_root.name[0] = '\0';

  struct menu_item files_root;
  memset(&files_root, 0, sizeof(struct menu_item));
  files_root.type = FOLDER;
  files_root.is_expanded = 1;
  files_root.name[0] = '\0';

  if (dp != NULL) {
    while (ep = readdir(dp)) {
      if (ep->d_type & DT_DIR) {
        ui_menu_add_button_with_value(menu_id, &dirs_root, ep->d_name, 0,
                                      ep->d_name, "(dir)")
            ->sub_id = MENU_SUB_ENTER_DIR;
      } else {
        include = 0;
        if (filter == FILTER_DISK) {
          include = test_disk_name(ep->d_name);
        } else if (filter == FILTER_TAPE) {
          include = test_tape_name(ep->d_name);
        } else if (filter == FILTER_CART) {
          include = test_cart_name(ep->d_name);
        } else if (filter == FILTER_SNAP) {
          include = test_snap_name(ep->d_name);
        } else if (filter == FILTER_PRGS) {
          include = test_prg_name(ep->d_name);
        } else if (filter == FILTER_DIRS) {
          include = 0;
        } else if (filter == FILTER_NONE) {
          include = 1;
        }
        if (include) {
          // Button name will be filename but it will be truncated
          // due to menu width.  Actual filename will be stored in
          // str_value which is never displayed except for text fields.
          struct menu_item *new_button =
              ui_menu_add_button(menu_id, &files_root, ep->d_name);
          new_button->sub_id = MENU_SUB_PICK_FILE;
          strncpy(new_button->str_value, ep->d_name, MAX_STR_VAL_LEN - 1);
        }
      }
    }

    (void)closedir(dp);
  }

  struct menu_item *dfc = dirs_root.first_child;
  merge_sort(&dfc);
  dirs_root.first_child = dfc;

  struct menu_item *ffc = files_root.first_child;
  merge_sort(&ffc);
  files_root.first_child = ffc;

  // Transfer ownership of dirs children first, then files. Childless
  // parents are on the stack.
  ui_add_all(&dirs_root, parent);
  ui_add_all(&files_root, parent);

  assert(dirs_root.first_child == NULL);
  assert(files_root.first_child == NULL);
}

static void files_cursor_listener(struct menu_item* parent,
                                  int new_pos) {
  // dir type is in value field
  current_dir_pos[parent->value] = new_pos;
}

static void show_files(DirType dir_type, FileFilter filter, int menu_id,
                       int reset_cur_pos) {
  // Show files
  struct menu_item *file_root = ui_push_menu(-1, -1);

  // Keep the type of files this list is for in value field.
  file_root->value = dir_type;

  file_root->cursor_listener_func = files_cursor_listener;

  if (menu_id == MENU_SAVE_SNAP_FILE ||
      (menu_id >= MENU_CREATE_D64_FILE && menu_id <= MENU_CREATE_TAP_FILE)) {
    struct menu_item *file_name_item = ui_menu_add_text_field(
       menu_id, file_root, "Enter name:", "");
    file_name_item->sub_id = MENU_SUB_PICK_FILE;
  }
  list_files(file_root, dir_type, filter, menu_id);

  if (reset_cur_pos) {
     current_dir_pos[dir_type] = 0;
  } else {
     // Position cursor to last known location for this dir type.
     ui_set_cur_pos(current_dir_pos[dir_type]);
  }
}


static void show_about() {
  struct menu_item *about_root = ui_push_menu(32, 8);
  char title[16];
  char desc[32];

  switch (emux_machine_class) {
  case BMC64_MACHINE_CLASS_C64:
    snprintf (title, 15, "%s%s %s", "BMC64", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal C64 Emulator", 31);
    break;
  case BMC64_MACHINE_CLASS_C128:
    snprintf (title, 15, "%s%s %s", "BMC128", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal C128 Emulator", 31);
    break;
  case BMC64_MACHINE_CLASS_VIC20:
    snprintf (title, 15, "%s%s %s", "BMVIC20", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal VIC20 Emulator", 31);
    break;
  case BMC64_MACHINE_CLASS_PLUS4:
  case BMC64_MACHINE_CLASS_PLUS4EMU:
    snprintf (title, 15, "%s%s %s", "BMPLUS4", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal PLUS/4 Emulator", 31);
    break;
  case BMC64_MACHINE_CLASS_PET:
    snprintf (title, 15, "%s%s %s", "BMPET", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal PET Emulator", 31);
    break;
  default:
    strncpy (title, "ERROR", 15);
    strncpy (desc, "Unknown Emulator", 31);
    break;
  }

  ui_menu_add_button(MENU_TEXT, about_root, title);
  ui_menu_add_button(MENU_TEXT, about_root, desc);

#ifdef RASPI_LITE
  ui_menu_add_button(MENU_TEXT, about_root, "For the Rasbperry Pi Zero");
#else
  ui_menu_add_button(MENU_TEXT, about_root, "For the Rasbperry Pi 2/3");
#endif

  ui_menu_add_divider(about_root);
  ui_menu_add_button(MENU_TEXT, about_root, "https://github.com/");
  ui_menu_add_button(MENU_TEXT, about_root, "         randyrossi/bmc64");
}

static void show_license() {
  int i;
  struct menu_item *license_root = ui_push_menu(-1, -1);
  for (i = 0; i < 510; i++) {
    ui_menu_add_button(MENU_TEXT, license_root, license[i]);
  }
}

static void configure_usb(int dev) {
  struct menu_item *usb_root = ui_push_menu(-1, -1);
  build_usb_menu(dev, usb_root);
}

static void configure_keyset(int num) {
  struct menu_item *keyset_root = ui_push_menu(-1, -1);
  build_keyset_menu(num, keyset_root);
}

static void configure_timing() {
  struct menu_item *timing_root = ui_push_menu(-1, -1);
  build_timing_menu(timing_root);
}

static void configure_gpio() {
  struct menu_item *gpio_root = ui_push_menu(-1, -1);
  build_gpio_menu(gpio_root);
}

// Show a pop up menu with the available drive volumes.
// The item's id will be passed along to every item created
// here. The action to perform is dicatated by sub_id.
static void filesystem_change_volume(struct menu_item *item) {
  struct menu_item *vol_root = ui_push_menu(12, 8);
  struct menu_item *item2;

  // SD card is always available
  item2 = ui_menu_add_button(item->id, vol_root, "SD");
  item2->sub_id = MENU_SUB_CHANGE_VOLUME;
  item2->value = MENU_VOLUME_SD;

  int available[3];
  circle_find_usb(&available);

  if (available[0]) {
    item2 = ui_menu_add_button(item->id, vol_root, "USB1");
    item2->sub_id = MENU_SUB_CHANGE_VOLUME;
    item2->value = MENU_VOLUME_USB1;
  }
  if (available[1]) {
    item2 = ui_menu_add_button(item->id, vol_root, "USB2");
    item2->sub_id = MENU_SUB_CHANGE_VOLUME;
    item2->value = MENU_VOLUME_USB2;
  }
  if (available[2]) {
    item2 = ui_menu_add_button(item->id, vol_root, "USB3");
    item2->sub_id = MENU_SUB_CHANGE_VOLUME;
    item2->value = MENU_VOLUME_USB3;
  }
}

static void drive_change_rom() {
  struct menu_item *root = ui_push_menu(12, 8);
  struct menu_item *item;

  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_1541, root, "1541...");
  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_1541II, root, "1541II...");
  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_1551, root, "1551...");
  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_1571, root, "1571...");
  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_1581, root, "1581...");
  item = ui_menu_add_button(MENU_DRIVE_CHANGE_ROM_CMDHD, root, "CMDHD...");
}

static void ui_set_hotkeys() {
  kbd_set_hotkey_function(0, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(1, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(2, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(3, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(4, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(5, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(6, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(7, 0, BTN_ASSIGN_UNDEF);

  // Apply hotkey selections to keyboard handler
  if (hotkey_cf1_item->value > 0) {
    kbd_set_hotkey_function(
        0, KEYCODE_F1, hotkey_cf1_item->choice_ints[hotkey_cf1_item->value]);
  }
  if (hotkey_cf3_item->value > 0) {
    kbd_set_hotkey_function(
        1, KEYCODE_F3, hotkey_cf3_item->choice_ints[hotkey_cf3_item->value]);
  }
  if (hotkey_cf5_item->value > 0) {
    kbd_set_hotkey_function(
        2, KEYCODE_F5, hotkey_cf5_item->choice_ints[hotkey_cf5_item->value]);
  }
  if (hotkey_cf7_item->value > 0) {
    kbd_set_hotkey_function(
        3, KEYCODE_F7, hotkey_cf7_item->choice_ints[hotkey_cf7_item->value]);
  }
  if (hotkey_tf1_item->value > 0) {
    kbd_set_hotkey_function(
        4, KEYCODE_F1, hotkey_tf1_item->choice_ints[hotkey_tf1_item->value]);
  }
  if (hotkey_tf3_item->value > 0) {
    kbd_set_hotkey_function(
        5, KEYCODE_F3, hotkey_tf3_item->choice_ints[hotkey_tf3_item->value]);
  }
  if (hotkey_tf5_item->value > 0) {
    kbd_set_hotkey_function(
        6, KEYCODE_F5, hotkey_tf5_item->choice_ints[hotkey_tf5_item->value]);
  }
  if (hotkey_tf7_item->value > 0) {
    kbd_set_hotkey_function(
        7, KEYCODE_F7, hotkey_tf7_item->choice_ints[hotkey_tf7_item->value]);
  }
}

// If any joystick is set to mouse, enable it in the emulator.
// FCIII apparently doesn't like the mouse enabled unless necessary
static void set_need_mouse() {
   int need_mouse = 0;
   int index;
   // Only ports 1 and 2 can be assigned a mouse.
   if (port_1_menu_item) {
      index = port_1_menu_item->value;
      if (port_1_menu_item->choice_ints[index] == JOYDEV_MOUSE) {
         need_mouse = 1;
      }
   }
   if (port_2_menu_item) {
      index = port_2_menu_item->value;
      if (port_1_menu_item->choice_ints[index] == JOYDEV_MOUSE) {
         need_mouse = 1;
      }
   }
   emux_set_int(Setting_Mouse, need_mouse);
}

// Sets joydev port 'p' (1-4) to JOYDEV_* value 'value' and makes sure
// all other ports get the mouse turned off if this port got a mouse.
static void set_joy_item_to_value(int p, int value) {
    joydevs[p-1].device = value;
    if (value == JOYDEV_MOUSE) {
      // If any other port has mouse, set it to none.
      for (int l = 0; l < MAX_JOY_PORTS; l++) {
         if (l == (p-1)) continue;

         struct menu_item* other;
         switch (l) {
            case 0:
               other = port_1_menu_item; break;
            case 1:
               other = port_2_menu_item; break;
            case 2:
               other = port_3_menu_item; break;
            case 3:
               other = port_4_menu_item; break;
            default:
               assert(0);
         }
         if (other && other->choice_ints[other->value] == JOYDEV_MOUSE) {
           emux_set_joy_port_device(l+1, JOYDEV_NONE);
           other->value = 0;
         }
      }
    }
    emux_set_joy_port_device(p, value);
}

void ui_set_joy_items() {
  int joydev;
  int i;
  for (joydev = 0; joydev < MAX_JOY_PORTS; joydev++) {
    struct menu_item *dst;

    if (joydevs[joydev].port == 1) {
      dst = port_1_menu_item;
    } else if (joydevs[joydev].port == 2) {
      dst = port_2_menu_item;
    } else if (joydevs[joydev].port == 3) {
      dst = port_3_menu_item;
    } else if (joydevs[joydev].port == 4) {
      dst = port_4_menu_item;
    } else {
      continue;
    }

    if (!dst)
      continue;

    // Find which choice matches the device selected and
    // make sure the menu item matches
    for (i = 0; i < dst->num_choices; i++) {
      if (dst->choice_ints[i] == joydevs[joydev].device) {
        dst->value = i;
        break;
      }
    }
  }

  if (port_1_menu_item) {
     set_joy_item_to_value(1,
         port_1_menu_item->choice_ints[port_1_menu_item->value]);
  }
  if (port_2_menu_item) {
     set_joy_item_to_value(2,
         port_2_menu_item->choice_ints[port_2_menu_item->value]);
  }
  if (port_3_menu_item) {
     set_joy_item_to_value(3,
         port_3_menu_item->choice_ints[port_3_menu_item->value]);
  }
  if (port_4_menu_item) {
     set_joy_item_to_value(4,
         port_4_menu_item->choice_ints[port_4_menu_item->value]);
  }
  set_need_mouse();
}

static int do_use_int_scaling(int layer, int silent) {
  int canvas_index;
  if (layer == FB_LAYER_VIC) {
    canvas_index = VIC_INDEX;
  } else if (layer == FB_LAYER_VDC) {
    canvas_index = VDC_INDEX;
  } else {
    if (!silent)
       ui_error("Bad display num");
    return 0;
  }

  int fbw, fbh, sx, sy;
  int display_num = canvas_index;
  // For the PET, 1st display is 40 column models, 2nd is 80 column models
  if (emux_machine_class == BMC64_MACHINE_CLASS_PET) {
     int cols;
     emux_get_int(Setting_VideoSize, &cols);
     display_num = cols == 40 ? 0 : 1;
  }
  circle_get_scaling_params(display_num, &fbw, &fbh, &sx, &sy);

  int dpw, dph, tmp;
  circle_get_fbl_dimensions(layer,
                            &dpw, &dph,
                            &tmp, &tmp,
                            &tmp, &tmp,
                            &tmp, &tmp);


  if (fbw <= 0 || fbh <= 0 || sx <= 0 || sy <= 0) {
     if (!silent)
        ui_error("Bad or missing params");
     return 0;
  }

  if (fbw % 2 != 0) {
     if (!silent)
        ui_error("fbw must be even");
     return 0;
  }

  if (fbh % 2 != 0) {
     if (!silent)
        ui_error("fbh must be even");
     return 0;
  }

  if (sx > dpw) {
     if (!silent)
        ui_error("sx too large for display");
     return 0;
  }

  if (sy > dph) {
     if (!silent)
        ui_error("sy too large for display");
     return 0;
  }

  h_integer_stretch[canvas_index] = sx;
  v_integer_stretch[canvas_index] = sy;

  h_border_item[canvas_index]->value =
     (fbw - canvas_state[canvas_index].gfx_w) / 2;
  if (h_border_item[canvas_index]->value >
         h_border_item[canvas_index]->max) {
     if (!silent)
        ui_error("fbw too large");
     h_border_item[canvas_index]->value =
        h_border_item[canvas_index]->max;
     return 0;
  } else if (h_border_item[canvas_index]->value <
                h_border_item[canvas_index]->min) {
     if (!silent)
        ui_error("fbw too small");
     h_border_item[canvas_index]->value =
        h_border_item[canvas_index]->min;
     return 0;
  }

  v_border_item[canvas_index]->value =
     (fbh - canvas_state[canvas_index].gfx_h) / 2;
  if (v_border_item[canvas_index]->value >
     v_border_item[canvas_index]->max) {
     if (!silent)
        ui_error("fbh too large");
     v_border_item[canvas_index]->value =
        v_border_item[canvas_index]->max;
     return 0;
  } else if (v_border_item[canvas_index]->value <
                v_border_item[canvas_index]->min) {
     if (!silent)
        ui_error("fbh too small");
     v_border_item[canvas_index]->value =
        v_border_item[canvas_index]->min;
     return 0;
  }

  h_stretch_item[canvas_index]->value =
     ceil((double)h_integer_stretch[canvas_index] * 1000.0 / (double)dph);
  v_stretch_item[canvas_index]->value =
     ceil((double)v_integer_stretch[canvas_index] * 1000.0 / (double)dph);

  use_h_integer_stretch[canvas_index] = 1;
  use_v_integer_stretch[canvas_index] = 1;
  return 1;
}

static void next_integer_scaling(int layer,
                                 int canvas_index,
                                 int dimension) {
  int dpw, dph, fbw, fbh, sw, sh, dw, dh;
  circle_get_fbl_dimensions(layer,
                            &dpw, &dph,
                            &fbw, &fbh,
                            &sw, &sh,
                            &dw, &dh);

  int dim = dimension == 0 ? sw : sh;
  int scaled_dim = dimension == 0 ? dw : dh;
  int max = dimension == 0 ? dpw : dph;

  int scale = scaled_dim / dim;
  scale = scale + 1;

  scaled_dim = dim * scale;
  if (scaled_dim > max) {
     // Start back at 1.
     if (dimension == 0)
        scaled_dim = sw;
     else
        scaled_dim = sh;
  }

  // Now express the scale as a ratio of the display height for the menu
  // This won't be the actual value that determines the final dimension
  // due to rounding errors.  'scaled dim' is what will be sent to
  // fbl.
  int menu_stretch_value = ceil((double)scaled_dim * 1000.0 / (double)dph);

  if (dimension == 0) {
     h_stretch_item[canvas_index]->value = menu_stretch_value;
     h_integer_stretch[canvas_index] = scaled_dim;
     use_h_integer_stretch[canvas_index] = 1;
  } else {
     v_stretch_item[canvas_index]->value = menu_stretch_value;
     v_integer_stretch[canvas_index] = scaled_dim;
     use_v_integer_stretch[canvas_index] = 1;
  }
}

static int save_settings() {
  FILE *fp;
  switch (emux_machine_class) {
  case BMC64_MACHINE_CLASS_C64:
    fp = fopen("/settings.txt", "w");
    break;
  case BMC64_MACHINE_CLASS_C128:
    fp = fopen("/settings-c128.txt", "w");
    break;
  case BMC64_MACHINE_CLASS_VIC20:
    fp = fopen("/settings-vic20.txt", "w");
    break;
  case BMC64_MACHINE_CLASS_PLUS4:
    fp = fopen("/settings-plus4.txt", "w");
    break;
  case BMC64_MACHINE_CLASS_PLUS4EMU:
    fp = fopen("/settings-plus4emu.txt", "w");
    break;
  case BMC64_MACHINE_CLASS_PET:
    fp = fopen("/settings-pet.txt", "w");
    break;
  default:
    printf("ERROR: Unhandled machine\n");
    return 1;
  }

  int r = emux_save_settings();
  if (r < 0) {
    printf("resource_save failed with %d\n", r);
    return 1;
  }

  if (fp == NULL)
    return 1;

  if (port_1_menu_item) {
    fprintf(fp, "port_1=%d\n", port_1_menu_item->value);
  }
  if (port_2_menu_item) {
    fprintf(fp, "port_2=%d\n", port_2_menu_item->value);
  }
  if (port_3_menu_item) {
    fprintf(fp, "port_3=%d\n", port_3_menu_item->value);
  }
  if (port_4_menu_item) {
    fprintf(fp, "port_4=%d\n", port_4_menu_item->value);
  }

  for (int k = 0;k < MAX_USB_DEVICES; k++) {
    fprintf(fp, "usb_%d=%d\n", k, usb_pref[k]);
    fprintf(fp, "usb_x_%d=%d\n", k, usb_x_axis[k]);
    fprintf(fp, "usb_y_%d=%d\n", k, usb_y_axis[k]);
    fprintf(fp, "usb_x_t_%d=%d\n", k, (int)(usb_x_thresh[k] * 100.0f));
    fprintf(fp, "usb_y_t_%d=%d\n", k, (int)(usb_y_thresh[k] * 100.0f));
  }

  fprintf(fp, "palette=%d\n", palette_item[0]->value);
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
    fprintf(fp, "palette2=%d\n", palette_item[1]->value);
  }

  for (int k = 0; k < MAX_USB_DEVICES; k++) {
    for (int i = 0; i < MAX_USB_BUTTONS; i++) {
      fprintf(fp, "usb_btn_%d=%d\n", k, usb_button_assignments[k][i]);
    }
  }
  fprintf(fp, "hotkey_cf1=%d\n", hotkey_cf1_item->value);
  fprintf(fp, "hotkey_cf3=%d\n", hotkey_cf3_item->value);
  fprintf(fp, "hotkey_cf5=%d\n", hotkey_cf5_item->value);
  fprintf(fp, "hotkey_cf7=%d\n", hotkey_cf7_item->value);
  fprintf(fp, "hotkey_tf1=%d\n", hotkey_tf1_item->value);
  fprintf(fp, "hotkey_tf3=%d\n", hotkey_tf3_item->value);
  fprintf(fp, "hotkey_tf5=%d\n", hotkey_tf5_item->value);
  fprintf(fp, "hotkey_tf7=%d\n", hotkey_tf7_item->value);
  // Can't change the 'overlay_*' names, legacy.
  fprintf(fp, "overlay=%d\n", statusbar_item->value);
  fprintf(fp, "overlay_padding=%d\n", statusbar_padding_item->value);
  fprintf(fp, "vkbd_trans=%d\n", vkbd_transparency_item->value);
  fprintf(fp, "tapereset=%d\n", tape_reset_with_machine_item->value);
  fprintf(fp, "reset_confirm=%d\n", reset_confirm_item->value);
  fprintf(fp, "scaling_interp=%d\n", scaling_interp_item->value);
  fprintf(fp, "gpio_config=%d\n", gpio_config_item->choice_ints[gpio_config_item->value]);
  fprintf(fp, "h_center_0=%d\n", h_center_item[0]->value);
  fprintf(fp, "v_center_0=%d\n", v_center_item[0]->value);
  fprintf(fp, "h_border_0=%d\n", h_border_item[0]->value);
  fprintf(fp, "v_border_0=%d\n", v_border_item[0]->value);
  fprintf(fp, "h_stretch_0=%d\n", h_stretch_item[0]->value);
  fprintf(fp, "v_stretch_0=%d\n", v_stretch_item[0]->value);
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     fprintf(fp, "h_center_1=%d\n", h_center_item[1]->value);
     fprintf(fp, "v_center_1=%d\n", v_center_item[1]->value);
     fprintf(fp, "h_border_1=%d\n", h_border_item[1]->value);
     fprintf(fp, "v_border_1=%d\n", v_border_item[1]->value);
     fprintf(fp, "h_stretch_1=%d\n", h_stretch_item[1]->value);
     fprintf(fp, "v_stretch_1=%d\n", v_stretch_item[1]->value);
  }

  int drive_type;

  emux_get_int_1(Setting_DriveNType, &drive_type, 8);
  fprintf(fp, "drive_type_8=%d\n", drive_type);
  emux_get_int_1(Setting_DriveNType, &drive_type, 9);
  fprintf(fp, "drive_type_9=%d\n", drive_type);
  emux_get_int_1(Setting_DriveNType, &drive_type, 10);
  fprintf(fp, "drive_type_10=%d\n", drive_type);
  emux_get_int_1(Setting_DriveNType, &drive_type, 11);
  fprintf(fp, "drive_type_11=%d\n", drive_type);

  fprintf(fp, "pot_x_high=%d\n", pot_x_high_value);
  fprintf(fp, "pot_x_low=%d\n", pot_x_low_value);
  fprintf(fp, "pot_y_high=%d\n", pot_y_high_value);
  fprintf(fp, "pot_y_low=%d\n", pot_y_low_value);

  fprintf(fp, "keyset_1_up=%d\n", keyset_codes[0][KEYSET_UP]);
  fprintf(fp, "keyset_1_down=%d\n", keyset_codes[0][KEYSET_DOWN]);
  fprintf(fp, "keyset_1_left=%d\n", keyset_codes[0][KEYSET_LEFT]);
  fprintf(fp, "keyset_1_right=%d\n", keyset_codes[0][KEYSET_RIGHT]);
  fprintf(fp, "keyset_1_fire=%d\n", keyset_codes[0][KEYSET_FIRE]);
  fprintf(fp, "keyset_1_potx=%d\n", keyset_codes[0][KEYSET_POTX]);
  fprintf(fp, "keyset_1_poty=%d\n", keyset_codes[0][KEYSET_POTY]);

  fprintf(fp, "keyset_2_up=%d\n", keyset_codes[1][KEYSET_UP]);
  fprintf(fp, "keyset_2_down=%d\n", keyset_codes[1][KEYSET_DOWN]);
  fprintf(fp, "keyset_2_left=%d\n", keyset_codes[1][KEYSET_LEFT]);
  fprintf(fp, "keyset_2_right=%d\n", keyset_codes[1][KEYSET_RIGHT]);
  fprintf(fp, "keyset_2_fire=%d\n", keyset_codes[1][KEYSET_FIRE]);
  fprintf(fp, "keyset_2_potx=%d\n", keyset_codes[1][KEYSET_POTX]);
  fprintf(fp, "keyset_2_poty=%d\n", keyset_codes[1][KEYSET_POTY]);

  fprintf(fp, "key_binding_1=%d\n", key_bindings[0]);
  fprintf(fp, "key_binding_2=%d\n", key_bindings[1]);
  fprintf(fp, "key_binding_3=%d\n", key_bindings[2]);
  fprintf(fp, "key_binding_4=%d\n", key_bindings[3]);
  fprintf(fp, "key_binding_5=%d\n", key_bindings[4]);
  fprintf(fp, "key_binding_6=%d\n", key_bindings[5]);

  fprintf(fp, "volume=%d\n", volume_item->value);
  fprintf(fp, "dir_convention=%d\n", dir_convention_item->value);
  fprintf(fp, "use_int_scaling_0=%d\n", use_scaling_params_item[0]->value);
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     fprintf(fp, "use_int_scaling_1=%d\n", use_scaling_params_item[1]->value);
  }

  for (int i = 0 ; i < NUM_GPIO_PINS; i++) {
     fprintf (fp, "custom_gpio=%d,%d\n", i, gpio_bindings[i]);
  }

  fprintf(fp,"s_curvature=%d\n", s_curvature_item->value);
  fprintf(fp,"s_curvature_x=%d\n", s_curvature_x_item->value);
  fprintf(fp,"s_curvature_y=%d\n", s_curvature_y_item->value);
  fprintf(fp,"s_sharper=%d\n", s_sharper_item->value);
  fprintf(fp,"s_mask=%d\n", s_mask_item->value);
  fprintf(fp,"s_mask_brightness=%d\n", s_mask_brightness_item->value);
  fprintf(fp,"s_scanlines=%d\n", s_scanlines_item->value);
  fprintf(fp,"s_multisample=%d\n", s_multisample_item->value);
  fprintf(fp,"s_scanline_weight=%d\n", s_scanline_weight_item->value);
  fprintf(fp,"s_scanline_gap_brightness=%d\n", s_scanline_gap_brightness_item->value);
  fprintf(fp,"s_bloom_factor=%d\n", s_bloom_factor_item->value);
  fprintf(fp,"s_gamma=%d\n", s_gamma_item->value);
  fprintf(fp,"s_input_gamma=%d\n", s_input_gamma_item->value);
  fprintf(fp,"s_output_gamma=%d\n", s_output_gamma_item->value);

  emux_save_additional_settings(fp);

  fclose(fp);

  return 0;
}

// Make joydev reflect menu choice
static void ui_set_joy_devs() {
  if (port_1_menu_item) {
    joydevs[0].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
  }

  if (port_2_menu_item) {
    joydevs[1].device = port_2_menu_item->choice_ints[port_2_menu_item->value];
  }

  if (port_3_menu_item) {
    joydevs[2].device = port_3_menu_item->choice_ints[port_3_menu_item->value];
  }

  if (port_4_menu_item) {
    joydevs[3].device = port_4_menu_item->choice_ints[port_4_menu_item->value];
  }
}

static void load_settings() {

  int tmp_value;

  emux_get_int(Setting_DriveSoundEmulation, &drive_sounds_item->value);
  emux_get_int(Setting_DriveSoundEmulationVolume, &drive_sounds_vol_item->value);

  brightness_item[0]->value = emux_get_color_brightness(0);
  contrast_item[0]->value = emux_get_color_contrast(0);
  gamma_item[0]->value = emux_get_color_gamma(0);
  tint_item[0]->value = emux_get_color_tint(0);
  saturation_item[0]->value = emux_get_color_saturation(0);

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
    brightness_item[1]->value = emux_get_color_brightness(1);
    contrast_item[1]->value = emux_get_color_contrast(1);
    gamma_item[1]->value = emux_get_color_gamma(1);
    tint_item[1]->value = emux_get_color_tint(1);
    saturation_item[1]->value = emux_get_color_saturation(1);
    emux_get_int(Setting_C128ColumnKey, &c40_80_column_item->value);
  }

  // Default pot values for buttons
  pot_x_high_value = 192;
  pot_x_low_value = 64;
  pot_y_high_value = 192;
  pot_y_low_value = 64;

  FILE *fp;
  switch (emux_machine_class) {
  case BMC64_MACHINE_CLASS_C64:
    fp = fopen("/settings.txt", "r");
    break;
  case BMC64_MACHINE_CLASS_C128:
    fp = fopen("/settings-c128.txt", "r");
    break;
  case BMC64_MACHINE_CLASS_VIC20:
    fp = fopen("/settings-vic20.txt", "r");
    break;
  case BMC64_MACHINE_CLASS_PLUS4:
    fp = fopen("/settings-plus4.txt", "r");
    break;
  case BMC64_MACHINE_CLASS_PLUS4EMU:
    fp = fopen("/settings-plus4emu.txt", "r");
    break;
  case BMC64_MACHINE_CLASS_PET:
    fp = fopen("/settings-pet.txt", "r");
    break;
  default:
    printf("ERROR: Unhandled machine\n");
    return;
  }

  if (fp == NULL)
    return;

  char name_value[256];
  size_t len;
  int value;
  int usb_btn_i[MAX_USB_DEVICES];
  memset(usb_btn_i, 0, sizeof(usb_btn_i));

  while (1) {
    char *line = fgets(name_value, 255, fp);
    if (feof(fp) || line == NULL) break;

    strcpy(name_value, line);

    char *name;
    char *value_str;
    get_key_and_value(name_value, &name, &value_str);
    if (!name || !value_str ||
       strlen(name) == 0 ||
          strlen(value_str) == 0) {
       continue;
    }

    value = atoi(value_str);

    if (emux_handle_loaded_setting(name, value_str, value)) {
       continue;
    }

    if (port_1_menu_item && strcmp(name, "port_1") == 0) {
      port_1_menu_item->value = value;
    } else if (port_2_menu_item && strcmp(name, "port_2") == 0) {
      port_2_menu_item->value = value;
    } else if (port_3_menu_item && strcmp(name, "port_3") == 0) {
      port_3_menu_item->value = value;
    } else if (port_4_menu_item && strcmp(name, "port_4") == 0) {
      port_4_menu_item->value = value;
    } else if (strcmp(name, "palette") == 0) {
      palette_item[0]->value = value;
      if (value >= palette_item[0]->num_choices) {
         palette_item[1]->value = palette_item[0]->num_choices - 1;
      }
    } else if (strcmp(name, "palette2") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      palette_item[1]->value = value;
      if (value >= palette_item[1]->num_choices) {
         palette_item[1]->value = palette_item[1]->num_choices - 1;
      }
    } else if (strcmp(name, "alt_f12") == 0) {
      // Old. Equivalent to cf7 = Menu
      hotkey_cf7_item->value = HOTKEY_CHOICE_MENU;
    } else if (strcmp(name, "overlay") == 0) { // legacy name
      statusbar_item->value = value;
    } else if (strcmp(name, "overlay_padding") == 0) { // legacy name
      statusbar_padding_item->value = value;
    } else if (strcmp(name, "vkbd_trans") == 0) {
      vkbd_transparency_item->value = value;
    } else if (strcmp(name, "tapereset") == 0) {
      tape_reset_with_machine_item->value = value;
    } else if (strcmp(name, "pot_x_high") == 0) {
      pot_x_high_value = value;
    } else if (strcmp(name, "pot_x_low") == 0) {
      pot_x_low_value = value;
    } else if (strcmp(name, "pot_y_high") == 0) {
      pot_y_high_value = value;
    } else if (strcmp(name, "pot_y_low") == 0) {
      pot_y_low_value = value;
    } else if (strcmp(name, "hotkey_cf1") == 0) {
      hotkey_cf1_item->value = value;
    } else if (strcmp(name, "hotkey_cf3") == 0) {
      hotkey_cf3_item->value = value;
    } else if (strcmp(name, "hotkey_cf5") == 0) {
      hotkey_cf5_item->value = value;
    } else if (strcmp(name, "hotkey_cf7") == 0) {
      hotkey_cf7_item->value = value;
    } else if (strcmp(name, "hotkey_tf1") == 0) {
      hotkey_tf1_item->value = value;
    } else if (strcmp(name, "hotkey_tf3") == 0) {
      hotkey_tf3_item->value = value;
    } else if (strcmp(name, "hotkey_tf5") == 0) {
      hotkey_tf5_item->value = value;
    } else if (strcmp(name, "hotkey_tf7") == 0) {
      hotkey_tf7_item->value = value;
    } else if (strcmp(name, "reset_confirm") == 0) {
      reset_confirm_item->value = value;
    } else if (strcmp(name, "scaling_interp") == 0) {
      scaling_interp_item->value = value;
    } else if (strcmp(name, "gpio_config") == 0) {
      // We save/restore the choice int and map back to
      // the value as index into the choices for this
      // param.
      switch(value) {
        case GPIO_CONFIG_NAV_JOY:
           gpio_config_item->value = 1;
           break;
        case GPIO_CONFIG_KYB_JOY:
           gpio_config_item->value = 2;
           break;
        case GPIO_CONFIG_WAVESHARE:
           gpio_config_item->value = 3;
           break;
        case GPIO_CONFIG_USERPORT:
           gpio_config_item->value = 4;
           break;
        case GPIO_CONFIG_CUSTOM:
           gpio_config_item->value = 5;
           break;
        default:
           // Disabled
           gpio_config_item->value = 0;
           break;
      }

      // Force disabled if kernel options says so.
      if (!circle_gpio_enabled()) {
         gpio_config_item->value = 0;
      }

      // Make sure pins are configured properly after load
      circle_reset_gpio(emu_get_gpio_config());
    } else if (strcmp(name, "keyset_1_up") == 0) {
      keyset_codes[0][KEYSET_UP] = value;
    } else if (strcmp(name, "keyset_1_down") == 0) {
      keyset_codes[0][KEYSET_DOWN] = value;
    } else if (strcmp(name, "keyset_1_left") == 0) {
      keyset_codes[0][KEYSET_LEFT] = value;
    } else if (strcmp(name, "keyset_1_right") == 0) {
      keyset_codes[0][KEYSET_RIGHT] = value;
    } else if (strcmp(name, "keyset_1_fire") == 0) {
      keyset_codes[0][KEYSET_FIRE] = value;
    } else if (strcmp(name, "keyset_1_potx") == 0) {
      keyset_codes[0][KEYSET_POTX] = value;
    } else if (strcmp(name, "keyset_1_poty") == 0) {
      keyset_codes[0][KEYSET_POTY] = value;
    } else if (strcmp(name, "keyset_2_up") == 0) {
      keyset_codes[1][KEYSET_UP] = value;
    } else if (strcmp(name, "keyset_2_down") == 0) {
      keyset_codes[1][KEYSET_DOWN] = value;
    } else if (strcmp(name, "keyset_2_left") == 0) {
      keyset_codes[1][KEYSET_LEFT] = value;
    } else if (strcmp(name, "keyset_2_right") == 0) {
      keyset_codes[1][KEYSET_RIGHT] = value;
    } else if (strcmp(name, "keyset_2_fire") == 0) {
      keyset_codes[1][KEYSET_FIRE] = value;
    } else if (strcmp(name, "keyset_2_potx") == 0) {
      keyset_codes[1][KEYSET_POTX] = value;
    } else if (strcmp(name, "keyset_2_poty") == 0) {
      keyset_codes[1][KEYSET_POTY] = value;
    } else if (strcmp(name, "key_binding_1") == 0) {
      key_bindings[0] = value;
    } else if (strcmp(name, "key_binding_2") == 0) {
      key_bindings[1] = value;
    } else if (strcmp(name, "key_binding_3") == 0) {
      key_bindings[2] = value;
    } else if (strcmp(name, "key_binding_4") == 0) {
      key_bindings[3] = value;
    } else if (strcmp(name, "key_binding_5") == 0) {
      key_bindings[4] = value;
    } else if (strcmp(name, "key_binding_6") == 0) {
      key_bindings[5] = value;
    } else if (strcmp(name, "h_center_0") == 0) {
      h_center_item[0]->value = value;
    } else if (strcmp(name, "v_center_0") == 0) {
      v_center_item[0]->value = value;
    } else if (strcmp(name, "h_border_trim_0") == 0) {
      // LEGACY NAME : menu value = max_border_w * value / 100.
      h_border_item[0]->value =
         h_border_item[0]->max * (1.0d - (value / 100.0d));
      // If this exists, we're going to default use_scaling_params to
      // 0 so we don't clobber user settings. This will never happen
      // again after the user saves at least once.
      use_scaling_params_item[0]->value = 0;
    } else if (strcmp(name, "v_border_trim_0") == 0) {
      // LEGACY NAME : menu value = max_border_h * value / 100.
      v_border_item[0]->value =
         v_border_item[0]->max * (1.0d - (value / 100.0d));
      // If this exists, we're going to default use_scaling_params to
      // 0 so we don't clobber user settings. This will never happen
      // again after the user saves at least once.
      use_scaling_params_item[0]->value = 0;
    } else if (strcmp(name, "aspect_0") == 0) {
      // LEGACY NAME : aspect * 10 = h_stretch
      h_stretch_item[0]->value = value * 10;
    } else if (strcmp(name, "h_border_0") == 0) {
      h_border_item[0]->value = value;
    } else if (strcmp(name, "v_border_0") == 0) {
      v_border_item[0]->value = value;
    } else if (strcmp(name, "h_stretch_0") == 0) {
      h_stretch_item[0]->value = value;
    } else if (strcmp(name, "v_stretch_0") == 0) {
      v_stretch_item[0]->value = value;
    } else if (strcmp(name, "h_center_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      h_center_item[1]->value = value;
    } else if (strcmp(name, "v_center_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      v_center_item[1]->value = value;
    } else if (strcmp(name, "h_border_trim_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      // LEGACY NAME : menu value = max_border_w * value / 100.
      h_border_item[1]->value = h_border_item[1]->max * (1.0d - (value / 100.0d));
      // If this exists, we're going to default use_scaling_params to
      // 0 so we don't clobber user settings. This will never happen
      // again after the user saves at least once.
      use_scaling_params_item[1]->value = 0;
    } else if (strcmp(name, "v_border_trim_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      // LEGACY NAME : menu value = max_border_h * value / 100.
      v_border_item[1]->value = v_border_item[1]->max * (1.0d - (value / 100.0d));
      // If this exists, we're going to default use_scaling_params to
      // 0 so we don't clobber user settings. This will never happen
      // again after the user saves at least once.
      use_scaling_params_item[1]->value = 0;
    } else if (strcmp(name, "aspect_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      // LEGACY NAME : aspect * 10 = h_stretch
      h_stretch_item[1]->value = value * 10;
    } else if (strcmp(name, "h_border_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      h_border_item[1]->value = value;
    } else if (strcmp(name, "v_border_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      v_border_item[1]->value = value;
    } else if (strcmp(name, "h_stretch_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      h_stretch_item[1]->value = value;
    } else if (strcmp(name, "v_stretch_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      v_stretch_item[1]->value = value;
    } else if (strcmp(name, "volume") == 0) {
      volume_item->value = value;
    } else if (strcmp(name, "dir_convention") == 0) {
      dir_convention_item->value = value;
    } else if (strcmp(name, "use_int_scaling_0") == 0) {
      use_scaling_params_item[0]->value = value;
    } else if (strcmp(name, "use_int_scaling_1") == 0 && emux_machine_class == BMC64_MACHINE_CLASS_C128) {
      use_scaling_params_item[1]->value = value;
    } else if (strcmp(name, "s_curvature") == 0) {
      s_curvature_item->value = value;
    } else if (strcmp(name, "s_curvature_x") == 0) {
      s_curvature_x_item->value = value;
    } else if (strcmp(name, "s_curvature_y") == 0) {
      s_curvature_y_item->value = value;
    } else if (strcmp(name, "s_sharper") == 0) {
      s_sharper_item->value = value;
    } else if (strcmp(name, "s_mask") == 0) {
      s_mask_item->value = value;
    } else if (strcmp(name, "s_mask_brightness") == 0) {
      s_mask_brightness_item->value = value;
    } else if (strcmp(name, "s_scanlines") == 0) {
      s_scanlines_item->value = value;
    } else if (strcmp(name, "s_multisample") == 0) {
      s_multisample_item->value = value;
    } else if (strcmp(name, "s_scanline_weight") == 0) {
      s_scanline_weight_item->value = value;
    } else if (strcmp(name, "s_scanline_gap_brightness") == 0) {
      s_scanline_gap_brightness_item->value = value;
    } else if (strcmp(name, "s_bloom_factor") == 0) {
      s_bloom_factor_item->value = value;
    } else if (strcmp(name, "s_gamma") == 0) {
      s_gamma_item->value = value;
    } else if (strcmp(name, "s_input_gamma") == 0) {
      s_input_gamma_item->value = value;
    } else if (strcmp(name, "s_output_gamma") == 0) {
      s_output_gamma_item->value = value;
    } else if (strcmp(name, "custom_gpio") == 0) {
      char* token = strtok (value_str, ",");
      if (token != NULL) {
         int pin_index = atoi(token);
         if (pin_index >=0 && pin_index < NUM_GPIO_PINS) {
            token = strtok (NULL, ",");
            unsigned int binding_value = token ? atoi(token) : 0;
            gpio_bindings[pin_index] = binding_value;
         }
      }
    } else {
      for (int k=0; k < MAX_USB_DEVICES; k++) {
       if (strcmp(name, usb_btn_name[k]) == 0) {
         if (value >= NUM_BUTTON_ASSIGNMENTS) {
            value = NUM_BUTTON_ASSIGNMENTS - 1;
         }
         usb_button_assignments[k][usb_btn_i[k]] = value;
         usb_btn_i[k]++;
         if (usb_btn_i[k] >= MAX_USB_BUTTONS) {
           usb_btn_i[k] = 0;
         }
       } else if (strcmp(name, usb_pref_name[k]) == 0) {
         usb_pref[k] = value;
       } else if (strcmp(name, usb_x_name[k]) == 0) {
         usb_x_axis[k] = value;
       } else if (strcmp(name, usb_y_name[k]) == 0) {
         usb_y_axis[k] = value;
       } else if (strcmp(name, usb_x_t_name[k]) == 0) {
         usb_x_thresh[k] = ((float)value) / 100.0f;
       } else if (strcmp(name, usb_y_t_name[k]) == 0) {
         usb_y_thresh[k] = ((float)value) / 100.0f;
       }
      }
    }
  }
  fclose(fp);

  emux_load_settings_done();

  emux_video_color_setting_changed(0);
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
    emux_video_color_setting_changed(1);
  }
}

// Swap ports 1 & 2
void menu_swap_joysticks() {
  if (port_1_menu_item && port_1_menu_item->choice_ints[port_1_menu_item->value]
          == JOYDEV_MOUSE) {
     emux_set_joy_port_device(1, JOYDEV_NONE);
  }

  if (port_2_menu_item && port_2_menu_item->choice_ints[port_2_menu_item->value]
       == JOYDEV_MOUSE) {
     emux_set_joy_port_device(2, JOYDEV_NONE);
  }

  int tmp = joydevs[0].device;
  joydevs[0].device = joydevs[1].device;
  joydevs[1].device = tmp;
  joyswap = 1 - joyswap;
  overlay_joyswap_changed(joyswap);
  ui_set_joy_items();
}

static void attach_cart(int menu_id, struct menu_item *item) {
  emux_attach_cart(menu_id, fullpath(DIR_CARTS, item->str_value));
}

// Reset current_dir_names according to preference.
static void set_current_dir_names() {
  int i;

  switch (dir_convention_item->value) {
     case MENU_DIR_CONVENTION_FOLDER_EMU:
        for (i = 0; i < NUM_DIR_TYPES; i++) {
          strcpy(current_dir_names[i], default_dir_names[i]);
          strcat(current_dir_names[i], machine_sub_dir);
        }
        strcpy(current_dir_names[DIR_ROOT], "/");
        break;
     case MENU_DIR_CONVENTION_EMU_FOLDER:
        for (i = 0; i < NUM_DIR_TYPES; i++) {
          strcpy(current_dir_names[i], machine_sub_dir);
          strcat(current_dir_names[i], default_dir_names[i]);
        }
        strcpy(current_dir_names[DIR_ROOT], machine_sub_dir);
        break;
     default:
        assert(0);
        break;
  }

  // These don't change
  strcpy(current_dir_names[DIR_ROMS], machine_sub_dir);
  strcpy(current_dir_names[DIR_IEC], "/");
}

static void select_file(struct menu_item *item) {
  switch (item->id) {
     case MENU_IEC_DIR:
       emux_set_iec_dir(unit, fullpath(DIR_IEC, ""));
       strcpy(last_iec_dir[unit-8], fullpath(DIR_IEC, ""));
       ui_pop_menu();
       return;
     case MENU_LOAD_SNAP_FILE:
       ui_info("Loading...");
       if (emux_load_state(fullpath(DIR_SNAPS, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Load snapshot failed");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_DISK_FILE:
       // Perform the attach
       ui_info("Attaching...");
       if (emux_attach_disk_image(unit, fullpath(DIR_DISKS, item->str_value)) <
           0) {
         ui_pop_menu();
         ui_error("Failed to attach disk image");
	 attached_disk_name[unit-8][0] = '\0';
       } else {
         ui_pop_all_and_toggle();
	 strcpy (attached_disk_name[unit-8], item->str_value);
       }
       return;
     case MENU_DRIVE_ROM_FILE_1541:
     case MENU_DRIVE_ROM_FILE_1541II:
     case MENU_DRIVE_ROM_FILE_1551:
     case MENU_DRIVE_ROM_FILE_1571:
     case MENU_DRIVE_ROM_FILE_1581:
     case MENU_DRIVE_ROM_FILE_CMDHD:
       emux_handle_rom_change(item, fullpath);
       // Two pops necessary here.
       ui_pop_menu();
       ui_pop_menu();
       return;
     case MENU_TAPE_FILE:
       ui_info("Attaching...");
       if (emux_attach_tape_image(fullpath(DIR_TAPES, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Failed to attach tape image");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     // NOTE: ROMs can't be fullpath or VICE complains.
     case MENU_KERNAL_FILE:
     case MENU_BASIC_FILE:
     case MENU_CHARGEN_FILE:
     case MENU_C128_LOAD_KERNAL_FILE:
     case MENU_C128_LOAD_BASIC_HI_FILE:
     case MENU_C128_LOAD_BASIC_LO_FILE:
     case MENU_C128_LOAD_CHARGEN_FILE:
     case MENU_C128_LOAD_64_KERNAL_FILE:
     case MENU_C128_LOAD_64_BASIC_FILE:
       emux_handle_rom_change(item, fullpath);
       ui_pop_all_and_toggle();
       return;
     case MENU_AUTOSTART_FILE:
       ui_info("Starting...");
       if (emux_autostart_file(fullpath(DIR_ROOT, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Failed to autostart file");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_LOADPRG_FILE:
       ui_info("Loading...");
       if (emux_autostart_file(fullpath(DIR_ROOT, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Failed to load file");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_C64_CART_FILE:
     case MENU_C64_CART_8K_FILE:
     case MENU_C64_CART_16K_FILE:
     case MENU_C64_CART_ULTIMAX_FILE:
     case MENU_VIC20_CART_DETECT_FILE:
     case MENU_VIC20_CART_GENERIC_FILE:
     case MENU_VIC20_CART_16K_2000_FILE:
     case MENU_VIC20_CART_16K_4000_FILE:
     case MENU_VIC20_CART_16K_6000_FILE:
     case MENU_VIC20_CART_8K_A000_FILE:
     case MENU_VIC20_CART_4K_B000_FILE:
     case MENU_VIC20_CART_BEHRBONZ_FILE:
     case MENU_VIC20_CART_UM_FILE:
     case MENU_VIC20_CART_FP_FILE:
     case MENU_VIC20_CART_MEGACART_FILE:
     case MENU_VIC20_CART_FINAL_EXPANSION_FILE:
     case MENU_PLUS4_CART_FILE:
     case MENU_PLUS4_CART_C0_LO_FILE:
     case MENU_PLUS4_CART_C0_HI_FILE:
     case MENU_PLUS4_CART_C1_LO_FILE:
     case MENU_PLUS4_CART_C1_HI_FILE:
     case MENU_PLUS4_CART_C2_LO_FILE:
     case MENU_PLUS4_CART_C2_HI_FILE:
       attach_cart(item->id, item);
       return;
     default:
       break;
  }

  // Handle saving snapshots.
  if (item->id == MENU_SAVE_SNAP_FILE) {
    char *fname = item->str_value;
    if (item->type == TEXTFIELD) {
      // Scrub the filename before passing it along
      fname = item->str_value;
      if (strlen(fname) == 0) {
        ui_error("Empty filename");
        return;
      } else if (strlen(fname) > MAX_FN_NAME) {
        ui_error("Too long");
        return;
      }
      char *dot = strchr(fname, '.');
      if (dot == NULL) {
        if (strlen(fname) + 4 <= MAX_FN_NAME) {
          strcat(fname, snap_filt_ext[0]);
        } else {
          ui_error("Too long");
          return;
        }
      } else {
        char l1 = tolower(dot[1]);
        char l2 = tolower(dot[2]);
        char l3 = tolower(dot[3]);
        if (l1 != snap_filt_ext[0][1] ||
            l2 != snap_filt_ext[0][2] ||
            l3 != snap_filt_ext[0][3] || dot[4] != '\0') {
          if (emux_machine_class == BMC64_MACHINE_CLASS_PLUS4EMU) {
             ui_error("Need .P4S extension");
          } else {
             ui_error("Need .VSF extension");
          }
          return;
        }
      }
    }
    ui_info("Saving...");
    if (emux_save_state(fullpath(DIR_SNAPS, fname)) < 0) {
      ui_pop_menu();
      ui_error("Save snapshot failed");
    } else {
      ui_pop_all_and_toggle();
    }
  }

  // Handle creating empty disk
  else if (item->id >= MENU_CREATE_D64_FILE &&
           item->id <= MENU_CREATE_DHD_FILE) {
    emux_create_disk(item, fullpath);
  }

  // Handle creating empty tape
  else if (item->id == MENU_CREATE_TAP_FILE) {
    emux_create_tape(item, fullpath);
  }
}

// Utility to determine current dir index from a menu file item
static int menu_file_item_to_dir_index(struct menu_item *item) {
  int index;
  switch (item->id) {
  case MENU_LOAD_SNAP_FILE:
  case MENU_SAVE_SNAP_FILE:
    return DIR_SNAPS;
  case MENU_DISK_FILE:
  case MENU_CREATE_D64_FILE:
  case MENU_CREATE_D67_FILE:
  case MENU_CREATE_D71_FILE:
  case MENU_CREATE_D80_FILE:
  case MENU_CREATE_D81_FILE:
  case MENU_CREATE_D82_FILE:
  case MENU_CREATE_D1M_FILE:
  case MENU_CREATE_D2M_FILE:
  case MENU_CREATE_D4M_FILE:
  case MENU_CREATE_G64_FILE:
  case MENU_CREATE_G71_FILE:
  case MENU_CREATE_P64_FILE:
  case MENU_CREATE_X64_FILE:
  case MENU_CREATE_DHD_FILE:
    return DIR_DISKS;
  case MENU_TAPE_FILE:
  case MENU_CREATE_TAP_FILE:
    return DIR_TAPES;
  case MENU_C64_CART_FILE:
  case MENU_C64_CART_8K_FILE:
  case MENU_C64_CART_16K_FILE:
  case MENU_C64_CART_ULTIMAX_FILE:
  case MENU_VIC20_CART_DETECT_FILE:
  case MENU_VIC20_CART_GENERIC_FILE:
  case MENU_VIC20_CART_16K_2000_FILE:
  case MENU_VIC20_CART_16K_4000_FILE:
  case MENU_VIC20_CART_16K_6000_FILE:
  case MENU_VIC20_CART_8K_A000_FILE:
  case MENU_VIC20_CART_4K_B000_FILE:
  case MENU_VIC20_CART_BEHRBONZ_FILE:
  case MENU_VIC20_CART_UM_FILE:
  case MENU_VIC20_CART_FP_FILE:
  case MENU_VIC20_CART_MEGACART_FILE:
  case MENU_VIC20_CART_FINAL_EXPANSION_FILE:
  case MENU_PLUS4_CART_FILE:
  case MENU_PLUS4_CART_C0_LO_FILE:
  case MENU_PLUS4_CART_C0_HI_FILE:
  case MENU_PLUS4_CART_C1_LO_FILE:
  case MENU_PLUS4_CART_C1_HI_FILE:
  case MENU_PLUS4_CART_C2_LO_FILE:
  case MENU_PLUS4_CART_C2_HI_FILE:
    return DIR_CARTS;
  case MENU_KERNAL_FILE:
  case MENU_BASIC_FILE:
  case MENU_CHARGEN_FILE:
  case MENU_DRIVE_ROM_FILE_1541:
  case MENU_DRIVE_ROM_FILE_1541II:
  case MENU_DRIVE_ROM_FILE_1551:
  case MENU_DRIVE_ROM_FILE_1571:
  case MENU_DRIVE_ROM_FILE_1581:
    return DIR_ROMS;
  case MENU_AUTOSTART_FILE:
  case MENU_LOADPRG_FILE:
    return DIR_ROOT;
  case MENU_IEC_DIR:
    return DIR_IEC;
  default:
    return -1;
  }
}

// Utility function to re-list same type of files given
// a file item.
static void relist_files_after_dir_change(struct menu_item *item) {
  switch (item->id) {
  case MENU_LOAD_SNAP_FILE:
    show_files(DIR_SNAPS, FILTER_SNAP, item->id, 1);
    break;
  case MENU_SAVE_SNAP_FILE:
    show_files(DIR_SNAPS, FILTER_SNAP, item->id, 1);
    break;
  case MENU_DISK_FILE:
  case MENU_CREATE_D64_FILE:
  case MENU_CREATE_D67_FILE:
  case MENU_CREATE_D71_FILE:
  case MENU_CREATE_D80_FILE:
  case MENU_CREATE_D81_FILE:
  case MENU_CREATE_D82_FILE:
  case MENU_CREATE_D1M_FILE:
  case MENU_CREATE_D2M_FILE:
  case MENU_CREATE_D4M_FILE:
  case MENU_CREATE_G64_FILE:
  case MENU_CREATE_G71_FILE:
  case MENU_CREATE_P64_FILE:
  case MENU_CREATE_X64_FILE:
  case MENU_CREATE_DHD_FILE:
    show_files(DIR_DISKS, FILTER_DISK, item->id, 1);
    break;
  case MENU_TAPE_FILE:
  case MENU_CREATE_TAP_FILE:
    show_files(DIR_TAPES, FILTER_TAPE, item->id, 1);
    break;
  case MENU_C64_CART_FILE:
    show_files(DIR_CARTS, FILTER_CART, item->id, 1);
    break;
  case MENU_C64_CART_8K_FILE:
  case MENU_C64_CART_16K_FILE:
  case MENU_C64_CART_ULTIMAX_FILE:
  case MENU_VIC20_CART_DETECT_FILE:
  case MENU_VIC20_CART_GENERIC_FILE:
  case MENU_VIC20_CART_16K_2000_FILE:
  case MENU_VIC20_CART_16K_4000_FILE:
  case MENU_VIC20_CART_16K_6000_FILE:
  case MENU_VIC20_CART_8K_A000_FILE:
  case MENU_VIC20_CART_4K_B000_FILE:
  case MENU_VIC20_CART_BEHRBONZ_FILE:
  case MENU_VIC20_CART_UM_FILE:
  case MENU_VIC20_CART_FP_FILE:
  case MENU_VIC20_CART_MEGACART_FILE:
  case MENU_VIC20_CART_FINAL_EXPANSION_FILE:
  case MENU_PLUS4_CART_FILE:
  case MENU_PLUS4_CART_C0_LO_FILE:
  case MENU_PLUS4_CART_C0_HI_FILE:
  case MENU_PLUS4_CART_C1_LO_FILE:
  case MENU_PLUS4_CART_C1_HI_FILE:
  case MENU_PLUS4_CART_C2_LO_FILE:
  case MENU_PLUS4_CART_C2_HI_FILE:
    show_files(DIR_CARTS, FILTER_NONE, item->id, 1);
    break;
  case MENU_KERNAL_FILE:
  case MENU_BASIC_FILE:
  case MENU_CHARGEN_FILE:
  case MENU_C128_LOAD_KERNAL_FILE:
  case MENU_C128_LOAD_BASIC_HI_FILE:
  case MENU_C128_LOAD_BASIC_LO_FILE:
  case MENU_C128_LOAD_CHARGEN_FILE:
  case MENU_C128_LOAD_64_KERNAL_FILE:
  case MENU_C128_LOAD_64_BASIC_FILE:
  case MENU_DRIVE_ROM_FILE_1541:
  case MENU_DRIVE_ROM_FILE_1541II:
  case MENU_DRIVE_ROM_FILE_1551:
  case MENU_DRIVE_ROM_FILE_1571:
  case MENU_DRIVE_ROM_FILE_1581:
    show_files(DIR_ROMS, FILTER_NONE, item->id, 1);
    break;
  case MENU_AUTOSTART_FILE:
    show_files(DIR_ROOT, FILTER_NONE, item->id, 1);
    break;
  case MENU_LOADPRG_FILE:
    show_files(DIR_ROOT, FILTER_PRGS, item->id, 1);
    break;
  case MENU_IEC_DIR:
    show_files(DIR_IEC, FILTER_DIRS, item->id, 1);
    break;
  default:
    break;
  }
}

static void up_dir(struct menu_item *item) {
  int i;
  int dir_index = menu_file_item_to_dir_index(item);
  if (dir_index < 0)
    return;
  // Remove last directory from current_dir_names
  i = strlen(current_dir_names[dir_index]) - 1;
  while (current_dir_names[dir_index][i] != '/' && i > 0)
    i--;
  current_dir_names[dir_index][i] = '\0';
  if (strlen(current_dir_names[dir_index]) == 0) {
    strcpy(current_dir_names[dir_index], "/");
  }
  ui_pop_menu();
  relist_files_after_dir_change(item);
}

static void enter_dir(struct menu_item *item) {
  int dir_index = menu_file_item_to_dir_index(item);
  if (dir_index < 0)
    return;
  // Append this item's value to current dir
  if (current_dir_names[dir_index][strlen(current_dir_names[dir_index]) - 1] !=
      '/') {
    strcat(current_dir_names[dir_index], "/");
  }
  strcat(current_dir_names[dir_index], item->str_value);
  ui_pop_menu();
  relist_files_after_dir_change(item);
}

static void toggle_warp(int value) {
  emux_set_warp(value);
  overlay_warp_changed(value);
  warp_item->value = value;
}

// Tell videoarch the new settings made from the menu.
static void do_video_settings(int layer) {

  double lpad;
  double rpad;
  double tpad;
  double bpad;
  int zlayer;

  struct menu_item* hcenter_item;
  struct menu_item* vcenter_item;
  struct menu_item* hborder_item;
  struct menu_item* vborder_item;
  struct menu_item* h_str_item;
  struct menu_item* v_str_item;
  int h_int_stretch;
  int v_int_stretch;
  int use_h_int_stretch;
  int use_v_int_stretch;

  int canvas_index;
  if (layer == FB_LAYER_VIC) {
     canvas_index = VIC_INDEX;
  } else if (layer == FB_LAYER_VDC) {
     canvas_index = VDC_INDEX;
  } else {
     return;
  }

  hcenter_item = h_center_item[canvas_index];
  vcenter_item = v_center_item[canvas_index];
  hborder_item = h_border_item[canvas_index];
  vborder_item = v_border_item[canvas_index];
  h_str_item = h_stretch_item[canvas_index];
  v_str_item = v_stretch_item[canvas_index];
  h_int_stretch = h_integer_stretch[canvas_index];
  v_int_stretch = v_integer_stretch[canvas_index];
  use_h_int_stretch = use_h_integer_stretch[canvas_index];
  use_v_int_stretch = use_v_integer_stretch[canvas_index];

  int hc = hcenter_item->value;
  int vc = vcenter_item->value;
  int vid_hc = hc;
  int vid_vc = vc;

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     if ((active_display_item->value == MENU_ACTIVE_DISPLAY_VICII && layer == FB_LAYER_VIC) ||
         (active_display_item->value == MENU_ACTIVE_DISPLAY_VDC && layer == FB_LAYER_VDC)) {
        lpad = 0; rpad = 0; tpad = 0; bpad = 0; zlayer = layer == FB_LAYER_VIC ? 0 : 1;
     } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE) {
        // VIC on the left, VDC on the right, always, no swapping
        use_h_int_stretch = 0;
        use_v_int_stretch = 0;
        if (layer == FB_LAYER_VIC) {
            lpad = 0; rpad = .50d; tpad = 0; bpad = 0; zlayer = 0;
        } else {
            lpad = .50d; rpad = 0; tpad = 0; bpad = 0; zlayer = 1;
        }
        // Always ignore centering in this mode
        vid_hc = 0;
        vid_vc = 0;
     } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_PIP) {
        if ((layer == FB_LAYER_VIC && pip_swapped_item->value == 0) ||
            (layer == FB_LAYER_VDC && pip_swapped_item->value == 1)) {
            // full screen for this layer
            lpad = 0; rpad = 0; tpad = 0; bpad = 0; zlayer = 0;
        } else {
            use_h_int_stretch = 0;
            use_v_int_stretch = 0;
            zlayer = 1;
            if (pip_location_item->value == MENU_PIP_TOP_LEFT) {
              // top left quad
              lpad = .05d; rpad = .65d; tpad = .05d; bpad = .65d;
            } else if (pip_location_item->value == MENU_PIP_TOP_RIGHT) {
              // top right quad
              lpad = .65d; rpad = .05d; tpad = .05d; bpad = .65d;
            } else if (pip_location_item->value == MENU_PIP_BOTTOM_RIGHT) {
              // bottom right quad
              lpad = .65d; rpad = .05d; tpad = .65d; bpad = .05d;
            } else if (pip_location_item->value == MENU_PIP_BOTTOM_LEFT) {
              // bottom left quad
              lpad = .05d; rpad = .65d; tpad = .65d; bpad = .05d;
            }
            // Always ignore centering in this mode
            vid_hc = 0;
            vid_vc = 0;
        }
    } else {
        return;
    }
  } else {
     // Only 1 display for this machine. Full screen.
     lpad = 0; rpad = 0; tpad = 0; bpad = 0; zlayer = 0;
  }

  int h = hborder_item->value;
  int v = vborder_item->value;
  double hs = (double)(h_str_item->value) / 1000.0d;
  double vs = (double)(v_str_item->value) / 1000.0d;

  double vid_hstretch = hs;
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128 &&
          active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE) {
     // For side-by-side, it makes more sense to fill horizontal then scale
     // vertical since we just cut horizontal in half. So pass in negative
     // hstretch.
     vid_hstretch = -hs;
  }

  // Tell videoarch about these changes
  emux_apply_video_adjustments(layer, vid_hc, vid_vc,
     h, v,
     vid_hstretch, vs,
     h_int_stretch, v_int_stretch,
     use_h_int_stretch, use_v_int_stretch,
     lpad, rpad, tpad, bpad, zlayer);

  if (layer == FB_LAYER_VIC) {
     // Make UI match VIC settings except for padding.
     emux_apply_video_adjustments(
        FB_LAYER_UI, hc, vc,
        h, v,
        hs, vs,
        h_integer_stretch[0], v_integer_stretch[0],
        use_h_integer_stretch[0], use_v_integer_stretch[0],
        0, 0, 0, 0, 3);
  }
}

static void menu_machine_reset(int type, int pop) {
  // The IEC dir may have been changed by the emulated machine. On reset,
  // we reset back to the last dir set by the user.
  emux_set_iec_dir(8, last_iec_dir[0]);
  emux_set_iec_dir(9, last_iec_dir[1]);
  emux_set_iec_dir(10, last_iec_dir[2]);
  emux_set_iec_dir(11, last_iec_dir[3]);
  emux_reset(type);
  if (pop) {
     ui_pop_all_and_toggle();
  }
}

static void reset_shader_params() {
  s_curvature_item->value = 0;
  s_curvature_x_item->value = 10;
  s_curvature_y_item->value = 15;
  s_sharper_item->value = 0;
  s_mask_item->value = 0;
  s_mask_brightness_item->value = 70;
  s_scanlines_item->value = 1;
  s_multisample_item->value = 1;
  s_scanline_weight_item->value = 60;
  s_scanline_gap_brightness_item->value = 12;
  s_bloom_factor_item->value = 150;
  s_gamma_item->value = 2;
  s_input_gamma_item->value = 240;
  s_output_gamma_item->value = 220;
}

static void sanity_check_shader_params(int itemid) {
    // All shader items should be disabled if shader is off
    s_curvature_item->disabled = 0;
    s_curvature_x_item->disabled = 0;
    s_curvature_y_item->disabled = 0;
    s_mask_item->disabled = 0;
    s_mask_brightness_item->disabled = 0;
    s_gamma_item->disabled = 0;
    s_scanlines_item->disabled = 0;
    s_multisample_item->disabled = 0;
    s_scanline_weight_item->disabled = 0;
    s_scanline_gap_brightness_item->disabled = 0;
    s_bloom_factor_item->disabled = 0;
    s_input_gamma_item->disabled = 0;
    s_output_gamma_item->disabled = 0;
    s_sharper_item->disabled = 0;
    if (!s_enable_shader_item->value) {
       s_curvature_item->disabled = 1;
       s_curvature_x_item->disabled = 1;
       s_curvature_y_item->disabled = 1;
       s_mask_item->disabled = 1;
       s_mask_brightness_item->disabled = 1;
       s_gamma_item->disabled = 1;
       s_scanlines_item->disabled = 1;
       s_multisample_item->disabled = 1;
       s_scanline_weight_item->disabled = 1;
       s_scanline_gap_brightness_item->disabled = 1;
       s_bloom_factor_item->disabled = 1;
       s_input_gamma_item->disabled = 1;
       s_output_gamma_item->disabled = 1;
       s_sharper_item->disabled = 1;
    }

    if (itemid == MENU_SHADER_SCANLINES &&
       s_scanlines_item->value &&
         s_mask_item->value == 2) {
       // Turn off Trinitron if user selects scanlines
       s_mask_item->value = 0;
    } else if (itemid == MENU_SHADER_MASK &&
       s_mask_item->value == 2 &&
         s_scanlines_item->value) {
       // Turn off Scanlines if user selects Trinitron mask
       s_scanlines_item->value = 0;
    }

    // If curvature is off, disable x and y too
    if (!s_curvature_item->value) {
       s_curvature_x_item->disabled = 1;
       s_curvature_y_item->disabled = 1;
    }

    // If scanlines are off, gamma is disabled.
    // If scanlines are off, weight and gap brightness disabled
    // If scanlines are off, bloom factor is disabled
    if (!s_scanlines_item->value) {
       s_multisample_item->disabled = 1;
       s_gamma_item->disabled = 1;
       s_scanline_weight_item->disabled = 1;
       s_scanline_gap_brightness_item->disabled = 1;
       s_bloom_factor_item->disabled = 1;
    }

    // If gamma is disabled, off or fake, input/output is disabled.
    if (s_gamma_item->disabled || s_gamma_item->value == 0 || s_gamma_item->value == 2) {
       s_input_gamma_item->disabled = 1;
       s_output_gamma_item->disabled = 1;
    }

    // If mask is off, mask brightness is disabled
    if (!s_mask_item->value) {
       s_mask_brightness_item->disabled = 1;
    }
}

static void handle_shader_param_change() {
  int curvature;
  float curvature_x;
  float curvature_y;
  int mask;
  float mask_brightness;
  int gamma;
  int fake_gamma;
  int scanlines;
  int multisample;
  float scanline_weight;
  float scanline_gap_brightness;
  float bloom_factor;
  float input_gamma;
  float output_gamma;
  int sharper;
  int bilinear_interpolation;

  curvature = s_curvature_item->value;
  curvature_x = (float)s_curvature_x_item->value / 100.0f;
  curvature_y = (float)s_curvature_y_item->value / 100.0f;
  mask = s_mask_item->value;
  mask_brightness = (float)s_mask_brightness_item->value / 100.0f;
  gamma = s_gamma_item->value > 0;
  fake_gamma = s_gamma_item->value == 2;
  scanlines = s_scanlines_item->value;
  multisample = s_multisample_item->value;
  scanline_weight = (float)s_scanline_weight_item->value / 10.0f;
  scanline_gap_brightness = (float)s_scanline_gap_brightness_item->value / 100.0f;
  bloom_factor = (float)s_bloom_factor_item->value / 100.0f;
  input_gamma = (float)s_input_gamma_item->value / 100.0f;
  output_gamma = (float)s_output_gamma_item->value / 100.0f;
  sharper = s_sharper_item->value;
  bilinear_interpolation = scaling_interp_item->value;

  circle_set_shader_params(curvature,
                        curvature_x,
                        curvature_y,
                        mask,
                        mask_brightness,
                        gamma,
                        fake_gamma,
                        scanlines,
                        multisample,
                        scanline_weight,
                        scanline_gap_brightness,
                        bloom_factor,
                        input_gamma,
                        output_gamma,
                        sharper,
                        bilinear_interpolation);

  // Setting shader params hides the layer.
  vic_showing = 0;
}

// Interpret what menu item changed and make the change to vice
static void menu_value_changed(struct menu_item *item) {
  struct machine_entry* head;
  int status = 0;
  int p;

  switch (item->id) {
  case MENU_ATTACH_DISK_8:
  case MENU_IECDEVICE_8:
  case MENU_IECDIR_8:
  case MENU_DRIVE_CHANGE_MODEL_8:
  case MENU_PARALLEL_8:
  case MENU_CMDHD_MODE_8:
    unit = 8;
    break;
  case MENU_ATTACH_DISK_9:
  case MENU_IECDEVICE_9:
  case MENU_IECDIR_9:
  case MENU_DRIVE_CHANGE_MODEL_9:
  case MENU_PARALLEL_9:
  case MENU_CMDHD_MODE_9:
    unit = 9;
    break;
  case MENU_ATTACH_DISK_10:
  case MENU_IECDEVICE_10:
  case MENU_IECDIR_10:
  case MENU_DRIVE_CHANGE_MODEL_10:
  case MENU_PARALLEL_10:
  case MENU_CMDHD_MODE_10:
    unit = 10;
    break;
  case MENU_ATTACH_DISK_11:
  case MENU_IECDEVICE_11:
  case MENU_IECDIR_11:
  case MENU_DRIVE_CHANGE_MODEL_11:
  case MENU_PARALLEL_11:
  case MENU_CMDHD_MODE_11:
    unit = 11;
    break;
  }

  if (emux_handle_menu_change(item)) {
    return;
  }

  switch (item->id) {
  case MENU_SAVE_SETTINGS:
    if (save_settings()) {
      ui_error("Problem saving");
    } else {
      ui_info("Settings saved");
    }
    return;
  case MENU_COLOR_PALETTE_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_change_palette(0, item->value);
    return;
  case MENU_COLOR_PALETTE_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_change_palette(1, item->value);
    return;
  case MENU_AUTOSTART_WARP:
    emux_set_int(Setting_AutostartWarp, item->value);
    return;
  case MENU_AUTOSTART:
    show_files(DIR_ROOT, FILTER_NONE, MENU_AUTOSTART_FILE, 0);
    return;
  case MENU_LOADPRG:
    show_files(DIR_ROOT, FILTER_PRGS, MENU_LOADPRG_FILE, 0);
    return;
  case MENU_SAVE_SNAP:
    show_files(DIR_SNAPS, FILTER_SNAP, MENU_SAVE_SNAP_FILE, 0);
    return;
  case MENU_LOAD_SNAP:
    show_files(DIR_SNAPS, FILTER_SNAP, MENU_LOAD_SNAP_FILE, 0);
    return;
  case MENU_CREATE_D64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D64_FILE, 0);
    return;
  case MENU_CREATE_D67:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D67_FILE, 0);
    return;
  case MENU_CREATE_D71:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D71_FILE, 0);
    return;
  case MENU_CREATE_D80:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D80_FILE, 0);
    return;
  case MENU_CREATE_D81:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D81_FILE, 0);
    return;
  case MENU_CREATE_D82:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D82_FILE, 0);
    return;
  case MENU_CREATE_D1M:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D1M_FILE, 0);
    return;
  case MENU_CREATE_D2M:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D2M_FILE, 0);
    return;
  case MENU_CREATE_D4M:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_D4M_FILE, 0);
    return;
  case MENU_CREATE_G64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_G64_FILE, 0);
    return;
  case MENU_CREATE_G71:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_G71_FILE, 0);
    return;
  case MENU_CREATE_P64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_P64_FILE, 0);
    return;
  case MENU_CREATE_X64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_X64_FILE, 0);
    return;
  case MENU_CREATE_DHD:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_DHD_FILE, 0);
    return;
  case MENU_CREATE_TAP:
    show_files(DIR_TAPES, FILTER_NONE, MENU_CREATE_TAP_FILE, 0);
    return;

  case MENU_IECDEVICE_8:
  case MENU_IECDEVICE_9:
  case MENU_IECDEVICE_10:
  case MENU_IECDEVICE_11:
    emux_set_int_1(Setting_IECDeviceN, item->value, unit);
    return;
  case MENU_PARALLEL_8:
  case MENU_PARALLEL_9:
  case MENU_PARALLEL_10:
  case MENU_PARALLEL_11:
    emux_set_int_1(Setting_DriveNParallelCable,
       item->choice_ints[item->value], unit);
    return;
  case MENU_CMDHD_MODE_8:
  case MENU_CMDHD_MODE_9:
  case MENU_CMDHD_MODE_10:
  case MENU_CMDHD_MODE_11:
    emux_set_int_1(Setting_DriveNCMDHDMode,
       item->choice_ints[item->value], unit);
    return;
  case MENU_IECDIR_8:
  case MENU_IECDIR_9:
  case MENU_IECDIR_10:
  case MENU_IECDIR_11:
    show_files(DIR_IEC, FILTER_DIRS, MENU_IEC_DIR, 0);
    return;
  case MENU_ATTACH_DISK_8:
  case MENU_ATTACH_DISK_9:
  case MENU_ATTACH_DISK_10:
  case MENU_ATTACH_DISK_11:
    show_files(DIR_DISKS, FILTER_DISK, MENU_DISK_FILE, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_1541:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_1541, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_1541II:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_1541II, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_1551:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_1551, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_1571:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_1571, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_1581:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_1581, 0);
    return;
  case MENU_DRIVE_CHANGE_ROM_CMDHD:
    show_files(DIR_ROMS, FILTER_NONE, MENU_DRIVE_ROM_FILE_CMDHD, 0);
    return;
  case MENU_ATTACH_TAPE:
    show_files(DIR_TAPES, FILTER_TAPE, MENU_TAPE_FILE, 0);
    return;
  case MENU_C64_ATTACH_CART:
    show_files(DIR_CARTS, FILTER_CART, MENU_C64_CART_FILE, 0);
    return;
  case MENU_C64_ATTACH_CART_8K:
    show_files(DIR_CARTS, FILTER_NONE, MENU_C64_CART_8K_FILE, 0);
    return;
  case MENU_C64_ATTACH_CART_16K:
    show_files(DIR_CARTS, FILTER_NONE, MENU_C64_CART_16K_FILE, 0);
    return;
  case MENU_C64_ATTACH_CART_ULTIMAX:
    show_files(DIR_CARTS, FILTER_NONE, MENU_C64_CART_ULTIMAX_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_DETECT:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_DETECT_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_GENERIC:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_GENERIC_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_16K_2000:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_16K_2000_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_16K_4000:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_16K_4000_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_16K_6000:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_16K_6000_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_8K_A000:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_8K_A000_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_4K_B000:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_4K_B000_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_BEHRBONZ:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_BEHRBONZ_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_UM:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_UM_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_FP:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_FP_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_MEGACART:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_MEGACART_FILE, 0);
    return;
  case MENU_VIC20_ATTACH_CART_FINAL_EXPANSION:
    show_files(DIR_CARTS, FILTER_NONE, MENU_VIC20_CART_FINAL_EXPANSION_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C0_LO:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C0_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C0_HI:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C0_HI_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C1_LO:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C1_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C1_HI:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C1_HI_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C2_LO:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C2_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C2_HI:
    show_files(DIR_CARTS, FILTER_NONE, MENU_PLUS4_CART_C2_HI_FILE, 0);
    return;
  case MENU_LOAD_KERNAL:
    show_files(DIR_ROMS, FILTER_NONE, MENU_KERNAL_FILE, 0);
    return;
  case MENU_LOAD_BASIC:
    show_files(DIR_ROMS, FILTER_NONE, MENU_BASIC_FILE, 0);
    return;
  case MENU_LOAD_CHARGEN:
    show_files(DIR_ROMS, FILTER_NONE, MENU_CHARGEN_FILE, 0);
    return;
  case MENU_C128_LOAD_KERNAL:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_KERNAL_FILE, 0);
    return;
  case MENU_C128_LOAD_BASIC_HI:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_BASIC_HI_FILE, 0);
    return;
  case MENU_C128_LOAD_BASIC_LO:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_BASIC_LO_FILE, 0);
    return;
  case MENU_C128_LOAD_CHARGEN:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_CHARGEN_FILE, 0);
    return;
  case MENU_C128_LOAD_64_KERNAL:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_64_KERNAL_FILE, 0);
    return;
  case MENU_C128_LOAD_64_BASIC:
    show_files(DIR_ROMS, FILTER_NONE, MENU_C128_LOAD_64_BASIC_FILE, 0);
    return;
  case MENU_MAKE_CART_DEFAULT:
    emux_set_cart_default();
    ui_info("Remember to save..");
    return;
  case MENU_DETACH_DISK_8:
    ui_info("Deatching...");
    emux_detach_disk(8);
    attached_disk_name[0][0] = '\0';
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_9:
    ui_info("Detaching...");
    emux_detach_disk(9);
    attached_disk_name[1][0] = '\0';
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_10:
    ui_info("Detaching...");
    emux_detach_disk(10);
    attached_disk_name[2][0] = '\0';
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_11:
    ui_info("Detaching...");
    emux_detach_disk(11);
    attached_disk_name[3][0] = '\0';
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_TAPE:
    ui_info("Detaching...");
    emux_detach_tape();
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_CART:
    ui_info("Detaching...");
    emux_detach_cart(0);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C0_LO:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C0_LO);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C0_HI:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C0_HI);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C1_LO:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C1_LO);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C1_HI:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C1_HI);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C2_LO:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C2_LO);
    ui_pop_all_and_toggle();
    return;
  case MENU_PLUS4_DETACH_CART_C2_HI:
    ui_info("Detaching...");
    emux_detach_cart(MENU_PLUS4_DETACH_CART_C2_HI);
    ui_pop_all_and_toggle();
    return;
  case MENU_SOFT_RESET:
    menu_machine_reset(1 /* soft */, 1 /* pop */);
    return;
  case MENU_HARD_RESET:
    menu_machine_reset(0 /* hard */, 1 /* pop */);
    return;
  case MENU_ABOUT:
    show_about();
    return;
  case MENU_LICENSE:
    show_license();
    return;
  case MENU_USB_0_CONFIGURE:
  case MENU_USB_1_CONFIGURE:
  case MENU_USB_2_CONFIGURE:
  case MENU_USB_3_CONFIGURE:
    configure_usb(item->id - MENU_USB_0_CONFIGURE);
    return;
  case MENU_CONFIGURE_KEYSET1:
    configure_keyset(0);
    return;
  case MENU_CONFIGURE_KEYSET2:
    configure_keyset(1);
    return;
  case MENU_CONFIGURE_GPIO:
    configure_gpio();
    return;
  case MENU_GPIO_CONFIG:
    // Ensure GPIO pins are correct for new mode.
    circle_reset_gpio(emu_get_gpio_config());
    return;
  case MENU_WARP_MODE:
    toggle_warp(item->value);
    return;
  case MENU_DEMO_MODE:
    raspi_demo_mode = item->value;
    demo_reset();
    return;
  case MENU_DRIVE_SOUND_EMULATION:
    emux_set_int(Setting_DriveSoundEmulation, item->value);
    return;
  case MENU_DRIVE_SOUND_EMULATION_VOLUME:
    emux_set_int(Setting_DriveSoundEmulationVolume, item->value);
    return;
  case MENU_COLOR_BRIGHTNESS_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_set_color_brightness(0, item->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_CONTRAST_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_set_color_contrast(0, item->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_GAMMA_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_set_color_gamma(0, item->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_TINT_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_set_color_tint(0, item->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_SATURATION_0:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    emux_set_color_saturation(0, item->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_RESET_0:
    emux_get_default_color_setting(
      &brightness_item[0]->value,
      &contrast_item[0]->value,
      &gamma_item[0]->value,
      &tint_item[0]->value,
      &saturation_item[0]->value
    );
    emux_set_color_brightness(0, brightness_item[0]->value);
    emux_set_color_contrast(0, contrast_item[0]->value);
    emux_set_color_gamma(0, gamma_item[0]->value);
    emux_set_color_tint(0, tint_item[0]->value);
    emux_set_color_saturation(0, saturation_item[0]->value);
    emux_video_color_setting_changed(0);
    return;
  case MENU_COLOR_BRIGHTNESS_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_set_color_brightness(1, item->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_COLOR_CONTRAST_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_set_color_contrast(1, item->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_COLOR_GAMMA_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_set_color_gamma(1, item->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_COLOR_TINT_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_set_color_tint(1, item->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_COLOR_SATURATION_1:
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    emux_set_color_saturation(1, item->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_COLOR_RESET_1:
    emux_get_default_color_setting(
      &brightness_item[1]->value,
      &contrast_item[1]->value,
      &gamma_item[1]->value,
      &tint_item[1]->value,
      &saturation_item[1]->value
    );
    emux_set_color_brightness(1, brightness_item[1]->value);
    emux_set_color_contrast(1, contrast_item[1]->value);
    emux_set_color_gamma(1, gamma_item[1]->value);
    emux_set_color_tint(1, tint_item[1]->value);
    emux_set_color_saturation(1, saturation_item[1]->value);
    emux_video_color_setting_changed(1);
    return;
  case MENU_SWAP_JOYSTICKS:
    menu_swap_joysticks();
    return;
  case MENU_JOYSTICK_PORT_1:
  case MENU_JOYSTICK_PORT_2:
  case MENU_JOYSTICK_PORT_3:
  case MENU_JOYSTICK_PORT_4:
    p = item->id - MENU_JOYSTICK_PORT_1 + 1;
    set_joy_item_to_value(p, item->choice_ints[item->value]);
    set_need_mouse();
    return;
  case MENU_TAPE_START:
    emux_tape_control(EMUX_TAPE_PLAY);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_STOP:
    emux_tape_control(EMUX_TAPE_STOP);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_REWIND:
    emux_tape_control(EMUX_TAPE_REWIND);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_FASTFWD:
    emux_tape_control(EMUX_TAPE_FASTFORWARD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RECORD:
    emux_tape_control(EMUX_TAPE_RECORD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET:
    emux_tape_control(EMUX_TAPE_RESET);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET_COUNTER:
    emux_tape_control(EMUX_TAPE_ZERO);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET_WITH_MACHINE:
    emux_set_int(Setting_DatasetteResetWithCPU,
                      tape_reset_with_machine_item->value);
    return;
  case MENU_DRIVE_CHANGE_MODEL_8:
  case MENU_DRIVE_CHANGE_MODEL_9:
  case MENU_DRIVE_CHANGE_MODEL_10:
  case MENU_DRIVE_CHANGE_MODEL_11:
    emux_drive_change_model(unit);
    return;
  case MENU_DRIVE_CHANGE_ROM:
    drive_change_rom();
    return;
  case MENU_DRIVE_MODEL_SELECT:
    emux_set_int_1(Setting_DriveNType, item->value, unit);
    ui_pop_all_and_toggle();
    return;
  case MENU_CALC_TIMING:
    configure_timing();
    return;
  case MENU_HOTKEY_CF1:
    kbd_set_hotkey_function(
        0, KEYCODE_F1, hotkey_cf1_item->choice_ints[hotkey_cf1_item->value]);
    return;
  case MENU_HOTKEY_CF3:
    kbd_set_hotkey_function(
        1, KEYCODE_F3, hotkey_cf3_item->choice_ints[hotkey_cf3_item->value]);
    return;
  case MENU_HOTKEY_CF5:
    kbd_set_hotkey_function(
        2, KEYCODE_F5, hotkey_cf5_item->choice_ints[hotkey_cf5_item->value]);
    return;
  case MENU_HOTKEY_CF7:
    kbd_set_hotkey_function(
        3, KEYCODE_F7, hotkey_cf7_item->choice_ints[hotkey_cf7_item->value]);
    return;
  case MENU_HOTKEY_TF1:
    kbd_set_hotkey_function(
        4, KEYCODE_F1, hotkey_tf1_item->choice_ints[hotkey_tf1_item->value]);
    return;
  case MENU_HOTKEY_TF3:
    kbd_set_hotkey_function(
        5, KEYCODE_F3, hotkey_tf3_item->choice_ints[hotkey_tf3_item->value]);
    return;
  case MENU_HOTKEY_TF5:
    kbd_set_hotkey_function(
        6, KEYCODE_F5, hotkey_tf5_item->choice_ints[hotkey_tf5_item->value]);
    return;
  case MENU_HOTKEY_TF7:
    kbd_set_hotkey_function(
        7, KEYCODE_F7, hotkey_tf7_item->choice_ints[hotkey_tf7_item->value]);
    return;
  case MENU_VIC20_MEMORY_3K:
    emux_set_int(Setting_RAMBlock0, item->value);
    return;
  case MENU_VIC20_MEMORY_8K_2000:
    emux_set_int(Setting_RAMBlock1, item->value);
    return;
  case MENU_VIC20_MEMORY_8K_4000:
    emux_set_int(Setting_RAMBlock2, item->value);
    return;
  case MENU_VIC20_MEMORY_8K_6000:
    emux_set_int(Setting_RAMBlock3, item->value);
    return;
  case MENU_VIC20_MEMORY_8K_A000:
    emux_set_int(Setting_RAMBlock5, item->value);
    return;
  case MENU_ACTIVE_DISPLAY:
  case MENU_PIP_LOCATION:
  case MENU_PIP_SWAPPED:
    if (active_display_item->value == MENU_ACTIVE_DISPLAY_VICII) {
       vic_enabled = 1;
       vdc_enabled = 0;
       do_video_settings(FB_LAYER_VIC);
    } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_VDC) {
       vdc_enabled = 1;
       vic_enabled = 0;
       do_video_settings(FB_LAYER_VDC);
    } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE ||
               active_display_item->value == MENU_ACTIVE_DISPLAY_PIP) {
       vdc_enabled = 1;
       vic_enabled = 1;
       do_video_settings(FB_LAYER_VIC);
       do_video_settings(FB_LAYER_VDC);
    }
    break;
  case MENU_INTEGER_SCALE_W_0:
    next_integer_scaling(FB_LAYER_VIC, VIC_INDEX, 0);
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    do_video_settings(FB_LAYER_VIC);
    break;
  case MENU_INTEGER_SCALE_H_0:
    next_integer_scaling(FB_LAYER_VIC, VIC_INDEX, 1);
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    do_video_settings(FB_LAYER_VIC);
    break;
  case MENU_INTEGER_SCALE_W_1:
    next_integer_scaling(FB_LAYER_VDC, VDC_INDEX, 0);
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    do_video_settings(FB_LAYER_VDC);
    break;
  case MENU_INTEGER_SCALE_H_1:
    next_integer_scaling(FB_LAYER_VDC, VDC_INDEX, 1);
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    do_video_settings(FB_LAYER_VDC);
    break;
  case MENU_H_CENTER_0:
  case MENU_V_CENTER_0:
  case MENU_H_BORDER_0:
  case MENU_V_BORDER_0:
  case MENU_H_STRETCH_0:
  case MENU_V_STRETCH_0:
    // Any manual adjustment to stretch, go back
    // to scaled dimensions.
    if (item->id == MENU_H_STRETCH_0 || item->id == MENU_H_BORDER_0) {
       use_h_integer_stretch[0] = 0;
       use_scaling_params_item[0]->value = 0;
    } else if (item->id == MENU_V_STRETCH_0 || item->id == MENU_V_BORDER_0) {
       use_v_integer_stretch[0] = 0;
       use_scaling_params_item[0]->value = 0;
    }
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    do_video_settings(FB_LAYER_VIC);
    break;
  case MENU_H_CENTER_1:
  case MENU_V_CENTER_1:
  case MENU_H_BORDER_1:
  case MENU_V_BORDER_1:
  case MENU_H_STRETCH_1:
  case MENU_V_STRETCH_1:
    // Any manual adjustment to stretch, go back
    // to scaled dimensions.
    if (item->id == MENU_H_STRETCH_1 || item->id == MENU_H_BORDER_1) {
       use_h_integer_stretch[1] = 0;
       use_scaling_params_item[1]->value = 0;
    } else if (item->id == MENU_V_STRETCH_1 || item->id == MENU_V_BORDER_1) {
       use_v_integer_stretch[1] = 0;
       use_scaling_params_item[1]->value = 0;
    }
    ui_canvas_reveal_temp(FB_LAYER_VDC);
    do_video_settings(FB_LAYER_VDC);
    break;
  case MENU_OVERLAY:
    statusbar_forced = 0;
    if (item->value == 1) {
      overlay_statusbar_enable();
    } else {
      overlay_statusbar_disable();
    }
    break;
  case MENU_OVERLAY_PADDING:
    overlay_change_padding(item->value);
    break;
  case MENU_VKBD_TRANSPARENCY:
    overlay_change_vkbd_transparency(item->value);
    break;
  case MENU_40_80_COLUMN:
    emux_set_int(Setting_C128ColumnKey, item->value);
    overlay_40_80_columns_changed(item->value);
    break;
  case MENU_VOLUME:
    circle_set_volume(item->value);
    break;
  case MENU_SWITCH_MACHINE:
    ui_confirm_wrapped("Reboot?",SWITCH_MSG,item->value,MENU_SWITCH_MACHINE);
    break;
  case MENU_CONFIRM_OK:
    ui_pop_menu();
    if (item->sub_id == MENU_SWITCH_MACHINE) {
      load_machines(&head);
      struct machine_entry* ptr = head;
      status = 0;
      while (ptr) {
          if (ptr->id == item->value) {
            status = switch_apply_files(ptr);
            break;
          }
          ptr = ptr->next;
      }
      free_machines(head);
      if (status) {
         char failcode[32];
         sprintf (failcode, "FAILURE (CODE %d)", status);
         ui_confirm_wrapped(failcode, SWITCH_FAIL_MSG,-1,-1);
      } else {
         reboot();
      }
    }
    break;
  case MENU_CONFIRM_CANCEL:
    ui_pop_menu();
    break;
  case MENU_DIR_CONVENTION:
    set_current_dir_names();
    break;
  case MENU_SHADER_ENABLE:
    sanity_check_shader_params(item->id);
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    // Despite what the menu says, don't allow this to enable the shader
    // when conditions apply.
    circle_realloc_fbl(FB_LAYER_VIC, allow_shader() ? item->value : 0);
    emux_set_int(Setting_VideoFilter, item->value ? MENU_VIDEO_FILTER_CRT : MENU_VIDEO_FILTER_NONE);
    handle_shader_param_change();
    vic_showing = 0;
    break;
  case MENU_SHADER_CURVATURE:
  case MENU_SHADER_CURVATURE_X:
  case MENU_SHADER_CURVATURE_Y:
  case MENU_SHADER_SCANLINES:
  case MENU_SHADER_MULTISAMPLE:
  case MENU_SHADER_SCANLINE_WEIGHT:
  case MENU_SHADER_SCANLINE_GAP_BRIGHTNESS:
  case MENU_SHADER_MASK:
  case MENU_SHADER_MASK_BRIGHTNESS:
  case MENU_SHADER_BLOOM:
  case MENU_SHADER_GAMMA:
  case MENU_SHADER_INPUT_GAMMA:
  case MENU_SHADER_OUTPUT_GAMMA:
  case MENU_SHADER_SHARPER:
    sanity_check_shader_params(item->id);
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    handle_shader_param_change();
    break;
  case MENU_SHADER_RESET_ALL:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    reset_shader_params();
    sanity_check_shader_params(item->id);
    handle_shader_param_change();
    break;
  case MENU_USE_SCALING_PARAMS_0:
    if (item->value) {
       if (do_use_int_scaling(FB_LAYER_VIC, 0 /* not silent */)) {
          ui_canvas_reveal_temp(FB_LAYER_VIC);
          do_video_settings(FB_LAYER_VIC);
       } else {
          use_scaling_params_item[VIC_INDEX]->value = 0;
       }
    }
    break;
  case MENU_USE_SCALING_PARAMS_1:
    if (item->value) {
       if (do_use_int_scaling(FB_LAYER_VDC, 0 /* not silent */)) {
          ui_canvas_reveal_temp(FB_LAYER_VDC);
          do_video_settings(FB_LAYER_VDC);
       } else {
          use_scaling_params_item[VDC_INDEX]->value = 0;
       }
    }
    break;
  case MENU_SCALING_INTERPOLATION:
    ui_canvas_reveal_temp(FB_LAYER_VIC);
    circle_set_interpolation(item->value); // dispmanx interpolation
    if (s_enable_shader_item->value) {
       sanity_check_shader_params(item->id);
       handle_shader_param_change();
    }
    break;
  }

  // Only items that were for file selection/nav should have these set...
  if (item->sub_id == MENU_SUB_PICK_FILE || item->sub_id == MENU_SUB_PICK_DIR) {
    select_file(item);
    return;
  } else if (item->sub_id == MENU_SUB_UP_DIR) {
    up_dir(item);
    return;
  } else if (item->sub_id == MENU_SUB_ENTER_DIR) {
    enter_dir(item);
    return;
  } else if (item->sub_id == MENU_SUB_SELECT_VOLUME) {
    filesystem_change_volume(item);
    return;
  } else if (item->sub_id == MENU_SUB_CHANGE_VOLUME) {
    switch (item->value) {
       case MENU_VOLUME_SD:
           strcpy (current_volume_name, "SD:");
           break;
       case MENU_VOLUME_USB1:
           strcpy (current_volume_name, "USB:");
           if (!usb1_mounted) { circle_mount_usb(0); usb1_mounted = 1; }
           break;
       case MENU_VOLUME_USB2:
           strcpy (current_volume_name, "USB2:");
           if (!usb2_mounted) { circle_mount_usb(1); usb2_mounted = 1; }
           break;
       case MENU_VOLUME_USB3:
           strcpy (current_volume_name, "USB3:");
           if (!usb3_mounted) { circle_mount_usb(2); usb3_mounted = 1; }
           break;
       default:
           break;
    }
    // Need to pop both change volume popup and old file list
    ui_pop_menu();
    ui_pop_menu();
    relist_files_after_dir_change(item);
    return;
  }
}

// Returns what input preference user has for this usb device
void emu_get_usb_pref(int device, int *usb_pref_dst, int *x_axis, int *y_axis,
                      float *x_thresh, float *y_thresh) {
  *usb_pref_dst = usb_pref[device];
  *x_axis = usb_x_axis[device];
  *y_axis = usb_y_axis[device];
  *x_thresh = usb_x_thresh[device];
  *y_thresh = usb_y_thresh[device];
}

// KEEP in sync with kernel.cpp, kbd.c, menu_usb.c
static void set_hotkey_choices(struct menu_item *item) {
  item->num_choices = 16;
  strcpy(item->choices[HOTKEY_CHOICE_NONE], function_to_string(BTN_ASSIGN_UNDEF));
  strcpy(item->choices[HOTKEY_CHOICE_MENU], function_to_string(BTN_ASSIGN_MENU));
  strcpy(item->choices[HOTKEY_CHOICE_WARP], function_to_string(BTN_ASSIGN_WARP));
  strcpy(item->choices[HOTKEY_CHOICE_STATUS_TOGGLE], function_to_string(BTN_ASSIGN_STATUS_TOGGLE));
  strcpy(item->choices[HOTKEY_CHOICE_SWAP_PORTS], function_to_string(BTN_ASSIGN_SWAP_PORTS));
  strcpy(item->choices[HOTKEY_CHOICE_TAPE_MENU], function_to_string(BTN_ASSIGN_TAPE_MENU));
  strcpy(item->choices[HOTKEY_CHOICE_CART_MENU], function_to_string(BTN_ASSIGN_CART_MENU));
  strcpy(item->choices[HOTKEY_CHOICE_CART_FREEZE], function_to_string(BTN_ASSIGN_CART_FREEZE));
  strcpy(item->choices[HOTKEY_CHOICE_RESET_MENU], function_to_string(BTN_ASSIGN_RESET_MENU));
  strcpy(item->choices[HOTKEY_CHOICE_RESET_HARD], function_to_string(BTN_ASSIGN_RESET_HARD));
  strcpy(item->choices[HOTKEY_CHOICE_RESET_SOFT], function_to_string(BTN_ASSIGN_RESET_SOFT));
  strcpy(item->choices[HOTKEY_CHOICE_ACTIVE_DISPLAY], function_to_string(BTN_ASSIGN_ACTIVE_DISPLAY));
  strcpy(item->choices[HOTKEY_CHOICE_PIP_LOCATION], function_to_string(BTN_ASSIGN_PIP_LOCATION));
  strcpy(item->choices[HOTKEY_CHOICE_PIP_SWAP], function_to_string(BTN_ASSIGN_PIP_SWAP));
  strcpy(item->choices[HOTKEY_CHOICE_40_80_COLUMN], function_to_string(BTN_ASSIGN_40_80_COLUMN));
  strcpy(item->choices[HOTKEY_CHOICE_FLUSH_DISK], function_to_string(BTN_ASSIGN_FLUSH_DISK));
  item->choice_ints[HOTKEY_CHOICE_NONE] = BTN_ASSIGN_UNDEF;
  item->choice_ints[HOTKEY_CHOICE_MENU] = BTN_ASSIGN_MENU;
  item->choice_ints[HOTKEY_CHOICE_WARP] = BTN_ASSIGN_WARP;
  item->choice_ints[HOTKEY_CHOICE_STATUS_TOGGLE] = BTN_ASSIGN_STATUS_TOGGLE;
  item->choice_ints[HOTKEY_CHOICE_SWAP_PORTS] = BTN_ASSIGN_SWAP_PORTS;
  item->choice_ints[HOTKEY_CHOICE_TAPE_MENU] = BTN_ASSIGN_TAPE_MENU;
  item->choice_ints[HOTKEY_CHOICE_CART_MENU] = BTN_ASSIGN_CART_MENU;
  item->choice_ints[HOTKEY_CHOICE_CART_FREEZE] = BTN_ASSIGN_CART_FREEZE;
  item->choice_ints[HOTKEY_CHOICE_RESET_MENU] = BTN_ASSIGN_RESET_MENU;
  item->choice_ints[HOTKEY_CHOICE_RESET_HARD] = BTN_ASSIGN_RESET_HARD;
  item->choice_ints[HOTKEY_CHOICE_RESET_SOFT] = BTN_ASSIGN_RESET_SOFT;
  item->choice_ints[HOTKEY_CHOICE_ACTIVE_DISPLAY] = BTN_ASSIGN_ACTIVE_DISPLAY;
  item->choice_ints[HOTKEY_CHOICE_PIP_LOCATION] = BTN_ASSIGN_PIP_LOCATION;
  item->choice_ints[HOTKEY_CHOICE_PIP_SWAP] = BTN_ASSIGN_PIP_SWAP;
  item->choice_ints[HOTKEY_CHOICE_40_80_COLUMN] = BTN_ASSIGN_40_80_COLUMN;
  item->choice_ints[HOTKEY_CHOICE_FLUSH_DISK] = BTN_ASSIGN_FLUSH_DISK;

  if (emux_machine_class == BMC64_MACHINE_CLASS_VIC20) {
     item->choice_disabled[HOTKEY_CHOICE_SWAP_PORTS] = 1;
  }

  if (emux_machine_class != BMC64_MACHINE_CLASS_C64 &&
      emux_machine_class != BMC64_MACHINE_CLASS_C128) {
     item->choice_disabled[HOTKEY_CHOICE_CART_FREEZE] = 1;
  }

  if (emux_machine_class != BMC64_MACHINE_CLASS_C128) {
     item->choice_disabled[HOTKEY_CHOICE_ACTIVE_DISPLAY] = 1;
     item->choice_disabled[HOTKEY_CHOICE_PIP_LOCATION] = 1;
     item->choice_disabled[HOTKEY_CHOICE_PIP_SWAP] = 1;
     item->choice_disabled[HOTKEY_CHOICE_40_80_COLUMN] = 1;
  }
}

static void menu_build_machine_switch(struct menu_item* parent) {
  struct menu_item* holder = ui_menu_add_folder(parent, "Switch");

  struct menu_item* vic20_r = ui_menu_add_folder(holder, "VIC20");
  struct menu_item* c64_r = ui_menu_add_folder(holder, "C64");
  struct menu_item* c128_r = ui_menu_add_folder(holder, "C128");
  struct menu_item* plus4_r = ui_menu_add_folder(holder, "Plus/4");
  struct menu_item* pet_r = ui_menu_add_folder(holder, "PET");

  struct machine_entry* head;
  load_machines(&head);

  struct machine_entry* ptr = head;
  struct menu_item* item;
  while (ptr) {
    switch (ptr->class) {
      case BMC64_MACHINE_CLASS_VIC20:
         item = ui_menu_add_button(MENU_SWITCH_MACHINE, vic20_r, ptr->desc);
         break;
      case BMC64_MACHINE_CLASS_C64:
         item = ui_menu_add_button(MENU_SWITCH_MACHINE, c64_r, ptr->desc);
         break;
      case BMC64_MACHINE_CLASS_C128:
         item = ui_menu_add_button(MENU_SWITCH_MACHINE, c128_r, ptr->desc);
         break;
      case BMC64_MACHINE_CLASS_PLUS4:
         item = ui_menu_add_button(MENU_SWITCH_MACHINE, plus4_r, ptr->desc);
         break;
      case BMC64_MACHINE_CLASS_PLUS4EMU:
         if (circle_get_model() >= 3) {
            item = ui_menu_add_button(MENU_SWITCH_MACHINE, plus4_r, ptr->desc);
         } else {
            item = NULL;
         }
         break;
      case BMC64_MACHINE_CLASS_PET:
         item = ui_menu_add_button(MENU_SWITCH_MACHINE, pet_r, ptr->desc);
         break;
      default:
         item = NULL;
         break;
    }

    if (item) {
       item->value = ptr->id;
    }

    ptr=ptr->next;
  }

  free_machines(head);
}

struct menu_item* add_joyport_options(struct menu_item* parent, int port) {
  int menu_id;
  switch (port) {
     case 1:
       menu_id = MENU_JOYSTICK_PORT_1;
       break;
     case 2:
       menu_id = MENU_JOYSTICK_PORT_2;
       break;
     case 3:
       menu_id = MENU_JOYSTICK_PORT_3;
       break;
     case 4:
       menu_id = MENU_JOYSTICK_PORT_4;
       break;
     default:
       assert(0);
  }

  struct menu_item* child = ui_menu_add_multiple_choice(
      menu_id, parent, "");
  sprintf (child->name, "Port %d", port);
  child->num_choices = 14;
  child->value = 0;
  strcpy(child->choices[0], "None");
  child->choice_ints[0] = JOYDEV_NONE;
  strcpy(child->choices[1], "USB Gamepad 1");
  child->choice_ints[1] = JOYDEV_USB_0;
  strcpy(child->choices[2], "USB Gamepad 2");
  child->choice_ints[2] = JOYDEV_USB_1;
  strcpy(child->choices[3], "GPIO Bank 1");
  child->choice_ints[3] = JOYDEV_GPIO_0;
  strcpy(child->choices[4], "GPIO Bank 2");
  child->choice_ints[4] = JOYDEV_GPIO_1;
  strcpy(child->choices[5], "CURS + SPACE");
  child->choice_ints[5] = JOYDEV_CURS_SP;
  strcpy(child->choices[6], "NUMPAD 64825");
  child->choice_ints[6] = JOYDEV_NUMS_1;
  strcpy(child->choices[7], "NUMPAD 17930");
  child->choice_ints[7] = JOYDEV_NUMS_2;
  strcpy(child->choices[8], "CURS + LCTRL");
  child->choice_ints[8] = JOYDEV_CURS_LC;
  strcpy(child->choices[9], "USB Mouse (1351)");
  child->choice_ints[9] = JOYDEV_MOUSE;
  strcpy(child->choices[10], "Custom Keyset 1");
  child->choice_ints[10] = JOYDEV_KEYSET1;
  strcpy(child->choices[11], "Custom Keyset 2");
  child->choice_ints[11] = JOYDEV_KEYSET2;
  strcpy(child->choices[12], "USB Gamepad 3");
  child->choice_ints[12] = JOYDEV_USB_2;
  strcpy(child->choices[13], "USB Gamepad 4");
  child->choice_ints[13] = JOYDEV_USB_3;

  if (emux_machine_class == BMC64_MACHINE_CLASS_PLUS4EMU || port > 2) {
     child->choice_disabled[9] = 1;
  }
  return child;
}

void build_menu(struct menu_item *root) {
  struct menu_item *parent;
  struct menu_item *video_parent;
  struct menu_item *drive_parent;
  struct menu_item *machine_parent;
  struct menu_item *tape_parent;
  struct menu_item *child;
  int dev;
  int i;
  int j;
  int k;
  int tmp;

  for (int k = 0; k < MAX_USB_DEVICES; k++) {
     sprintf (usb_btn_name[k], "usb_btn_%d", k);
     sprintf (usb_pref_name[k], "usb_%d", k);
     sprintf (usb_x_name[k], "usb_x_%d", k);
     sprintf (usb_y_name[k], "usb_y_%d", k);
     sprintf (usb_x_t_name[k], "usb_x_t_%d", k);
     sprintf (usb_y_t_name[k], "usb_y_t_%d", k);
  }

  attached_disk_name[0][0] = '\0';
  attached_disk_name[1][0] = '\0';
  attached_disk_name[2][0] = '\0';
  attached_disk_name[3][0] = '\0';

  emux_load_additional_settings();

  // TODO: This doesn't really belong here. Need to sort
  // out init order of structs.
  for (dev = 0; dev < MAX_JOY_PORTS; dev++) {
    memset(&joydevs[dev], 0, sizeof(struct joydev_config));
    joydevs[dev].port = dev + 1;
    joydevs[dev].device = JOYDEV_NONE;
  }

  strcpy(current_volume_name, default_volume_name);

  if (emux_machine_class == BMC64_MACHINE_CLASS_PLUS4EMU) {
     strcpy(snap_filt_ext[0],".p4s");
  } else {
     strcpy(snap_filt_ext[0],".vsf");
  }

  char machine_info_txt[64];
  machine_info_txt[0] = '\0';

  switch (emux_machine_class) {
  case BMC64_MACHINE_CLASS_C64:
    strcat(machine_info_txt,"C64 ");
    strcpy(machine_sub_dir, "/C64");
    break;
  case BMC64_MACHINE_CLASS_C128:
    strcat(machine_info_txt,"C128 ");
    strcpy(machine_sub_dir, "/C128");
    break;
  case BMC64_MACHINE_CLASS_VIC20:
    strcat(machine_info_txt,"VIC20 ");
    strcpy(machine_sub_dir, "/VIC20");
    break;
  case BMC64_MACHINE_CLASS_PLUS4:
  case BMC64_MACHINE_CLASS_PLUS4EMU:
    strcat(machine_info_txt,"PLUS/4 ");
    strcpy(machine_sub_dir, "/PLUS4");
    break;
  case BMC64_MACHINE_CLASS_PET:
    strcat(machine_info_txt,"PET ");
    strcpy(machine_sub_dir, "/PET");
    break;
  default:
    strcat(machine_info_txt,"??? ");
    strcpy(machine_sub_dir, "/");
    break;
  }


  char scratch[16];
  switch (circle_get_machine_timing()) {
  case MACHINE_TIMING_NTSC_HDMI:
    strcat(machine_info_txt, "NTSC 60Hz HDMI");
    break;
  case MACHINE_TIMING_NTSC_DPI:
    strcat(machine_info_txt, "NTSC 60Hz DPI");
    break;
  case MACHINE_TIMING_NTSC_COMPOSITE:
  case MACHINE_TIMING_NTSC_CUSTOM_HDMI:
  case MACHINE_TIMING_NTSC_CUSTOM_DPI:
    strcat(machine_info_txt, "NTSC ");
    sprintf (scratch,"%.3f", emux_calculate_fps());
    strcat (machine_info_txt, scratch);
    strcat(machine_info_txt, "Hz ");
    switch (circle_get_machine_timing()) {
      case MACHINE_TIMING_NTSC_COMPOSITE:
        strcat(machine_info_txt, "Composite");
        break;
      case MACHINE_TIMING_NTSC_CUSTOM_HDMI:
        strcat(machine_info_txt, "Custom HDMI");
        break;
      case MACHINE_TIMING_NTSC_CUSTOM_DPI:
        strcat(machine_info_txt, "Custom DPI");
        break;
      default:
        break;
    }
    break;
  case MACHINE_TIMING_PAL_HDMI:
    strcat(machine_info_txt, "PAL 50Hz HDMI");
    break;
  case MACHINE_TIMING_PAL_DPI:
    strcat(machine_info_txt, "PAL 50Hz DPI");
    break;
  case MACHINE_TIMING_PAL_COMPOSITE:
  case MACHINE_TIMING_PAL_CUSTOM_HDMI:
  case MACHINE_TIMING_PAL_CUSTOM_DPI:
    strcat(machine_info_txt, "PAL ");
    sprintf (scratch,"%.3f", emux_calculate_fps());
    strcat (machine_info_txt, scratch);
    strcat(machine_info_txt, "Hz ");
    switch (circle_get_machine_timing()) {
      case MACHINE_TIMING_PAL_COMPOSITE:
        strcat(machine_info_txt, "Composite");
        break;
      case MACHINE_TIMING_PAL_CUSTOM_HDMI:
        strcat(machine_info_txt, "Custom HDMI");
        break;
      case MACHINE_TIMING_PAL_CUSTOM_DPI:
        strcat(machine_info_txt, "Custom DPI");
        break;
      default:
        break;
    }
    break;
  default:
    strcat(machine_info_txt, "Error");
    break;
  }

  ui_menu_add_button(MENU_TEXT, root, machine_info_txt);

  ui_menu_add_button(MENU_ABOUT, root, "About...");
  ui_menu_add_button(MENU_LICENSE, root, "License...");

  ui_menu_add_divider(root);

  switch (emux_machine_class) {
    case BMC64_MACHINE_CLASS_PLUS4EMU:
     ui_menu_add_button(MENU_LOADPRG, root, "Load .PRG File...");
     break;
    case BMC64_MACHINE_CLASS_PET:
     break;
    default:
     ui_menu_add_button(MENU_AUTOSTART, root, "Autostart Prg/Disk...");
     emux_get_int(Setting_AutostartWarp, &tmp);
     ui_menu_add_toggle(MENU_AUTOSTART_WARP, root, "Autostart Warp", tmp);
     break;
  }

  machine_parent = ui_menu_add_folder(root, "Machine");
    emux_add_machine_options(machine_parent);
    menu_build_machine_switch(machine_parent);

  drive_parent = ui_menu_add_folder(root, "Drives");
    // (-1) Options applicable to all drives
    emux_add_drive_option(drive_parent, -1);

    parent = ui_menu_add_folder(drive_parent, "Drive 8");
    ui_menu_add_button(MENU_ATTACH_DISK_8, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_8, parent, "Detach Disk");
    if (emux_machine_class != BMC64_MACHINE_CLASS_VIC20 && emux_machine_class != BMC64_MACHINE_CLASS_PET) {
     emux_get_int_1(Setting_IECDeviceN, &tmp, 8);
     ui_menu_add_toggle(MENU_IECDEVICE_8, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_8, parent, "Select IEC Dir...");
    }
    emux_add_drive_option(parent, 8);

    if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_8, parent, "Change Model...");
    }

  // More than 1 drive costs too much. Limit to drive 8.
  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
    parent = ui_menu_add_folder(drive_parent, "Drive 9");
    ui_menu_add_button(MENU_ATTACH_DISK_9, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_9, parent, "Detach Disk");
    if (emux_machine_class != BMC64_MACHINE_CLASS_VIC20 && emux_machine_class != BMC64_MACHINE_CLASS_PET) {
     emux_get_int_1(Setting_IECDeviceN, &tmp, 9);
     ui_menu_add_toggle(MENU_IECDEVICE_9, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_9, parent, "Select IEC Dir...");
    }
    emux_add_drive_option(parent, 9);

    if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_9, parent, "Change Model...");
    }

    parent = ui_menu_add_folder(drive_parent, "Drive 10");
    ui_menu_add_button(MENU_ATTACH_DISK_10, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_10, parent, "Detach Disk");
    if (emux_machine_class != BMC64_MACHINE_CLASS_VIC20 && emux_machine_class != BMC64_MACHINE_CLASS_PET) {
     emux_get_int_1(Setting_IECDeviceN, &tmp, 10);
     ui_menu_add_toggle(MENU_IECDEVICE_10, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_10, parent, "Select IEC Dir...");
    }
    emux_add_drive_option(parent, 10);
    if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_10, parent, "Change Model...");
    }

    parent = ui_menu_add_folder(drive_parent, "Drive 11");
    ui_menu_add_button(MENU_ATTACH_DISK_11, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_11, parent, "Detach Disk");
    if (emux_machine_class != BMC64_MACHINE_CLASS_VIC20 && emux_machine_class != BMC64_MACHINE_CLASS_PET) {
     emux_get_int_1(Setting_IECDeviceN, &tmp, 11);
     ui_menu_add_toggle(MENU_IECDEVICE_11, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_11, parent, "Select IEC Dir...");
    }
    emux_add_drive_option(parent, 11);
    if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_11, parent, "Change Model...");
    }
  }

  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
    ui_menu_add_button(MENU_DRIVE_CHANGE_ROM, drive_parent, "Change ROM...");
  }

  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
    parent = ui_menu_add_folder(drive_parent, "Create empty Disk");
      ui_menu_add_button(MENU_CREATE_D64, parent, "D64...");
      ui_menu_add_button(MENU_CREATE_D67, parent, "D67...");
      ui_menu_add_button(MENU_CREATE_D71, parent, "D71...");
      ui_menu_add_button(MENU_CREATE_D80, parent, "D80...");
      ui_menu_add_button(MENU_CREATE_D81, parent, "D81...");
      ui_menu_add_button(MENU_CREATE_D82, parent, "D82...");
      ui_menu_add_button(MENU_CREATE_D1M, parent, "D1M...");
      ui_menu_add_button(MENU_CREATE_D2M, parent, "D2M...");
      ui_menu_add_button(MENU_CREATE_D4M, parent, "D4M...");
      ui_menu_add_button(MENU_CREATE_G64, parent, "G64...");
      ui_menu_add_button(MENU_CREATE_G71, parent, "G71...");
      ui_menu_add_button(MENU_CREATE_P64, parent, "P64...");
      ui_menu_add_button(MENU_CREATE_X64, parent, "X64...");
      //ui_menu_add_button(MENU_CREATE_DHD, parent, "DHD..."); // VICE doesn't do this
  }

  parent = emux_add_cartridge_options(root);

  parent = ui_menu_add_folder(root, "Tape");

    ui_menu_add_button(MENU_ATTACH_TAPE, parent, "Attach tape image...");
    ui_menu_add_button(MENU_DETACH_TAPE, parent, "Detach tape image");

    tape_parent = ui_menu_add_folder(parent, "Datasette controls (.tap)...");
    ui_menu_add_button(MENU_TAPE_START, tape_parent, "Play");
    ui_menu_add_button(MENU_TAPE_STOP, tape_parent, "Stop");
    ui_menu_add_button(MENU_TAPE_REWIND, tape_parent, "Rewind");
    ui_menu_add_button(MENU_TAPE_FASTFWD, tape_parent, "FastFwd");
    ui_menu_add_button(MENU_TAPE_RECORD, tape_parent, "Record");
    ui_menu_add_button(MENU_TAPE_RESET, tape_parent, "Reset");
    ui_menu_add_button(MENU_TAPE_RESET_COUNTER, tape_parent, "Reset Counter");
    emux_get_int(Setting_DatasetteResetWithCPU, &tmp);
    tape_reset_with_machine_item =
      ui_menu_add_toggle(MENU_TAPE_RESET_WITH_MACHINE, tape_parent,
                         "Reset Tape with Machine Reset", tmp);
    emux_add_tape_options(tape_parent);

    ui_menu_add_button(MENU_CREATE_TAP, parent, "Create empty Tape...");

  ui_menu_add_divider(root);

  // TODO: Load/Save snapshot on PET is crashy. Figure out if upstream
  // has fixed this.
  if (emux_machine_class != BMC64_MACHINE_CLASS_PET) {
     parent = ui_menu_add_folder(root, "Snapshots");
     ui_menu_add_button(MENU_LOAD_SNAP, parent, "Load Snapshot...");
     ui_menu_add_button(MENU_SAVE_SNAP, parent, "Save Snapshot...");
  }

  video_parent = parent = ui_menu_add_folder(root, "Video");

  scaling_interp_item = ui_menu_add_toggle_labels(
     MENU_SCALING_INTERPOLATION, parent,
        "Scaling Interpolation", 1, "Off", "On");

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     // For C128, we split video options under video into VICII
     // and VDC submenus since there are two displays.  Otherwise,
     // when there is only one display, everything falls under
     // video directly.
     active_display_item = child =
        ui_menu_add_multiple_choice(MENU_ACTIVE_DISPLAY, parent,
           "Active Display");
     child->num_choices = 4;
     child->value = MENU_ACTIVE_DISPLAY_VICII;
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_VICII], "VICII");
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_VDC], "VDC");
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE], "Side-By-Side");
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_PIP], "PIP");
     // Someday, we can add "Both" as an option for Pi4?

     pip_location_item = child =
        ui_menu_add_multiple_choice(MENU_PIP_LOCATION, parent,
           "PIP Location");
     child->num_choices = 4;
     child->value = MENU_PIP_TOP_RIGHT;
     strcpy(child->choices[MENU_PIP_TOP_LEFT], "Top Left");
     strcpy(child->choices[MENU_PIP_TOP_RIGHT], "Top Right");
     strcpy(child->choices[MENU_PIP_BOTTOM_RIGHT], "Bottom Right");
     strcpy(child->choices[MENU_PIP_BOTTOM_LEFT], "Bottom Left");

     pip_swapped_item =
        ui_menu_add_toggle(MENU_PIP_SWAPPED, parent, "Swap PIP", 0);

     parent = ui_menu_add_folder(video_parent, "VICII");
  }

  use_scaling_params_item[0] = ui_menu_add_toggle_labels(
     MENU_USE_SCALING_PARAMS_0, parent, "Apply scaling params at boot", 1,
        "No","Yes");

  struct menu_item *shader = ui_menu_add_folder(parent, "CRT Shader");

     int crt_filter;
     emux_get_int(Setting_VideoFilter, &crt_filter);
     s_enable_shader_item =
        ui_menu_add_toggle_labels(MENU_SHADER_ENABLE, shader,
           "Enable CRT Shader?", crt_filter != MENU_VIDEO_FILTER_NONE, "No", "Yes");

     if (!allow_shader()) {
        s_enable_shader_item->value = 0;
        s_enable_shader_item->disabled = 1;
        strcpy (s_enable_shader_item->custom_toggle_label[0], "Disabled");
        strcpy (s_enable_shader_item->custom_toggle_label[1], "Disabled");
     }

     s_curvature_item =
       ui_menu_add_toggle(MENU_SHADER_CURVATURE, shader, "Curvature", 0);

     s_curvature_x_item =
       ui_menu_add_range(MENU_SHADER_CURVATURE_X, shader, "H Curvature Amount",
          0, 30, 1, 10);

     s_curvature_y_item =
       ui_menu_add_range(MENU_SHADER_CURVATURE_Y, shader, "V Curvature Amount",
          0, 30, 1, 15);

     s_sharper_item = ui_menu_add_toggle(
        MENU_SHADER_SHARPER, shader, "Sharper Horizontal Blend", 0);

     s_mask_item = ui_menu_add_multiple_choice(
        MENU_SHADER_MASK, shader, "Mask Type");
     s_mask_item->num_choices = 3;
     s_mask_item->value = 0;
     strcpy(s_mask_item->choices[0], "None");
     strcpy(s_mask_item->choices[1], "Green/Magenta");
     strcpy(s_mask_item->choices[2], "Trinitron");

     s_mask_brightness_item = ui_menu_add_range(
        MENU_SHADER_MASK_BRIGHTNESS, shader, "Mask Brightness",
           0, 100, 1, 70);

     s_scanlines_item =
        ui_menu_add_toggle(MENU_SHADER_SCANLINES, shader, "Scanlines", 1);

     s_scanline_weight_item =
        ui_menu_add_range(
           MENU_SHADER_SCANLINE_WEIGHT, shader, "Scanline Weight",
              0, 150, 1, 60);

     s_scanline_gap_brightness_item = ui_menu_add_range(
        MENU_SHADER_SCANLINE_GAP_BRIGHTNESS, shader, "Scanline Gap Brightness",
           0, 100, 1, 12);

     s_multisample_item =
        ui_menu_add_toggle(MENU_SHADER_MULTISAMPLE, shader, "Multisample", 1);

     s_bloom_factor_item = ui_menu_add_range(
        MENU_SHADER_BLOOM, shader, "Bloom Factor",
           0, 500, 10, 150);

     s_gamma_item =
        ui_menu_add_multiple_choice(MENU_SHADER_GAMMA, shader, "Gamma Correction");
     s_gamma_item->num_choices = 3;
     s_gamma_item->value = 2;
     strcpy(s_gamma_item->choices[0], "Off");
     strcpy(s_gamma_item->choices[1], "On");
     strcpy(s_gamma_item->choices[2], "Fake (Fast)");

     s_input_gamma_item = ui_menu_add_range(
        MENU_SHADER_INPUT_GAMMA, shader, "Input Gamma",
           0, 500, 10, 240);

     s_output_gamma_item = ui_menu_add_range(
        MENU_SHADER_OUTPUT_GAMMA, shader, "Output Gamma",
           0, 500, 10, 220);

     ui_menu_add_button(MENU_SHADER_RESET_ALL, shader, "Reset");

  palette_item[0] = emux_add_palette_options(MENU_COLOR_PALETTE_0, parent);

  child = ui_menu_add_folder(parent, "Color Adjustments...");

  brightness_item[0] =
      ui_menu_add_range(MENU_COLOR_BRIGHTNESS_0, child, "Brightness",
         0, 2000,
            10, emux_get_color_brightness(0));
  contrast_item[0] =
      ui_menu_add_range(MENU_COLOR_CONTRAST_0, child, "Contrast",
         0, 2000,
            10, emux_get_color_contrast(0));
  gamma_item[0] =
      ui_menu_add_range(MENU_COLOR_GAMMA_0, child, "Gamma",
         0, 4000,
            10, emux_get_color_gamma(0));
  tint_item[0] =
      ui_menu_add_range(MENU_COLOR_TINT_0, child, "Tint",
         0, 2000,
            10, emux_get_color_tint(0));
  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
     saturation_item[0] =
         ui_menu_add_range(MENU_COLOR_SATURATION_0, child, "Saturation",
            0, 2000,
               10, emux_get_color_saturation(0));
  } else {
     saturation_item[0] = (struct menu_item *)malloc(sizeof(struct menu_item));
     memset(saturation_item[0], 0, sizeof(struct menu_item));
  }

  ui_menu_add_button(MENU_COLOR_RESET_0, child, "Reset");

  int defaultHStretch;
  int defaultVStretch;
  if (emux_machine_class == BMC64_MACHINE_CLASS_VIC20) {
     defaultHStretch = DEFAULT_VIC_H_STRETCH;
     defaultVStretch = DEFAULT_VIC_V_STRETCH;
  } else {
     defaultHStretch = DEFAULT_VICII_H_STRETCH;
     defaultVStretch = DEFAULT_VICII_V_STRETCH;
  }

  h_center_item[0] =
      ui_menu_add_range(MENU_H_CENTER_0, parent, "H Center",
          -48, 48, 1, 0);
  v_center_item[0] =
      ui_menu_add_range(MENU_V_CENTER_0, parent, "V Center",
          -48, 48, 1, 0);
  h_border_item[0] =
      ui_menu_add_range(MENU_H_BORDER_0, parent, "H Border (px)",
          0, canvas_state[VIC_INDEX].max_border_w,
             1, canvas_state[VIC_INDEX].max_border_w);
  v_border_item[0] =
      ui_menu_add_range(MENU_V_BORDER_0, parent, "V Border (px)",
          0, canvas_state[VIC_INDEX].max_border_h,
             1, canvas_state[VIC_INDEX].max_border_h);
  child = h_stretch_item[0] =
      ui_menu_add_range(MENU_H_STRETCH_0, parent, "H Stretch Factor",
           500, canvas_state[VIC_INDEX].max_stretch_h ?
              canvas_state[VIC_INDEX].max_stretch_h : 1800,
                 5, defaultHStretch);
  child->divisor = 1000;
  child = v_stretch_item[0] =
      ui_menu_add_range(MENU_V_STRETCH_0, parent, "V Stretch Factor",
           500, 1000, 5, defaultVStretch);
  child->divisor = 1000;

  ui_menu_add_button(MENU_INTEGER_SCALE_W_0, parent, "Next H Integer Scale");
  ui_menu_add_button(MENU_INTEGER_SCALE_H_0, parent, "Next V Integer Scale");

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     parent = ui_menu_add_folder(video_parent, "VDC");

     use_scaling_params_item[1] = ui_menu_add_toggle_labels(
        MENU_USE_SCALING_PARAMS_1, parent, "Apply scaling params at boot", 1,
           "No","Yes");

     palette_item[1] = emux_add_palette_options(MENU_COLOR_PALETTE_1, parent);

     child = ui_menu_add_folder(parent, "Color Adjustments...");

     brightness_item[1] =
         ui_menu_add_range(MENU_COLOR_BRIGHTNESS_1, child, "Brightness",
            0, 2000,
               10, emux_get_color_brightness(1));
     contrast_item[1] =
         ui_menu_add_range(MENU_COLOR_CONTRAST_1, child, "Contrast",
            0, 2000,
               10, emux_get_color_contrast(1));
     gamma_item[1] =
         ui_menu_add_range(MENU_COLOR_GAMMA_1, child, "Gamma",
            0, 4000,
               10, emux_get_color_gamma(1));
     tint_item[1] =
         ui_menu_add_range(MENU_COLOR_TINT_1, child, "Tint",
            0, 2000,
               10, emux_get_color_tint(1));

     if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
        saturation_item[1] =
            ui_menu_add_range(MENU_COLOR_SATURATION_1, child, "Saturation",
               0, 2000,
                  10, emux_get_color_saturation(1));
     } else {
        saturation_item[1] = (struct menu_item *)malloc(sizeof(struct menu_item));
        memset(saturation_item[1], 0, sizeof(struct menu_item));
     }

     ui_menu_add_button(MENU_COLOR_RESET_1, child, "Reset");

     h_center_item[1] =
         ui_menu_add_range(MENU_H_CENTER_1, parent, "H Center",
             -48, 48, 1, 0);
     v_center_item[1] =
         ui_menu_add_range(MENU_V_CENTER_1, parent, "V Center",
             -48, 48, 1, 0);
     h_border_item[1] =
         ui_menu_add_range(MENU_H_BORDER_1, parent, "H Border (px)",
             0, canvas_state[VDC_INDEX].max_border_w,
                1, canvas_state[VDC_INDEX].max_border_w);
     v_border_item[1] =
         ui_menu_add_range(MENU_V_BORDER_1, parent, "V Border (px)",
             0, canvas_state[VDC_INDEX].max_border_h,
                1, canvas_state[VDC_INDEX].max_border_h);
     child = h_stretch_item[1] =
         ui_menu_add_range(MENU_H_STRETCH_1, parent, "H Stretch Factor",
              500, canvas_state[VDC_INDEX].max_stretch_h ?
                 canvas_state[VDC_INDEX].max_stretch_h : 1800,
                    5, DEFAULT_VDC_H_STRETCH);
     child->divisor = 1000;
     child = v_stretch_item[1] =
         ui_menu_add_range(MENU_V_STRETCH_1, parent, "V Stretch Factor",
              500, 1000, 5, DEFAULT_VDC_V_STRETCH);
     child->divisor = 1000;

     ui_menu_add_button(MENU_INTEGER_SCALE_W_1, parent, "Next H Integer Scale");
     ui_menu_add_button(MENU_INTEGER_SCALE_H_1, parent, "Next V Integer Scale");
  }

  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
     ui_menu_add_button(MENU_CALC_TIMING, video_parent,
                     "Custom HDMI/DPI mode timing calc...");
  }

  parent = ui_menu_add_folder(root, "Sound");

  volume_item = ui_menu_add_range(MENU_VOLUME, parent,
      "Volume ", 0, 100, 1, 100);

  emux_add_sound_options(parent);

  parent = ui_menu_add_folder(root, "Keyboard");

  emux_add_keyboard_options(parent);

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     c40_80_column_item = ui_menu_add_toggle_labels(
        MENU_40_80_COLUMN, parent, "40/80 Column", 1 /* default 40 col */,
        "Down","Up");
  }

  child = hotkey_cf1_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_CF1, parent, "C= + F1 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_cf1_item);
  child = hotkey_cf3_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_CF3, parent, "C= + F3 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_cf3_item);
  child = hotkey_cf5_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_CF5, parent, "C= + F5 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_cf5_item);
  child = hotkey_cf7_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_CF7, parent, "C= + F7 Hotkey");
  child->value = HOTKEY_CHOICE_MENU;
  set_hotkey_choices(hotkey_cf7_item);
  child = hotkey_tf1_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_TF1, parent,
         "CTRL + F1 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_tf1_item);
  child = hotkey_tf3_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_TF3, parent,
         "CTRL + F3 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_tf3_item);
  child = hotkey_tf5_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_TF5, parent,
         "CTRL + F5 Hotkey");
  child->value = HOTKEY_CHOICE_NONE;
  set_hotkey_choices(hotkey_tf5_item);
  child = hotkey_tf7_item =
      ui_menu_add_multiple_choice(MENU_HOTKEY_TF7, parent,
         "CTRL + F7 Hotkey");
  child->value = HOTKEY_CHOICE_MENU;
  set_hotkey_choices(hotkey_tf7_item);

  parent = ui_menu_add_folder(root, "Joyports");

  if (emu_get_num_joysticks() > 1) {
      ui_menu_add_button(MENU_SWAP_JOYSTICKS, parent, "Swap Joystick Ports");
  }

  port_1_menu_item = NULL;
  if (emu_get_num_joysticks() > 0) {
    port_1_menu_item = add_joyport_options(parent, 1);
  }
  port_2_menu_item = NULL;
  if (emu_get_num_joysticks() > 1) {
    port_2_menu_item = add_joyport_options(parent, 2);
  }
  port_3_menu_item = NULL;
  port_4_menu_item = NULL;

  emux_add_userport_joys(parent);

  ui_menu_add_button(MENU_USB_0_CONFIGURE, parent, "Configure USB Gamepad 1...");
  ui_menu_add_button(MENU_USB_1_CONFIGURE, parent, "Configure USB Gamepad 2...");
  ui_menu_add_button(MENU_USB_2_CONFIGURE, parent, "Configure USB Gamepad 3...");
  ui_menu_add_button(MENU_USB_3_CONFIGURE, parent, "Configure USB Gamepad 4...");

  for (int k = 0; k < MAX_USB_DEVICES; k++) {
    usb_pref[k] = 0;
    usb_x_axis[k] = 0;
    usb_y_axis[k] = 1;
    usb_x_thresh[k] = .50;
    usb_y_thresh[k] = .50;
  }

  for (j = 0; j < MAX_USB_BUTTONS; j++) {
    for (k = 0; k < MAX_USB_DEVICES; k++) {
      usb_button_assignments[k][j] = (j == 0 ? BTN_ASSIGN_FIRE : BTN_ASSIGN_UNDEF);
    }
    usb_button_bits[j] = 1 << j;
  }

  ui_menu_add_button(MENU_CONFIGURE_KEYSET1, parent, "Configure Keyset 1...");
  ui_menu_add_button(MENU_CONFIGURE_KEYSET2, parent, "Configure Keyset 2...");

  parent = ui_menu_add_folder(root, "GPIO");

  child = gpio_config_item =
      ui_menu_add_multiple_choice(MENU_GPIO_CONFIG, parent, "Config");
     child->num_choices = 6;
     child->value = 0;
     strcpy(child->choices[0], "Disabled");
     strcpy(child->choices[1], "#1 (Nav+Joy)");
     strcpy(child->choices[2], "#2 (Kyb+Joy)");
     strcpy(child->choices[3], "#3 (Waveshare Hat)");
     if (circle_gpio_outputs_enabled() &&
         emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU &&
         emux_machine_class != BMC64_MACHINE_CLASS_PLUS4) {
        strcpy(child->choices[4], "#4 (Userport+Joy)");
     } else {
        strcpy(child->choices[4], "#4 (N/A)");
     }
     strcpy(child->choices[5], "#5 (Custom)");
     child->choice_ints[0] = GPIO_CONFIG_DISABLED;
     child->choice_ints[1] = GPIO_CONFIG_NAV_JOY;
     child->choice_ints[2] = GPIO_CONFIG_KYB_JOY;
     child->choice_ints[3] = GPIO_CONFIG_WAVESHARE;
     child->choice_ints[4] = GPIO_CONFIG_USERPORT;
     child->choice_ints[5] = GPIO_CONFIG_CUSTOM;

     if (!circle_gpio_enabled()) {
        child->choice_disabled[1] = 1;
        child->choice_disabled[2] = 1;
        child->choice_disabled[3] = 1;
        child->choice_disabled[4] = 1;
        child->choice_disabled[5] = 1;
     }

     if (circle_gpio_enabled()) {
        ui_menu_add_button(MENU_CONFIGURE_GPIO,
                        parent, "Configure Custom GPIO...");
     }

  ui_menu_add_divider(root);

  parent = ui_menu_add_folder(root, "Prefs");

  if (emux_machine_class != BMC64_MACHINE_CLASS_PLUS4EMU) {
    drive_sounds_item = ui_menu_add_toggle(MENU_DRIVE_SOUND_EMULATION, parent,
                                         "Drive sound emulation", 0);
    drive_sounds_vol_item =
        ui_menu_add_range(MENU_DRIVE_SOUND_EMULATION_VOLUME, parent,
                        "Drive sound emulation volume", 0, 1000, 100, 1000);
  }

  statusbar_item =
      ui_menu_add_multiple_choice(MENU_OVERLAY, parent, "Show Status Bar");
  statusbar_item->num_choices = 3;
  statusbar_item->value = 0;
  strcpy(statusbar_item->choices[OVERLAY_NEVER], "Never");
  strcpy(statusbar_item->choices[OVERLAY_ALWAYS], "Always");
  strcpy(statusbar_item->choices[OVERLAY_ON_ACTIVITY], "On Activity");

  statusbar_padding_item =
      ui_menu_add_range(MENU_OVERLAY_PADDING, parent, "Status Bar Padding",
          0, 64, 1, 0);

  vkbd_transparency_item =
      ui_menu_add_range(MENU_VKBD_TRANSPARENCY, parent, "Keyboard Transparency %",
          0, 50, 1, 0);

  reset_confirm_item = ui_menu_add_toggle(MENU_RESET_CONFIRM, parent,
                                          "Confirm Reset from Emulator", 1);

  char emu_folder[16];
  char folder_emu[16];

  strcpy (emu_folder, machine_sub_dir);
  strcat (emu_folder, "/dir");

  strcpy (folder_emu, "/dir");
  strcat (folder_emu, machine_sub_dir);

  dir_convention_item = ui_menu_add_toggle_labels(
        MENU_DIR_CONVENTION, parent, "Look for files in", 0,
        folder_emu, emu_folder);

  warp_item = ui_menu_add_toggle(MENU_WARP_MODE, root, "Warp Mode", 0);

  // This is an undocumented feature for now. Keep invisible unless it
  // is activated by cmdline.txt
  if (raspi_demo_mode) {
    ui_menu_add_toggle(MENU_DEMO_MODE, root, "Demo Mode", raspi_demo_mode);
  }

  parent = ui_menu_add_folder(root, "Reset");
  ui_menu_add_button(MENU_SOFT_RESET, parent, "Soft Reset");
  ui_menu_add_button(MENU_HARD_RESET, parent, "Hard Reset");

  ui_menu_add_button(MENU_SAVE_SETTINGS, root, "Save settings");

  ui_set_on_value_changed_callback(menu_value_changed);

  load_settings();

  if (use_scaling_params_item[0]->value) {
     if (!do_use_int_scaling(FB_LAYER_VIC, 1 /* silent */)) {
        use_scaling_params_item[VIC_INDEX]->value = 0;
     }
  }
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128 &&
         use_scaling_params_item[1]->value) {
     if (!do_use_int_scaling(FB_LAYER_VDC, 1 /* silent */)) {
        use_scaling_params_item[VDC_INDEX]->value = 0;
     }
  }

  // Apply shader params
  sanity_check_shader_params(s_enable_shader_item->id);
  circle_realloc_fbl(FB_LAYER_VIC, allow_shader() ? s_enable_shader_item->value : 0);
  handle_shader_param_change();

  set_current_dir_names();

  circle_set_volume(volume_item->value);

  emux_change_palette(0, palette_item[0]->value);
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
    emux_change_palette(1, palette_item[1]->value);
  }
  ui_set_hotkeys();
  ui_set_joy_devs();
  ui_set_joy_items();

  do_video_settings(FB_LAYER_VIC);
  circle_set_interpolation(scaling_interp_item->value);

  // If we were saved with the 80 column key down, let's make the
  // active display the VDC.  If this is not wanted, we'll need
  // another flag to control this behavior.  But this is probably
  // what most people want.
  if (emux_machine_class == BMC64_MACHINE_CLASS_C128 &&
      c40_80_column_item->value == 0) {
    active_display_item->value = MENU_ACTIVE_DISPLAY_VDC;
    vdc_enabled = 1;
    vic_enabled = 0;
  }

  if (emux_machine_class == BMC64_MACHINE_CLASS_C128) {
     do_video_settings(FB_LAYER_VDC);
  }
  overlay_init(statusbar_padding_item->value,
               c40_80_column_item->value,
               vkbd_transparency_item->value);

  emux_set_joy_pot_x(0, pot_x_high_value);
  emux_set_joy_pot_x(1, pot_x_high_value);
  emux_set_joy_pot_y(0, pot_y_high_value);
  emux_set_joy_pot_y(1, pot_y_high_value);

  emux_set_video_cache(0);
  emux_set_hw_scale(0);

  // This can somehow get turned off. Make sure its always 1.
  emux_set_int(Setting_Datasette, 1);

  // For now, all our drives will always be file system devices.
  emux_set_int_1(Setting_FileSystemDeviceN, 1, 8);
  emux_set_int_1(Setting_FileSystemDeviceN, 1, 9);
  emux_set_int_1(Setting_FileSystemDeviceN, 1, 10);
  emux_set_int_1(Setting_FileSystemDeviceN, 1, 11);

  // Restore last iec dirs for all drives
  const char *tmpf;
  emux_get_string_1(Setting_FSDeviceNDir, &tmpf, 8);
  strcpy (last_iec_dir[0], tmpf);
  emux_get_string_1(Setting_FSDeviceNDir, &tmpf, 9);
  strcpy (last_iec_dir[1], tmpf);
  emux_get_string_1(Setting_FSDeviceNDir, &tmpf, 10);
  strcpy (last_iec_dir[2], tmpf);
  emux_get_string_1(Setting_FSDeviceNDir, &tmpf, 11);
  strcpy (last_iec_dir[3], tmpf);
}

int statusbar_never(void) {
  return statusbar_item->value == OVERLAY_NEVER;
}

int statusbar_always(void) {
  return statusbar_item->value == OVERLAY_ALWAYS || statusbar_forced;
}

// Stuff to do when menu is activated
void menu_about_to_activate() {
  emux_get_int(Setting_WarpMode, &warp_item->value);
}

// Stuff to do before going back to emulator
void menu_about_to_deactivate() {}

// These are called on the main loop
void menu_quick_func(int button_assignment) {
  int value;

  if (emux_handle_quick_func(button_assignment, fullpath)) {
    return;
  }

  switch (button_assignment) {
  case BTN_ASSIGN_WARP:
    emux_get_int(Setting_WarpMode, &value);
    toggle_warp(1 - value);
    break;
  case BTN_ASSIGN_SWAP_PORTS:
    menu_swap_joysticks();
    break;
  case BTN_ASSIGN_VKBD_TOGGLE:
    if (vkbd_showing) {
       vkbd_disable();
    } else {
       vkbd_enable();
    }
    break;
  case BTN_ASSIGN_STATUS_TOGGLE:
    // Ignore this if it's already showing.
    if (statusbar_item->value == OVERLAY_ALWAYS)
      return;

    if (statusbar_showing || statusbar_forced) {
      // Dismiss
      statusbar_forced = 0;
      overlay_statusbar_dismiss();
    } else {
      statusbar_forced = 1;
      overlay_statusbar_enable();
    }
    break;
  case BTN_ASSIGN_TAPE_MENU:
    show_tape_osd_menu();
    break;
  case BTN_ASSIGN_CART_MENU:
    emux_show_cart_osd_menu();
    break;
  case BTN_ASSIGN_RESET_MENU:
    show_reset_osd_menu();
    return;
  case BTN_ASSIGN_RESET_HARD:
    if (reset_confirm_item->value) {
      // Will come back here with HARD2 if confirmed.
      show_confirm_osd_menu(BTN_ASSIGN_RESET_HARD2);
      return;
    }
  // fallthrough
  case BTN_ASSIGN_RESET_HARD2:
    menu_machine_reset(0 /* hard */, 0 /* no pop */);
    break;
  case BTN_ASSIGN_RESET_SOFT:
    if (reset_confirm_item->value) {
      // Will come back here with SOFT2 if confirmed.
      show_confirm_osd_menu(BTN_ASSIGN_RESET_SOFT2);
      return;
    }
  // fallthrough
  case BTN_ASSIGN_RESET_SOFT2:
    menu_machine_reset(1 /* soft */, 0 /* no pop */);
    break;
  case BTN_ASSIGN_ACTIVE_DISPLAY:
    active_display_item->value++;
    if (active_display_item->value > 3) {
       active_display_item->value = 0;
    }
    menu_value_changed(active_display_item);
    break;
  case BTN_ASSIGN_PIP_LOCATION:
    pip_location_item->value++;
    if (pip_location_item->value > 3) {
       pip_location_item->value = 0;
    }
    menu_value_changed(pip_location_item);
    break;
  case BTN_ASSIGN_PIP_SWAP:
    pip_swapped_item->value = 1 - pip_swapped_item->value;
    menu_value_changed(pip_swapped_item);
    break;
  case BTN_ASSIGN_40_80_COLUMN:
    c40_80_column_item->value = 1 - c40_80_column_item->value;
    menu_value_changed(c40_80_column_item);
    break;
  default:
    break;
  }
}

int emu_get_gpio_config() {
  return gpio_config_item->choice_ints[gpio_config_item->value];
}

int emu_get_num_joysticks(void) {
  if (emux_machine_class == BMC64_MACHINE_CLASS_VIC20) {
    return 1;
  } else if (emux_machine_class == BMC64_MACHINE_CLASS_PET) {
    return 0;
  }
  return 2;
}

const char* function_to_string(int button_func) {
  switch (button_func) {
    case BTN_ASSIGN_UNDEF:
       return "None";
    case BTN_ASSIGN_FIRE:
       return "Fire";
    case BTN_ASSIGN_MENU:
       return "Menu";
    case BTN_ASSIGN_WARP:
       return "Warp";
    case BTN_ASSIGN_STATUS_TOGGLE:
       return "Status Toggle";
    case BTN_ASSIGN_SWAP_PORTS:
       return "Swap Ports";
    case BTN_ASSIGN_UP:
       return "Up";
    case BTN_ASSIGN_DOWN:
       return "Down";
    case BTN_ASSIGN_LEFT:
       return "Left";
    case BTN_ASSIGN_RIGHT:
       return "Right";
    case BTN_ASSIGN_POTX:
       return "POT X";
    case BTN_ASSIGN_POTY:
       return "POT Y";
    case BTN_ASSIGN_TAPE_MENU:
       return "Tape OSD";
    case BTN_ASSIGN_CART_MENU:
       return "Cart OSD";
    case BTN_ASSIGN_CART_FREEZE:
       return "Cart Freeze";
    case BTN_ASSIGN_RESET_MENU:
       return "Reset OSD";
    case BTN_ASSIGN_RESET_HARD:
       return "Hard Reset";
    case BTN_ASSIGN_RESET_SOFT:
       return "Soft Reset";
    case BTN_ASSIGN_RUN_STOP_BACK:
       return "Menu Back";
    case BTN_ASSIGN_CUSTOM_KEY_1:
       return "Custom Key 1";
    case BTN_ASSIGN_CUSTOM_KEY_2:
       return "Custom Key 2";
    case BTN_ASSIGN_CUSTOM_KEY_3:
       return "Custom Key 3";
    case BTN_ASSIGN_CUSTOM_KEY_4:
       return "Custom Key 4";
    case BTN_ASSIGN_CUSTOM_KEY_5:
       return "Custom Key 5";
    case BTN_ASSIGN_CUSTOM_KEY_6:
       return "Custom Key 6";
    case BTN_ASSIGN_ACTIVE_DISPLAY:
       return "Cycle Display";
    case BTN_ASSIGN_PIP_LOCATION:
       return "PIP Location";
    case BTN_ASSIGN_PIP_SWAP:
       return "PIP Swap";
    case BTN_ASSIGN_40_80_COLUMN:
       return "40/80 Column Key";
    case BTN_ASSIGN_VKBD_TOGGLE:
       return "Virtual Keyboard";
    case BTN_ASSIGN_FLUSH_DISK:
       return "Flush Disks";
    default:
       return "Unknown";
  }
}

void emux_geometry_changed(int layer) {

  // Update the allowed min for border trim items. This lets the user
  // start padding the edges with negative trim values.
  // These are expressed in terms of percentage of the max because they
  // are going into the range item.
  int canvas_index = -1;
  if (layer == FB_LAYER_VIC) {
     canvas_index = VIC_INDEX;
  } else if (layer == FB_LAYER_VDC) {
     canvas_index = VDC_INDEX;
  }

  int dpx, dpy, fbw, fbh, sw, sh, dw, dh;
  circle_get_fbl_dimensions(layer,
                            &dpx, &dpy,
                            &fbw, &fbh,
                            &sw, &sh,
                            &dw, &dh);

  if (canvas_index >= 0) {
    int max_padding_w = MIN(
        canvas_state[canvas_index].extra_offscreen_border_left,
        canvas_state[canvas_index].extra_offscreen_border_right);
    int max_padding_h = canvas_state[canvas_index].first_displayed_line;

    // Update the allowed max h stretch based on the display width and height
    double max_scale = ceil((double)dpx / (double)dpy) * 1000;

    if (h_border_item[canvas_index]) h_border_item[canvas_index]->min = 0;
    if (v_border_item[canvas_index]) v_border_item[canvas_index]->min = 0;
    if (h_border_item[canvas_index]) h_border_item[canvas_index]->max = canvas_state[canvas_index].max_border_w + max_padding_w;
    if (v_border_item[canvas_index]) v_border_item[canvas_index]->max = canvas_state[canvas_index].max_border_h + max_padding_h;
    if (h_stretch_item[canvas_index]) h_stretch_item[canvas_index]->max = max_scale;

    // Stuff these into the canvas state
    canvas_state[canvas_index].max_padding_w = max_padding_w;
    canvas_state[canvas_index].max_padding_h = max_padding_h;
    canvas_state[canvas_index].max_stretch_h = max_scale;
  }

  if (layer == FB_LAYER_VIC) {
     // When the first display changes, we need to update the UI since
     // it's frame buffer dimensions must match.
     ui_geometry_changed(dpx, dpy, fbw, fbh, sw, sh, dw, dh);
  }
}

void emux_frame_buffer_changed(int layer) {
  int canvas_index = layer == FB_LAYER_VIC ? VIC_INDEX : VDC_INDEX;
  if (use_scaling_params_item[canvas_index]->value) {
     if (!do_use_int_scaling(layer, 1 /* silent */)) {
        use_scaling_params_item[canvas_index]->value = 0;
     }
  }
  do_video_settings(layer);
}
