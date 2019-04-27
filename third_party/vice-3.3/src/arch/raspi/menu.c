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

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <assert.h>

#include "attach.h"
#include "cartridge.h"
#include "machine.h"
#include "tape.h"
#include "ui.h"
#include "text.h"
#include "joy.h"
#include "resources.h"
#include "autostart.h"
#include "util.h"
#include "datasette.h"
#include "menu_usb.h"
#include "menu_timing.h"
#include "sid.h"
#include "demo.h"
#include "drive.h"
#include "overlay.h"
#include "raspi_machine.h"

// For filename filters
#define FILTER_NONE 0
#define FILTER_DISK 1
#define FILTER_CART 2
#define FILTER_TAPE 3
#define FILTER_SNAP 4

extern struct joydev_config joydevs[2];

// These can be saved
struct menu_item* port_1_menu_item;
struct menu_item* port_2_menu_item;
int usb_pref_0;
int usb_pref_1;
int usb_x_axis_0;
int usb_y_axis_0;
int usb_x_axis_1;
int usb_y_axis_1;
int usb_0_button_assignments[16];
int usb_1_button_assignments[16];
int usb_0_button_bits[16]; // never change
int usb_1_button_bits[16]; // never change
struct menu_item *palette_item;
struct menu_item *keyboard_type_item;
struct menu_item *drive_sounds_item;
struct menu_item *drive_sounds_vol_item;
struct menu_item *menu_alt_f12_item;
struct menu_item *sid_engine_item;
struct menu_item *sid_model_item;
struct menu_item *sid_filter_item;
struct menu_item *overlay_item;
struct menu_item *tape_reset_with_machine_item;
struct menu_item *brightness_item;
struct menu_item *contrast_item;
struct menu_item *gamma_item;
struct menu_item *tint_item;

static int unit;
static int joyswap;
static int force_overlay;

// Held here, exported for kernal to read
int pot_x_high_value;
int pot_x_low_value;
int pot_y_high_value;
int pot_y_low_value;

const int num_disk_ext = 13;
static char disk_filt_ext[13][5] =
    {".d64",".d67",".d71",".d80",".d81",".d82",
     ".d1m",".d2m",".d4m",".g64",".g41",".p64",
     ".x64"};

const int num_tape_ext = 2;
static char tape_filt_ext[2][5] = { ".t64",".tap" };

const int num_cart_ext = 2;
static char cart_filt_ext[2][5] = { ".crt",".bin" };

const int num_snap_ext = 1;
static char snap_filt_ext[1][5] = { ".vsf" };

#define TEST_FILTER_MACRO(funcname, numvar, filtarray) \
static int funcname(char *name) { \
   int include = 0; \
   int len = strlen(name); \
   int i; \
   for (i = 0 ; i < numvar; i++) { \
      if (len > 4 && !strcasecmp(name + len - 4, filtarray[i])) { \
         include = 1; \
         break; \
      } \
   } \
   return include; \
}

// For file type dialogs. Determines what dir we start in. Used
// as index into default_dir_names and current_dir_names.
#define DIR_ROOT 0
#define DIR_DISKS 1
#define DIR_TAPES 2
#define DIR_CARTS 3
#define DIR_SNAPS 4
#define DIR_ROMS 5
#define NUM_DIR_TYPES 6

// What directories to initialize file search dialogs with for
// each type of file.
// TODO: Make these start dirs configurable.
static const char default_dir_names[NUM_DIR_TYPES][16] = {
   "/", "/disks", "/tapes", "/carts", "/snapshots", "/roms"
};

// Keep track of current directory for each type of file.
static char current_dir_names[NUM_DIR_TYPES][256];
static char dir_scratch[256];

TEST_FILTER_MACRO(test_disk_name, num_disk_ext, disk_filt_ext);
TEST_FILTER_MACRO(test_tape_name, num_tape_ext, tape_filt_ext);
TEST_FILTER_MACRO(test_cart_name, num_cart_ext, cart_filt_ext);
TEST_FILTER_MACRO(test_snap_name, num_snap_ext, snap_filt_ext);

// Clears the file menu and populates it with files.
static void list_files(struct menu_item* parent, int dir_type,
                          int filter, int menu_id) {
  DIR *dp;
  struct dirent *ep;
  int i;
  int include;

  char *currentDir = current_dir_names[dir_type];
  dp = opendir (currentDir);
  if (dp == NULL) {
     strcpy (dir_scratch, "(");
     strcat (dir_scratch, currentDir);
     strcat (dir_scratch, " Not Found - Using /)");

     // Fall back to root
     strcpy(current_dir_names[dir_type], "/");
     currentDir = current_dir_names[dir_type];
     dp = opendir (currentDir);
     ui_menu_add_button(MENU_TEXT, parent, dir_scratch);
     if (dp == NULL) {
        return;
     }
  }

  ui_menu_add_button(MENU_TEXT, parent, currentDir);
  ui_menu_add_divider(parent);

  if (strcmp(currentDir,"/") != 0) {
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

  // TODO : Prefetch and order dirs at top
  if (dp != NULL) {
    while (ep = readdir (dp)) {
      if (ep->d_type & DT_DIR) {
        ui_menu_add_button_with_value(
           menu_id, &dirs_root, ep->d_name, 0, ep->d_name, "(dir)")
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
          struct menu_item* new_button = ui_menu_add_button(
             menu_id, &files_root, ep->d_name);
          new_button->sub_id = MENU_SUB_PICK_FILE;
          strncpy(new_button->str_value, ep->d_name, MAX_STR_VAL_LEN-1);
        }
      }
    }

    (void) closedir (dp);
  }

  // Transfer ownership of dirs children first, then files. Childless
  // parents are on the stack.
  ui_add_all(&dirs_root, parent);
  ui_add_all(&files_root, parent);

  assert(dirs_root.first_child == NULL);
  assert(files_root.first_child == NULL);
}

