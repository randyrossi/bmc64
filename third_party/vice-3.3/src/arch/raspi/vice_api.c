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
#include "sid-resources.h"
#include "userport/userport_joystick.h"

// RASPI includes
#include "circle.h"
#include "keycodes.h"

struct menu_item *sid_engine_item;
struct menu_item *sid_model_item;
struct menu_item *sid_filter_item;
struct menu_item *sid_resampling_item;
struct menu_item *keyboard_mapping_item;

// TODO: Fix these
extern struct menu_item *port_3_menu_item;
extern struct menu_item *port_4_menu_item;
extern struct menu_item* add_joyport_options(struct menu_item* parent, int port);
extern void ui_set_joy_items();

struct menu_item *enable_item;
struct menu_item *swap_item;
struct menu_item *adapter_type_item;

void raspi_keymap_changed(int, int, signed long);

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
    case VICE_MACHINE_PET:
       emux_machine_class = BMC64_MACHINE_CLASS_PET;
       break;
    default:
       assert(0);
       break;
  }
}

static int vice_keymap_index_to_bmc(int value) {
   switch (value) {
      case KBD_INDEX_SYM:
         return KEYBOARD_MAPPING_SYM;
      case KBD_INDEX_POS:
         return KEYBOARD_MAPPING_POS;
      case KBD_INDEX_USERPOS:
         return KEYBOARD_MAPPING_MAXI;
      default:
         return KEYBOARD_MAPPING_SYM;
   }
}

static int bmc_keymap_index_to_vice(int value) {
   switch (value) {
      case KEYBOARD_MAPPING_SYM:
        return KBD_INDEX_SYM;
      case KEYBOARD_MAPPING_POS:
        return KBD_INDEX_POS;
      case KEYBOARD_MAPPING_MAXI:
        return KBD_INDEX_USERPOS;
      default:
         return KBD_INDEX_SYM;
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

  if (machine_class == VICE_MACHINE_PET) {
     // This is a hack to get sound working after a snapshot load.
     // For some reason, the sound engine is closed after a load
     // Snapshots are disabled for PET but keeping this here in case
     // it's needed again.
     int sid_engine;
     resources_get_int("SidEngine", &sid_engine);
     resources_set_int("SidEngine", 1-sid_engine);
     resources_set_int("SidEngine", sid_engine);
  }

  int tmp;
  resources_get_int("UserportJoy", &tmp);
  enable_item->value = tmp;

  resources_get_int("UserportJoyType", &tmp);
  for (int i=0;i<adapter_type_item->num_choices;i++) {
     if (adapter_type_item->choice_ints[0] == tmp) {
         adapter_type_item->value = i;
         break;
     }
  }

  // Do other menu items too.
  //   Drive%iType, Drive%iParallelCable
  //   Drive%iRAM2000-A000
  //   KeymapIndex, SidEngine, SidModel, SidFilters, DriveSoundEmulation
  //   DriveSoundEmulationVolume, C128ColumnKey, DatasetteResetWithCPU
  //   IECDevice%i, FSDevice%iDir

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

  static int num_supported_drives = 12;
  static int supported_drives[] = {
     DRIVE_TYPE_1541,
     DRIVE_TYPE_1541II,
     DRIVE_TYPE_1551,
     DRIVE_TYPE_1571,
     DRIVE_TYPE_1581,
     DRIVE_TYPE_2031,
     DRIVE_TYPE_2040,
     DRIVE_TYPE_3040,
     DRIVE_TYPE_4040,
     DRIVE_TYPE_1001,
     DRIVE_TYPE_8050,
     DRIVE_TYPE_8250,
  };

  static const char* drive_labels[] = {
     "1541",
     "1541II",
     "1551",
     "1571",
     "1581",
     "2031",
     "2040",
     "3040",
     "4040",
     "1001",
     "8050",
     "8250",
  };

  for (int i = 0 ; i < num_supported_drives; i++) {
    if (drive_check_type(supported_drives[i], unit - 8) > 0) {
      item = ui_menu_add_button(MENU_DRIVE_MODEL_SELECT, model_root, drive_labels[i]);
      item->value = supported_drives[i];
      if (current_drive_type == supported_drives[i]) {
        strcat(item->displayed_value, " (*)");
      }
    }
  }
}

void emux_add_drive_option(struct menu_item* root, int drive) {
  if (emux_machine_class != BMC64_MACHINE_CLASS_C64 &&
      emux_machine_class != BMC64_MACHINE_CLASS_C128) {
    return;
  }

  struct menu_item* parent = ui_menu_add_folder(root, "Options");

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
  switch (port_num) {
  case 1:
     resources_set_int("JoyPort1Device", vice_id);
     break;
  case 2:
     resources_set_int("JoyPort2Device", vice_id);
     break;
  case 3:
     resources_set_int("JoyPort3Device", vice_id);
     break;
  case 4:
     resources_set_int("JoyPort4Device", vice_id);
     break;
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

static int viceSidResamplingToBmcChoice(int method) {
  switch (method) {
  case SID_RESID_SAMPLING_FAST:
    return MENU_SID_SAMPLING_FAST;
  case SID_RESID_SAMPLING_INTERPOLATION:
    return MENU_SID_SAMPLING_INTERPOLATION;
  case SID_RESID_SAMPLING_RESAMPLING:
    return MENU_SID_SAMPLING_RESAMPLING;
  case SID_RESID_SAMPLING_FAST_RESAMPLING:
    return MENU_SID_SAMPLING_FAST_RESAMPLING;
  default:
    return MENU_SID_SAMPLING_FAST;
  }
}

void emux_add_tape_options(struct menu_item* parent) {
}

void emux_add_keyboard_options(struct menu_item* parent) {
  keyboard_mapping_item = ui_menu_add_multiple_choice(
      MENU_KEYBOARD_MAPPING, parent, "Mapping");
  keyboard_mapping_item->num_choices = 3;

  int tmp_value;
  resources_get_int("KeymapIndex", &tmp_value);
  keyboard_mapping_item->value = vice_keymap_index_to_bmc(tmp_value);
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_SYM], "Symbolic");
  keyboard_mapping_item->choice_ints[KEYBOARD_MAPPING_SYM] = KBD_INDEX_SYM;
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_POS], "Positional");
  keyboard_mapping_item->choice_ints[KEYBOARD_MAPPING_POS] = KBD_INDEX_POS;
  strcpy(keyboard_mapping_item->choices[KEYBOARD_MAPPING_MAXI], "Maxi");
  keyboard_mapping_item->choice_ints[KEYBOARD_MAPPING_MAXI] = KBD_INDEX_USERPOS;
}

