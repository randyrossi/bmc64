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

#include "attach.h"
#include "autostart.h"
#include "cartridge.h"
#include "datasette.h"
#include "demo.h"
#include "drive.h"
#include "diskimage.h"
#include "joy.h"
#include "joyport.h"
#include "joyport/joystick.h"
#include "kbd.h"
#include "keyboard.h"
#include "machine.h"
#include "menu_cart_osd.h"
#include "menu_confirm_osd.h"
#include "menu_tape_osd.h"
#include "menu_timing.h"
#include "menu_usb.h"
#include "menu_keyset.h"
#include "overlay.h"
#include "raspi_machine.h"
#include "raspi_util.h"
#include "resources.h"
#include "sid.h"
#include "tape.h"
#include "text.h"
#include "ui.h"
#include "util.h"
#include "vdrive-internal.h"

// This feature is hidden for now. Activated 'pcb' input method where
// both keyboard and joysticks can be hooked up with a PCB or
// to eliminate USB keyboard.  Power switch is actually a power switch
// and the power port is actually power unlike the Keyrah.
//#define RASPI_SUPPORT_PCB 1

// For filename filters
typedef enum {
   FILTER_NONE,
   FILTER_DISK,
   FILTER_CART,
   FILTER_TAPE,
   FILTER_SNAP,
} FileFilter;

extern struct joydev_config joydevs[MAX_JOY_PORTS];

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
struct menu_item *sid_engine_item;
struct menu_item *sid_model_item;
struct menu_item *sid_filter_item;
struct menu_item *overlay_item;
struct menu_item *tape_reset_with_machine_item;

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
struct menu_item *use_pcb_item;
struct menu_item *active_display_item;

struct menu_item *h_border_item_0;
struct menu_item *v_border_item_0;
struct menu_item *aspect_item_0;

struct menu_item *h_border_item_1;
struct menu_item *v_border_item_1;
struct menu_item *aspect_item_1;

static int unit;
static int joyswap;
static int force_overlay;

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
#define NUM_DIR_TYPES 6
typedef enum {
   DIR_ROOT,
   DIR_DISKS,
   DIR_TAPES,
   DIR_CARTS,
   DIR_SNAPS,
   DIR_ROMS,
} DirType;

// What directories to initialize file search dialogs with for
// each type of file.
// TODO: Make these start dirs configurable.
static const char default_dir_names[NUM_DIR_TYPES][16] = {
    "/", "/disks", "/tapes", "/carts", "/snapshots", "/roms"};

// Keep track of current directory for each type of file.
static char current_dir_names[NUM_DIR_TYPES][256];

// Temp storage for full path name concatenations.
static char full_path_str[256];

// Keep track of last known position in the file list.
static int current_dir_pos[NUM_DIR_TYPES];

TEST_FILTER_MACRO(test_disk_name, num_disk_ext, disk_filt_ext);
TEST_FILTER_MACRO(test_tape_name, num_tape_ext, tape_filt_ext);
TEST_FILTER_MACRO(test_cart_name, num_cart_ext, cart_filt_ext);
TEST_FILTER_MACRO(test_snap_name, num_snap_ext, snap_filt_ext);