static void show_files(int dir_type, int filter, int menu_id) {
   // Show files
   struct menu_item* file_root = ui_push_menu(-1, -1);
   if (menu_id == MENU_SAVE_SNAP_FILE) {
      ui_menu_add_text_field(menu_id, file_root, "Enter name:", "");
   }
   list_files(file_root, dir_type, filter, menu_id);
}

static void show_about() {
   struct menu_item* about_root = ui_push_menu(32, 8);
   
   switch (machine_class) {
     case VICE_MACHINE_C64:
        ui_menu_add_button(MENU_TEXT, about_root, "BMC64 v1.6");
        ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal C64 Emulator");
        break;
     case VICE_MACHINE_C128:
        ui_menu_add_button(MENU_TEXT, about_root, "BMC128 v1.6");
        ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal C128 Emulator");
        break;
     case VICE_MACHINE_VIC20:
        ui_menu_add_button(MENU_TEXT, about_root, "BMVC20 v1.6");
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
   struct menu_item* license_root = ui_push_menu(-1, -1);
   for (i=0;i<510;i++) {
      ui_menu_add_button(MENU_TEXT, license_root, license[i]);
   }
}

static void configure_usb(int dev) {
   struct menu_item* usb_root = ui_push_menu(-1, -1);
   build_usb_menu(dev, usb_root);
}

static void configure_timing() {
   struct menu_item* timing_root = ui_push_menu(-1, -1);
   build_timing_menu(timing_root);
}

static void drive_change_model() {
   struct menu_item* model_root = ui_push_menu(12, 8);
   struct menu_item* item;

   int current_drive_type;
   resources_get_int_sprintf("Drive%iType", &current_drive_type, unit);

   item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "None");
   item->value = DRIVE_TYPE_NONE;
   if (current_drive_type == DRIVE_TYPE_NONE) {
     strcat(item->displayed_value," (*)");
   }

   if (drive_check_type(DRIVE_TYPE_1541, unit-8) >= 0) {
     item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1541");
     item->value = DRIVE_TYPE_1541;
     if (current_drive_type == DRIVE_TYPE_1541) {
        strcat(item->displayed_value," (*)");
     }
   }
   if (drive_check_type(DRIVE_TYPE_1541II, unit-8) >= 0) {
     item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1541II");
     item->value = DRIVE_TYPE_1541II;
     if (current_drive_type == DRIVE_TYPE_1541II) {
        strcat(item->displayed_value," (*)");
     }
   }
   if (drive_check_type(DRIVE_TYPE_1571, unit-8) >= 0) {
     item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1571");
     item->value = DRIVE_TYPE_1571;
     if (current_drive_type == DRIVE_TYPE_1571) {
        strcat(item->displayed_value," (*)");
     }
   }
   if (drive_check_type(DRIVE_TYPE_1581, unit-8) >= 0) {
     item = ui_menu_add_button(MENU_DRIVE_SELECT, model_root, "1581");
     item->value = DRIVE_TYPE_1581;
     if (current_drive_type == DRIVE_TYPE_1581) {
        strcat(item->displayed_value," (*)");
     }
   }
}