void emux_add_sound_options(struct menu_item* parent) {

  // The pet has terrible lag when using ReSid, use FAST since it only
  // ever makes simple beeps anyway.
  if (machine_class == VICE_MACHINE_PET) {
     resources_set_int("SidEngine", SID_ENGINE_FASTSID);
     resources_set_int("SidModel", SID_MODEL_6581);
     resources_set_int("SidFilters", 0);
     return;
  }

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

  if (circle_get_model() >= 3 ) {
     child = sid_resampling_item =
         ui_menu_add_multiple_choice(MENU_SID_SAMPLING, parent, "Resampling");
     child->num_choices = 4;
     strcpy(child->choices[MENU_SID_SAMPLING_FAST], "Fast");
     strcpy(child->choices[MENU_SID_SAMPLING_INTERPOLATION], "Interpolation");
     strcpy(child->choices[MENU_SID_SAMPLING_RESAMPLING], "Resampling");
     strcpy(child->choices[MENU_SID_SAMPLING_FAST_RESAMPLING], "Fast Resampling");
     child->choice_ints[MENU_SID_SAMPLING_FAST] = SID_RESID_SAMPLING_FAST;
     child->choice_ints[MENU_SID_SAMPLING_INTERPOLATION] = SID_RESID_SAMPLING_INTERPOLATION;
     child->choice_ints[MENU_SID_SAMPLING_RESAMPLING] = SID_RESID_SAMPLING_RESAMPLING;
     child->choice_ints[MENU_SID_SAMPLING_FAST_RESAMPLING] = SID_RESID_SAMPLING_FAST_RESAMPLING;

     if (circle_get_model() < 4) {
        child->choice_disabled[MENU_SID_SAMPLING_RESAMPLING] = 1;
     }
  }

  int tmp_value;

  resources_get_int("SidEngine", &tmp_value);
  sid_engine_item->value = viceSidEngineToBmcChoice(tmp_value);

  resources_get_int("SidModel", &tmp_value);
  sid_model_item->value = viceSidModelToBmcChoice(tmp_value);

  resources_get_int("SidFilters", &sid_filter_item->value);

  if (circle_get_model() >= 3 ) {
    resources_get_int("SidResidSampling", &tmp_value);
    sid_resampling_item->value = viceSidResamplingToBmcChoice(tmp_value);
  }
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
     // These are the same values. Not bothering to translate...
     resources_set_int("SidResidSampling", value);
     break;
   case Setting_SidResidPassband:
     resources_set_int("SidResidPassband", value);
     resources_set_int("SidResid8580Passband", value);
     break;
   case Setting_SidResidGain:
     resources_set_int("SidResidGain", value);
     resources_set_int("SidResid8580Gain", value);
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
    case Setting_SidResidSampling:
      resources_get_int("SidResidSampling", dest);
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
    case MENU_KEYBOARD_MAPPING:
      resources_set_int("KeymapIndex", bmc_keymap_index_to_vice(item->value));
      if (item->value == KEYBOARD_MAPPING_MAXI) {
         resources_set_string("KeymapUserPosFile", "rpi_maxi_pos.vkm");
      } else {
         resources_set_string("KeymapUserPosFile", "");
      }
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
}

void emux_get_default_color_setting(int *brightness, int *contrast,
                                    int *gamma, int *tint, int *saturation) {
    *brightness = 1000;
    *contrast = 1250;
    *gamma = 2200;
    *tint = 1000;
    *saturation = 1000;
}

int emux_handle_loaded_setting(char *name, char* value_str, int value) {
  return 0;
}

