/*
 * menu_c64.c
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

#include "../raspi_machine.h"

#include <memory.h>

// VICE includes
#include "c64/c64.h"
#include "resources.h"
#include "keyboard.h"
#include "cartridge.h"

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "ui.h"
#include "keycodes.h"

static int reu_size_to_index[8] =
    { 128, 256, 512, 1024, 2048, 4096, 8192, 16384 };

static struct menu_item *ide64_autodetect_items[4];
static struct menu_item *ide64_cylinders_items[4];
static struct menu_item *ide64_heads_items[4];
static struct menu_item *ide64_sectors_items[4];

static void update_ide64_geometry_enabled(int device) {
  int autodetect = ide64_autodetect_items[device]->value != 0;
  ide64_cylinders_items[device]->disabled = autodetect;
  ide64_heads_items[device]->disabled = autodetect;
  ide64_sectors_items[device]->disabled = autodetect;
}

#ifdef HAVE_NETWORK
static struct menu_item *ide64_usb_address_item;

static void update_ide64_usb_enabled(int enabled) {
  ide64_usb_address_item->disabled = enabled == 0;
}
#endif

static void menu_value_changed(struct menu_item *item) {
   switch (item->id) {
      case MENU_REU:
         resources_set_int("REU", item->value);
         break;
      case MENU_REU_SIZE:
         if (item->value >=0 && item->value < 8)
            resources_set_int("REUsize", reu_size_to_index[item->value]);
         break;
      case MENU_IDE64_VERSION:
        resources_set_int("IDE64version", item->value);
        break;
      case MENU_IDE64_RTC_SAVE:
        resources_set_int("IDE64RTCSave", item->value);
        break;
  #ifdef HAVE_NETWORK
      case MENU_IDE64_USB_SERVER:
        resources_set_int("IDE64USBServer", item->value);
        update_ide64_usb_enabled(item->value);
        break;
      case MENU_IDE64_USB_SERVER_ADDRESS:
        resources_set_string("IDE64USBServerAddress", item->str_value);
        break;
    #endif
      case MENU_IDE64_AUTODETECT_1:
      case MENU_IDE64_AUTODETECT_2:
      case MENU_IDE64_AUTODETECT_3:
      case MENU_IDE64_AUTODETECT_4: {
        int device = item->id - MENU_IDE64_AUTODETECT_1;
        resources_set_int_sprintf("IDE64AutodetectSize%i", item->value,
                          device + 1);
        update_ide64_geometry_enabled(device);
        break;
      }
      case MENU_IDE64_CYLINDERS_1:
      case MENU_IDE64_CYLINDERS_2:
      case MENU_IDE64_CYLINDERS_3:
      case MENU_IDE64_CYLINDERS_4:
        resources_set_int_sprintf("IDE64Cylinders%i", item->value,
                          item->id - MENU_IDE64_CYLINDERS_1 + 1);
        break;
      case MENU_IDE64_HEADS_1:
      case MENU_IDE64_HEADS_2:
      case MENU_IDE64_HEADS_3:
      case MENU_IDE64_HEADS_4:
        resources_set_int_sprintf("IDE64Heads%i", item->value,
                          item->id - MENU_IDE64_HEADS_1 + 1);
        break;
      case MENU_IDE64_SECTORS_1:
      case MENU_IDE64_SECTORS_2:
      case MENU_IDE64_SECTORS_3:
      case MENU_IDE64_SECTORS_4:
        resources_set_int_sprintf("IDE64Sectors%i", item->value,
                          item->id - MENU_IDE64_SECTORS_1 + 1);
        break;
      default:
         break;
   }
}

unsigned long emux_calculate_timing(double fps) {
  if (fps >= 49 && fps <= 51) {
    return C64_PAL_CYCLES_PER_LINE * C64_PAL_SCREEN_LINES * fps;
  } else if (fps >= 59 && fps <= 61) {
    return C64_NTSC_CYCLES_PER_LINE * C64_NTSC_SCREEN_LINES * fps;
  } else {
    return 0;
  }
}

double emux_calculate_fps() {
  if (is_ntsc()) {
     return (double)circle_cycles_per_sec() / (C64_NTSC_CYCLES_PER_LINE * C64_NTSC_SCREEN_LINES);
  }
  return (double)circle_cycles_per_sec() / (C64_PAL_CYCLES_PER_LINE * C64_PAL_SCREEN_LINES);
}

void emux_set_color_brightness(int display_num, int value) {
  resources_set_int("VICIIColorBrightness", value);
}

void emux_set_color_contrast(int display_num, int value) {
  resources_set_int("VICIIColorContrast", value);
}

void emux_set_color_gamma(int display_num, int value) {
  resources_set_int("VICIIColorGamma", value);
}

void emux_set_color_tint(int display_num, int value) {
  resources_set_int("VICIIColorTint", value);
}

void emux_set_color_saturation(int display_num, int value) {
  resources_set_int("VICIIColorSaturation", value);
}

void emux_set_video_cache(int value) {
  resources_set_int("VICIIVideoCache", value);
}

void emux_set_hw_scale(int value) {
  resources_set_int("VICIIHwScale", value);
}

int emux_get_color_brightness(int display_num) {
  int value;
  resources_get_int("VICIIColorBrightness", &value);
  return value;
}

int emux_get_color_contrast(int display_num) {
  int value;
  resources_get_int("VICIIColorContrast", &value);
  return value;
}

int emux_get_color_gamma(int display_num) {
  int value;
  resources_get_int("VICIIColorGamma", &value);
  return value;
}

int emux_get_color_tint(int display_num) {
  int value;
  resources_get_int("VICIIColorTint", &value);
  return value;
}

int emux_get_color_saturation(int display_num) {
  int value;
  resources_get_int("VICIIColorSaturation", &value);
  return value;
}

void cartridge_freeze(void) {
  keyboard_clear_keymatrix();
  cartridge_trigger_freeze();
}

struct menu_item* emux_add_palette_options(int menu_id, struct menu_item* parent) {
  struct menu_item* palette_item =
      ui_menu_add_multiple_choice(menu_id, parent, "Color Palette");
  palette_item->num_choices = 17;
  palette_item->value = 0;
  strcpy(palette_item->choices[0], "VICE");
  strcpy(palette_item->choices[1], "Pepto (PAL)");
  strcpy(palette_item->choices[2], "Pepto (old PAL)");
  strcpy(palette_item->choices[3], "Pepto (NTSC, Sony)");
  strcpy(palette_item->choices[4], "Pepto (NTSC)");
  strcpy(palette_item->choices[5], "Colodore (PAL)");
  strcpy(palette_item->choices[6], "ChristopherJam");
  strcpy(palette_item->choices[7], "C64HQ");
  strcpy(palette_item->choices[8], "C64S");
  strcpy(palette_item->choices[9], "CCS64");
  strcpy(palette_item->choices[10], "Frodo");
  strcpy(palette_item->choices[11], "Godot");
  strcpy(palette_item->choices[12], "PC64");
  strcpy(palette_item->choices[13], "RGB");
  strcpy(palette_item->choices[14], "Deekay");
  strcpy(palette_item->choices[15], "Ptoing");
  strcpy(palette_item->choices[16], "Community Colors");
  return palette_item;
}

void emux_add_machine_options(struct menu_item* parent) {
  struct menu_item* roms_parent = ui_menu_add_folder(parent, "ROMs...");
  ui_menu_add_button(MENU_LOAD_KERNAL, roms_parent, "Load Kernal ROM...");
  ui_menu_add_button(MENU_LOAD_BASIC, roms_parent, "Load Basic ROM...");
  ui_menu_add_button(MENU_LOAD_CHARGEN, roms_parent, "Load Chargen ROM...");
}

struct menu_item* emux_add_cartridge_options(struct menu_item* root) {
  struct menu_item* parent = ui_menu_add_folder(root, "Cartridge");
  ui_menu_add_button(MENU_C64_ATTACH_CART, parent, "Attach cart...");
  ui_menu_add_button(MENU_C64_ATTACH_CART_8K, parent, "Attach 8k raw...");
  ui_menu_add_button(MENU_C64_ATTACH_CART_16K, parent, "Attach 16 raw...");
  ui_menu_add_button(MENU_C64_ATTACH_CART_ULTIMAX, parent, "Attach Ultimax raw...");
  ui_menu_add_button(MENU_DETACH_CART, parent, "Detach cartridge");

  ui_menu_add_button(MENU_TEXT, parent, "");
  ui_menu_add_button(MENU_MAKE_CART_DEFAULT, parent,
                     "Set current cart default (Need Save)");

  ui_menu_add_button(MENU_SAVE_EASYFLASH, parent, "Save EasyFlash Now");
  ui_menu_add_button(MENU_CART_FREEZE, parent, "Cartridge Freeze");
  struct menu_item* child = ui_menu_add_folder(parent, "Ram Expansion");

  int tmp;
  resources_get_int("REU", &tmp);
  struct menu_item* reu_item =
     ui_menu_add_toggle(MENU_REU, child, "Ram Expansion", tmp);
  reu_item->on_value_changed = menu_value_changed;

  struct menu_item* sizes_item =
      ui_menu_add_multiple_choice(MENU_REU_SIZE, child, "Memory Size");
  sizes_item->on_value_changed = menu_value_changed;
  sizes_item->num_choices = 8;

  resources_get_int("REUsize", &tmp);
  sizes_item->value = 2;
  for (int t=0;t<8;t++) {
    if (tmp == reu_size_to_index[t])
       sizes_item->value = t;
  }

  strcpy(sizes_item->choices[0], "128k");
  strcpy(sizes_item->choices[1], "256k");
  strcpy(sizes_item->choices[2], "512k");
  strcpy(sizes_item->choices[3], "1024k");
  strcpy(sizes_item->choices[4], "2048k");
  strcpy(sizes_item->choices[5], "4096k");
  strcpy(sizes_item->choices[6], "8192k");
  strcpy(sizes_item->choices[7], "16384k");

    parent = ui_menu_add_folder(parent, "IDE64");
    child = ui_menu_add_multiple_choice(MENU_IDE64_VERSION, parent,
                      "Cartridge Version");
    child->on_value_changed = menu_value_changed;
    child->num_choices = 3;
    resources_get_int("IDE64version", &child->value);
    strcpy(child->choices[0], "V3");
    strcpy(child->choices[1], "V4.1");
    strcpy(child->choices[2], "V4.2");

    resources_get_int("IDE64RTCSave", &tmp);
    child = ui_menu_add_toggle(MENU_IDE64_RTC_SAVE, parent,
                 "Save RTC When Changed", tmp);
    child->on_value_changed = menu_value_changed;

  #ifdef HAVE_NETWORK
    resources_get_int("IDE64USBServer", &tmp);
    child = ui_menu_add_toggle(MENU_IDE64_USB_SERVER, parent,
                 "Enable USB Server", tmp);
    child->on_value_changed = menu_value_changed;

    const char *usb_address;
    resources_get_string("IDE64USBServerAddress", &usb_address);
    ide64_usb_address_item = ui_menu_add_text_field_limit(
      MENU_IDE64_USB_SERVER_ADDRESS, parent, "USB Server Address",
      (char *)usb_address, MAX_STR_VAL_LEN - 1);
    ide64_usb_address_item->on_value_changed = menu_value_changed;
    ide64_usb_address_item->textfield_right_aligned = 1;
    update_ide64_usb_enabled(tmp);
  #endif

    for (int device = 0; device < 4; device++) {
    char label[20];
    sprintf(label, "ATA Device %d", device + 1);
    struct menu_item *device_menu = ui_menu_add_folder(parent, label);

    child = ui_menu_add_button(MENU_IDE64_IMAGE_1 + device, device_menu, "Attach Image...");

    resources_get_int_sprintf("IDE64AutodetectSize%i", &tmp, device + 1);
    ide64_autodetect_items[device] = ui_menu_add_toggle(
      MENU_IDE64_AUTODETECT_1 + device, device_menu, "Autodetect", tmp);
    ide64_autodetect_items[device]->on_value_changed = menu_value_changed;

    resources_get_int_sprintf("IDE64Cylinders%i", &tmp, device + 1);
    ide64_cylinders_items[device] = ui_menu_add_range(
      MENU_IDE64_CYLINDERS_1 + device, device_menu, "Cylinders", 1, 65535,
      1, tmp);
    ide64_cylinders_items[device]->on_value_changed = menu_value_changed;

    resources_get_int_sprintf("IDE64Heads%i", &tmp, device + 1);
    ide64_heads_items[device] = ui_menu_add_range(
      MENU_IDE64_HEADS_1 + device, device_menu, "Heads", 1, 16, 1, tmp);
    ide64_heads_items[device]->on_value_changed = menu_value_changed;

    resources_get_int_sprintf("IDE64Sectors%i", &tmp, device + 1);
    ide64_sectors_items[device] = ui_menu_add_range(
      MENU_IDE64_SECTORS_1 + device, device_menu, "Sectors", 1, 63, 1,
      tmp);
    ide64_sectors_items[device]->on_value_changed = menu_value_changed;

    update_ide64_geometry_enabled(device);
    }

  return parent;
}

void emux_machine_load_settings_done(void) {
}

void machine_keymap_changed(int row, int col, signed long sym) {
  if (row == 7 && col == 5 && !commodore_key_sym_set) {
     commodore_key_sym = sym;
     commodore_key_sym_set = 1;
  } else if (row == 7 && col == 2 && !ctrl_key_sym_set) {
     ctrl_key_sym = sym;
     ctrl_key_sym_set = 1;
  } else if (row == -3 && col == 0 && !restore_key_sym_set) {
     restore_key_sym = sym;
     restore_key_sym_set = 1;
  }
}