static void ui_set_joy_items()
{
   int joydev;
   int i;
   for (joydev =0 ; joydev < 2; joydev++) {
      struct menu_item* dst;

      if (joydevs[joydev].port == 1) {
         dst = port_1_menu_item;
      } else if (joydevs[joydev].port == 2) {
         dst = port_2_menu_item;
      } else {
         continue;
      }

      // Find which choice matches the device selected and
      // make sure the menu item matches
      for (i=0;i<dst->num_choices;i++) {
        if (dst->choice_ints[i] == joydevs[joydev].device) {
           dst->value = i;
           break;
        }
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
        fp = fopen("/settings.txt","w");
        break;
     case VICE_MACHINE_C128:
        fp = fopen("/settings-c128.txt","w");
        break;
     case VICE_MACHINE_VIC20:
        fp = fopen("/settings-vic20.txt","w");
        break;
     default:
        printf ("ERROR: Unhandled machine\n");
        return 1;
   }

   int r = resources_save(NULL);
   if (r < 0) {
      printf ("resource_save failed with %d\n",r);
      return 1;
   }

   if (fp == NULL) return 1;

   fprintf(fp,"port_1=%d\n",port_1_menu_item->value);
   fprintf(fp,"port_2=%d\n",port_2_menu_item->value);
   fprintf(fp,"usb_0=%d\n",usb_pref_0);
   fprintf(fp,"usb_1=%d\n",usb_pref_1);
   fprintf(fp,"usb_x_0=%d\n",usb_x_axis_0);
   fprintf(fp,"usb_y_0=%d\n",usb_y_axis_0);
   fprintf(fp,"usb_x_1=%d\n",usb_x_axis_1);
   fprintf(fp,"usb_y_1=%d\n",usb_y_axis_1);
   fprintf(fp,"palette=%d\n",palette_item->value);
   fprintf(fp,"keyboard_type=%d\n",keyboard_type_item->value);

   for (i=0;i<16;i++) {
      fprintf(fp,"usb_btn_0=%d\n",usb_0_button_assignments[i]);
   }
   for (i=0;i<16;i++) {
      fprintf(fp,"usb_btn_1=%d\n",usb_1_button_assignments[i]);
   }
   fprintf(fp,"alt_f12=%d\n",menu_alt_f12_item->value);

   fprintf(fp,"overlay=%d\n",overlay_item->value);
   fprintf(fp,"tapereset=%d\n",tape_reset_with_machine_item->value);

   int drive_type;

   resources_get_int_sprintf("Drive%iType", &drive_type, 8);
   fprintf(fp,"drive_type_8=%d\n",drive_type);
   resources_get_int_sprintf("Drive%iType", &drive_type, 9);
   fprintf(fp,"drive_type_9=%d\n",drive_type);
   resources_get_int_sprintf("Drive%iType", &drive_type, 10);
   fprintf(fp,"drive_type_10=%d\n",drive_type);
   resources_get_int_sprintf("Drive%iType", &drive_type, 11);
   fprintf(fp,"drive_type_11=%d\n",drive_type);

   fprintf(fp,"pot_x_high=%d\n",pot_x_high_value);
   fprintf(fp,"pot_x_low=%d\n",pot_x_low_value);
   fprintf(fp,"pot_y_high=%d\n",pot_y_high_value);
   fprintf(fp,"pot_y_low=%d\n",pot_y_low_value);

   fclose(fp);

   return 0;
}

// Make joydev reflect menu choice
static void ui_set_joy_devs() {
   if (joydevs[0].port == 1)
     joydevs[0].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
   else if (joydevs[0].port == 2)
     joydevs[0].device = port_2_menu_item->choice_ints[port_2_menu_item->value];

   if (joydevs[1].port == 1)
     joydevs[1].device = port_1_menu_item->choice_ints[port_1_menu_item->value];
   else if (joydevs[1].port == 2)
     joydevs[1].device = port_2_menu_item->choice_ints[port_2_menu_item->value];
}

static void load_settings() {

   int tmp_value;

   resources_get_int("SidEngine", &tmp_value);
   sid_engine_item->value = viceSidEngineToBmcChoice(tmp_value);

   resources_get_int("SidModel", &tmp_value);
   sid_model_item->value = viceSidModelToBmcChoice(tmp_value);

   resources_get_int("SidFilters", &sid_filter_item->value);
   resources_get_int("DriveSoundEmulation", &drive_sounds_item->value);
   resources_get_int("DriveSoundEmulationVolume", &drive_sounds_vol_item->value );
   brightness_item->value = get_color_brightness();
   contrast_item->value = get_color_contrast();
   gamma_item->value = get_color_gamma();
   tint_item->value = get_color_tint();
   video_color_setting_changed();

   // Default pot values for buttons
   pot_x_high_value = 192;
   pot_x_low_value = 64;
   pot_y_high_value = 192;
   pot_y_low_value = 64;

   FILE *fp;
   switch (machine_class) {
     case VICE_MACHINE_C64:
        fp = fopen("/settings.txt","r");
        break;
     case VICE_MACHINE_C128:
        fp = fopen("/settings-c128.txt","r");
        break;
     case VICE_MACHINE_VIC20:
        fp = fopen("/settings-vic20.txt","r");
        break;
     default:
        printf ("ERROR: Unhandled machine\n");
        return;
   }

   if (fp == NULL) return;

   char name_value[80];
   int value;
   int usb_btn_0_i = 0;
   int usb_btn_1_i = 0;
   while (1) {
      name_value[0] = '\0';
      // Looks like circle-stdlib doesn't support something like %s=%d
      int st = fscanf(fp,"%s", name_value);
      if (name_value[0] == '\0' || st == EOF || feof(fp)) break;
      char *name = strtok(name_value, "=");
      if (name == NULL) break;
      char *value_str = strtok(NULL, "=");
      if (value_str == NULL) break;
      value = atoi(value_str);

      if (strcmp(name,"port_1")==0) { port_1_menu_item->value = value; }
      else if (strcmp(name,"port_2")==0) { port_2_menu_item->value = value; }
      else if (strcmp(name,"usb_0")==0) { usb_pref_0 = value; }
      else if (strcmp(name,"usb_1")==0) { usb_pref_1 = value; }
      else if (strcmp(name,"usb_x_0")==0) { usb_x_axis_0 = value; }
      else if (strcmp(name,"usb_y_0")==0) { usb_y_axis_0 = value; }
      else if (strcmp(name,"usb_x_1")==0) { usb_x_axis_1 = value; }
      else if (strcmp(name,"usb_y_1")==0) { usb_y_axis_1 = value; }
      else if (strcmp(name,"palette")==0) {
         palette_item->value = value;
         video_canvas_change_palette(palette_item->value);
      }
      else if (strcmp(name,"keyboard_type")==0) {
         keyboard_type_item->value = value;
      } else if (strcmp(name,"usb_btn_0")==0) {
         usb_0_button_assignments[usb_btn_0_i] = value;
         usb_btn_0_i++;
         if (usb_btn_0_i >= 16) {
            usb_btn_0_i = 0;
         }
      } else if (strcmp(name,"usb_btn_1")==0) {
         usb_1_button_assignments[usb_btn_1_i] = value;
         usb_btn_1_i++;
         if (usb_btn_1_i >= 16) {
            usb_btn_1_i = 0;
         }
      } else if (strcmp(name,"alt_f12")==0) {
         menu_alt_f12_item->value = value;
      } else if (strcmp(name,"overlay")==0) {
         overlay_item->value = value;
      } else if (strcmp(name,"tapereset")==0) {
         tape_reset_with_machine_item->value = value;
      } else if (strcmp(name,"pot_x_high")==0) {
         pot_x_high_value = value;
      } else if (strcmp(name,"pot_x_low")==0) {
         pot_x_low_value = value;
      } else if (strcmp(name,"pot_y_high")==0) {
         pot_y_high_value = value;
      } else if (strcmp(name,"pot_y_low")==0) {
         pot_y_low_value = value;
      }
   }
   fclose(fp);
}

void menu_swap_joysticks() {
   int tmp = joydevs[0].device;
   joydevs[0].device = joydevs[1].device;
   joydevs[1].device = tmp;
   joyswap = 1 - joyswap;
   overlay_joyswap_changed(joyswap);
   ui_set_joy_items();
}

static char* fullpath(int dir_type, char* name) {
   strcpy(dir_scratch, current_dir_names[dir_type]);
   strcat(dir_scratch, "/");
   strcat(dir_scratch, name);
   return dir_scratch;
}

static void select_file(struct menu_item* item) {
   if (item->id == MENU_LOAD_SNAP_FILE) {
      ui_info("Loading...");
      if(machine_read_snapshot(fullpath(DIR_SNAPS, item->str_value),0) < 0) {
          ui_pop_menu();
          ui_error("Load snapshot failed");
      } else {
          ui_pop_all_and_toggle();
      }
   } else if (item->id == MENU_DISK_FILE) {
         // Perform the attach
         ui_info("Attaching...");
         if (file_system_attach_disk(unit,
                fullpath(DIR_DISKS, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach disk image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_TAPE_FILE) {
         ui_info("Attaching...");
         if (tape_image_attach(1, fullpath(DIR_TAPES, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach tape image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_CART_FILE) {
         ui_info("Attaching...");
         if (cartridge_attach_image(
                 CARTRIDGE_CRT, fullpath(DIR_CARTS, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach cart image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_CART_8K_FILE) {
         ui_info("Attaching...");
         if (cartridge_attach_image(CARTRIDGE_GENERIC_8KB,
                fullpath(DIR_CARTS, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach cart image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_CART_16K_FILE) {
         ui_info("Attaching...");
         if (cartridge_attach_image(CARTRIDGE_GENERIC_16KB,
                fullpath(DIR_CARTS, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach cart image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_CART_ULTIMAX_FILE) {
         ui_info("Attaching...");
         if (cartridge_attach_image(CARTRIDGE_ULTIMAX,
                fullpath(DIR_CARTS, item->str_value)) < 0) {
            ui_pop_menu();
            ui_error("Failed to attach cart image");
         } else {
            ui_pop_all_and_toggle();
         }
   } else if (item->id == MENU_KERNAL_FILE) {
         resources_set_string("KernalName", item->str_value);
         ui_pop_all_and_toggle();
   } else if (item->id == MENU_BASIC_FILE) {
         resources_set_string("BasicName", item->str_value);
         ui_pop_all_and_toggle();
   } else if (item->id == MENU_CHARGEN_FILE) {
         resources_set_string("ChargenName", item->str_value);
         ui_pop_all_and_toggle();
   } else if (item->id == MENU_AUTOSTART_FILE) {
         ui_info("Starting...");
         if (autostart_autodetect(fullpath(DIR_ROOT, item->str_value),
                "*", 0, AUTOSTART_MODE_RUN) < 0) {
            ui_pop_menu();
            ui_error("Failed to autostart file");
         } else {
            ui_pop_all_and_toggle();
         }
   }
}

// Utility to determine current dir index from a menu file item
static int menu_file_item_to_dir_index(struct menu_item* item) {
  int index;
  switch (item->id) {
     case MENU_LOAD_SNAP_FILE:
     case MENU_SAVE_SNAP_FILE:
       return DIR_SNAPS;
     case MENU_DISK_FILE:
       return DIR_DISKS;
     case MENU_TAPE_FILE:
       return DIR_TAPES;
     case MENU_CART_FILE:
     case MENU_CART_8K_FILE:
     case MENU_CART_16K_FILE:
     case MENU_CART_ULTIMAX_FILE:
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
static void relist_files(struct menu_item* item) {
  switch (item->id) {
     case MENU_LOAD_SNAP_FILE:
       show_files(DIR_SNAPS, FILTER_SNAP, item->id);
       break;
     case MENU_SAVE_SNAP_FILE:
       show_files(DIR_SNAPS, FILTER_SNAP, item->id);
       break;
     case MENU_DISK_FILE:
       show_files(DIR_DISKS, FILTER_DISK, item->id);
       break;
     case MENU_TAPE_FILE:
       show_files(DIR_TAPES, FILTER_TAPE, item->id);
       break;
     case MENU_CART_FILE:
       show_files(DIR_CARTS, FILTER_CART, item->id);
       break;
     case MENU_CART_8K_FILE:
     case MENU_CART_16K_FILE:
     case MENU_CART_ULTIMAX_FILE:
       show_files(DIR_CARTS, FILTER_NONE, item->id);
       break;
     case MENU_KERNAL_FILE:
     case MENU_BASIC_FILE:
     case MENU_CHARGEN_FILE:
       show_files(DIR_ROMS, FILTER_NONE, item->id);
       break;
     case MENU_AUTOSTART_FILE:
       show_files(DIR_ROOT, FILTER_NONE, item->id);
       break;
     default:
       break;
  }
}

static void up_dir(struct menu_item* item) {
  int i;
  int dir_index = menu_file_item_to_dir_index(item);
  if (dir_index < 0) return;
  // Remove last directory from current_dir_names
  i = strlen(current_dir_names[dir_index])-1;
  while (current_dir_names[dir_index][i] != '/' && i > 0) i--;
  current_dir_names[dir_index][i] = '\0';
  if (strlen(current_dir_names[dir_index]) == 0) {
     strcpy(current_dir_names[dir_index], "/");
  }
  ui_pop_menu();
  relist_files(item);
}

static void enter_dir(struct menu_item* item) {
  int dir_index = menu_file_item_to_dir_index(item);
  if (dir_index < 0) return;
  // Append this item's value to current dir
  if (current_dir_names[dir_index][strlen(current_dir_names[dir_index])-1]
          != '/') {
     strcat(current_dir_names[dir_index], "/");
  }
  strcat(current_dir_names[dir_index], item->str_value);
  ui_pop_menu();
  relist_files(item);
}

static void toggle_warp(int value) {
  resources_set_int("WarpMode", value);
  raspi_warp = value;
  overlay_warp_changed(value);
}

// Interpret what menu item changed and make the change to vice
static void menu_value_changed(struct menu_item* item) {
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
      case MENU_COLOR_PALETTE:
         video_canvas_change_palette(item->value);
         return;
      case MENU_AUTOSTART:
         show_files(DIR_ROOT, FILTER_NONE, MENU_AUTOSTART_FILE);
         return;
      case MENU_SAVE_SNAP:
         show_files(DIR_SNAPS, FILTER_SNAP, MENU_SAVE_SNAP_FILE);
         return;
      case MENU_LOAD_SNAP:
         show_files(DIR_SNAPS, FILTER_SNAP, MENU_LOAD_SNAP_FILE);
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
         show_files(DIR_DISKS, FILTER_DISK, MENU_DISK_FILE);
         return;
      case MENU_ATTACH_TAPE:
         show_files(DIR_TAPES, FILTER_TAPE, MENU_TAPE_FILE);
         return;
      case MENU_ATTACH_CART:
         show_files(DIR_CARTS, FILTER_CART, MENU_CART_FILE);
         return;
      case MENU_ATTACH_CART_8K:
         show_files(DIR_CARTS, FILTER_NONE, MENU_CART_8K_FILE);
         return;
      case MENU_ATTACH_CART_16K:
         show_files(DIR_CARTS, FILTER_NONE, MENU_CART_16K_FILE);
         return;
      case MENU_ATTACH_CART_ULTIMAX:
         show_files(DIR_CARTS, FILTER_NONE, MENU_CART_ULTIMAX_FILE);
         return;
      case MENU_LOAD_KERNAL:
         show_files(DIR_ROMS, FILTER_NONE, MENU_KERNAL_FILE);
         return;
      case MENU_LOAD_BASIC:
         show_files(DIR_ROMS, FILTER_NONE, MENU_BASIC_FILE);
         return;
      case MENU_LOAD_CHARGEN:
         show_files(DIR_ROMS, FILTER_NONE, MENU_CHARGEN_FILE);
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
      case MENU_COLOR_BRIGHTNESS:
         set_color_brightness(item->value);
         video_color_setting_changed();
         return;
      case MENU_COLOR_CONTRAST:
         set_color_contrast(item->value);
         video_color_setting_changed();
         return;
      case MENU_COLOR_GAMMA:
         set_color_gamma(item->value);
         video_color_setting_changed();
         return;
      case MENU_COLOR_TINT:
         set_color_tint(item->value);
         video_color_setting_changed();
         return;
      case MENU_COLOR_RESET:
         brightness_item->value = 1000;
         contrast_item->value = 1250;
         gamma_item->value = 2200;
         tint_item->value = 1000;
         set_color_brightness(brightness_item->value);
         set_color_contrast(contrast_item->value);
         set_color_gamma(gamma_item->value);
         set_color_tint(tint_item->value);
         video_color_setting_changed();
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
         return;
      case MENU_JOYSTICK_PORT_2:
         // device in port 2 was changed
         if (joydevs[0].port == 2) {
            joydevs[0].device = item->choice_ints[item->value];
         } else if (joydevs[1].port == 2) {
            joydevs[1].device = item->choice_ints[item->value];
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
   }

   // Only items that were for file selection/nav should have these set...
   if (item->sub_id == MENU_SUB_PICK_FILE) {
      select_file(item);
   } else if (item->sub_id == MENU_SUB_UP_DIR) {
      up_dir(item);
   } else if (item->sub_id == MENU_SUB_ENTER_DIR) {
      enter_dir(item);
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
         char* dot = strchr(fname, '.');
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
                (dot[3] != 'f' && dot[3] != 'F') ||
                 dot[4] != '\0') {
              ui_error("Need .VSF extension");
              return;
            }
         }
      }
      ui_info("Saving...");
      if(machine_write_snapshot(fullpath(DIR_SNAPS, fname), 1, 1, 0) < 0) {
          ui_pop_menu();
          ui_error("Save snapshot failed");
      } else {
          ui_pop_all_and_toggle();
      }
   }
}

// Returns what input preference user has for this usb device
void circle_usb_pref(int device, int *usb_pref, int* x_axis, int *y_axis) {
   if (device == 0) {
      *usb_pref = usb_pref_0;
      *x_axis = usb_x_axis_0;
      *y_axis = usb_y_axis_0;
   }
   else if (device == 1) {
      *usb_pref = usb_pref_1;
      *x_axis = usb_x_axis_1;
      *y_axis = usb_y_axis_1;
   } else {
      *usb_pref = -1;
      *x_axis = -1;
      *y_axis = -1;
   }
}

int menu_get_keyboard_type(void) {
   return keyboard_type_item->value;
}

int menu_alt_f12(void) {
   return menu_alt_f12_item->value;
}

void build_menu(struct menu_item* root) {
   struct menu_item* parent;
   struct menu_item* child;
   int dev;
   int i;
   int j;
   int tmp;

   // TODO: This doesn't really belong here. Need to sort
   // out init order of structs.
   for (dev = 0; dev < 2; dev++ ) {
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
      ui_menu_add_button(MENU_LOAD_KERNAL, parent, "Load Kernal ROM...");
      ui_menu_add_button(MENU_LOAD_BASIC, parent, "Load Basic ROM...");
      ui_menu_add_button(MENU_LOAD_CHARGEN, parent, "Load Chargen ROM...");

   ui_menu_add_divider(root);

   parent = ui_menu_add_folder(root, "Drive 8");
      resources_get_int_sprintf("IECDevice%i", &tmp, 8);
      ui_menu_add_toggle(MENU_IECDEVICE_8, parent, "IEC FileSystem", tmp);
      ui_menu_add_button(MENU_ATTACH_DISK_8, parent, "Attach Disk...");
      ui_menu_add_button(MENU_DETACH_DISK_8, parent, "Detach Disk");
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_8, parent, "Change Model...");

   parent = ui_menu_add_folder(root, "Drive 9");
      ui_menu_add_button(MENU_ATTACH_DISK_9, parent, "Attach Disk...");
      ui_menu_add_button(MENU_DETACH_DISK_9, parent, "Detach Disk");
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_9, parent, "Change Model...");

   parent = ui_menu_add_folder(root, "Drive 10");
      ui_menu_add_button(MENU_ATTACH_DISK_10, parent, "Attach Disk...");
      ui_menu_add_button(MENU_DETACH_DISK_10, parent, "Detach Disk");
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_10, parent, "Change Model...");

   parent = ui_menu_add_folder(root, "Drive 11");
      ui_menu_add_button(MENU_ATTACH_DISK_11, parent, "Attach Disk...");
      ui_menu_add_button(MENU_DETACH_DISK_11, parent, "Detach Disk");
      ui_menu_add_button(MENU_DRIVE_CHANGE_MODEL_11, parent, "Change Model...");

   parent = ui_menu_add_folder(root, "Attach cartridge");
      ui_menu_add_button(MENU_ATTACH_CART, parent, "Attach cart...");
      ui_menu_add_button(MENU_ATTACH_CART_8K, parent, "Attach 8k raw...");
      ui_menu_add_button(MENU_ATTACH_CART_16K, parent, "Attach 16 raw...");
      ui_menu_add_button(MENU_ATTACH_CART_ULTIMAX, parent, "Attach Ultimax raw...");
      ui_menu_add_button(MENU_MAKE_CART_DEFAULT, parent, "Set current cart default (Need Save)");

   ui_menu_add_button(MENU_DETACH_CART, root, "Detach cartridge");

   ui_menu_add_button(MENU_ATTACH_TAPE, root, "Attach tape image...");
   ui_menu_add_button(MENU_DETACH_TAPE, root, "Detach tape image");

   parent = ui_menu_add_folder(root, "Datasette controls (.tap)...");
      ui_menu_add_button(MENU_TAPE_START, parent, "Play");
      ui_menu_add_button(MENU_TAPE_STOP, parent, "Stop");
      ui_menu_add_button(MENU_TAPE_REWIND, parent, "Rewind");
      ui_menu_add_button(MENU_TAPE_FASTFWD, parent, "FastFwd");
      ui_menu_add_button(MENU_TAPE_RECORD, parent, "Record");
      ui_menu_add_button(MENU_TAPE_RESET, parent, "Reset");
      ui_menu_add_button(MENU_TAPE_RESET_COUNTER, parent, "Reset Counter");
      resources_get_int("DatasetteResetWithCPU", &tmp);
      tape_reset_with_machine_item = ui_menu_add_toggle(
          MENU_TAPE_RESET_WITH_MACHINE, parent,
              "Reset Tape with Machine Reset", tmp);

   ui_menu_add_divider(root);

   parent = ui_menu_add_folder(root, "Snapshots");
      ui_menu_add_button(MENU_LOAD_SNAP, parent, "Load Snapshot...");
      ui_menu_add_button(MENU_SAVE_SNAP, parent, "Save Snapshot...");

   parent = ui_menu_add_folder(root, "Video");

      palette_item = ui_menu_add_multiple_choice(MENU_COLOR_PALETTE, parent, "Color Palette");
      palette_item->num_choices = 5;
      palette_item->value = 1;
      strcpy (palette_item->choices[0], "Default");
      strcpy (palette_item->choices[1], "Vice");
      strcpy (palette_item->choices[2], "C64hq");
      strcpy (palette_item->choices[3], "Pepto-Ntsc");
      strcpy (palette_item->choices[4], "Pepto-Pal");

      child = ui_menu_add_folder(parent, "Color Adjustments...");

         brightness_item = ui_menu_add_range(MENU_COLOR_BRIGHTNESS,
             child, "Brightness", 0, 2000, 100, get_color_brightness());
         contrast_item = ui_menu_add_range(MENU_COLOR_CONTRAST,
             child, "Contrast", 0, 2000, 100, get_color_contrast());
         gamma_item = ui_menu_add_range(MENU_COLOR_GAMMA,
             child, "Gamma", 0, 4000, 100, get_color_gamma());
         tint_item = ui_menu_add_range(MENU_COLOR_TINT,
             child, "Tint", 0, 2000, 100, get_color_tint());
         ui_menu_add_button(MENU_COLOR_RESET, child, "Reset");

      ui_menu_add_button(MENU_CALC_TIMING, parent, "Custom HDMI mode timing calc...");

   parent = ui_menu_add_folder(root, "Sound");
      // Resid by default
      child = sid_engine_item = ui_menu_add_multiple_choice(
          MENU_SID_ENGINE, parent,
          "Sid Engine");
      child->num_choices = 2;
      child->value = MENU_SID_ENGINE_RESID;
      strcpy (child->choices[MENU_SID_ENGINE_FAST], "Fast");
      strcpy (child->choices[MENU_SID_ENGINE_RESID], "ReSid");
      child->choice_ints[MENU_SID_ENGINE_FAST] = SID_ENGINE_FASTSID;
      child->choice_ints[MENU_SID_ENGINE_RESID] = SID_ENGINE_RESID;

      // 6581 by default
      child = sid_model_item = ui_menu_add_multiple_choice(
          MENU_SID_MODEL, parent,
          "Sid Model");
      child->num_choices = 2;
      child->value = MENU_SID_MODEL_6581;
      strcpy (child->choices[MENU_SID_MODEL_6581], "6581");
      strcpy (child->choices[MENU_SID_MODEL_8580], "8580");
      child->choice_ints[MENU_SID_MODEL_6581] = SID_MODEL_6581;
      child->choice_ints[MENU_SID_MODEL_8580] = SID_MODEL_8580;

      // Filter on by default
      child = sid_filter_item = ui_menu_add_toggle(
          MENU_SID_FILTER, parent,
          "Sid Filter", 0);

   parent = ui_menu_add_folder(root, "Keyboard");
      child = keyboard_type_item = ui_menu_add_multiple_choice(
          MENU_KEYBOARD_TYPE, parent,
          "Layout (Needs Save+Reboot)");
      child->num_choices = 2;
      child->value = KEYBOARD_TYPE_US;
      strcpy (child->choices[KEYBOARD_TYPE_US], "US");
      strcpy (child->choices[KEYBOARD_TYPE_UK], "UK");

      child = menu_alt_f12_item = ui_menu_add_multiple_choice(
          MENU_KEYBOARD_MENU_ALT_F12, parent,
          "Alternate Menu Key");
      child->num_choices = 2;
      child->value = ALT_F12_COMMODOREF7;
      strcpy (child->choices[ALT_F12_DISABLED], "Disabled");
      strcpy (child->choices[ALT_F12_COMMODOREF7], "C= + F7");

   parent = ui_menu_add_folder(root, "Joystick");
      ui_menu_add_button(MENU_SWAP_JOYSTICKS, parent, "Swap Joystick Ports");
      child = port_1_menu_item = ui_menu_add_multiple_choice(
          MENU_JOYSTICK_PORT_1, parent,
          "Joystick Port 1");
      child->num_choices = 9;
      child->value = 0;
      strcpy (child->choices[0], "None"); child->choice_ints[0] = JOYDEV_NONE;
      strcpy (child->choices[1], "USB 1"); child->choice_ints[1] = JOYDEV_USB_0;
      strcpy (child->choices[2], "USB 2"); child->choice_ints[2] = JOYDEV_USB_1;
      strcpy (child->choices[3], "GPIO Bank 1"); child->choice_ints[3] = JOYDEV_GPIO_0;
      strcpy (child->choices[4], "GPIO Bank 2"); child->choice_ints[4] = JOYDEV_GPIO_1;
      strcpy (child->choices[5], "CURS + SPACE"); child->choice_ints[5] = JOYDEV_CURS_SP;
      strcpy (child->choices[6], "NUMPAD 64825"); child->choice_ints[6] = JOYDEV_NUMS_1;
      strcpy (child->choices[7], "NUMPAD 17930"); child->choice_ints[7] = JOYDEV_NUMS_2;
      strcpy (child->choices[8], "CURS + LCTRL"); child->choice_ints[8] = JOYDEV_CURS_LC;

      child = port_2_menu_item = ui_menu_add_multiple_choice(
          MENU_JOYSTICK_PORT_2, parent,
          "Joystick Port 2");
      child->num_choices = 9;
      child->value = 0;
      strcpy (child->choices[0], "None"); child->choice_ints[0] = JOYDEV_NONE;
      strcpy (child->choices[1], "USB 1"); child->choice_ints[1] = JOYDEV_USB_0;
      strcpy (child->choices[2], "USB 2"); child->choice_ints[2] = JOYDEV_USB_1;
      strcpy (child->choices[3], "GPIO Bank 1"); child->choice_ints[3] = JOYDEV_GPIO_0;
      strcpy (child->choices[4], "GPIO Bank 2"); child->choice_ints[4] = JOYDEV_GPIO_1;
      strcpy (child->choices[5], "CURS + SPACE"); child->choice_ints[5] = JOYDEV_CURS_SP;
      strcpy (child->choices[6], "NUMPAD 64825"); child->choice_ints[6] = JOYDEV_NUMS_1;
      strcpy (child->choices[7], "NUMPAD 17930"); child->choice_ints[7] = JOYDEV_NUMS_2;
      strcpy (child->choices[8], "CURS + LCTRL"); child->choice_ints[8] = JOYDEV_CURS_LC;

      ui_set_joy_items();

      ui_menu_add_button(MENU_CONFIGURE_USB_0, parent, "Configure USB 1...");
      ui_menu_add_button(MENU_CONFIGURE_USB_1, parent, "Configure USB 2...");

      usb_pref_0 = 0;
      usb_pref_1 = 0;
      usb_x_axis_0 = 0;
      usb_y_axis_0 = 1;
      usb_x_axis_1 = 0;
      usb_y_axis_1 = 1;
      for (j=0;j<16;j++) {
         usb_0_button_assignments[j] = (j==0 ? BTN_ASSIGN_FIRE : BTN_ASSIGN_UNDEF);
         usb_1_button_assignments[j] = (j==0 ? BTN_ASSIGN_FIRE : BTN_ASSIGN_UNDEF);
         usb_0_button_bits[j] = 1 << j;
         usb_1_button_bits[j] = 1 << j;
      }

   ui_menu_add_divider(root);


   parent = ui_menu_add_folder(root, "Prefs");


      drive_sounds_item = ui_menu_add_toggle(MENU_DRIVE_SOUND_EMULATION,
         parent, "Drive sound emulation", 0);
      drive_sounds_vol_item = ui_menu_add_range(MENU_DRIVE_SOUND_EMULATION_VOLUME,
         parent, "Drive sound emulation volume", 0, 1000, 100, 1000);

      overlay_item = ui_menu_add_multiple_choice(MENU_OVERLAY, parent, "Show Status Bar");
      overlay_item->num_choices = 3;
      overlay_item->value = 0;
      strcpy (overlay_item->choices[OVERLAY_NEVER], "Never");
      strcpy (overlay_item->choices[OVERLAY_ALWAYS], "Always");
      strcpy (overlay_item->choices[OVERLAY_ON_ACTIVITY], "On Activity");

   ui_menu_add_toggle(MENU_WARP_MODE, root, "Warp Mode", 0);

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

   // Always turn off resampling
   resources_set_int("SidResidSampling", 0);
   set_video_cache(0);
   set_hw_scale(0);

   ui_set_joy_devs();
}

int overlay_enabled(void) {
   return overlay_item->value != OVERLAY_NEVER;
}

int overlay_forced(void) {
   return overlay_item->value == OVERLAY_ALWAYS || force_overlay;
}

// Stuff to do when menu is activated
void menu_about_to_activate() {
}

// Stuff to do before going back to emulator
void menu_about_to_deactivate() {
}

void menu_quick_func(int button_assignment) {
   int value;
   switch(button_assignment) {
      case BTN_ASSIGN_WARP:
         resources_get_int("WarpMode", &value);
         toggle_warp(1 - value);
         break;
      case BTN_ASSIGN_SWAP_PORTS:
         menu_swap_joysticks();
         break;
      case BTN_ASSIGN_STATUS_TOGGLE:
         // Ignore this if it's already showing.
         if (overlay_item->value == OVERLAY_ALWAYS) return;

         if (overlay_showing || force_overlay) {
            // Dismiss
            force_overlay = 0;
            overlay_force_timeout();
         } else {
            force_overlay = 1;
         }
         break;
      default:
         break;
   }
}