void emux_load_settings_done(void) {
  emux_machine_load_settings_done();
}

static void swap_userport_joysticks() {
  int tmp = joydevs[2].device;
  joydevs[2].device = joydevs[3].device;
  joydevs[3].device = tmp;
  ui_set_joy_items();
}

static void menu_value_changed(struct menu_item *item) {
   switch (item->id) {
      case MENU_USERPORT_JOYSTICKS:
         resources_set_int("UserportJoy", item->value);
         break;
      case MENU_SWAP_USERPORT_JOYSTICKS:
         swap_userport_joysticks();
         break;
      case MENU_USERPORT_TYPE:
         resources_set_int("UserportJoyType", item->choice_ints[item->value]);
         break;
      default:
         break;
   }
}

void emux_add_userport_joys(struct menu_item* parent) {
  struct menu_item* parent2 =
     ui_menu_add_folder(parent,
        "Userport Joystick Adapter");
  int value;
  resources_get_int("UserportJoy", &value);
  enable_item =
     ui_menu_add_toggle(MENU_USERPORT_JOYSTICKS, parent2, "Enable", value);
  swap_item =
     ui_menu_add_button(MENU_SWAP_USERPORT_JOYSTICKS, parent2,
        "Swap Joystick Ports");
  port_3_menu_item = add_joyport_options(parent2, 3);
  port_4_menu_item = add_joyport_options(parent2, 4);

  enable_item->on_value_changed = menu_value_changed;
  swap_item->on_value_changed = menu_value_changed;

  adapter_type_item =
      ui_menu_add_multiple_choice(MENU_USERPORT_TYPE, parent2, "Adapter Type");
  adapter_type_item->num_choices = 6;
  resources_get_int("UserportJoyType", &value);
  adapter_type_item->value = value;
  strcpy(adapter_type_item->choices[0], "CGA");
  strcpy(adapter_type_item->choices[1], "PET");
  strcpy(adapter_type_item->choices[2], "Hummer");
  strcpy(adapter_type_item->choices[3], "OEM");
  strcpy(adapter_type_item->choices[4], "HIT");
  strcpy(adapter_type_item->choices[5], "Kingsoft");
  strcpy(adapter_type_item->choices[6], "Starbyte");
  adapter_type_item->choice_ints[0] = USERPORT_JOYSTICK_CGA;
  adapter_type_item->choice_ints[1] = USERPORT_JOYSTICK_PET;
  adapter_type_item->choice_ints[2] = USERPORT_JOYSTICK_HUMMER;
  adapter_type_item->choice_ints[3] = USERPORT_JOYSTICK_OEM;
  adapter_type_item->choice_ints[4] = USERPORT_JOYSTICK_HIT;
  adapter_type_item->choice_ints[5] = USERPORT_JOYSTICK_KINGSOFT;
  adapter_type_item->choice_ints[6] = USERPORT_JOYSTICK_STARBYTE;
  adapter_type_item->on_value_changed = menu_value_changed;

  switch (machine_class) {
    case VICE_MACHINE_VIC20:
    case VICE_MACHINE_PET:
       adapter_type_item->choice_disabled[4] = 1;
       adapter_type_item->choice_disabled[5] = 1;
       adapter_type_item->choice_disabled[6] = 1;
       break;
    case VICE_MACHINE_PLUS4:
       adapter_type_item->choice_disabled[0] = 1;
       adapter_type_item->choice_disabled[4] = 1;
       adapter_type_item->choice_disabled[5] = 1;
       adapter_type_item->choice_disabled[6] = 1;
       break;
    default:
       break;
  }
}

uint8_t circle_get_userport_ddr(void) {
  switch (machine_class) {
    case VICE_MACHINE_C64:
    case VICE_MACHINE_C128:
    case VICE_MACHINE_VIC20:
    case VICE_MACHINE_PET:
      return userport_get_ddr();
      break;
    default:
      break;
  }
  return 0;
}

uint8_t circle_get_userport(void) {
  switch (machine_class) {
    case VICE_MACHINE_C64:
    case VICE_MACHINE_C128:
    case VICE_MACHINE_VIC20:
    case VICE_MACHINE_PET:
      return userport_get();
      break;
    default:
      break;
  }
  return 0xff;
}

void circle_set_userport(uint8_t value) {
  switch (machine_class) {
    case VICE_MACHINE_C64:
    case VICE_MACHINE_C128:
    case VICE_MACHINE_VIC20:
    case VICE_MACHINE_PET:
      userport_set(value);
      break;
    default:
      break;
  }
}

void raspi_keymap_changed(int row, int col, signed long sym) {
  if (row == -1 && col == -1) {
     // Reset. Mark as not set and default to sane values.
     commodore_key_sym_set = 0;
     ctrl_key_sym_set = 0;
     restore_key_sym_set = 0;
     commodore_key_sym = KEYCODE_LeftControl;
     ctrl_key_sym = KEYCODE_Tab;
     restore_key_sym = KEYCODE_PageUp;
  }

  machine_keymap_changed(row, col, sym);
}
