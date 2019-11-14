/*
 * vice_api.c - VICE specific impl of emux_api.h
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

#include "emux_api.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// VICE includes
#include "raspi_machine.h"
#include "autostart.h"
#include "diskimage.h"
#include "attach.h"
#include "cartridge.h"
#include "interrupt.h"
#include "machine.h"
#include "videoarch.h"
#include "menu.h"
#include "menu_timing.h"
#include "ui.h"
#include "keyboard.h"
#include "demo.h"
#include "datasette.h"
#include "resources.h"
#include "drive.h"
#include "joyport.h"
#include "joyport/joystick.h"
#include "vdrive-internal.h"
#include "tape.h"
#include "sid.h"

// RASPI includes
#include "circle.h"

struct menu_item *sid_engine_item;
struct menu_item *sid_model_item;
struct menu_item *sid_filter_item;

void emu_machine_init(void) {
  switch (machine_class) {
    case VICE_MACHINE_C64:
       emux_machine_class = BMC64_MACHINE_CLASS_C64;
       break;
    case VICE_MACHINE_C128:
       emux_machine_class = BMC64_MACHINE_CLASS_C128;
       break;
    case VICE_MACHINE_VIC20:
       emux_machine_class = BMC64_MACHINE_CLASS_VIC20;
       break;
    case VICE_MACHINE_PLUS4:
       emux_machine_class = BMC64_MACHINE_CLASS_PLUS4;
       break;
    default:
       assert(0);
       break;
  }
}

void emux_trap_main_loop_ui(void) {
  interrupt_maincpu_trigger_trap(emu_pause_trap, 0);
}

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data) {
  interrupt_maincpu_trigger_trap(trap_func, data);
}

void emux_kbd_set_latch_keyarr(int row, int col, int value) {
  demo_reset_timeout();
  keyboard_set_keyarr(row, col, value);
}

int emux_attach_disk_image(int unit, char* filename) {
  return file_system_attach_disk(unit, filename);
}

void emux_detach_disk(int unit) {
  file_system_detach_disk(unit);
}

void emux_detach_cart(int bank) {
  // Ignore bank for vice
  cartridge_detach_image(CARTRIDGE_NONE);
}

void emux_set_cart_default(void) {
   cartridge_set_default();
}

void emux_reset(int soft) {
  machine_trigger_reset(soft ? 
      MACHINE_RESET_MODE_SOFT : MACHINE_RESET_MODE_HARD);
}

int emux_save_state(char *filename) {
  return machine_write_snapshot(filename, 1, 1, 0);
}

int emux_load_state(char *filename) {
  int status = machine_read_snapshot(filename, 0);
  // Somehow, this gets turned off. Vice bug?
  resources_set_int("Datasette", 1);
  return status;
}

int emux_tape_control(int cmd) {
  switch (cmd) {
    case EMUX_TAPE_PLAY:
      datasette_control(DATASETTE_CONTROL_START);
      break;
    case EMUX_TAPE_STOP:
      datasette_control(DATASETTE_CONTROL_STOP);
      break;
    case EMUX_TAPE_REWIND:
      datasette_control(DATASETTE_CONTROL_REWIND);
      break;
    case EMUX_TAPE_FASTFORWARD:
      datasette_control(DATASETTE_CONTROL_FORWARD);
      break;
    case EMUX_TAPE_RECORD:
      datasette_control(DATASETTE_CONTROL_RECORD);
      break;
    case EMUX_TAPE_RESET:
      datasette_control(DATASETTE_CONTROL_RESET);
      break;
    case EMUX_TAPE_ZERO:
      datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
      break;
    default:
      assert(0);
      break;
  }
}

int emux_autostart_file(char* filename) {
   return autostart_autodetect(filename, NULL, 0, AUTOSTART_MODE_RUN);
}

void emux_drive_change_model(int unit) {
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

void emux_add_drive_option(struct menu_item* root, int drive) {
  if (emux_machine_class != BMC64_MACHINE_CLASS_C64 &&
      emux_machine_class != BMC64_MACHINE_CLASS_C128) {
    return;
  }

  struct menu_item* parent = ui_menu_add_folder(parent, "Options");

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

  int id;
  switch (drive) {
     case 8:
        id = MENU_PARALLEL_8;
        break;
     case 9:
        id = MENU_PARALLEL_9;
        break;
     case 10:
        id = MENU_PARALLEL_10;
        break;
     case 11:
        id = MENU_PARALLEL_11;
        break;
     default:
        id = MENU_PARALLEL_8;
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

  resources_get_int_sprintf("Drive%iRAM2000", &tmp, drive);
  ui_menu_add_toggle(MENU_DRIVE_RAM_2000, parent, "RAM 2000", tmp)
     ->sub_id = drive;

  resources_get_int_sprintf("Drive%iRAM4000", &tmp, drive);
  ui_menu_add_toggle(MENU_DRIVE_RAM_4000, parent, "RAM 4000", tmp)
     ->sub_id = drive;

  resources_get_int_sprintf("Drive%iRAM6000", &tmp, drive);
  ui_menu_add_toggle(MENU_DRIVE_RAM_6000, parent, "RAM 6000", tmp)
     ->sub_id = drive;

  resources_get_int_sprintf("Drive%iRAM8000", &tmp, drive);
  ui_menu_add_toggle(MENU_DRIVE_RAM_8000, parent, "RAM 8000", tmp)
     ->sub_id = drive;

  resources_get_int_sprintf("Drive%iRAMA000", &tmp, drive);
  ui_menu_add_toggle(MENU_DRIVE_RAM_A000, parent, "RAM A000", tmp)
     ->sub_id = drive;
}

void emux_create_disk(struct menu_item* item, fullpath_func fullpath) {
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

void emux_set_joy_port_device(int port_num, int dev_id) {
  int vice_id = JOYPORT_ID_NONE;
  switch (dev_id) {
     case JOYDEV_NONE:
        vice_id = JOYPORT_ID_NONE;
        break;
     case JOYDEV_MOUSE:
        vice_id = JOYPORT_ID_MOUSE_1351;
        break;
     default:
        vice_id = JOYPORT_ID_JOYSTICK;
        break;
  }
  if (port_num == 1) {
     resources_set_int("JoyPort1Device", vice_id);
  } else {
     resources_set_int("JoyPort2Device", vice_id);
  }
}

void emux_set_joy_pot_x(int value) {
   joystick_set_potx(value);
}

void emux_set_joy_pot_y(int value) {
   joystick_set_poty(value);
}

int emux_attach_tape_image(char* filename) {
   return tape_image_attach(1, filename);
}

void emux_detach_tape(void) {
   tape_image_detach(1);
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

void emux_add_tape_options(struct menu_item* parent) {
}

void emux_add_sound_options(struct menu_item* parent) {
  // Resid by default
  struct menu_item* child = sid_engine_item =
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

  int tmp_value;

  resources_get_int("SidEngine", &tmp_value);
  sid_engine_item->value = viceSidEngineToBmcChoice(tmp_value);
  
  resources_get_int("SidModel", &tmp_value);
  sid_model_item->value = viceSidModelToBmcChoice(tmp_value);
  
  resources_get_int("SidFilters", &sid_filter_item->value);
}

void emux_set_warp(int warp) {
  resources_set_int("WarpMode", warp);
  raspi_warp = warp;
}

void emux_handle_rom_change(struct menu_item* item, fullpath_func fullpath) {
  // Make the rom change. These can't be fullpath or VICE complains.
  switch (item->id) {
     case MENU_DRIVE_ROM_FILE_1541:
       resources_set_string("DosName1541", item->str_value);
       return;
     case MENU_DRIVE_ROM_FILE_1541II:
       resources_set_string("DosName1541ii", item->str_value);
       return;
     case MENU_DRIVE_ROM_FILE_1551:
       resources_set_string("DosName1551", item->str_value);
       return;
     case MENU_DRIVE_ROM_FILE_1571:
       resources_set_string("DosName1571", item->str_value);
       return;
     case MENU_DRIVE_ROM_FILE_1581:
       resources_set_string("DosName1581", item->str_value);
       return;
     case MENU_KERNAL_FILE:
       resources_set_string("KernalName", item->str_value);
       return;
     case MENU_BASIC_FILE:
       resources_set_string("BasicName", item->str_value);
       return;
     case MENU_CHARGEN_FILE:
       resources_set_string("ChargenName", item->str_value);
       return;
     case MENU_C128_LOAD_KERNAL_FILE:
       resources_set_string("KernalIntName", item->str_value);
       return;
     case MENU_C128_LOAD_BASIC_HI_FILE:
       resources_set_string("BasicHiName", item->str_value);
       return;
     case MENU_C128_LOAD_BASIC_LO_FILE:
       resources_set_string("BasicLoName", item->str_value);
       return;
     case MENU_C128_LOAD_CHARGEN_FILE:
       resources_set_string("ChargenIntName", item->str_value);
       return;
     case MENU_C128_LOAD_64_KERNAL_FILE:
       resources_set_string("Kernal64Name", item->str_value);
       return;
     case MENU_C128_LOAD_64_BASIC_FILE:
       resources_set_string("Basic64Name", item->str_value);
       return;
     default:
       assert(0);
  }
}

void emux_set_iec_dir(int unit, char* dir) {
  resources_set_string_sprintf("FSDevice%iDir", dir, unit);
}

void emux_set_int(IntSetting setting, int value) {
 switch (setting) {
   case Setting_C128ColumnKey:
     resources_set_int("C128ColumnKey", value);
     break;
   case Setting_Datasette:
     resources_set_int("Datasette", value);
     break;
   case Setting_DatasetteResetWithCPU:
     resources_set_int("DatasetteResetWithCPU", value);
     break;
   case Setting_DriveSoundEmulation:
     resources_set_int("DriveSoundEmulation", value);
     break;
   case Setting_DriveSoundEmulationVolume:
     resources_set_int("DriveSoundEmulationVolume", value);
     break;
   case Setting_Mouse:
     resources_set_int("Mouse", value);
     break;
   case Setting_RAMBlock0:
     resources_set_int("RAMBlock0", value);
     break;
   case Setting_RAMBlock1:
     resources_set_int("RAMBlock1", value);
     break;
   case Setting_RAMBlock2:
     resources_set_int("RAMBlock2", value);
     break;
   case Setting_RAMBlock3:
     resources_set_int("RAMBlock3", value);
     break;
   case Setting_RAMBlock5:
     resources_set_int("RAMBlock5", value);
     break;
   case Setting_SidEngine:
     resources_set_int("SidEngine", value);
     break;
   case Setting_SidFilters:
     resources_set_int("SidFilters", value);
     break;
   case Setting_SidModel:
     resources_set_int("SidModel", value);
     break;
   case Setting_SidResidSampling:
     resources_set_int("SidResidSampling", value);
     break;
   default:
     assert(0);
 }
}

void emux_set_int_1(IntSetting setting, int value, int param) {
 switch (setting) {
   case Setting_FileSystemDeviceN:
     resources_set_int_sprintf("FileSystemDevice%i", value, param);
     break;
   case Setting_DriveNParallelCable:
     resources_set_int_sprintf("Drive%iParallelCable", value, param);
     break;
   case Setting_DriveNType:
     resources_set_int_sprintf("Drive%iType", value, param);
     break;
   case Setting_IECDeviceN:
     resources_set_int_sprintf("IECDevice%i", value, param);
     break;
   default:
     assert(0);
 }
}

void emux_get_int(IntSetting setting, int* dest) {
  switch (setting) {
    case Setting_WarpMode:
      resources_get_int("WarpMode", dest);
      break;
    case Setting_DriveSoundEmulation:
      resources_get_int("DriveSoundEmulation", dest);
      break;
    case Setting_DriveSoundEmulationVolume:
      resources_get_int("DriveSoundEmulationVolume", dest);
      break;
    case Setting_C128ColumnKey:
      resources_get_int("C128ColumnKey", dest);
      break;
    case Setting_DatasetteResetWithCPU:
      resources_get_int("DatasetteResetWithCPU", dest);
      break;
    default:
      assert(0);
  }
}

void emux_get_int_1(IntSetting setting, int* dest, int param) {
  switch (setting) {
    case Setting_DriveNType:
      resources_get_int_sprintf("Drive%iType", dest, param);
      break;
    case Setting_IECDeviceN:
      resources_get_int_sprintf("IECDevice%i", dest, param);
      break;
    default:
      assert(0);
  }
}

void emux_get_string_1(StringSetting setting, const char** dest, int param) {
  switch (setting) {
    case Setting_FSDeviceNDir:
      resources_get_string_sprintf("FSDevice%iDir", dest, param);
      break;
    default:
      assert(0);
  }
}

int emux_save_settings(void) {
   return resources_save(NULL);
}

int emux_handle_menu_change(struct menu_item* item) {
  switch (item->id) {
    case MENU_SAVE_EASYFLASH:
      if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
        ui_error("Problem saving");
      } else {
        ui_pop_all_and_toggle();
      }
      return 1;
    case MENU_CART_FREEZE:
      cartridge_freeze();
      ui_pop_all_and_toggle();
      return 1;
    case MENU_DRIVE_RAM_2000:
      resources_set_int_sprintf("Drive%iRAM2000", item->value, item->sub_id);
      return 1;
    case MENU_DRIVE_RAM_4000:
      resources_set_int_sprintf("Drive%iRAM4000", item->value, item->sub_id);
      return 1;
    case MENU_DRIVE_RAM_6000:
      resources_set_int_sprintf("Drive%iRAM6000", item->value, item->sub_id);
      return 1;
    case MENU_DRIVE_RAM_8000:
      resources_set_int_sprintf("Drive%iRAM8000", item->value, item->sub_id);
      return 1;
    case MENU_DRIVE_RAM_A000:
      resources_set_int_sprintf("Drive%iRAMA000", item->value, item->sub_id);
      return 1;
    default:
      break;
  }

  return 0;
}

int emux_handle_quick_func(int button_func) {
  switch (button_func) {
    case BTN_ASSIGN_CART_FREEZE:
       cartridge_freeze();
       return 1;
    default:
       break;
  }
  return 0;
}

void emux_load_additional_settings() {
  // Vice settings are automatically loaded by the emulator. Nothing
  // to do here.
}

void emux_save_additional_settings(FILE *fp) {
  // Vice settings are persisted to vice.ini when emux_save_settings is
  // called. Nothing to do here.
}

void emux_get_default_color_setting(int *brightness, int *contrast,
                                    int *gamma, int *tint) {
    *brightness = 1000;
    *contrast = 1250;
    *gamma = 2200;
    *tint = 1000;
}
