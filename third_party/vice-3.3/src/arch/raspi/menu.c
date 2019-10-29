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

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// VICE includes
#include "attach.h"
#include "autostart.h"
#include "cartridge.h"
#include "datasette.h"
#include "drive.h"
#include "diskimage.h"
#include "joyport.h"
#include "joyport/joystick.h"
#include "keyboard.h"
#include "machine.h"
#include "resources.h"
#include "sid.h"
#include "tape.h"
#include "text.h"
#include "util.h"
#include "vdrive-internal.h"

// RASPI Includes
#include "demo.h"
#include "joy.h"
#include "kbd.h"
#include "menu_cart_osd.h"
#include "menu_confirm_osd.h"
#include "menu_tape_osd.h"
#include "menu_timing.h"
#include "menu_usb.h"
#include "menu_keyset.h"
#include "menu_switch.h"
#include "raspi_machine.h"
#include "overlay.h"
#include "raspi_util.h"
#include "ui.h"

#define VERSION_STRING "3.0"

#ifdef RASPI_LITE
#define VARIANT_STRING "-Lite"
#else
#define VARIANT_STRING ""
#endif

#define DEFAULT_VICII_ASPECT 145
#define DEFAULT_VICII_H_BORDER_TRIM 0
#define DEFAULT_VICII_V_BORDER_TRIM 0

#define DEFAULT_VIC_ASPECT 145
#define DEFAULT_VIC_H_BORDER_TRIM 30
#define DEFAULT_VIC_V_BORDER_TRIM 12

#define DEFAULT_VDC_ASPECT 145
#define DEFAULT_VDC_H_BORDER_TRIM 20
#define DEFAULT_VDC_V_BORDER_TRIM 40

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
} FileFilter;

// These can be saved
struct menu_item *port_1_menu_item;
struct menu_item *port_2_menu_item;
int usb_pref_0;
int usb_pref_1;
int usb_x_axis_0;
int usb_y_axis_0;
int usb_x_axis_1;
int usb_y_axis_1;
float usb_x_thresh_0;
float usb_y_thresh_0;
float usb_x_thresh_1;
float usb_y_thresh_1;
int usb_0_button_assignments[MAX_USB_BUTTONS];
int usb_1_button_assignments[MAX_USB_BUTTONS];
int usb_button_bits[MAX_USB_BUTTONS]; // never change
long keyset_codes[2][7];
long key_bindings[6];
struct menu_item *keyboard_type_item;
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
struct menu_item *sid_engine_item;
struct menu_item *sid_model_item;
struct menu_item *sid_filter_item;
struct menu_item *statusbar_item;
struct menu_item *statusbar_padding_item;
struct menu_item *tape_reset_with_machine_item;
struct menu_item *vkbd_transparency_item;

struct menu_item *palette_item_0;
struct menu_item *brightness_item_0;
struct menu_item *contrast_item_0;
struct menu_item *gamma_item_0;
struct menu_item *tint_item_0;

struct menu_item *palette_item_1;
struct menu_item *brightness_item_1;
struct menu_item *contrast_item_1;
struct menu_item *gamma_item_1;
struct menu_item *tint_item_1;

struct menu_item *warp_item;
struct menu_item *reset_confirm_item;
struct menu_item *gpio_config_item;
struct menu_item *active_display_item;

struct menu_item *h_center_item_0;
struct menu_item *v_center_item_0;
struct menu_item *h_border_item_0;
struct menu_item *v_border_item_0;
struct menu_item *aspect_item_0;

struct menu_item *h_center_item_1;
struct menu_item *v_center_item_1;
struct menu_item *h_border_item_1;
struct menu_item *v_border_item_1;
struct menu_item *aspect_item_1;

struct menu_item *pip_location_item;
struct menu_item *pip_swapped_item;

struct menu_item *c40_80_column_item;

static int unit;
static int joyswap;
static int statusbar_forced;

// Held here, exported for menu_usb to read
int pot_x_high_value;
int pot_x_low_value;
int pot_y_high_value;
int pot_y_low_value;

const int num_disk_ext = 13;
static char disk_filt_ext[13][5] = {".d64", ".d67", ".d71", ".d80", ".d81",
                                    ".d82", ".d1m", ".d2m", ".d4m", ".g64",
                                    ".g41", ".p64", ".x64"};

const int num_tape_ext = 2;
static char tape_filt_ext[2][5] = {".t64", ".tap"};

const int num_cart_ext = 2;
static char cart_filt_ext[2][5] = {".crt", ".bin"};

const int num_snap_ext = 1;
static char snap_filt_ext[1][5] = {".vsf"};

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

