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

// For filename filters
#define FILTER_NONE 0
#define FILTER_DISK 1
#define FILTER_CART 2
#define FILTER_TAPE 3

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
struct menu_item *palette_item;
struct menu_item *keyboard_type_item;
struct menu_item *drive_sounds_item;
struct menu_item *drive_sounds_vol_item;

int unit;
const int num_disk_ext = 13;
char disk_filt_ext[13][5] =
    {".d64",".d67",".d71",".d80",".d81",".d82",
     ".d1m",".d2m",".d4m",".g64",".g41",".p64",
     ".x64"};

const int num_tape_ext = 2;
char tape_filt_ext[2][5] = { ".t64",".tap" };

const int num_cart_ext = 2;
char cart_filt_ext[2][5] = { ".crt",".bin" };

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

TEST_FILTER_MACRO(test_disk_name, num_disk_ext, disk_filt_ext);
TEST_FILTER_MACRO(test_tape_name, num_tape_ext, tape_filt_ext);
TEST_FILTER_MACRO(test_cart_name, num_cart_ext, cart_filt_ext);

// Clears the file menu and populates with files
// Only the root dir is supported
static void list_files(struct menu_item* parent, int filter, int menu_id) {
  DIR *dp;
  struct dirent *ep;     
  int i;
  int include;

  dp = opendir (".");

  if (dp != NULL) {
    while (ep = readdir (dp)) {
      include = 0;
      if (filter == FILTER_DISK) {
         include = test_disk_name(ep->d_name);
      } else if (filter == FILTER_TAPE) {
         include = test_tape_name(ep->d_name);
      } else if (filter == FILTER_CART) {
         include = test_cart_name(ep->d_name);
      } else if (filter == FILTER_NONE) {
         include = 1;
      }
      if (include) {
         ui_menu_add_button(menu_id, parent, ep->d_name);
      }
    }

    (void) closedir (dp);
  }
}

static void show_files(int filter, int menu_id) {
   // Show files
   struct menu_item* file_root = ui_push_menu();
   list_files(file_root, filter, menu_id);
}

static void show_about() {
   struct menu_item* about_root = ui_push_menu();
   ui_menu_add_button(MENU_TEXT, about_root, "BMC64 v1.0.4");
   ui_menu_add_button(MENU_TEXT, about_root, "A Bare Metal C64 Emulator");
   ui_menu_add_button(MENU_TEXT, about_root, "For the Rasbperry Pi 2/3");
   ui_menu_add_divider(about_root);
   ui_menu_add_button(MENU_TEXT, about_root, "https://github.com/");
   ui_menu_add_button(MENU_TEXT, about_root, "         randyrossi/bmc64");
}

static void show_license() {
   int i;
   struct menu_item* license_root = ui_push_menu();
   for (i=0;i<510;i++) {
      ui_menu_add_button(MENU_TEXT, license_root, license[i]);
   }
}

static void configure_usb(int dev) {
   struct menu_item* usb_root = ui_push_menu();
   build_usb_menu(dev, usb_root);   
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

static void save_settings() {
   FILE *fp = fopen("settings.txt","w");
   if (fp == NULL) return;

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
   fprintf(fp,"drive_sounds=%d\n",drive_sounds_item->value);
   fprintf(fp,"drive_sounds_vol=%d\n",drive_sounds_vol_item->value);
   fclose(fp);
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
   FILE *fp = fopen("settings.txt","r");
   if (fp == NULL) return;
   char name_value[80];
   int value;
   while (1) {
      name_value[0] = '\0';
      // Looks like circle-stdlib doesn't support something like %s=%d
      int st = fscanf(fp,"%s", name_value);
      if (name_value[0] == '\0' || st == EOF || feof(fp)) break;
      char *name = strtok(name_value, "=");
      if (name == NULL) break;
      char *value_str = strtok(NULL, "=");
      if (value_str == NULL) break;
      int value = atoi(value_str);

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
      }
      else if (strcmp(name,"drive_sounds")==0) {
         drive_sounds_item->value = value;
         resources_set_int("DriveSoundEmulation", value);
      }
      else if (strcmp(name,"drive_sounds_vol")==0) {
         drive_sounds_vol_item->value = value;
         resources_set_int("DriveSoundEmulationVolume", value);
      }
   }
   fclose(fp);

   ui_set_joy_devs();
}