// Clears the file menu and populates it with files.
static void list_files(struct menu_item *parent,
                       DirType dir_type, FileFilter filter,
                       int menu_id) {
  DIR *dp;
  struct dirent *ep;
  int i;
  int include;

  char *currentDir = current_dir_names[dir_type];
  dp = opendir(currentDir);
  if (dp == NULL) {
    char dir_scratch[256];
    strcpy(dir_scratch, "(");
    strcat(dir_scratch, currentDir);
    strcat(dir_scratch, " Not Found - Using /)");

    // Fall back to root
    strcpy(current_dir_names[dir_type], "/");
    currentDir = current_dir_names[dir_type];
    dp = opendir(currentDir);
    ui_menu_add_button(MENU_TEXT, parent, dir_scratch);
    if (dp == NULL) {
      return;
    }
  }

  ui_menu_add_button(MENU_TEXT, parent, currentDir);
  ui_menu_add_divider(parent);

  if (strcmp(currentDir, "/") != 0) {
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

  switch (machine_class) {
  case VICE_MACHINE_C64:
    ui_menu_add_button(MENU_TEXT, about_root, "BMC64 v2.0");
    ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal C64 Emulator");
    break;
  case VICE_MACHINE_C128:
    ui_menu_add_button(MENU_TEXT, about_root, "BMC128 v2.0");
    ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal C128 Emulator");
    break;
  case VICE_MACHINE_VIC20:
    ui_menu_add_button(MENU_TEXT, about_root, "BMVIC20 v2.0");
    ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal Vic20 Emulator");
    break;
  default:
    ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal ??? Emulator");
    break;
  }
  ui_menu_add_button(MENU_TEXT, about_root, "For the Rasbperry Pi 2/3");
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

static void drive_change_model() {
  struct menu_item *model_root = ui_push_menu(12, 8);
  struct menu_item *item;

  int current_drive_type;
  resources_get_int_sprintf("Drive%iType", &current_drive_type, unit);

  item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "None");
  item->value = DRIVE_TYPE_NONE;
  if (current_drive_type == DRIVE_TYPE_NONE) {
    strcat(item->displayed_value, " (*)");
  }

  if (drive_check_type(DRIVE_TYPE_1541, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1541");
    item->value = DRIVE_TYPE_1541;
    if (current_drive_type == DRIVE_TYPE_1541) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1541II, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1541II");
    item->value = DRIVE_TYPE_1541II;
    if (current_drive_type == DRIVE_TYPE_1541II) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1571, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1571");
    item->value = DRIVE_TYPE_1571;
    if (current_drive_type == DRIVE_TYPE_1571) {
      strcat(item->displayed_value, " (*)");
    }
  }
  if (drive_check_type(DRIVE_TYPE_1581, unit - 8) > 0) {
    item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1581");
    item->value = DRIVE_TYPE_1581;
    if (current_drive_type == DRIVE_TYPE_1581) {
      strcat(item->displayed_value, " (*)");
    }
  }
}

static void ui_set_hotkeys() {
  kbd_set_hotkey_function(0, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(1, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(2, 0, BTN_ASSIGN_UNDEF);
  kbd_set_hotkey_function(3, 0, BTN_ASSIGN_UNDEF);

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
}

static void ui_set_joy_items() {
  int joydev;
  int i;
  for (joydev = 0; joydev < circle_num_joysticks(); joydev++) {
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
    if (circle_num_joysticks() > 1 &&
        port_2_menu_item->choice_ints[port_2_menu_item->value]
            == JOYDEV_MOUSE) {
       resources_set_int("JoyPort2Device", JOYPORT_ID_NONE);
       port_2_menu_item->value = 0;
    }
    resources_set_int("JoyPort1Device", JOYPORT_ID_MOUSE_1351);
  } else {
    resources_set_int("JoyPort1Device", JOYPORT_ID_JOYSTICK);
  }

  if (circle_num_joysticks() > 1) {
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
  fprintf(fp, "overlay=%d\n", overlay_item->value);
  fprintf(fp, "tapereset=%d\n", tape_reset_with_machine_item->value);
  fprintf(fp, "reset_confirm=%d\n", reset_confirm_item->value);
#ifdef RASPI_SUPPORT_PCB
  fprintf(fp, "pcb=%d\n", use_pcb_item->value);
#endif

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

  fclose(fp);

  return 0;
}

// Make joydev reflect menu choice
static void ui_set_joy_devs() {
  if (joydevs[0].port == 1) {
    joydevs[0].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
  } else if (circle_num_joysticks() > 1 && joydevs[0].port == 2) {
    joydevs[0].device = port_2_menu_item->choice_ints[port_2_menu_item->value];
  }

  if (joydevs[1].port == 1) {
    joydevs[1].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
  } else if (circle_num_joysticks() > 1 && joydevs[1].port == 2) {
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
  resources_get_int("DriveSoundEmulation", &drive_sounds_item->value);
  resources_get_int("DriveSoundEmulationVolume", &drive_sounds_vol_item->value);

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
    } else if (strcmp(name, "overlay") == 0) {
      overlay_item->value = value;
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
    } else if (strcmp(name, "reset_confirm") == 0) {
      reset_confirm_item->value = value;
    } else if (strcmp(name, "pcb") == 0) {
#ifdef RASPI_SUPPORT_PCB
      use_pcb_item->value = value;
#endif
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

static char *fullpath(DirType dir_type, char *name) {
  strcpy(full_path_str, current_dir_names[dir_type]);
  strcat(full_path_str, "/");
  strcat(full_path_str, name);
  return full_path_str;
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
     case MENU_TAPE_FILE:
       ui_info("Attaching...");
       if (tape_image_attach(1, fullpath(DIR_TAPES, item->str_value)) < 0) {
         ui_pop_menu();
         ui_error("Failed to attach tape image");
       } else {
         ui_pop_all_and_toggle();
       }
       return;
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
    return DIR_CARTS;
  case MENU_KERNAL_FILE:
  case MENU_BASIC_FILE:
  case MENU_CHARGEN_FILE:
    return DIR_ROMS;
  case MENU_AUTOSTART_FILE:
    return DIR_ROOT;
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
    show_files(DIR_CARTS, FILTER_NONE, item->id, 1);
    break;
  case MENU_KERNAL_FILE:
  case MENU_BASIC_FILE:
  case MENU_CHARGEN_FILE:
    show_files(DIR_ROMS, FILTER_NONE, item->id, 1);
    break;
  case MENU_AUTOSTART_FILE:
    show_files(DIR_ROOT, FILTER_NONE, item->id, 1);
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

static void do_video_settings(int layer,
                              struct menu_item* hborder_item,
                              struct menu_item* vborder_item,
                              struct menu_item* aspect_item) {
  double h = (double)(100-hborder_item->value) / 100.0d;
  double v = (double)(100-vborder_item->value) / 100.0d;
  double a = (double)(aspect_item->value) / 100.0d;

  // Tell videoarch about these changes
  apply_video_adjustments(layer, h, v, a);
}

// Interpret what menu item changed and make the change to vice
static void menu_value_changed(struct menu_item *item) {
  switch (item->id) {
  case MENU_ATTACH_DISK_8:
  case MENU_IECDEVICE_8:
  case MENU_DRIVE_CHANGE_MODEL_8:
    unit = 8;
    break;
  case MENU_ATTACH_DISK_9:
  case MENU_IECDEVICE_9:
  case MENU_DRIVE_CHANGE_MODEL_9:
    unit = 9;
    break;
  case MENU_ATTACH_DISK_10:
  case MENU_IECDEVICE_10:
  case MENU_DRIVE_CHANGE_MODEL_10:
    unit = 10;
    break;
  case MENU_ATTACH_DISK_11:
  case MENU_IECDEVICE_11:
  case MENU_DRIVE_CHANGE_MODEL_11:
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
    video_request_transparency(FB_LAYER_VIC);
    video_canvas_change_palette(0, item->value);
    return;
  case MENU_COLOR_PALETTE_1:
    video_request_transparency(FB_LAYER_VDC);
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
  case MENU_ATTACH_DISK_8:
  case MENU_ATTACH_DISK_9:
  case MENU_ATTACH_DISK_10:
  case MENU_ATTACH_DISK_11:
    show_files(DIR_DISKS, FILTER_DISK, MENU_DISK_FILE, 0);
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
    cartridge_detach_image(CARTRIDGE_CRT);
    ui_pop_all_and_toggle();
    return;
  case MENU_SOFT_RESET:
    resources_set_string_sprintf("FSDevice%iDir", "/", 8);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    ui_pop_all_and_toggle();
    return;
  case MENU_HARD_RESET:
    resources_set_string_sprintf("FSDevice%iDir", "/", 8);
    machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    ui_pop_all_and_toggle();
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
    video_request_transparency(FB_LAYER_VIC);
    set_color_brightness(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_CONTRAST_0:
    video_request_transparency(FB_LAYER_VIC);
    set_color_contrast(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_GAMMA_0:
    video_request_transparency(FB_LAYER_VIC);
    set_color_gamma(0, item->value);
    video_color_setting_changed(0);
    return;
  case MENU_COLOR_TINT_0:
    video_request_transparency(FB_LAYER_VIC);
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
    video_request_transparency(FB_LAYER_VDC);
    set_color_brightness(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_CONTRAST_1:
    video_request_transparency(FB_LAYER_VDC);
    set_color_contrast(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_GAMMA_1:
    video_request_transparency(FB_LAYER_VDC);
    set_color_gamma(1, item->value);
    video_color_setting_changed(1);
    return;
  case MENU_COLOR_TINT_1:
    video_request_transparency(FB_LAYER_VDC);
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
  case MENU_DRIVE_SELECT:
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
  case MENU_SAVE_EASYFLASH:
    if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
      ui_error("Problem saving");
    } else {
      ui_pop_all_and_toggle();
    }
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
    if (item->value == MENU_ACTIVE_DISPLAY_VICII) {
       enable_vic(1);
       enable_vdc(0);
    } else if (item->value == MENU_ACTIVE_DISPLAY_VDC) {
       enable_vdc(1);
       enable_vic(0);
    }
    break;
  case MENU_H_BORDER_0:
  case MENU_V_BORDER_0:
  case MENU_ASPECT_0:
    video_request_transparency(FB_LAYER_VIC);
    do_video_settings(FB_LAYER_VIC,
        h_border_item_0,
        v_border_item_0,
        aspect_item_0);
    break;
  case MENU_H_BORDER_1:
  case MENU_V_BORDER_1:
  case MENU_ASPECT_1:
    video_request_transparency(FB_LAYER_VDC);
    do_video_settings(FB_LAYER_VDC,
        h_border_item_1,
        v_border_item_1,
        aspect_item_1);
    break;
  
  }

  // Only items that were for file selection/nav should have these set...
  if (item->sub_id == MENU_SUB_PICK_FILE) {
    select_file(item);
    return;
  } else if (item->sub_id == MENU_SUB_UP_DIR) {
    up_dir(item);
    return;
  } else if (item->sub_id == MENU_SUB_ENTER_DIR) {
    enter_dir(item);
    return;
  }
}

// Returns what input preference user has for this usb device
void circle_usb_pref(int device, int *usb_pref, int *x_axis, int *y_axis,
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
  item->num_choices = 10;
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

  if (machine_class != VICE_MACHINE_C64 && machine_class != VICE_MACHINE_C128) {
     item->choice_disabled[HOTKEY_CHOICE_SWAP_PORTS] = 1;
     item->choice_disabled[HOTKEY_CHOICE_CART_FREEZE] = 1;
  }
}

void build_menu(struct menu_item *root) {
  struct menu_item *parent;
  struct menu_item *video_parent;
  struct menu_item *drive_parent;
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
  for (i = 0; i < NUM_DIR_TYPES; i++) {
    strcpy(current_dir_names[i], default_dir_names[i]);
  }

  switch (machine_class) {
  case VICE_MACHINE_C64:
    strcpy(current_dir_names[DIR_ROMS], "/C64");
    break;
  case VICE_MACHINE_C128:
    strcpy(current_dir_names[DIR_ROMS], "/C128");
    break;
  case VICE_MACHINE_VIC20:
    strcpy(current_dir_names[DIR_ROMS], "/VIC20");
    break;
  default:
    break;
  }

  switch (circle_get_machine_timing()) {
  case MACHINE_TIMING_NTSC_HDMI:
    ui_menu_add_button(MENU_TEXT, root, "Timing: NTSC 60Hz HDMI");
    break;
  case MACHINE_TIMING_NTSC_COMPOSITE:
    ui_menu_add_button(MENU_TEXT, root, "Timing: NTSC 60Hz Composite");
    break;
  case MACHINE_TIMING_NTSC_CUSTOM:
    ui_menu_add_button(MENU_TEXT, root, "Timing: NTSC 60Hz Custom");
    break;
  case MACHINE_TIMING_PAL_HDMI:
    ui_menu_add_button(MENU_TEXT, root, "Timing: PAL 50Hz HDMI");
    break;
  case MACHINE_TIMING_PAL_COMPOSITE:
    ui_menu_add_button(MENU_TEXT, root, "Timing: PAL 50Hz Composite");
    break;
  case MACHINE_TIMING_PAL_CUSTOM:
    ui_menu_add_button(MENU_TEXT, root, "Timing: PAL 50Hz Custom");
    break;
  default:
    ui_menu_add_button(MENU_TEXT, root, "Timing: ERROR");
    break;
  }

  ui_menu_add_button(MENU_ABOUT, root, "About...");
  ui_menu_add_button(MENU_LICENSE, root, "License...");

  ui_menu_add_divider(root);

  parent = ui_menu_add_folder(root, "ROMs...");
  if (machine_class == VICE_MACHINE_VIC20 ||
         machine_class == VICE_MACHINE_C64) {
     ui_menu_add_button(MENU_LOAD_KERNAL, parent, "Load Kernal ROM...");
     ui_menu_add_button(MENU_LOAD_BASIC, parent, "Load Basic ROM...");
     ui_menu_add_button(MENU_LOAD_CHARGEN, parent, "Load Chargen ROM...");
  } else if (machine_class == VICE_MACHINE_C128) {
     ui_menu_add_button(MENU_C128_LOAD_KERNAL, parent, "Load C128 Kernal ROM...");
     ui_menu_add_button(MENU_C128_LOAD_BASIC_HI, parent, "Load C128 Basic HI ROM...");
     ui_menu_add_button(MENU_C128_LOAD_BASIC_LO, parent, "Load C128 Basic LO ROM...");
     ui_menu_add_button(MENU_C128_LOAD_CHARGEN, parent, "Load C128 Chargen ROM...");
     ui_menu_add_button(MENU_C128_LOAD_64_KERNAL, parent, "Load C64 Kernal ROM...");
     ui_menu_add_button(MENU_C128_LOAD_64_BASIC, parent, "Load C64 Basic ROM...");
  }

  ui_menu_add_divider(root);

  ui_menu_add_button(MENU_AUTOSTART, root, "Autostart Prg/Disk...");

  parent = ui_menu_add_folder(root, "Create empty Disk");
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

  ui_menu_add_divider(root);

  drive_parent = ui_menu_add_folder(root, "Drives");

    parent = ui_menu_add_folder(drive_parent, "Drive 8");

    if (machine_class != VICE_MACHINE_VIC20) {
     resources_get_int_sprintf("IECDevice%i", &tmp, 8);
     ui_menu_add_toggle(MENU_IECDEVICE_8, parent, "IEC FileSystem", tmp);
    }

    ui_menu_add_button(MENU_ATTACH_DISK_8, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_8, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_8, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 9");
    ui_menu_add_button(MENU_ATTACH_DISK_9, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_9, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_9, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 10");
    ui_menu_add_button(MENU_ATTACH_DISK_10, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_10, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_10, parent, "Change Model...");

    parent = ui_menu_add_folder(drive_parent, "Drive 11");
    ui_menu_add_button(MENU_ATTACH_DISK_11, parent, "Attach Disk...");
    ui_menu_add_button(MENU_DETACH_DISK_11, parent, "Detach Disk");
    ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_11, parent, "Change Model...");

  parent = ui_menu_add_folder(root, "Cartridge");
  if (machine_class == VICE_MACHINE_VIC20) {
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_GENERIC, parent, "Attach generic cart...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_BEHRBONZ, parent, "Attach behrbonz cart...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_MEGACART, parent, "Attach megacart cart...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_FINAL_EXPANSION, parent, "Attach final expansion cart...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_UM, parent, "Attach UM cart...");
    ui_menu_add_button(MENU_VIC20_ATTACH_CART_FP, parent, "Attach FP cart...");
    ui_menu_add_button(MENU_DETACH_CART, parent, "Detach cartridge");

    parent = ui_menu_add_folder(parent, "Add to generic cartridge");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_DETECT, parent, "Smart attach...");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_2000, parent, "Attach 4/8/16k $2000...");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_4000, parent, "Attach 4/8/16k $4000...");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_16K_6000, parent, "Attach 4/8k/16k $6000...");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_8K_A000, parent, "Attach 4/8k $A000...");
       ui_menu_add_button(MENU_VIC20_ATTACH_CART_4K_B000, parent, "Attach 4k $B000...");

  } else {
    ui_menu_add_button(MENU_C64_ATTACH_CART, parent, "Attach cart...");
    ui_menu_add_button(MENU_C64_ATTACH_CART_8K, parent, "Attach 8k raw...");
    ui_menu_add_button(MENU_C64_ATTACH_CART_16K, parent, "Attach 16 raw...");
    ui_menu_add_button(MENU_C64_ATTACH_CART_ULTIMAX, parent, "Attach Ultimax raw...");
    ui_menu_add_button(MENU_DETACH_CART, parent, "Detach cartridge");
  }

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
     child->num_choices = 2;
     child->value = MENU_ACTIVE_DISPLAY_VICII; // TODO grab from settings?
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_VICII], "VICII");
     strcpy(child->choices[MENU_ACTIVE_DISPLAY_VDC], "VDC");
     // Someday, we can add "Both" as an option for Pi4

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

  h_border_item_0 =
      ui_menu_add_range(MENU_H_BORDER_0, parent, "H Border Trim %", 0, 100, 1, 0);
  v_border_item_0 =
      ui_menu_add_range(MENU_V_BORDER_0, parent, "V Border Trim %", 0, 100, 1, 0);
  child = aspect_item_0 =
      ui_menu_add_range(MENU_ASPECT_0, parent, "Aspect Ratio",
           100, 180, 1, 160); // default to 1.6
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

     h_border_item_1 =
         ui_menu_add_range(MENU_H_BORDER_1, parent, "H Border Trim %", 0, 100, 1, 0);
     v_border_item_1 =
         ui_menu_add_range(MENU_V_BORDER_1, parent, "V Border Trim %", 0, 100, 1, 0);
     child = aspect_item_1 =
         ui_menu_add_range(MENU_ASPECT_1, parent, "Aspect Ratio",
              100, 180, 1, 160); // default to 1.6
     child->divisor = 100;
  }

  ui_menu_add_button(MENU_CALC_TIMING, video_parent,
                     "Custom HDMI mode timing calc...");

  parent = ui_menu_add_folder(root, "Sound");
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

  if (machine_class == VICE_MACHINE_VIC20) {
     parent = ui_menu_add_folder(root, "Memory");

     int block_value;
     int blocks;
     resources_get_int("RAMBlock0", &block_value);
     blocks = block_value;
     resources_get_int("RAMBlock1", &block_value);
     blocks |= (block_value << 1);
     resources_get_int("RAMBlock2", &block_value);
     blocks |= (block_value << 2);
     resources_get_int("RAMBlock3", &block_value);
     blocks |= (block_value << 3);
     resources_get_int("RAMBlock5", &block_value);
     blocks |= (block_value << 5);

     ui_menu_add_toggle(MENU_VIC20_MEMORY_3K, parent,
                                         "3Kb ($0400)", blocks & VIC20_BLOCK_0 ? 1: 0);
     ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_2000, parent,
                                         "8kb ($2000)", blocks & VIC20_BLOCK_1 ? 1: 0);
     ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_4000, parent,
                                         "8kb ($4000)", blocks & VIC20_BLOCK_2 ? 1 : 0);
     ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_6000, parent,
                                         "8kb ($6000)", blocks & VIC20_BLOCK_3 ? 1 : 0);
     ui_menu_add_toggle(MENU_VIC20_MEMORY_8K_A000, parent,
                                         "8kb ($A000)", blocks & VIC20_BLOCK_5 ? 1 : 0);
  }

  parent = ui_menu_add_folder(root, "Keyboard");
  child = keyboard_type_item = ui_menu_add_multiple_choice(
      MENU_KEYBOARD_TYPE, parent, "Layout (Needs Save+Reboot)");
  child->num_choices = 2;
  child->value = KEYBOARD_TYPE_US;
  strcpy(child->choices[KEYBOARD_TYPE_US], "US");
  strcpy(child->choices[KEYBOARD_TYPE_UK], "UK");

#ifdef RASPI_SUPPORT_PCB
  if (machine_class == VICE_MACHINE_C64) {
    child = use_pcb_item = ui_menu_add_toggle(
        MENU_USE_PCB, parent, "Use Keyboard/DB9 PCB", 0);
  }
#endif

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

  parent = ui_menu_add_folder(root, "Joyports");

  if (circle_num_joysticks() > 1) {
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

  if (circle_num_joysticks() > 1) {
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

  drive_sounds_item = ui_menu_add_toggle(MENU_DRIVE_SOUND_EMULATION, parent,
                                         "Drive sound emulation", 0);
  drive_sounds_vol_item =
      ui_menu_add_range(MENU_DRIVE_SOUND_EMULATION_VOLUME, parent,
                        "Drive sound emulation volume", 0, 1000, 100, 1000);

  overlay_item =
      ui_menu_add_multiple_choice(MENU_OVERLAY, parent, "Show Status Bar");
  overlay_item->num_choices = 3;
  overlay_item->value = 0;
  strcpy(overlay_item->choices[OVERLAY_NEVER], "Never");
  strcpy(overlay_item->choices[OVERLAY_ALWAYS], "Always");
  strcpy(overlay_item->choices[OVERLAY_ON_ACTIVITY], "On Activity");

  reset_confirm_item = ui_menu_add_toggle(MENU_RESET_CONFIRM, parent,
                                          "Confirm Reset from Emulator", 1);

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
  video_canvas_change_palette(0, palette_item_0->value);
  if (machine_class == VICE_MACHINE_C128) {
    video_canvas_change_palette(1, palette_item_1->value);
  }
  ui_set_hotkeys();
  ui_set_joy_devs();

  joystick_set_potx(pot_x_high_value);
  joystick_set_poty(pot_y_high_value);

  // Always turn off resampling
  resources_set_int("SidResidSampling", 0);
  set_video_cache(0);
  set_hw_scale(0);

  // This can somehow get turned off. Make sure its always 1.
  resources_set_int("Datasette", 1);
  resources_set_int("Mouse", 1);
}

int overlay_enabled(void) { return overlay_item->value != OVERLAY_NEVER; }

int overlay_forced(void) {
  return overlay_item->value == OVERLAY_ALWAYS || force_overlay;
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
  case BTN_ASSIGN_STATUS_TOGGLE:
    // Ignore this if it's already showing.
    if (overlay_item->value == OVERLAY_ALWAYS)
      return;

    if (overlay_showing || force_overlay) {
      // Dismiss
      force_overlay = 0;
      overlay_force_timeout();
    } else {
      force_overlay = 1;
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
    resources_set_string_sprintf("FSDevice%iDir", "/", 8);
    machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    break;
  case BTN_ASSIGN_RESET_SOFT:
    if (reset_confirm_item->value) {
      // Will come back here with SOFT2 if confirmed.
      show_confirm_osd_menu(BTN_ASSIGN_RESET_SOFT2);
      return;
    }
  // fallthrough
  case BTN_ASSIGN_RESET_SOFT2:
    resources_set_string_sprintf("FSDevice%iDir", "/", 8);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    break;
  default:
    break;
  }
}

int circle_use_pcb() {
#ifdef RASPI_SUPPORT_PCB
  if (machine_class == VICE_MACHINE_C64) {
    return use_pcb_item->value;
  }
#endif
  return 0;
}

int circle_num_joysticks(void) {
  if (machine_class == VICE_MACHINE_VIC20) {
    return 1;
  }
  return 2;
}