// For file type dialogs. Determines what dir we start in. Used
// as index into default_dir_names and current_dir_names.
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
      (menu_id >= MENU_CREATE_D64_FILE && menu_id <= MENU_CREATE_X64_FILE)) {
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

  switch (machine_class) {
  case VICE_MACHINE_C64:
    snprintf (title, 15, "%s%s %s", "BMC64", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal C64 Emulator", 31);
    break;
  case VICE_MACHINE_C128:
    snprintf (title, 15, "%s%s %s", "BMC128", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal C128 Emulator", 31);
    break;
  case VICE_MACHINE_VIC20:
    snprintf (title, 15, "%s%s %s", "BMVIC20", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal VIC20 Emulator", 31);
    break;
  case VICE_MACHINE_PLUS4:
    snprintf (title, 15, "%s%s %s", "BMPLUS4", VARIANT_STRING, VERSION_STRING);
    strncpy (desc, "A Bare Metal PLUS/4 Emulator", 31);
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

static void confirm_dialog(int id, int value) {
  struct menu_item *root = ui_push_menu(8, 2);

  struct menu_item *child;
  child = ui_menu_add_button(MENU_CONFIRM_OK, root, "OK");
  child->value = value;
  child->sub_id = id;

  child = ui_menu_add_button(MENU_CONFIRM_CANCEL, root, "CANCEL");
}

static void drive_change_model() {
  struct menu_item *model_root = ui_push_menu(12, 8);
  struct menu_item *item;

  int current_drive_type;
  resources_get_int_sprintf("Drive%iType", &current_drive_type, unit);

  item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "None");
  item->value = DRIVE_TYPE_NONE;
  if (current_drive_type == DRIVE_TYPE_NONE) {
    strcat(item->displayed_value, " (*)");
  }

  if (drive_check_type(DRIVE_TYPE_1541, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "1541");
    item->value = DRIVE_TYPE_1541;
    if (current_drive_type == DRIVE_TYPE_1541) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1541II, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "1541II");
    item->value = DRIVE_TYPE_1541II;
    if (current_drive_type == DRIVE_TYPE_1541II) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1551, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "1551");
    item->value = DRIVE_TYPE_1551;
    if (current_drive_type == DRIVE_TYPE_1551) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1571, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "1571");
    item->value = DRIVE_TYPE_1571;
    if (current_drive_type == DRIVE_TYPE_1571) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1581, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, "1581");
    item->value = DRIVE_TYPE_1581;
    if (current_drive_type == DRIVE_TYPE_1581) {
      strcat(item->displayed_value, " (*)");
    }
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

static void ui_set_joy_items() {
  int joydev;
  int i;
  for (joydev = 0; joydev < emu_get_num_joysticks(); joydev++) {
    struct menu_item *dst;

    if (joydevs[joydev].port == 1) {
      dst = port_1_menu_item;
    } else if (joydevs[joydev].port == 2) {
      dst = port_2_menu_item;
    } else {
      continue;
    }

    // Find which choice matches the device selected and
    // make sure the menu item matches
    for (i = 0; i < dst->num_choices; i++) {
      if (dst->choice_ints[i] == joydevs[joydev].device) {
        dst->value = i;
        break;
      }
    }
  }

  int value = port_1_menu_item->value;
  if (port_1_menu_item->choice_ints[value] == JOYDEV_NONE) {
    resources_set_int("JoyPort1Device", JOYPORT_ID_NONE);
  } else if (port_1_menu_item->choice_ints[value] == JOYDEV_MOUSE) {
    if (emu_get_num_joysticks() > 1 &&
        port_2_menu_item->choice_ints[port_2_menu_item->value]
            == JOYDEV_MOUSE) {
       resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
       port_2_menu_item->value = 0;
    }
    resources_set_int("JoyPort1Device", JOYPORT_ID_MOUSE_1351);
  } else {
    resources_set_int("JoyPort1Device", JOYPORT_ID_JOYSTICK);
  }

  if (emu_get_num_joysticks() > 1) {
     value = port_2_menu_item->value;
     if (port_2_menu_item->choice_ints[value] == JOYDEV_NONE) {
       resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
     } else if (port_2_menu_item->choice_ints[value] == JOYDEV_MOUSE) {
       if (port_1_menu_item->choice_ints[port_1_menu_item->value]
           == JOYDEV_MOUSE) {
          resources_set_int("JoyPort1Device", JOYPORT_ID_NONE);
          port_1_menu_item->value = 0;
       }
       resources_set_int("JoyPort2Device", JOYPORT_ID_MOUSE_1351);
     } else {
       resources_set_int("JoyPort2Device", JOYPORT_ID_JOYSTICK);
     }
  }
}

static int viceSidEngineToBmcChoice(int viceEngine) {
  switch (viceEngine) {
  case SID_ENGINE_FASTSID:
    return MENU_SID_ENGINE_FAST;
  case SID_ENGINE_RESID:
    return MENU_SID_ENGINE_RESID;
  default:
    return MENU_SID_ENGINE_RESID;
  }
}

static int viceSidModelToBmcChoice(int viceModel) {
  switch (viceModel) {
  case SID_MODEL_6581:
    return MENU_SID_MODEL_6581;
  case SID_MODEL_8580:
    return MENU_SID_MODEL_8580;
  default:
    return MENU_SID_MODEL_6581;
  }
}

static int save_settings() {
  int i;

  FILE *fp;
  switch (machine_class) {
  case VICE_MACHINE_C64:
    fp = fopen("/settings.txt", "w");
    break;
  case VICE_MACHINE_C128:
    fp = fopen("/settings-c128.txt", "w");
    break;
  case VICE_MACHINE_VIC20:
    fp = fopen("/settings-vic20.txt", "w");
    break;
  case VICE_MACHINE_PLUS4:
    fp = fopen("/settings-plus4.txt", "w");
    break;
  default:
    printf("ERROR: Unhandled machine\n");
    return 1;
  }

  int r = resources_save(NULL);
  if (r < 0) {
    printf("resource_save failed with %d\n", r);
    return 1;
  }

  if (fp == NULL)
    return 1;

  fprintf(fp, "port_1=%d\n", port_1_menu_item->value);
  fprintf(fp, "port_2=%d\n", port_2_menu_item->value);
  fprintf(fp, "usb_0=%d\n", usb_pref_0);
  fprintf(fp, "usb_1=%d\n", usb_pref_1);
  fprintf(fp, "usb_x_0=%d\n", usb_x_axis_0);
  fprintf(fp, "usb_y_0=%d\n", usb_y_axis_0);
  fprintf(fp, "usb_x_1=%d\n", usb_x_axis_1);
  fprintf(fp, "usb_y_1=%d\n", usb_y_axis_1);
  fprintf(fp, "usb_x_t_0=%d\n", (int)(usb_x_thresh_0 * 100.0f));
  fprintf(fp, "usb_y_t_0=%d\n", (int)(usb_y_thresh_0 * 100.0f));
  fprintf(fp, "usb_x_t_1=%d\n", (int)(usb_x_thresh_1 * 100.0f));
  fprintf(fp, "usb_y_t_1=%d\n", (int)(usb_y_thresh_1 * 100.0f));
  fprintf(fp, "palette=%d\n", palette_item_0->value);
  if (machine_class == VICE_MACHINE_C128) {
    fprintf(fp, "palette2=%d\n", palette_item_1->value);
  }
  fprintf(fp, "keyboard_type=%d\n", keyboard_type_item->value);

  for (i = 0; i < MAX_USB_BUTTONS; i++) {
    fprintf(fp, "usb_btn_0=%d\n", usb_0_button_assignments[i]);
  }
  for (i = 0; i < MAX_USB_BUTTONS; i++) {
    fprintf(fp, "usb_btn_1=%d\n", usb_1_button_assignments[i]);
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
  fprintf(fp, "gpio_config=%d\n", gpio_config_item->value);
  fprintf(fp, "h_center_0=%d\n", h_center_item_0->value);
  fprintf(fp, "v_center_0=%d\n", v_center_item_0->value);
  fprintf(fp, "h_border_trim_0=%d\n", h_border_item_0->value);
  fprintf(fp, "v_border_trim_0=%d\n", v_border_item_0->value);
  fprintf(fp, "aspect_0=%d\n", aspect_item_0->value);
  if (machine_class == VICE_MACHINE_C128) {
     fprintf(fp, "h_center_1=%d\n", h_center_item_1->value);
     fprintf(fp, "v_center_1=%d\n", v_center_item_1->value);
     fprintf(fp, "h_border_trim_1=%d\n", h_border_item_1->value);
     fprintf(fp, "v_border_trim_1=%d\n", v_border_item_1->value);
     fprintf(fp, "aspect_1=%d\n", aspect_item_1->value);
  }

  int drive_type;

  resources_get_int_sprintf("Drive%iType", &drive_type, 8);
  fprintf(fp, "drive_type_8=%d\n", drive_type);
  resources_get_int_sprintf("Drive%iType", &drive_type, 9);
  fprintf(fp, "drive_type_9=%d\n", drive_type);
  resources_get_int_sprintf("Drive%iType", &drive_type, 10);
  fprintf(fp, "drive_type_10=%d\n", drive_type);
  resources_get_int_sprintf("Drive%iType", &drive_type, 11);
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

  fclose(fp);

  return 0;
}

// Make joydev reflect menu choice
static void ui_set_joy_devs() {
  if (joydevs[0].port == 1) {
    joydevs[0].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
  } else if (emu_get_num_joysticks() > 1 && joydevs[0].port == 2) {
    joydevs[0].device = port_2_menu_item->choice_ints[port_2_menu_item->value];
  }

  if (joydevs[1].port == 1) {
    joydevs[1].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
  } else if (emu_get_num_joysticks() > 1 && joydevs[1].port == 2) {
    joydevs[1].device = port_2_menu_item->choice_ints[port_2_menu_item->value];
  }
}

static void load_settings() {

  int tmp_value;

  resources_get_int("SidEngine", &tmp_value);
  sid_engine_item->value = viceSidEngineToBmcChoice(tmp_value);

  resources_get_int("SidModel", &tmp_value);
  sid_model_item->value = viceSidModelToBmcChoice(tmp_value);

  resources_get_int("SidFilters", &sid_filter_item->value);
#ifndef RASPI_LITE
  resources_get_int("DriveSoundEmulation", &drive_sounds_item->value);
  resources_get_int("DriveSoundEmulationVolume", &drive_sounds_vol_item->value);
#endif

  brightness_item_0->value = get_color_brightness(0);
  contrast_item_0->value = get_color_contrast(0);
  gamma_item_0->value = get_color_gamma(0);
  tint_item_0->value = get_color_tint(0);
  video_color_setting_changed(0);

  if (machine_class == VICE_MACHINE_C128) {
    brightness_item_1->value = get_color_brightness(1);
    contrast_item_1->value = get_color_contrast(1);
    gamma_item_1->value = get_color_gamma(1);
    tint_item_1->value = get_color_tint(1);
    video_color_setting_changed(1);
    resources_get_int("C128ColumnKey", &c40_80_column_item->value);
  }

  // Default pot values for buttons
  pot_x_high_value = 192;
  pot_x_low_value = 64;
  pot_y_high_value = 192;
  pot_y_low_value = 64;

  FILE *fp;
  switch (machine_class) {
  case VICE_MACHINE_C64:
    fp = fopen("/settings.txt", "r");
    break;
  case VICE_MACHINE_C128:
    fp = fopen("/settings-c128.txt", "r");
    break;
  case VICE_MACHINE_VIC20:
    fp = fopen("/settings-vic20.txt", "r");
    break;
  case VICE_MACHINE_PLUS4:
    fp = fopen("/settings-plus4.txt", "r");
    break;
  default:
    printf("ERROR: Unhandled machine\n");
    return;
  }

  if (fp == NULL)
    return;

  char name_value[80];
  int value;
  int usb_btn_0_i = 0;
  int usb_btn_1_i = 0;
  while (1) {
    name_value[0] = '\0';
    // Looks like circle-stdlib doesn't support something like %s=%d
    int st = fscanf(fp, "%s", name_value);
    if (name_value[0] == '\0' || st == EOF || feof(fp))
      break;
    char *name = strtok(name_value, "=");
    if (name == NULL)
      break;
    char *value_str = strtok(NULL, "=");
    if (value_str == NULL)
      break;
    value = atoi(value_str);

    if (strcmp(name, "port_1") == 0) {
      port_1_menu_item->value = value;
    } else if (strcmp(name, "port_2") == 0) {
      port_2_menu_item->value = value;
    } else if (strcmp(name, "usb_0") == 0) {
      usb_pref_0 = value;
    } else if (strcmp(name, "usb_1") == 0) {
      usb_pref_1 = value;
    } else if (strcmp(name, "usb_x_0") == 0) {
      usb_x_axis_0 = value;
    } else if (strcmp(name, "usb_y_0") == 0) {
      usb_y_axis_0 = value;
    } else if (strcmp(name, "usb_x_1") == 0) {
      usb_x_axis_1 = value;
    } else if (strcmp(name, "usb_y_1") == 0) {
      usb_y_axis_1 = value;
    } else if (strcmp(name, "usb_x_t_0") == 0) {
      usb_x_thresh_0 = ((float)value) / 100.0f;
    } else if (strcmp(name, "usb_y_t_0") == 0) {
      usb_y_thresh_0 = ((float)value) / 100.0f;
    } else if (strcmp(name, "usb_x_t_1") == 0) {
      usb_x_thresh_1 = ((float)value) / 100.0f;
    } else if (strcmp(name, "usb_y_t_1") == 0) {
      usb_y_thresh_1 = ((float)value) / 100.0f;
    } else if (strcmp(name, "palette") == 0) {
      palette_item_0->value = value;
    } else if (strcmp(name, "palette2") == 0 && machine_class == VICE_MACHINE_C128) {
      palette_item_1->value = value;
    } else if (strcmp(name, "keyboard_type") == 0) {
      keyboard_type_item->value = value;
    } else if (strcmp(name, "usb_btn_0") == 0) {
      if (value >= NUM_BUTTON_ASSIGNMENTS) {
         value = NUM_BUTTON_ASSIGNMENTS - 1;
      }
      usb_0_button_assignments[usb_btn_0_i] = value;
      usb_btn_0_i++;
      if (usb_btn_0_i >= MAX_USB_BUTTONS) {
        usb_btn_0_i = 0;
      }
    } else if (strcmp(name, "usb_btn_1") == 0) {
      if (value >= NUM_BUTTON_ASSIGNMENTS) {
         value = NUM_BUTTON_ASSIGNMENTS - 1;
      }
      usb_1_button_assignments[usb_btn_1_i] = value;
      usb_btn_1_i++;
      if (usb_btn_1_i >= MAX_USB_BUTTONS) {
        usb_btn_1_i = 0;
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
    } else if (strcmp(name, "gpio_config") == 0) {
      gpio_config_item->value = value;
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
      h_center_item_0->value = value;
    } else if (strcmp(name, "v_center_0") == 0) {
      v_center_item_0->value = value;
    } else if (strcmp(name, "h_border_trim_0") == 0) {
      h_border_item_0->value = value;
    } else if (strcmp(name, "v_border_trim_0") == 0) {
      v_border_item_0->value = value;
    } else if (strcmp(name, "aspect_0") == 0) {
      aspect_item_0->value = value;
    } else if (strcmp(name, "h_center_1") == 0 && machine_class == VICE_MACHINE_C128) {
      h_center_item_1->value = value;
    } else if (strcmp(name, "v_center_1") == 0 && machine_class == VICE_MACHINE_C128) {
      v_center_item_1->value = value;
    } else if (strcmp(name, "h_border_trim_1") == 0 && machine_class == VICE_MACHINE_C128) {
      h_border_item_1->value = value;
    } else if (strcmp(name, "v_border_trim_1") == 0 && machine_class == VICE_MACHINE_C128) {
      v_border_item_1->value = value;
    } else if (strcmp(name, "aspect_1") == 0 && machine_class == VICE_MACHINE_C128) {
      aspect_item_1->value = value;
    } else if (strcmp(name, "volume") == 0) {
      volume_item->value = value;
    }
  }
  fclose(fp);
}

void menu_swap_joysticks() {
  if (port_1_menu_item->choice_ints[port_1_menu_item->value]
          == JOYDEV_MOUSE) {
     resources_set_int("JoyPort1Device", JOYPORT_ID_NONE);
  }

  if (port_2_menu_item->choice_ints[port_2_menu_item->value]
       == JOYDEV_MOUSE) {
     resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
  }

  int tmp = joydevs[0].device;
  joydevs[0].device = joydevs[1].device;
  joydevs[1].device = tmp;
  joyswap = 1 - joyswap;
  overlay_joyswap_changed(joyswap);
  ui_set_joy_items();
}

static void attach_cart(struct menu_item *item, int cart_type) {
  ui_info("Attaching...");
  if (cartridge_attach_image(cart_type,
            fullpath(DIR_CARTS, item->str_value)) < 0) {
     ui_pop_menu();
     ui_error("Failed to attach cart image");
  } else {
     ui_pop_all_and_toggle();
  }
}

static void select_file(struct menu_item *item) {
  switch (item->id) {
     case MENU_IEC_DIR:
       resources_set_string_sprintf("FSDevice%iDir",
          fullpath(DIR_IEC, ""), unit);
       strcpy(last_iec_dir[unit-8], fullpath(DIR_IEC, ""));
       ui_pop_menu();
       return;
     case MENU_LOAD_SNAP_FILE:
       ui_info("Loading...");
       if (machine_read_snapshot(fullpath(DIR_SNAPS, item->str_value), 0) < 0) {
         ui_pop_menu();
         ui_error("Load snapshot failed");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_DISK_FILE:
       // Perform the attach
       ui_info("Attaching...");
       if (file_system_attach_disk(unit, fullpath(DIR_DISKS, item->str_value)) <
           0) {
         ui_pop_menu();
         ui_error("Failed to attach disk image");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_DRIVE_ROM_FILE_1541:
     case MENU_DRIVE_ROM_FILE_1541II:
     case MENU_DRIVE_ROM_FILE_1551:
     case MENU_DRIVE_ROM_FILE_1571:
     case MENU_DRIVE_ROM_FILE_1581:
       // Make the rom change. These can't be fullpath or VICE complains.
       switch (item->id) {
          case MENU_DRIVE_ROM_FILE_1541:
             resources_set_string("DosName1541", item->str_value);
             break;
          case MENU_DRIVE_ROM_FILE_1541II:
             resources_set_string("DosName1541ii", item->str_value);
             break;
          case MENU_DRIVE_ROM_FILE_1551:
             resources_set_string("DosName1551", item->str_value);
             break;
          case MENU_DRIVE_ROM_FILE_1571:
             resources_set_string("DosName1571", item->str_value);
             break;
          case MENU_DRIVE_ROM_FILE_1581:
             resources_set_string("DosName1581", item->str_value);
             break;
       }
       ui_pop_menu();
       ui_pop_menu();
       return;
     case MENU_TAPE_FILE:
       ui_info("Attaching...");
       if (tape_image_attach(1, fullpath(DIR_TAPES, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Failed to attach tape image");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     // NOTE: ROMs can't be fullpath or VICE complains.
     case MENU_KERNAL_FILE:
       resources_set_string("KernalName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_BASIC_FILE:
       resources_set_string("BasicName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_CHARGEN_FILE:
       resources_set_string("ChargenName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_KERNAL_FILE:
       resources_set_string("KernalIntName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_BASIC_HI_FILE:
       resources_set_string("BasicHiName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_BASIC_LO_FILE:
       resources_set_string("BasicLoName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_CHARGEN_FILE:
       resources_set_string("ChargenIntName", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_64_KERNAL_FILE:
       resources_set_string("Kernal64Name", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_C128_LOAD_64_BASIC_FILE:
       resources_set_string("Basic64Name", item->str_value);
       ui_pop_all_and_toggle();
       return;
     case MENU_AUTOSTART_FILE:
       ui_info("Starting...");
       if (autostart_autodetect(fullpath(DIR_ROOT, item->str_value), NULL, 0,
                             AUTOSTART_MODE_RUN) < 0) {
         ui_pop_menu();
         ui_error("Failed to autostart file");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
     case MENU_C64_CART_FILE:
       attach_cart(item, CARTRIDGE_CRT);
       return;
     case MENU_C64_CART_8K_FILE:
       attach_cart(item, CARTRIDGE_GENERIC_8KB);
       return;
     case MENU_C64_CART_16K_FILE:
       attach_cart(item, CARTRIDGE_GENERIC_16KB);
       return;
     case MENU_C64_CART_ULTIMAX_FILE:
       attach_cart(item, CARTRIDGE_ULTIMAX);
       return;
     case MENU_VIC20_CART_DETECT_FILE:
       attach_cart(item, CARTRIDGE_VIC20_DETECT);
       return;
     case MENU_VIC20_CART_GENERIC_FILE:
       attach_cart(item, CARTRIDGE_VIC20_GENERIC);
       return;
     case MENU_VIC20_CART_16K_2000_FILE:
       attach_cart(item, CARTRIDGE_VIC20_16KB_2000);
       return;
     case MENU_VIC20_CART_16K_4000_FILE:
       attach_cart(item, CARTRIDGE_VIC20_16KB_4000);
       return;
     case MENU_VIC20_CART_16K_6000_FILE:
       attach_cart(item, CARTRIDGE_VIC20_16KB_6000);
       return;
     case MENU_VIC20_CART_8K_A000_FILE:
       attach_cart(item, CARTRIDGE_VIC20_8KB_A000);
       return;
     case MENU_VIC20_CART_4K_B000_FILE:
       attach_cart(item, CARTRIDGE_VIC20_4KB_B000);
       return;
     case MENU_VIC20_CART_BEHRBONZ_FILE:
       attach_cart(item, CARTRIDGE_VIC20_BEHRBONZ);
       return;
     case MENU_VIC20_CART_UM_FILE:
       attach_cart(item, CARTRIDGE_VIC20_UM);
       return;
     case MENU_VIC20_CART_FP_FILE:
       attach_cart(item, CARTRIDGE_VIC20_FP);
       return;
     case MENU_VIC20_CART_MEGACART_FILE:
       attach_cart(item, CARTRIDGE_VIC20_MEGACART);
       return;
     case MENU_VIC20_CART_FINAL_EXPANSION_FILE:
       attach_cart(item, CARTRIDGE_VIC20_FINAL_EXPANSION);
       return;
     case MENU_PLUS4_CART_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_DETECT);
       return;
     case MENU_PLUS4_CART_C0_LO_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C0LO);
       return;
     case MENU_PLUS4_CART_C0_HI_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C0HI);
       return;
     case MENU_PLUS4_CART_C1_LO_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C1LO);
       return;
     case MENU_PLUS4_CART_C1_HI_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C1HI);
       return;
     case MENU_PLUS4_CART_C2_LO_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C2LO);
       return;
     case MENU_PLUS4_CART_C2_HI_FILE:
       attach_cart(item, CARTRIDGE_PLUS4_16KB_C2HI);
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
          strcat(fname, ".vsf");
        } else {
          ui_error("Too long");
          return;
        }
      } else {
        if ((dot[1] != 'v' && dot[1] != 'V') ||
            (dot[2] != 's' && dot[2] != 'S') ||
            (dot[3] != 'f' && dot[3] != 'F') || dot[4] != '\0') {
          ui_error("Need .VSF extension");
          return;
        }
      }
    }
    ui_info("Saving...");
    if (machine_write_snapshot(fullpath(DIR_SNAPS, fname), 1, 1, 0) < 0) {
      ui_pop_menu();
      ui_error("Save snapshot failed");
    } else {
      ui_pop_all_and_toggle();
    }
  }

  // Handle creating empty disk
  else if (item->id >= MENU_CREATE_D64_FILE && item->id <= MENU_CREATE_X64_FILE) {
     char ext[5];
     int image_type;
     switch (item->id) {
       case MENU_CREATE_D64_FILE:
         image_type = DISK_IMAGE_TYPE_D64;
         strcpy(ext, ".d64");
         break;
       case MENU_CREATE_D67_FILE:
         image_type = DISK_IMAGE_TYPE_D67;
         strcpy(ext, ".d67");
         break;
       case MENU_CREATE_D71_FILE:
         image_type = DISK_IMAGE_TYPE_D71;
         strcpy(ext, ".d71");
         break;
       case MENU_CREATE_D80_FILE:
         image_type = DISK_IMAGE_TYPE_D80;
         strcpy(ext, ".d80");
         break;
       case MENU_CREATE_D81_FILE:
         image_type = DISK_IMAGE_TYPE_D81;
         strcpy(ext, ".d81");
         break;
       case MENU_CREATE_D82_FILE:
         image_type = DISK_IMAGE_TYPE_D82;
         strcpy(ext, ".d82");
         break;
       case MENU_CREATE_D1M_FILE:
         image_type = DISK_IMAGE_TYPE_D1M;
         strcpy(ext, ".d1m");
         break;
       case MENU_CREATE_D2M_FILE:
         image_type = DISK_IMAGE_TYPE_D2M;
         strcpy(ext, ".d2m");
         break;
       case MENU_CREATE_D4M_FILE:
         image_type = DISK_IMAGE_TYPE_D4M;
         strcpy(ext, ".d4m");
         break;
       case MENU_CREATE_G64_FILE:
         image_type = DISK_IMAGE_TYPE_G64;
         strcpy(ext, ".g64");
         break;
       case MENU_CREATE_P64_FILE:
         image_type = DISK_IMAGE_TYPE_P64;
         strcpy(ext, ".p64");
         break;
       case MENU_CREATE_X64_FILE:
         image_type = DISK_IMAGE_TYPE_X64;
         strcpy(ext, ".x64");
         break;
       default:
         return;
     }

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
          strcat(fname, ext);
        } else {
          ui_error("Too long");
          return;
        }
      } else {
        if (strncasecmp(dot, ext, 4) != 0) {
          ui_error("Wrong extension");
          return;
        }
      }
    } else {
      // Don't allow overwriting an existing file. Just ignore it.
      return;
    }

    ui_info("Creating...");
    if (vdrive_internal_create_format_disk_image(
         fullpath(DIR_DISKS, fname), "DISK", image_type) < 0) {
      ui_pop_menu();
      ui_error("Create disk image failed");
    } else {
      ui_pop_menu();
      ui_pop_menu();
      ui_info("Disk Created");
    }
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
  case MENU_CREATE_P64_FILE:
  case MENU_CREATE_X64_FILE:
    return DIR_DISKS;
  case MENU_TAPE_FILE:
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
  case MENU_CREATE_P64_FILE:
  case MENU_CREATE_X64_FILE:
    show_files(DIR_DISKS, FILTER_DISK, item->id, 1);
    break;
  case MENU_TAPE_FILE:
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
  resources_set_int("WarpMode", value);
  raspi_warp = value;
  overlay_warp_changed(value);
  warp_item->value = value;
}

// Tell videoarch the new settings made from the menu.
static void do_video_settings(int layer,
                              struct menu_item* hcenter_item,
                              struct menu_item* vcenter_item,
                              struct menu_item* hborder_item,
                              struct menu_item* vborder_item,
                              struct menu_item* aspect_item) {

  double lpad;
  double rpad;
  double tpad;
  double bpad;
  int zlayer;

  int hc = hcenter_item->value;
  int vc = vcenter_item->value;
  int vid_hc = hc;
  int vid_vc = vc;

  if (machine_class == VICE_MACHINE_C128) {
     if ((active_display_item->value == MENU_ACTIVE_DISPLAY_VICII && layer == FB_LAYER_VIC) ||
         (active_display_item->value == MENU_ACTIVE_DISPLAY_VDC && layer == FB_LAYER_VDC)) {
        lpad = 0; rpad = 0; tpad = 0; bpad = 0; zlayer = layer == FB_LAYER_VIC ? 0 : 1;
     } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE) {
        // VIC on the left, VDC on the right, always, no swapping
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

  double h = (double)(100-hborder_item->value) / 100.0d;
  double v = (double)(100-vborder_item->value) / 100.0d;
  double a = (double)(aspect_item->value) / 100.0d;

  double vid_a = a;
  if (machine_class == VICE_MACHINE_C128 &&
          active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE) {
     // For side-by-side, it makes more sense to fill horizontal then scale
     // vertical since we just cut horizontal in half. So pass in negative
     // aspect.
     vid_a = -a;
  }

  // Tell videoarch about these changes
  apply_video_adjustments(layer, vid_hc, vid_vc, h, v, vid_a, lpad, rpad, tpad, bpad, zlayer);
  if (layer == FB_LAYER_VIC) {
     apply_video_adjustments(FB_LAYER_UI, hc, vc, h, v, a, 0, 0, 0, 0, 3);
  }
}

static void menu_machine_reset(int type, int pop) {
  // The IEC dir may have been changed by the emulated machine. On reset,
  // we reset back to the last dir set by the user.
  resources_set_string_sprintf("FSDevice%iDir", last_iec_dir[0], 8);
  resources_set_string_sprintf("FSDevice%iDir", last_iec_dir[1], 9);
  resources_set_string_sprintf("FSDevice%iDir", last_iec_dir[2], 10);
  resources_set_string_sprintf("FSDevice%iDir", last_iec_dir[3], 11);
  machine_trigger_reset(type);
  if (pop) {
     ui_pop_all_and_toggle();
  }
}

static void do_easy_flash() {
    if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
      ui_error("Problem saving");
    } else {
      ui_pop_all_and_toggle();
    }
}

// Interpret what menu item changed and make the change to vice
static void menu_value_changed(struct menu_item *item) {
  struct machine_entry* head;
  int status = 0;

  switch (item->id) {
  case MENU_ATTACH_DISK_8:
  case MENU_IECDEVICE_8:
  case MENU_IECDIR_8:
  case MENU_DRIVE_CHANGE_MODEL_8:
  case MENU_PARALLEL_8:
    unit = 8;
    break;
  case MENU_ATTACH_DISK_9:
  case MENU_IECDEVICE_9:
  case MENU_IECDIR_9:
  case MENU_DRIVE_CHANGE_MODEL_9:
  case MENU_PARALLEL_9:
    unit = 9;
    break;
  case MENU_ATTACH_DISK_10:
  case MENU_IECDEVICE_10:
  case MENU_IECDIR_10:
  case MENU_DRIVE_CHANGE_MODEL_10:
  case MENU_PARALLEL_10:
    unit = 10;
    break;
  case MENU_ATTACH_DISK_11:
  case MENU_IECDEVICE_11:
  case MENU_IECDIR_11:
  case MENU_DRIVE_CHANGE_MODEL_11:
  case MENU_PARALLEL_11:
    unit = 11;
    break;
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
    video_canvas_reveal_temp(FB_LAYER_VIC);
    video_canvas_change_palette(0, item->value);
    return;
  case MENU_COLOR_PALETTE_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    video_canvas_change_palette(1, item->value);
    return;
  case MENU_AUTOSTART:
    show_files(DIR_ROOT, FILTER_NONE, MENU_AUTOSTART_FILE, 0);
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
  case MENU_CREATE_P64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_P64_FILE, 0);
    return;
  case MENU_CREATE_X64:
    show_files(DIR_DISKS, FILTER_NONE, MENU_CREATE_X64_FILE, 0);
    return;

  case MENU_IECDEVICE_8:
  case MENU_IECDEVICE_9:
  case MENU_IECDEVICE_10:
  case MENU_IECDEVICE_11:
    resources_set_int_sprintf("IECDevice%i", item->value, unit);
    return;
  case MENU_PARALLEL_8:
  case MENU_PARALLEL_9:
  case MENU_PARALLEL_10:
  case MENU_PARALLEL_11:
    resources_set_int_sprintf("Drive%iParallelCable",
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
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C0_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C0_HI:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C0_HI_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C1_LO:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C1_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C1_HI:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C1_HI_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C2_LO:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C2_LO_FILE, 0);
    return;
  case MENU_PLUS4_ATTACH_CART_C2_HI:
    show_files(DIR_CARTS, FILTER_CART, MENU_PLUS4_CART_C2_HI_FILE, 0);
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
    cartridge_set_default();
    ui_info("Remember to save..");
    return;
  case MENU_DETACH_DISK_8:
    ui_info("Deatching...");
    file_system_detach_disk(8);
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_9:
    ui_info("Detaching...");
    file_system_detach_disk(9);
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_10:
    ui_info("Detaching...");
    file_system_detach_disk(10);
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_DISK_11:
    ui_info("Detaching...");
    file_system_detach_disk(11);
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_TAPE:
    ui_info("Detaching...");
    tape_image_detach(1);
    ui_pop_all_and_toggle();
    return;
  case MENU_DETACH_CART:
    ui_info("Detaching...");
    cartridge_detach_image(CARTRIDGE_NONE);
    ui_pop_all_and_toggle();
    return;
  case MENU_SOFT_RESET:
    menu_machine_reset(MACHINE_RESET_MODE_SOFT, 1 /* pop */);
    return;
  case MENU_HARD_RESET:
    menu_machine_reset(MACHINE_RESET_MODE_HARD, 1 /* pop */);
    return;
  case MENU_ABOUT:
    show_about();
    return;
  case MENU_LICENSE:
    show_license();
    return;
  case MENU_CONFIGURE_USB_0:
    configure_usb(0);
    return;
  case MENU_CONFIGURE_USB_1:
    configure_usb(1);
    return;
  case MENU_CONFIGURE_KEYSET1:
    configure_keyset(0);
    return;
  case MENU_CONFIGURE_KEYSET2:
    configure_keyset(1);
    return;
  case MENU_WARP_MODE:
    toggle_warp(item->value);
    return;
  case MENU_DEMO_MODE:
    raspi_demo_mode = item->value;
    demo_reset();
    return;
  case MENU_DRIVE_SOUND_EMULATION:
    resources_set_int("DriveSoundEmulation", item->value);
    return;
  case MENU_DRIVE_SOUND_EMULATION_VOLUME:
    resources_set_int("DriveSoundEmulationVolume", item->value);
    return;
  case MENU_COLOR_BRIGHTNESS_0:
    video_canvas_reveal_temp(FB_LAYER_VIC);
    set_color_brightness(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_CONTRAST_0:
    video_canvas_reveal_temp(FB_LAYER_VIC);
    set_color_contrast(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_GAMMA_0:
    video_canvas_reveal_temp(FB_LAYER_VIC);
    set_color_gamma(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_TINT_0:
    video_canvas_reveal_temp(FB_LAYER_VIC);
    set_color_tint(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_RESET_0:
    brightness_item_0->value = 1000;
    contrast_item_0->value = 1250;
    gamma_item_0->value = 2200;
    tint_item_0->value = 1000;
    set_color_brightness(0, brightness_item_0->value);
    set_color_contrast(0, contrast_item_0->value);
    set_color_gamma(0, gamma_item_0->value);
    set_color_tint(0, tint_item_0->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_BRIGHTNESS_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    set_color_brightness(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_CONTRAST_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    set_color_contrast(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_GAMMA_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    set_color_gamma(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_TINT_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    set_color_tint(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_RESET_1:
    brightness_item_1->value = 1000;
    contrast_item_1->value = 1250;
    gamma_item_1->value = 2200;
    tint_item_1->value = 1000;
    set_color_brightness(1, brightness_item_1->value);
    set_color_contrast(1, contrast_item_1->value);
    set_color_gamma(1, gamma_item_1->value);
    set_color_tint(1, tint_item_1->value);
    video_color_setting_changed(1);
    return;
  case MENU_SWAP_JOYSTICKS:
    menu_swap_joysticks();
    return;
  case MENU_JOYSTICK_PORT_1:
    // device in port 1 was changed
    if (joydevs[0].port == 1) {
      joydevs[0].device = item->choice_ints[item->value];
    } else if (joydevs[1].port == 1) {
      joydevs[1].device = item->choice_ints[item->value];
    }
    if (item->choice_ints[item->value] == JOYDEV_NONE) {
      resources_set_int("JoyPort1Device", JOYPORT_ID_NONE);
    } else if (item->choice_ints[item->value] == JOYDEV_MOUSE) {
      if (port_2_menu_item->choice_ints[port_2_menu_item->value]
          == JOYDEV_MOUSE) {
         resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
         port_2_menu_item->value = 0;
      }
      resources_set_int("JoyPort1Device", JOYPORT_ID_MOUSE_1351);
    } else {
      resources_set_int("JoyPort1Device", JOYPORT_ID_JOYSTICK);
    }
    return;
  case MENU_JOYSTICK_PORT_2:
    // device in port 2 was changed
    if (joydevs[0].port == 2) {
      joydevs[0].device = item->choice_ints[item->value];
    } else if (joydevs[1].port == 2) {
      joydevs[1].device = item->choice_ints[item->value];
    }
    if (item->choice_ints[item->value] == JOYDEV_NONE) {
      resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
    } else if (item->choice_ints[item->value] == JOYDEV_MOUSE) {
      if (port_1_menu_item->choice_ints[port_1_menu_item->value]
          == JOYDEV_MOUSE) {
         resources_set_int("JoyPort1Device", JOYPORT_ID_NONE);
         port_1_menu_item->value = 0;
      }
      resources_set_int("JoyPort2Device", JOYPORT_ID_MOUSE_1351);
    } else {
      resources_set_int("JoyPort2Device", JOYPORT_ID_JOYSTICK);
    }
    return;
  case MENU_TAPE_START:
    datasette_control(DATASETTE_CONTROL_START);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_STOP:
    datasette_control(DATASETTE_CONTROL_STOP);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_REWIND:
    datasette_control(DATASETTE_CONTROL_REWIND);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_FASTFWD:
    datasette_control(DATASETTE_CONTROL_FORWARD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RECORD:
    datasette_control(DATASETTE_CONTROL_RECORD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET:
    datasette_control(DATASETTE_CONTROL_RESET);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET_COUNTER:
    datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET_WITH_MACHINE:
    resources_set_int("DatasetteResetWithCPU",
                      tape_reset_with_machine_item->value);
    return;
  case MENU_SID_ENGINE:
    resources_set_int("SidEngine", item->choice_ints[item->value]);
    resources_set_int("SidResidSampling", 0);
    return;
  case MENU_SID_MODEL:
    resources_set_int("SidModel", item->choice_ints[item->value]);
    resources_set_int("SidResidSampling", 0);
    return;
  case MENU_SID_FILTER:
    resources_set_int("SidFilters", item->value);
    resources_set_int("SidResidSampling", 0);
    return;

  case MENU_DRIVE_CHANGE_MODEL_8:
  case MENU_DRIVE_CHANGE_MODEL_9:
  case MENU_DRIVE_CHANGE_MODEL_10:
  case MENU_DRIVE_CHANGE_MODEL_11:
    drive_change_model();
    return;
  case MENU_DRIVE_CHANGE_ROM:
    drive_change_rom();
    return;
  case MENU_DRIVE_MODEL_SELECT:
    resources_set_int_sprintf("Drive%iType", item->value, unit);
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
  case MENU_SAVE_EASYFLASH:
    do_easy_flash();
    return;
  case MENU_CART_FREEZE:
    keyboard_clear_keymatrix();
    raspi_cartridge_trigger_freeze();
    ui_pop_all_and_toggle();
    return;
  case MENU_VIC20_MEMORY_3K:
    resources_set_int("RAMBlock0", item->value);
    return;
  case MENU_VIC20_MEMORY_8K_2000:
    resources_set_int("RAMBlock1", item->value);
    return;
  case MENU_VIC20_MEMORY_8K_4000:
    resources_set_int("RAMBlock2", item->value);
    return;
  case MENU_VIC20_MEMORY_8K_6000:
    resources_set_int("RAMBlock3", item->value);
    return;
  case MENU_VIC20_MEMORY_8K_A000:
    resources_set_int("RAMBlock5", item->value);
    return;
  case MENU_ACTIVE_DISPLAY:
  case MENU_PIP_LOCATION:
  case MENU_PIP_SWAPPED:
    if (active_display_item->value == MENU_ACTIVE_DISPLAY_VICII) {
       enable_vic(1);
       enable_vdc(0);
       do_video_settings(FB_LAYER_VIC,
           h_center_item_0,
           v_center_item_0,
           h_border_item_0,
           v_border_item_0,
           aspect_item_0);
    } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_VDC) {
       enable_vdc(1);
       enable_vic(0);
       do_video_settings(FB_LAYER_VDC,
           h_center_item_1,
           v_center_item_1,
           h_border_item_1,
           v_border_item_1,
           aspect_item_1);
    } else if (active_display_item->value == MENU_ACTIVE_DISPLAY_SIDE_BY_SIDE ||
               active_display_item->value == MENU_ACTIVE_DISPLAY_PIP) {
       enable_vdc(1);
       enable_vic(1);
       do_video_settings(FB_LAYER_VIC,
           h_center_item_0,
           v_center_item_0,
           h_border_item_0,
           v_border_item_0,
           aspect_item_0);
       do_video_settings(FB_LAYER_VDC,
           h_center_item_1,
           v_center_item_1,
           h_border_item_1,
           v_border_item_1,
           aspect_item_1);
    }
    break;
  case MENU_H_CENTER_0:
  case MENU_V_CENTER_0:
  case MENU_H_BORDER_0:
  case MENU_V_BORDER_0:
  case MENU_ASPECT_0:
    video_canvas_reveal_temp(FB_LAYER_VIC);
    do_video_settings(FB_LAYER_VIC,
        h_center_item_0,
        v_center_item_0,
        h_border_item_0,
        v_border_item_0,
        aspect_item_0);
    break;
  case MENU_H_CENTER_1:
  case MENU_V_CENTER_1:
  case MENU_H_BORDER_1:
  case MENU_V_BORDER_1:
  case MENU_ASPECT_1:
    video_canvas_reveal_temp(FB_LAYER_VDC);
    do_video_settings(FB_LAYER_VDC,
        h_center_item_1,
        v_center_item_1,
        h_border_item_1,
        v_border_item_1,
        aspect_item_1);
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
    resources_set_int("C128ColumnKey", item->value);
    overlay_40_80_columns_changed(item->value);
    break;
  case MENU_VOLUME:
    circle_set_volume(item->value);
    break;
  case MENU_SWITCH_MACHINE:
    confirm_dialog(MENU_SWITCH_MACHINE, item->value);
    break;
  case MENU_CONFIRM_OK:
    ui_pop_menu();
    if (item->sub_id == MENU_SWITCH_MACHINE) {
      load_machines(&head);
      struct machine_entry* ptr = head;
      status = 0;
      while (ptr) {
          if (ptr->id == item->value) {
            status = apply_config(ptr, circle_get_model());
            status |= apply_cmdline(ptr);
            break;
          }
          ptr = ptr->next;
      }
      free_machines(head);
      if (status) {
         char failcode[32];
         sprintf (failcode, "FAILURE (CODE %d)", status);
         ui_info_wrapped(failcode, SWITCH_FAIL_MSG);
      } else {
         ui_info_wrapped("Success - PLEASE REBOOT", SWITCH_MSG);
      }
    }
    break;
  case MENU_CONFIRM_CANCEL:
    ui_pop_menu();
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
void emu_get_usb_pref(int device, int *usb_pref, int *x_axis, int *y_axis,
                      float *x_thresh, float *y_thresh) {
  if (device == 0) {
    *usb_pref = usb_pref_0;
    *x_axis = usb_x_axis_0;
    *y_axis = usb_y_axis_0;
    *x_thresh = usb_x_thresh_0;
    *y_thresh = usb_y_thresh_0;
  } else if (device == 1) {
    *usb_pref = usb_pref_1;
    *x_axis = usb_x_axis_1;
    *y_axis = usb_y_axis_1;
    *x_thresh = usb_x_thresh_1;
    *y_thresh = usb_y_thresh_1;
  } else {
    *usb_pref = -1;
    *x_axis = -1;
    *y_axis = -1;
    *x_thresh = .50;
    *y_thresh = .50;
  }
}

int menu_get_keyboard_type(void) { return keyboard_type_item->value; }

// KEEP in sync with kernel.cpp, kbd.c, menu_usb.c
static void set_hotkey_choices(struct menu_item *item) {
  item->num_choices = 14;
  strcpy(item->choices[HOTKEY_CHOICE_NONE], "None");
  strcpy(item->choices[HOTKEY_CHOICE_MENU], "Menu");
  strcpy(item->choices[HOTKEY_CHOICE_WARP], "Warp");
  strcpy(item->choices[HOTKEY_CHOICE_STATUS_TOGGLE], "Toggle Status");
  strcpy(item->choices[HOTKEY_CHOICE_SWAP_PORTS], "Swap Ports");
  strcpy(item->choices[HOTKEY_CHOICE_TAPE_MENU], "Tape OSD");
  strcpy(item->choices[HOTKEY_CHOICE_CART_MENU], "Cart OSD");
  strcpy(item->choices[HOTKEY_CHOICE_CART_FREEZE], "Cart Freeze");
  strcpy(item->choices[HOTKEY_CHOICE_RESET_HARD], "Hard Reset");
  strcpy(item->choices[HOTKEY_CHOICE_RESET_SOFT], "Soft Reset");
  strcpy(item->choices[HOTKEY_CHOICE_ACTIVE_DISPLAY], "Change Active Display");
  strcpy(item->choices[HOTKEY_CHOICE_PIP_LOCATION], "Change PIP Location");
  strcpy(item->choices[HOTKEY_CHOICE_PIP_SWAP], "Swap PIP");
  strcpy(item->choices[HOTKEY_CHOICE_40_80_COLUMN], "40/80 Column");
  item->choice_ints[HOTKEY_CHOICE_NONE] = BTN_ASSIGN_UNDEF;
  item->choice_ints[HOTKEY_CHOICE_MENU] = BTN_ASSIGN_MENU;
  item->choice_ints[HOTKEY_CHOICE_WARP] = BTN_ASSIGN_WARP;
  item->choice_ints[HOTKEY_CHOICE_STATUS_TOGGLE] = BTN_ASSIGN_STATUS_TOGGLE;
  item->choice_ints[HOTKEY_CHOICE_SWAP_PORTS] = BTN_ASSIGN_SWAP_PORTS;
  item->choice_ints[HOTKEY_CHOICE_TAPE_MENU] = BTN_ASSIGN_TAPE_MENU;
  item->choice_ints[HOTKEY_CHOICE_CART_MENU] = BTN_ASSIGN_CART_MENU;
  item->choice_ints[HOTKEY_CHOICE_CART_FREEZE] = BTN_ASSIGN_CART_FREEZE;
  item->choice_ints[HOTKEY_CHOICE_RESET_HARD] = BTN_ASSIGN_RESET_HARD;
  item->choice_ints[HOTKEY_CHOICE_RESET_SOFT] = BTN_ASSIGN_RESET_SOFT;
  item->choice_ints[HOTKEY_CHOICE_ACTIVE_DISPLAY] = BTN_ASSIGN_ACTIVE_DISPLAY;
  item->choice_ints[HOTKEY_CHOICE_PIP_LOCATION] = BTN_ASSIGN_PIP_LOCATION;
  item->choice_ints[HOTKEY_CHOICE_PIP_SWAP] = BTN_ASSIGN_PIP_SWAP;
  item->choice_ints[HOTKEY_CHOICE_40_80_COLUMN] = BTN_ASSIGN_40_80_COLUMN;

  if (machine_class == VICE_MACHINE_VIC20) {
     item->choice_disabled[HOTKEY_CHOICE_SWAP_PORTS] = 1;
     item->choice_disabled[HOTKEY_CHOICE_CART_FREEZE] = 1;
  }

  if (machine_class != VICE_MACHINE_C128) {
     item->choice_disabled[HOTKEY_CHOICE_ACTIVE_DISPLAY] = 1;
     item->choice_disabled[HOTKEY_CHOICE_PIP_LOCATION] = 1;
     item->choice_disabled[HOTKEY_CHOICE_PIP_SWAP] = 1;
     item->choice_disabled[HOTKEY_CHOICE_40_80_COLUMN] = 1;
  }
}

static void add_parallel_cable_option(struct menu_item* parent, int id, int drive) {
  if (machine_class != VICE_MACHINE_C64 &&
      machine_class != VICE_MACHINE_C128) {
    return;
  }

  int tmp;
  resources_get_int_sprintf("Drive%iParallelCable", &tmp, drive);

  int index = 0;
  switch (tmp) {
    case DRIVE_PC_NONE:
       index = 0; break;
    case DRIVE_PC_STANDARD:
       index = 1; break;
    case DRIVE_PC_DD3:
       index = 2; break;
    case DRIVE_PC_FORMEL64:
       index = 3; break;
    default:
       return;
  }

  struct menu_item* child =
      ui_menu_add_multiple_choice(id, parent, "Parallel Cable");
  child->num_choices = 4;
  child->value = index;
  strcpy(child->choices[0], "None");
  strcpy(child->choices[1], "Standard");
  strcpy(child->choices[2], "Dolphin DOS");
  strcpy(child->choices[3], "Formel 64");
  child->choice_ints[0] = DRIVE_PC_NONE;
  child->choice_ints[1] = DRIVE_PC_STANDARD;
  child->choice_ints[2] = DRIVE_PC_DD3;
  child->choice_ints[3] = DRIVE_PC_FORMEL64;
}

static void menu_build_machine_switch(struct menu_item* parent) {
  struct menu_item* holder = ui_menu_add_folder(parent, "Switch");

  struct menu_item* vic20_r = ui_menu_add_folder(holder, "VIC20");
  struct menu_item* c64_r = ui_menu_add_folder(holder, "C64");
  struct menu_item* c128_r = ui_menu_add_folder(holder, "C128");
  struct menu_item* plus4_r = ui_menu_add_folder(holder, "Plus/4");

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
      default:
         item = NULL;
         break;
    }

    item->value = ptr->id;

    ptr=ptr->next;
  }

  free_machines(head);
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
  int tmp;

  // TODO: This doesn't really belong here. Need to sort
  // out init order of structs.
  for (dev = 0; dev < MAX_JOY_PORTS; dev++) {
    memset(&joydevs[dev], 0, sizeof(struct joydev_config));
    joydevs[dev].port = dev + 1;
    joydevs[dev].device = JOYDEV_NONE;
  }

  // TODO: Make these start dirs configurable.
  strcpy(current_volume_name, default_volume_name);
  for (i = 0; i < NUM_DIR_TYPES; i++) {
    strcpy(current_dir_names[i], default_dir_names[i]);
  }

  char machine_info_txt[64];
  char machine_sub_dir[16];
  machine_info_txt[0] = '\0';

  switch (machine_class) {
  case VICE_MACHINE_C64:
    strcat(machine_info_txt,"C64 ");
    strcpy(machine_sub_dir, "/C64");
    break;
  case VICE_MACHINE_C128:
    strcat(machine_info_txt,"C128 ");
    strcpy(machine_sub_dir, "/C128");
    break;
  case VICE_MACHINE_VIC20:
    strcat(machine_info_txt,"VIC20 ");
    strcpy(machine_sub_dir, "/VIC20");
    break;
  case VICE_MACHINE_PLUS4:
    strcat(machine_info_txt,"PLUS/4 ");
    strcpy(machine_sub_dir, "/PLUS4");
    break;
  default:
    strcat(machine_info_txt,"??? ");
    break;
  }

  strcat(current_dir_names[DIR_DISKS],machine_sub_dir);
  strcat(current_dir_names[DIR_TAPES],machine_sub_dir);
  strcat(current_dir_names[DIR_CARTS],machine_sub_dir);
  strcat(current_dir_names[DIR_SNAPS],machine_sub_dir);
  strcpy(current_dir_names[DIR_ROMS], machine_sub_dir);

  switch (circle_get_machine_timing()) {
  case MACHINE_TIMING_NTSC_HDMI:
    strcat(machine_info_txt, "NTSC 60Hz HDMI");
    break;
  case MACHINE_TIMING_NTSC_COMPOSITE:
    strcat(machine_info_txt, "NTSC 60Hz Composite");
    break;
  case MACHINE_TIMING_NTSC_CUSTOM:
    strcat(machine_info_txt, "NTSC 60Hz Custom");
    break;
  case MACHINE_TIMING_PAL_HDMI:
    strcat(machine_info_txt, "PAL 50Hz HDMI");
    break;
  case MACHINE_TIMING_PAL_COMPOSITE:
    strcat(machine_info_txt, "PAL 50Hz Composite");
    break;
  case MACHINE_TIMING_PAL_CUSTOM:
    strcat(machine_info_txt, "PAL 50Hz Custom");
    break;
  default:
    strcat(machine_info_txt, "Error");
    break;
  }

  ui_menu_add_button(MENU_TEXT, root, machine_info_txt);

  ui_menu_add_button(MENU_ABOUT, root, "About...");
  ui_menu_add_button(MENU_LICENSE, root, "License...");

  ui_menu_add_divider(root);

  ui_menu_add_button(MENU_AUTOSTART, root, "Autostart Prg/Disk...");

  machine_parent = ui_menu_add_folder(root, "Machine");
    menu_build_machine(machine_parent);
    menu_build_machine_switch(machine_parent);

  drive_parent = ui_menu_add_folder(root, "Drives");

    parent = ui_menu_add_folder(drive_parent, "Drive 8");

    if (machine_class != VICE_MACHINE_VIC20) {
     resources_get_int_sprintf("IECDevice%i", &tmp, 8);
     ui_menu_add_toggle(MENU_IECDEVICE_8, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_8, parent, "Select IEC Dir...");
    }
    add_parallel_cable_option(parent, MENU_PARALLEL_8, 8);
    ui_menu_add_button(MENU_ATTACH_DISK_8, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_8, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_8, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 9");
    if (machine_class != VICE_MACHINE_VIC20) {
     resources_get_int_sprintf("IECDevice%i", &tmp, 9);
     ui_menu_add_toggle(MENU_IECDEVICE_9, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_9, parent, "Select IEC Dir...");
    }
    add_parallel_cable_option(parent, MENU_PARALLEL_9, 9);
    ui_menu_add_button(MENU_ATTACH_DISK_9, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_9, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_9, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 10");
    if (machine_class != VICE_MACHINE_VIC20) {
     resources_get_int_sprintf("IECDevice%i", &tmp, 10);
     ui_menu_add_toggle(MENU_IECDEVICE_10, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_10, parent, "Select IEC Dir...");
    }
    add_parallel_cable_option(parent, MENU_PARALLEL_10, 10);
    ui_menu_add_button(MENU_ATTACH_DISK_10, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_10, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_10, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 11");
    if (machine_class != VICE_MACHINE_VIC20) {
     resources_get_int_sprintf("IECDevice%i", &tmp, 11);
     ui_menu_add_toggle(MENU_IECDEVICE_11, parent, "IEC FileSystem", tmp);
     ui_menu_add_button(MENU_IECDIR_11, parent, "Select IEC Dir...");
    }
    add_parallel_cable_option(parent, MENU_PARALLEL_11, 11);
    ui_menu_add_button(MENU_ATTACH_DISK_11, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_11, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_11, parent, "Change Model...");

    ui_menu_add_button(MENU_DRIVE_CHANGE_ROM, drive_parent, "Change ROM...");

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
      ui_menu_add_button(MENU_CREATE_P64, parent, "P64...");
      ui_menu_add_button(MENU_CREATE_X64, parent, "X64...");

  parent = menu_build_cartridge(root);
  ui_menu_add_button(MENU_TEXT, parent, "");
  ui_menu_add_button(MENU_MAKE_CART_DEFAULT, parent,
                     "Set current cart default (Need Save)");

  if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C128) {
    ui_menu_add_button(MENU_SAVE_EASYFLASH, parent, "Save EasyFlash Now");
    ui_menu_add_button(MENU_CART_FREEZE, parent, "Cartridge Freeze");
  }

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
    resources_get_int("DatasetteResetWithCPU", &tmp);
    tape_reset_with_machine_item =
      ui_menu_add_toggle(MENU_TAPE_RESET_WITH_MACHINE, tape_parent,
                         "Reset Tape with Machine Reset", tmp);

  ui_menu_add_divider(root);

  parent = ui_menu_add_folder(root, "Snapshots");
  ui_menu_add_button(MENU_LOAD_SNAP, parent, "Load Snapshot...");
  ui_menu_add_button(MENU_SAVE_SNAP, parent, "Save Snapshot...");

  video_parent = parent = ui_menu_add_folder(root, "Video");

  if (machine_class == VICE_MACHINE_C128) {
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

  palette_item_0 = menu_build_palette_options(MENU_COLOR_PALETTE_0, parent);

  child = ui_menu_add_folder(parent, "Color Adjustments...");

  brightness_item_0 =
      ui_menu_add_range(MENU_COLOR_BRIGHTNESS_0, child, "Brightness", 0, 2000,
                        100, get_color_brightness(0));
  contrast_item_0 = ui_menu_add_range(MENU_COLOR_CONTRAST_0, child, "Contrast", 0,
                                    2000, 100, get_color_contrast(0));
  gamma_item_0 = ui_menu_add_range(MENU_COLOR_GAMMA_0, child, "Gamma", 0, 4000, 100,
                                 get_color_gamma(0));
  tint_item_0 = ui_menu_add_range(MENU_COLOR_TINT_0, child, "Tint", 0, 2000, 100,
                                get_color_tint(0));
  ui_menu_add_button(MENU_COLOR_RESET_0, child, "Reset");

  int defaultHBorderTrim;
  int defaultVBorderTrim;
  int defaultAspect;
  if (machine_class == VICE_MACHINE_VIC20) {
     defaultHBorderTrim = DEFAULT_VIC_H_BORDER_TRIM;
     defaultVBorderTrim = DEFAULT_VIC_V_BORDER_TRIM;
     defaultAspect = DEFAULT_VIC_ASPECT;
  } else {
     defaultHBorderTrim = DEFAULT_VICII_H_BORDER_TRIM;
     defaultVBorderTrim = DEFAULT_VICII_V_BORDER_TRIM;
     defaultAspect = DEFAULT_VICII_ASPECT;
  }

  h_center_item_0 =
      ui_menu_add_range(MENU_H_CENTER_0, parent, "H Center",
          -48, 48, 1, 0);
  v_center_item_0 =
      ui_menu_add_range(MENU_V_CENTER_0, parent, "V Center",
          -48, 48, 1, 0);
  h_border_item_0 =
      ui_menu_add_range(MENU_H_BORDER_0, parent, "H Border Trim %",
          0, 100, 1, defaultHBorderTrim);
  v_border_item_0 =
      ui_menu_add_range(MENU_V_BORDER_0, parent, "V Border Trim %",
          0, 100, 1, defaultVBorderTrim);
  child = aspect_item_0 =
      ui_menu_add_range(MENU_ASPECT_0, parent, "Aspect Ratio",
           100, 180, 1, defaultAspect);
  child->divisor = 100;

  if (machine_class == VICE_MACHINE_C128) {
     parent = ui_menu_add_folder(video_parent, "VDC");

     palette_item_1 = menu_build_palette_options(MENU_COLOR_PALETTE_1, parent);

     child = ui_menu_add_folder(parent, "Color Adjustments...");

     brightness_item_1 =
         ui_menu_add_range(MENU_COLOR_BRIGHTNESS_1, child, "Brightness", 0, 2000,
                           100, get_color_brightness(1));
     contrast_item_1 = ui_menu_add_range(MENU_COLOR_CONTRAST_1, child, "Contrast", 0,
                                       2000, 100, get_color_contrast(1));
     gamma_item_1 = ui_menu_add_range(MENU_COLOR_GAMMA_1, child, "Gamma", 0, 4000,
                                       100, get_color_gamma(1));
     tint_item_1 = ui_menu_add_range(MENU_COLOR_TINT_1, child, "Tint", 0, 2000, 100,
                                get_color_tint(1));
     ui_menu_add_button(MENU_COLOR_RESET_1, child, "Reset");

     h_center_item_1 =
         ui_menu_add_range(MENU_H_CENTER_1, parent, "H Center",
             -48, 48, 1, 0);
     v_center_item_1 =
         ui_menu_add_range(MENU_V_CENTER_1, parent, "V Center",
             -48, 48, 1, 0);
     h_border_item_1 =
         ui_menu_add_range(MENU_H_BORDER_1, parent, "H Border Trim %",
             0, 100, 1, DEFAULT_VDC_H_BORDER_TRIM);
     v_border_item_1 =
         ui_menu_add_range(MENU_V_BORDER_1, parent, "V Border Trim %",
             0, 100, 1, DEFAULT_VDC_V_BORDER_TRIM);
     child = aspect_item_1 =
         ui_menu_add_range(MENU_ASPECT_1, parent, "Aspect Ratio",
              100, 180, 1, DEFAULT_VDC_ASPECT);
     child->divisor = 100;
  }

  ui_menu_add_button(MENU_CALC_TIMING, video_parent,
                     "Custom HDMI mode timing calc...");

  parent = ui_menu_add_folder(root, "Sound");

  volume_item = ui_menu_add_range(MENU_VOLUME, parent,
      "Volume ", 0, 100, 1, 100);

  // Resid by default
  child = sid_engine_item =
      ui_menu_add_multiple_choice(MENU_SID_ENGINE, parent, "Sid Engine");
  child->num_choices = 2;
  child->value = MENU_SID_ENGINE_RESID;
  strcpy(child->choices[MENU_SID_ENGINE_FAST], "Fast");
  strcpy(child->choices[MENU_SID_ENGINE_RESID], "ReSid");
  child->choice_ints[MENU_SID_ENGINE_FAST] = SID_ENGINE_FASTSID;
  child->choice_ints[MENU_SID_ENGINE_RESID] = SID_ENGINE_RESID;

  // 6581 by default
  child = sid_model_item =
      ui_menu_add_multiple_choice(MENU_SID_MODEL, parent, "Sid Model");
  child->num_choices = 2;
  child->value = MENU_SID_MODEL_6581;
  strcpy(child->choices[MENU_SID_MODEL_6581], "6581");
  strcpy(child->choices[MENU_SID_MODEL_8580], "8580");
  child->choice_ints[MENU_SID_MODEL_6581] = SID_MODEL_6581;
  child->choice_ints[MENU_SID_MODEL_8580] = SID_MODEL_8580;

  // Filter on by default
  child = sid_filter_item =
      ui_menu_add_toggle(MENU_SID_FILTER, parent, "Sid Filter", 0);

  parent = ui_menu_add_folder(root, "Keyboard");
  child = keyboard_type_item = ui_menu_add_multiple_choice(
      MENU_KEYBOARD_TYPE, parent, "Layout (Needs Save+Reboot)");
  child->num_choices = 2;
  child->value = KEYBOARD_TYPE_US;
  strcpy(child->choices[KEYBOARD_TYPE_US], "US");
  strcpy(child->choices[KEYBOARD_TYPE_UK], "UK");

  if (machine_class == VICE_MACHINE_C128) {
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

  child = port_1_menu_item = ui_menu_add_multiple_choice(
      MENU_JOYSTICK_PORT_1, parent, "Port 1");
  child->num_choices = 12;
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

  if (emu_get_num_joysticks() > 1) {
    child = port_2_menu_item = ui_menu_add_multiple_choice(
        MENU_JOYSTICK_PORT_2, parent, "Port 2");
    child->num_choices = 12;
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
  }

  ui_menu_add_button(MENU_CONFIGURE_USB_0, parent, "Configure USB Joy 1...");
  ui_menu_add_button(MENU_CONFIGURE_USB_1, parent, "Configure USB Joy 2...");

  usb_pref_0 = 0;
  usb_pref_1 = 0;
  usb_x_axis_0 = 0;
  usb_y_axis_0 = 1;
  usb_x_axis_1 = 0;
  usb_y_axis_1 = 1;
  usb_x_thresh_0 = .50;
  usb_y_thresh_0 = .50;
  usb_x_thresh_1 = .50;
  usb_y_thresh_1 = .50;
  for (j = 0; j < MAX_USB_BUTTONS; j++) {
    usb_0_button_assignments[j] = (j == 0 ? BTN_ASSIGN_FIRE : BTN_ASSIGN_UNDEF);
    usb_1_button_assignments[j] = (j == 0 ? BTN_ASSIGN_FIRE : BTN_ASSIGN_UNDEF);
    usb_button_bits[j] = 1 << j;
  }

  ui_menu_add_button(MENU_CONFIGURE_KEYSET1, parent, "Configure Keyset 1...");
  ui_menu_add_button(MENU_CONFIGURE_KEYSET2, parent, "Configure Keyset 2...");

  ui_menu_add_divider(root);

  parent = ui_menu_add_folder(root, "Prefs");

#ifndef RASPI_LITE
  drive_sounds_item = ui_menu_add_toggle(MENU_DRIVE_SOUND_EMULATION, parent,
                                         "Drive sound emulation", 0);
  drive_sounds_vol_item =
      ui_menu_add_range(MENU_DRIVE_SOUND_EMULATION_VOLUME, parent,
                        "Drive sound emulation volume", 0, 1000, 100, 1000);
#endif

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

  child = gpio_config_item =
      ui_menu_add_multiple_choice(MENU_GPIO_CONFIG, parent, "GPIO Config");
     child->num_choices = 3;
     child->value = 0;
     strcpy(child->choices[0], "#1 (Nav+Joy)");
     strcpy(child->choices[1], "#2 (Kyb+Joy)");
     strcpy(child->choices[2], "#3 (Waveshare Hat)");

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

  circle_set_volume(volume_item->value);

  video_canvas_change_palette(0, palette_item_0->value);
  if (machine_class == VICE_MACHINE_C128) {
    video_canvas_change_palette(1, palette_item_1->value);
  }
  ui_set_hotkeys();
  ui_set_joy_devs();

  apply_video_adjustments(FB_LAYER_VIC,
     h_center_item_0->value,
     v_center_item_0->value,
     (double)(100-h_border_item_0->value) / 100.0d,
     (double)(100-v_border_item_0->value) / 100.0d,
     (double)(aspect_item_0->value) / 100.0d,
     0.0d, 0.0d, 0.0d, 0.0d, 0);

  // Menu gets the same adjustments
  apply_video_adjustments(FB_LAYER_UI,
     h_center_item_0->value,
     v_center_item_0->value,
     (double)(100-h_border_item_0->value) / 100.0d,
     (double)(100-v_border_item_0->value) / 100.0d,
     (double)(aspect_item_0->value) / 100.0d,
     0.0d, 0.0d, 0.0d, 0.0d, 3);

  if (machine_class == VICE_MACHINE_C128) {
     apply_video_adjustments(FB_LAYER_VDC,
        h_center_item_1->value,
        v_center_item_1->value,
        (double)(100-h_border_item_1->value) / 100.0d,
        (double)(100-v_border_item_1->value) / 100.0d,
        (double)(aspect_item_1->value) / 100.0d,
        0.0d, 0.0d, 0.0d, 0.0d, 1);
  }

  video_init_overlay(statusbar_padding_item->value,
                     c40_80_column_item->value,
                     vkbd_transparency_item->value);

  joystick_set_potx(pot_x_high_value);
  joystick_set_poty(pot_y_high_value);

  // Always turn off resampling
  resources_set_int("SidResidSampling", 0);
  set_video_cache(0);
  set_hw_scale(0);

#ifdef RASPI_LITE
  resources_set_int("DriveSoundEmulation", 0);
  resources_set_int("DriveSoundEmulationVolume", 0);
#endif

  // This can somehow get turned off. Make sure its always 1.
  resources_set_int("Datasette", 1);
  resources_set_int("Mouse", 1);

  // For now, all our drives will always be file system devices.
  resources_set_int("FileSystemDevice8", 1);
  resources_set_int("FileSystemDevice9", 1);
  resources_set_int("FileSystemDevice10", 1);
  resources_set_int("FileSystemDevice11", 1);

  // Restore last iec dirs for all drives
  const char *tmpf;
  resources_get_string_sprintf("FSDevice%iDir", &tmpf, 8);
  strcpy (last_iec_dir[0], tmpf);
  resources_get_string_sprintf("FSDevice%iDir", &tmpf, 9);
  strcpy (last_iec_dir[1], tmpf);
  resources_get_string_sprintf("FSDevice%iDir", &tmpf, 10);
  strcpy (last_iec_dir[2], tmpf);
  resources_get_string_sprintf("FSDevice%iDir", &tmpf, 11);
  strcpy (last_iec_dir[3], tmpf);
}

int statusbar_never(void) {
  return statusbar_item->value == OVERLAY_NEVER;
}

int statusbar_always(void) {
  return statusbar_item->value == OVERLAY_ALWAYS || statusbar_forced;
}

// Stuff to do when menu is activated
void menu_about_to_activate() {}

// Stuff to do before going back to emulator
void menu_about_to_deactivate() {}

// These are called on the main loop
void menu_quick_func(int button_assignment) {
  int value;
  switch (button_assignment) {
  case BTN_ASSIGN_WARP:
    resources_get_int("WarpMode", &value);
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
    show_cart_osd_menu();
    break;
  case BTN_ASSIGN_CART_FREEZE:
    keyboard_clear_keymatrix();
    raspi_cartridge_trigger_freeze();
    break;
  case BTN_ASSIGN_RESET_HARD:
    if (reset_confirm_item->value) {
      // Will come back here with HARD2 if confirmed.
      show_confirm_osd_menu(BTN_ASSIGN_RESET_HARD2);
      return;
    }
  // fallthrough
  case BTN_ASSIGN_RESET_HARD2:
    menu_machine_reset(MACHINE_RESET_MODE_HARD, 0 /* no pop */);
    break;
  case BTN_ASSIGN_RESET_SOFT:
    if (reset_confirm_item->value) {
      // Will come back here with SOFT2 if confirmed.
      show_confirm_osd_menu(BTN_ASSIGN_RESET_SOFT2);
      return;
    }
  // fallthrough
  case BTN_ASSIGN_RESET_SOFT2:
    menu_machine_reset(MACHINE_RESET_MODE_SOFT, 0 /* no pop */);
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
  return gpio_config_item->value;
}

int emu_get_num_joysticks(void) {
  if (machine_class == VICE_MACHINE_VIC20) {
    return 1;
  }
  return 2;
}