// Interpret what menu item changed and make the change to vice
static void menu_value_changed(struct menu_item* item) {
   int tmp;

   switch (item->id) {
      case MENU_ATTACH_DISK_8:
      case MENU_IECDEVICE_8:
         unit = 8;
         break;
      case MENU_ATTACH_DISK_9:
      case MENU_IECDEVICE_9:
         unit = 9;
         break;
      case MENU_ATTACH_DISK_10:
      case MENU_IECDEVICE_10:
         unit = 10;
         break;
      case MENU_ATTACH_DISK_11:
      case MENU_IECDEVICE_11:
         unit = 11;
         break;
   }

   switch (item->id) {
      case MENU_SAVE_SETTINGS:
         save_settings();
         break;
      case MENU_COLOR_PALETTE:
         video_canvas_change_palette(item->value);
         return;
      case MENU_AUTOSTART:
         show_files(FILTER_NONE, MENU_AUTOSTART_FILE);
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
         show_files(FILTER_DISK, MENU_DISK_FILE);
         return;
      case MENU_ATTACH_TAPE:
         show_files(FILTER_TAPE, MENU_TAPE_FILE);
         return;
      case MENU_ATTACH_CART:
         show_files(FILTER_CART, MENU_CART_FILE);
         return;
      case MENU_ATTACH_CART_8K:
         show_files(FILTER_NONE, MENU_CART_8K_FILE);
         return;
      case MENU_ATTACH_CART_16K:
         show_files(FILTER_NONE, MENU_CART_16K_FILE);
         return;
      case MENU_ATTACH_CART_ULTIMAX:
         show_files(FILTER_NONE, MENU_CART_ULTIMAX_FILE);
         return;
      case MENU_DETACH_DISK_8:
         file_system_detach_disk(8);
         ui_toggle();
         return;
      case MENU_DETACH_DISK_9:
         file_system_detach_disk(9);
         ui_toggle();
         return;
      case MENU_DETACH_DISK_10:
         file_system_detach_disk(10);
         ui_toggle();
         return;
      case MENU_DETACH_DISK_11:
         file_system_detach_disk(11);
         ui_toggle();
         return;
      case MENU_DETACH_TAPE:
         tape_image_detach(1);
         ui_toggle();
         return;
      case MENU_DETACH_CART:
         cartridge_detach_image(CARTRIDGE_CRT);
         ui_toggle();
         return;
      case MENU_SOFT_RESET:
         machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
         ui_toggle();
         return;
      case MENU_HARD_RESET:
         machine_trigger_reset(MACHINE_RESET_MODE_HARD);
         ui_toggle();
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
         resources_set_int("WarpMode", item->value);
         raspi_warp = item->value;
         return;
      case MENU_DRIVE_SOUND_EMULATION:
         resources_set_int("DriveSoundEmulation", item->value);
         return;
      case MENU_DRIVE_SOUND_EMULATION_VOLUME:
         resources_set_int("DriveSoundEmulationVolume", item->value);
         return;
      case MENU_SWAP_JOYSTICKS:
         tmp = joydevs[0].device;
         joydevs[0].device = joydevs[1].device;
         joydevs[1].device = tmp;
         ui_set_joy_items();
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
         ui_toggle();
         return;
      case MENU_TAPE_STOP:
         datasette_control(DATASETTE_CONTROL_STOP);
         ui_toggle();
         return;
      case MENU_TAPE_REWIND:
         datasette_control(DATASETTE_CONTROL_REWIND);
         ui_toggle();
         return;
      case MENU_TAPE_RESET:
         datasette_control(DATASETTE_CONTROL_RESET);
         ui_toggle();
         return;
   }

   // This is selection of a file
   if (item->id == MENU_DISK_FILE) {
         // Perform the attach
         file_system_attach_disk(unit, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_TAPE_FILE) {
         tape_image_attach(1, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_CART_FILE) {
         cartridge_attach_image(CARTRIDGE_CRT, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_CART_8K_FILE) {
         cartridge_attach_image(CARTRIDGE_GENERIC_8KB, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_CART_16K_FILE) {
         cartridge_attach_image(CARTRIDGE_GENERIC_16KB, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_CART_ULTIMAX_FILE) {
         cartridge_attach_image(CARTRIDGE_ULTIMAX, item->name);
         ui_pop_menu();
         ui_toggle();
   } else if (item->id == MENU_AUTOSTART_FILE) {
         autostart_autodetect(item->name, "*", 0, AUTOSTART_MODE_RUN);
         ui_pop_menu();
         ui_toggle();
   }
}

void menu_gamepad_event(int device, int button, int dpad) {
   // Use this later for gamepad config
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

void build_menu(struct menu_item* root) {
   struct menu_item* parent;
   struct menu_item* child1;
   int dev;

   // TODO: This doesn't really belong here. Need to sort
   // out init order of structs.
   for (dev = 0; dev < 2; dev++ ) {
      memset(&joydevs[dev], 0, sizeof(struct joydev_config));
      joydevs[dev].port = dev + 1;
      joydevs[dev].device = JOYDEV_NONE;
   }

   ui_menu_add_button(MENU_ABOUT, root, "About...");
   ui_menu_add_button(MENU_LICENSE, root, "License...");

   ui_menu_add_divider(root);

   parent = ui_menu_add_folder(root, "IEC Device");
      ui_menu_add_toggle(MENU_IECDEVICE_8, parent, "Unit #8", 0);

   parent = ui_menu_add_folder(root, "Attach Disk Image");
      ui_menu_add_button(MENU_ATTACH_DISK_8, parent, "Unit #8");
      ui_menu_add_button(MENU_ATTACH_DISK_9, parent, "Unit #9");
      ui_menu_add_button(MENU_ATTACH_DISK_10, parent, "Unit #10");
      ui_menu_add_button(MENU_ATTACH_DISK_11, parent, "Unit #11");

   parent = ui_menu_add_folder(root, "Detach Disk Image");
      ui_menu_add_button(MENU_DETACH_DISK_8, parent, "Unit #8");
      ui_menu_add_button(MENU_DETACH_DISK_9, parent, "Unit #9");
      ui_menu_add_button(MENU_DETACH_DISK_10, parent, "Unit #10");
      ui_menu_add_button(MENU_DETACH_DISK_11, parent, "Unit #11");

   parent = ui_menu_add_folder(root, "Attach cartridge");
      ui_menu_add_button(MENU_ATTACH_CART, parent, "Attach cart...");
      ui_menu_add_button(MENU_ATTACH_CART_8K, parent, "Attach 8k raw...");
      ui_menu_add_button(MENU_ATTACH_CART_16K, parent, "Attach 16 raw...");
      ui_menu_add_button(MENU_ATTACH_CART_ULTIMAX, parent, "Attach Ultimax raw...");

   ui_menu_add_button(MENU_DETACH_CART, root, "Detach cartridge");

   ui_menu_add_button(MENU_ATTACH_TAPE, root, "Attach tape image...");
   ui_menu_add_button(MENU_DETACH_TAPE, root, "Detach tape image");

   parent = ui_menu_add_folder(root, "Datasette controls (.tap)...");
      ui_menu_add_button(MENU_TAPE_START, parent, "Play");
      ui_menu_add_button(MENU_TAPE_STOP, parent, "Stop");
      ui_menu_add_button(MENU_TAPE_REWIND, parent, "Rewind");
      ui_menu_add_button(MENU_TAPE_RESET, parent, "Reset");

   ui_menu_add_divider(root);

   parent = ui_menu_add_folder(root, "Keyboard");
      child1 = keyboard_type_item = ui_menu_add_multiple_choice(
          MENU_KEYBOARD_TYPE, parent, 
          "Layout");
      child1->num_choices = 2;
      child1->value = 0;
      strcpy (child1->choices[KEYBOARD_TYPE_US], "US");
      strcpy (child1->choices[KEYBOARD_TYPE_UK], "UK");

   parent = ui_menu_add_folder(root, "Joystick");
      ui_menu_add_button(MENU_SWAP_JOYSTICKS, parent, "Swap Joystick Ports");
      child1 = port_1_menu_item = ui_menu_add_multiple_choice(
          MENU_JOYSTICK_PORT_1, parent, 
          "Joystick Port 1");
      child1->num_choices = 8;
      child1->value = 0;
      strcpy (child1->choices[0], "None"); child1->choice_ints[0] = JOYDEV_NONE;
      strcpy (child1->choices[1], "USB 1"); child1->choice_ints[1] = JOYDEV_USB_0;
      strcpy (child1->choices[2], "USB 2"); child1->choice_ints[2] = JOYDEV_USB_1;
      strcpy (child1->choices[3], "GPIO 1"); child1->choice_ints[3] = JOYDEV_GPIO_0;
      strcpy (child1->choices[4], "GPIO 2"); child1->choice_ints[4] = JOYDEV_GPIO_1;
      strcpy (child1->choices[5], "CURS + SPACE"); child1->choice_ints[5] = JOYDEV_CURS;
      strcpy (child1->choices[6], "NUMPAD 64825"); child1->choice_ints[6] = JOYDEV_NUMS_1;
      strcpy (child1->choices[7], "NUMPAD 17930"); child1->choice_ints[7] = JOYDEV_NUMS_2;

      child1 = port_2_menu_item = ui_menu_add_multiple_choice(
          MENU_JOYSTICK_PORT_2, parent, 
          "Joystick Port 2");
      child1->num_choices = 8;
      child1->value = 0;
      strcpy (child1->choices[0], "None"); child1->choice_ints[0] = JOYDEV_NONE;
      strcpy (child1->choices[1], "USB 1"); child1->choice_ints[1] = JOYDEV_USB_0;
      strcpy (child1->choices[2], "USB 2"); child1->choice_ints[2] = JOYDEV_USB_1;
      strcpy (child1->choices[3], "GPIO 1"); child1->choice_ints[3] = JOYDEV_GPIO_0;
      strcpy (child1->choices[4], "GPIO 2"); child1->choice_ints[4] = JOYDEV_GPIO_1;
      strcpy (child1->choices[5], "CURS + SPACE"); child1->choice_ints[5] = JOYDEV_CURS;
      strcpy (child1->choices[6], "NUMPAD 64825"); child1->choice_ints[6] = JOYDEV_NUMS_1;
      strcpy (child1->choices[7], "NUMPAD 17930"); child1->choice_ints[7] = JOYDEV_NUMS_2;

      ui_set_joy_items();

      ui_menu_add_button(MENU_CONFIGURE_USB_0, parent, "Configure USB 1...");
      ui_menu_add_button(MENU_CONFIGURE_USB_1, parent, "Configure USB 2...");

      usb_pref_0 = 0;
      usb_pref_1 = 0;
      usb_x_axis_0 = 0;
      usb_y_axis_0 = 1;
      usb_x_axis_1 = 0;
      usb_y_axis_1 = 1;

   ui_menu_add_divider(root);

   palette_item = parent = ui_menu_add_multiple_choice(MENU_COLOR_PALETTE, root, "Color Palette");
   parent->num_choices = 4;
   parent->value = 0;
   strcpy (parent->choices[0], "Default");
   strcpy (parent->choices[1], "Vice");
   strcpy (parent->choices[2], "C64hq");
   strcpy (parent->choices[3], "Pepto-Ntsc");

   ui_menu_add_toggle(MENU_WARP_MODE, root, "Warp Mode", 0);

   drive_sounds_item = ui_menu_add_toggle(MENU_DRIVE_SOUND_EMULATION,
      root, "Drive sound emulation", 0);
   drive_sounds_vol_item = ui_menu_add_range(MENU_DRIVE_SOUND_EMULATION_VOLUME,
      root, "Drive sound emulation volume", 0, 1000, 100, 1000);

   parent = ui_menu_add_folder(root, "Reset");
      ui_menu_add_button(MENU_SOFT_RESET, parent, "Soft Reset");
      ui_menu_add_button(MENU_HARD_RESET, parent, "Hard Reset");

   ui_menu_add_button(MENU_SAVE_SETTINGS, root, "Save settings");

   ui_set_on_value_changed_callback(menu_value_changed);

   load_settings();
}

