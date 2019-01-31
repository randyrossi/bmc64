/*
 * menu_drive.c - Implementation of the common drive settings menu for the SDL UI.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "vice.h"

#include "attach.h"
#include "autostart-prg.h"
#include "charset.h"
#include "drive.h"
#include "diskimage.h"
#include "fliplist.h"
#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_drive.h"
#ifdef HAVE_OPENCBM
#include "opencbmlib.h"
#endif
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "uimsgbox.h"
#include "util.h"
#include "vdrive-internal.h"

enum {
    UI_FLIP_ADD,
    UI_FLIP_REMOVE,
    UI_FLIP_NEXT,
    UI_FLIP_PREVIOUS,
    UI_FLIP_LOAD,
    UI_FLIP_SAVE
};

static int check_memory_expansion(int memory, int type)
{
    switch (memory) {
        case 0x2000:
            return drive_check_expansion2000(type);
            break;
        case 0x4000:
            return drive_check_expansion4000(type);
            break;
        case 0x6000:
            return drive_check_expansion6000(type);
            break;
        case 0x8000:
            return drive_check_expansion8000(type);
            break;
        default:
            return drive_check_expansionA000(type);
    }
    return 0;
}

static int has_fs(void)
{
    if (machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0 || machine_class == VICE_MACHINE_PET) {
        return 0;
    }
    return 1;
}

static int is_fs(int type)
{
    return ((type == ATTACH_DEVICE_FS || type == ATTACH_DEVICE_REAL || type == ATTACH_DEVICE_RAW) && has_fs());
}

static int get_drive_type(int drive)
{
    int iecdevice = 0;
    int fsdevice;
    int drivetype;

    if (has_fs()) {
        resources_get_int_sprintf("IECDevice%i", &iecdevice, drive);
        resources_get_int_sprintf("FileSystemDevice%i", &fsdevice, drive);
    }
    resources_get_int_sprintf("Drive%iType", &drivetype, drive);
    if (iecdevice) {
        return fsdevice;
    } else {
        return drivetype;
    }
}

static int check_current_drive_type(int type, int drive)
{
    int iecdevice = 0;
    int fsdevice;
    int drivetype;

    if (has_fs()) {
        resources_get_int_sprintf("IECDevice%i", &iecdevice, drive);
        resources_get_int_sprintf("FileSystemDevice%i", &fsdevice, drive);
    }
    resources_get_int_sprintf("Drive%iType", &drivetype, drive);
    if (iecdevice) {
        if (type == fsdevice) {
            return 1;
        }
    } else {
        if (type == drivetype) {
            return 1;
        }
    }
    return 0;
}

static char *get_drive_type_string(int drive)
{
    int type;

    type = get_drive_type(drive);
    switch (type) {
        case 0:                  return MENU_SUBMENU_STRING " none";
        case ATTACH_DEVICE_FS:   return MENU_SUBMENU_STRING " directory";
#ifdef HAVE_OPENCBM
        case ATTACH_DEVICE_REAL: return MENU_SUBMENU_STRING " real drive";
#endif
#ifdef HAVE_RAWDRIVE
        case ATTACH_DEVICE_RAW:  return MENU_SUBMENU_STRING " block device";
#endif
        case DRIVE_TYPE_1540:    return MENU_SUBMENU_STRING " 1540";
        case DRIVE_TYPE_1541:    return MENU_SUBMENU_STRING " 1541";
        case DRIVE_TYPE_1541II:  return MENU_SUBMENU_STRING " 1541-II";
        case DRIVE_TYPE_1551:    return MENU_SUBMENU_STRING " 1551";
        case DRIVE_TYPE_1570:    return MENU_SUBMENU_STRING " 1570";
        case DRIVE_TYPE_1571:    return MENU_SUBMENU_STRING " 1571";
        case DRIVE_TYPE_1571CR:  return MENU_SUBMENU_STRING " 1571CR";
        case DRIVE_TYPE_1581:    return MENU_SUBMENU_STRING " 1581";
        case DRIVE_TYPE_2000:    return MENU_SUBMENU_STRING " 2000";
        case DRIVE_TYPE_4000:    return MENU_SUBMENU_STRING " 4000";
        case DRIVE_TYPE_2031:    return MENU_SUBMENU_STRING " 2031";
        case DRIVE_TYPE_2040:    return MENU_SUBMENU_STRING " 2040";
        case DRIVE_TYPE_3040:    return MENU_SUBMENU_STRING " 3040";
        case DRIVE_TYPE_4040:    return MENU_SUBMENU_STRING " 4040";
        case DRIVE_TYPE_1001:    return MENU_SUBMENU_STRING " 1001";
        case DRIVE_TYPE_8050:    return MENU_SUBMENU_STRING " 8050";
        case DRIVE_TYPE_8250:    return MENU_SUBMENU_STRING " 8250";
        default:                 return MENU_SUBMENU_STRING " ????";
    }
}

UI_MENU_DEFINE_TOGGLE(DriveTrueEmulation)
UI_MENU_DEFINE_TOGGLE(DriveSoundEmulation)
UI_MENU_DEFINE_TOGGLE(VirtualDevices)

static UI_MENU_CALLBACK(set_hide_p00_files_callback)
{
    int drive;
    int hide_p00;

    drive = vice_ptr_to_int(param);
    if (activated) {
        resources_get_int_sprintf("FSDevice%iHideCBMFiles", &hide_p00, drive);
        resources_set_int_sprintf("FSDevice%iHideCBMFiles", !hide_p00, drive);
    } else {
        resources_get_int_sprintf("FSDevice%iHideCBMFiles", &hide_p00, drive);
        if (hide_p00) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_write_p00_files_callback)
{
    int drive;
    int write_p00;

    drive = vice_ptr_to_int(param);
    if (activated) {
        resources_get_int_sprintf("FSDevice%iSaveP00", &write_p00, drive);
        resources_set_int_sprintf("FSDevice%iSaveP00", !write_p00, drive);
    } else {
        resources_get_int_sprintf("FSDevice%iSaveP00", &write_p00, drive);
        if (write_p00) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_read_p00_files_callback)
{
    int drive;
    int read_p00;

    drive = vice_ptr_to_int(param);
    if (activated) {
        resources_get_int_sprintf("FSDevice%iConvertP00", &read_p00, drive);
        resources_set_int_sprintf("FSDevice%iConvertP00", !read_p00, drive);
    } else {
        resources_get_int_sprintf("FSDevice%iConvertP00", &read_p00, drive);
        if (read_p00) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_directory_callback)
{
    char *name;
    int drive;

    drive = vice_ptr_to_int(param);
    if (activated) {
        if (check_current_drive_type(ATTACH_DEVICE_FS, drive)) {
            name = sdl_ui_file_selection_dialog("Select directory", FILEREQ_MODE_CHOOSE_DIR);
            if (name != NULL) {
                resources_set_string_sprintf("FSDevice%iDir", name, drive);
                lib_free(name);
            }
        }
    } else {
        if (!check_current_drive_type(ATTACH_DEVICE_FS, drive)) {
            return MENU_NOT_AVAILABLE_STRING;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(attach_disk_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Select disk image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (file_system_attach_disk(vice_ptr_to_int(param), name) < 0) {
                ui_error("Cannot attach disk image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(detach_disk_callback)
{
    int parameter;

    if (activated) {
        parameter = vice_ptr_to_int(param);
        if (parameter == 0) {
            file_system_detach_disk(8);
            file_system_detach_disk(9);
            file_system_detach_disk(10);
            file_system_detach_disk(11);
        } else {
            file_system_detach_disk(parameter);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(fliplist_callback)
{
    char *name;

    if (activated) {
        switch (vice_ptr_to_int(param)) {
            case UI_FLIP_ADD:
                fliplist_add_image(8);
                break;
            case UI_FLIP_REMOVE:
                fliplist_remove(8, NULL);
                break;
            case UI_FLIP_NEXT:
                fliplist_attach_head(8, 1);
                break;
            case UI_FLIP_PREVIOUS:
                fliplist_attach_head(8, 0);
                break;
            case UI_FLIP_LOAD:
                name = sdl_ui_file_selection_dialog("Select fliplist to load", FILEREQ_MODE_CHOOSE_FILE);
                if (name != NULL) {
                    if (fliplist_load_list((unsigned int)-1, name, 0) != 0) {
                        ui_error("Cannot load fliplist.");
                    }
                    lib_free(name);
                }
                break;
            case UI_FLIP_SAVE:
            default:
                name = sdl_ui_file_selection_dialog("Select fliplist to save", FILEREQ_MODE_SAVE_FILE);
                if (name != NULL) {
                    util_add_extension(&name, "vfl");
                    if (fliplist_save_list((unsigned int)-1, name) != 0) {
                        ui_error("Cannot save fliplist.");
                    }
                    lib_free(name);
                }
                break;
        }
    }
    return NULL;
}

#define DRIVE_SHOW_IDLE_CALLBACK(x)                         \
    static UI_MENU_CALLBACK(drive_##x##_show_idle_callback) \
    {                                                       \
        int type;                                           \
                                                            \
        type = get_drive_type(x);                           \
                                                            \
        if (drive_check_idle_method(type)) {                \
            return MENU_SUBMENU_STRING;                     \
        }                                                   \
        return MENU_NOT_AVAILABLE_STRING;                   \
    }

DRIVE_SHOW_IDLE_CALLBACK(8)
DRIVE_SHOW_IDLE_CALLBACK(9)
DRIVE_SHOW_IDLE_CALLBACK(10)
DRIVE_SHOW_IDLE_CALLBACK(11)

#define DRIVE_SHOW_EXTEND_CALLBACK(x)                         \
    static UI_MENU_CALLBACK(drive_##x##_show_extend_callback) \
    {                                                         \
        int type;                                             \
                                                              \
        type = get_drive_type(x);                             \
                                                              \
        if (drive_check_extend_policy(type)) {                \
            return MENU_SUBMENU_STRING;                       \
        }                                                     \
        return MENU_NOT_AVAILABLE_STRING;                     \
    }

DRIVE_SHOW_EXTEND_CALLBACK(8)
DRIVE_SHOW_EXTEND_CALLBACK(9)
DRIVE_SHOW_EXTEND_CALLBACK(10)
DRIVE_SHOW_EXTEND_CALLBACK(11)

#define DRIVE_SHOW_EXPAND_CALLBACK(x)                         \
    static UI_MENU_CALLBACK(drive_##x##_show_expand_callback) \
    {                                                         \
        int type;                                             \
                                                              \
        type = get_drive_type(x);                             \
                                                              \
        if (drive_check_expansion(type)) {                    \
            return MENU_SUBMENU_STRING;                       \
        }                                                     \
        return MENU_NOT_AVAILABLE_STRING;                     \
    }

DRIVE_SHOW_EXPAND_CALLBACK(8)
DRIVE_SHOW_EXPAND_CALLBACK(9)
DRIVE_SHOW_EXPAND_CALLBACK(10)
DRIVE_SHOW_EXPAND_CALLBACK(11)

#define DRIVE_SHOW_EXBOARD_CALLBACK(x)                                  \
    static UI_MENU_CALLBACK(drive_##x##_show_exboard_callback)          \
    {                                                                   \
        int type;                                                       \
                                                                        \
        type = get_drive_type(x);                                       \
                                                                        \
        if (drive_check_profdos(type) || drive_check_supercard(type)) { \
            return MENU_SUBMENU_STRING;                                 \
        }                                                               \
        return MENU_NOT_AVAILABLE_STRING;                               \
    }

DRIVE_SHOW_EXBOARD_CALLBACK(8)
DRIVE_SHOW_EXBOARD_CALLBACK(9)
DRIVE_SHOW_EXBOARD_CALLBACK(10)
DRIVE_SHOW_EXBOARD_CALLBACK(11)

#define DRIVE_SHOW_TYPE_CALLBACK(x)                         \
    static UI_MENU_CALLBACK(drive_##x##_show_type_callback) \
    {                                                       \
        return get_drive_type_string(x);                    \
    }

DRIVE_SHOW_TYPE_CALLBACK(8)
DRIVE_SHOW_TYPE_CALLBACK(9)
DRIVE_SHOW_TYPE_CALLBACK(10)
DRIVE_SHOW_TYPE_CALLBACK(11)

static UI_MENU_CALLBACK(set_idle_callback)
{
    int drive;
    int parameter;
    int current;
    int idle = 0;

    drive = (int)(vice_ptr_to_int(param) >> 8);
    parameter = (int)(vice_ptr_to_int(param) & 0xff);
    current = get_drive_type(drive);

    if (activated) {
        if (drive_check_idle_method(current)) {
            resources_set_int_sprintf("Drive%iIdleMethod", parameter, drive);
        }
    } else {
        if (!drive_check_idle_method(current)) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            resources_get_int_sprintf("Drive%iIdleMethod", &idle, drive);
            if (idle == parameter) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_extend_callback)
{
    int drive;
    int parameter;
    int current;
    int extend = 0;

    drive = (int)(vice_ptr_to_int(param) >> 8);
    parameter = (int)(vice_ptr_to_int(param) & 0xff);
    current = get_drive_type(drive);

    if (activated) {
        if (drive_check_extend_policy(current)) {
            resources_set_int_sprintf("Drive%iExtendImagePolicy", parameter, drive);
        }
    } else {
        if (!drive_check_extend_policy(current)) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            resources_get_int_sprintf("Drive%iExtendImagePolicy", &extend, drive);
            if (extend == parameter) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

#define DRIVE_SHOW_PARALLEL_CALLBACK(x)                         \
    static UI_MENU_CALLBACK(drive_##x##_show_parallel_callback) \
    {                                                           \
        int type;                                               \
                                                                \
        type = get_drive_type(x);                               \
                                                                \
        if (drive_check_parallel_cable(type)) {                 \
            return MENU_SUBMENU_STRING;                         \
        }                                                       \
        return MENU_NOT_AVAILABLE_STRING;                       \
    }

DRIVE_SHOW_PARALLEL_CALLBACK(8)
DRIVE_SHOW_PARALLEL_CALLBACK(9)
DRIVE_SHOW_PARALLEL_CALLBACK(10)
DRIVE_SHOW_PARALLEL_CALLBACK(11)

static UI_MENU_CALLBACK(set_par_callback)
{
    int drive, type;
    int current;
    int par;

    drive = vice_ptr_to_int(param) >> 16;
    type = vice_ptr_to_int(param) & 0x0f;
    current = get_drive_type(drive);

    if (activated) {
        if (machine_class != VICE_MACHINE_VIC20 && drive_check_parallel_cable(current)) {
            resources_set_int_sprintf("Drive%iParallelCable", type, drive);
        }
    } else {
        if (machine_class == VICE_MACHINE_VIC20 || !drive_check_parallel_cable(current)) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            resources_get_int_sprintf("Drive%iParallelCable", &par, drive);
            if (par == type) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

#define DRIVE_PARALLEL_MENU(x)                                   \
    static ui_menu_entry_t drive_##x##_parallel_menu[] = {       \
        { "None",                                                \
          MENU_ENTRY_OTHER_TOGGLE,                               \
          set_par_callback,                                      \
          (ui_callback_data_t)(DRIVE_PC_NONE + (x << 16)) },     \
        { "Standard",                                            \
          MENU_ENTRY_OTHER_TOGGLE,                               \
          set_par_callback,                                      \
          (ui_callback_data_t)(DRIVE_PC_STANDARD + (x << 16)) }, \
        { "Dolphin DOS",                                         \
          MENU_ENTRY_OTHER_TOGGLE,                               \
          set_par_callback,                                      \
          (ui_callback_data_t)(DRIVE_PC_DD3 + (x << 16)) },      \
        { "Formel 64",                                           \
          MENU_ENTRY_OTHER_TOGGLE,                               \
          set_par_callback,                                      \
          (ui_callback_data_t)(DRIVE_PC_FORMEL64 + (x << 16)) }, \
        SDL_MENU_LIST_END                                        \
    };

DRIVE_PARALLEL_MENU(8)
DRIVE_PARALLEL_MENU(9)
DRIVE_PARALLEL_MENU(10)
DRIVE_PARALLEL_MENU(11)

UI_MENU_DEFINE_SLIDER(Drive8RPM, 25000, 35000)
UI_MENU_DEFINE_SLIDER(Drive9RPM, 25000, 35000)
UI_MENU_DEFINE_SLIDER(Drive10RPM, 25000, 35000)
UI_MENU_DEFINE_SLIDER(Drive11RPM, 25000, 35000)

UI_MENU_DEFINE_SLIDER(Drive8Wobble, 0, 1000)
UI_MENU_DEFINE_SLIDER(Drive9Wobble, 0, 1000)
UI_MENU_DEFINE_SLIDER(Drive10Wobble, 0, 1000)
UI_MENU_DEFINE_SLIDER(Drive11Wobble, 0, 1000)


/* patch some things that are slightly different in the emulators */
void uidrive_menu_create(void)
{
    int newend = 4;

    if (machine_class == VICE_MACHINE_VIC20) {
        newend = 1;
    } else if (machine_class == VICE_MACHINE_PLUS4) {
        newend = 2;
    }
    memset(&drive_8_parallel_menu[newend], 0, sizeof(ui_menu_entry_t));
    memset(&drive_9_parallel_menu[newend], 0, sizeof(ui_menu_entry_t));
    memset(&drive_10_parallel_menu[newend], 0, sizeof(ui_menu_entry_t));
    memset(&drive_11_parallel_menu[newend], 0, sizeof(ui_menu_entry_t));
}

static UI_MENU_CALLBACK(set_expand_callback)
{
    int drive;
    int parameter;
    int current;
    int memory;

    drive = (int)(vice_ptr_to_int(param) >> 16);
    parameter = (int)(vice_ptr_to_int(param) & 0xffff);

    current = get_drive_type(drive);

    if (activated) {
        if (drive_check_expansion(current) && check_memory_expansion(parameter, current)) {
            resources_get_int_sprintf("Drive%iRAM%X", &memory, drive, parameter);
            resources_set_int_sprintf("Drive%iRAM%X", !memory, drive, parameter);
        }
    } else {
        if (!drive_check_extend_policy(current) || !check_memory_expansion(parameter, current)) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            resources_get_int_sprintf("Drive%iRAM%X", &memory, drive, parameter);
            if (memory) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_exboard_callback)
{
    int drive;
    int parameter;
    int type;
    int memory;
    int available = 0;

    drive = (int)(vice_ptr_to_int(param) >> 16);
    parameter = (int)(vice_ptr_to_int(param) & 0xffff);

    type = get_drive_type(drive);

    switch (parameter) {
        case 0:
            available = drive_check_profdos(type);
            resources_get_int_sprintf("Drive%iProfDOS", &memory, drive);
            break;
        case 1:
            available = drive_check_supercard(type);
            resources_get_int_sprintf("Drive%iSuperCard", &memory, drive);
            break;
    }

    if (activated) {
        if (available) {
            switch (parameter) {
                case 0:
                    resources_set_int_sprintf("Drive%iProfDOS", !memory, drive);
                    break;
                case 1:
                    resources_set_int_sprintf("Drive%iSuperCard", !memory, drive);
                    break;
            }
        }
    } else {
        if (!available) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            if (memory) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_drive_type_callback)
{
    int drive;
    int parameter;
    int support = 0;
    int current;

    drive = (int)(vice_ptr_to_int(param) >> 16);
    parameter = (int)(vice_ptr_to_int(param) & 0xffff);

    if (parameter == ATTACH_DEVICE_REAL) {
#ifdef HAVE_OPENCBM
        support = opencbmlib_is_available();
#else
        support = 0;
#endif
    } else {
        support = (is_fs(parameter) || drive_check_type(parameter, drive - 8));
    }
    current = check_current_drive_type(parameter, drive);

    if (activated) {
        if (support) {
            if (parameter == ATTACH_DEVICE_REAL) {
#ifdef HAVE_OPENCBM
                resources_set_int_sprintf("IECDevice%i", 1, drive);
                resources_set_int_sprintf("FileSystemDevice%i", parameter, drive);
#endif
            } else if (is_fs(parameter)) {
                resources_set_int_sprintf("IECDevice%i", 1, drive);
                resources_set_int_sprintf("FileSystemDevice%i", parameter, drive);
            } else {
                if (has_fs()) {
                    resources_set_int_sprintf("IECDevice%i", 0, drive);
                }
                resources_set_int_sprintf("Drive%iType", parameter, drive);
            }
        }
    } else {
        if (!support) {
            return MENU_NOT_AVAILABLE_STRING;
        } else {
            if (current) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static int new_disk_image_type = DISK_IMAGE_TYPE_D64;

static UI_MENU_CALLBACK(set_disk_type_callback)
{
    int disk_type;

    disk_type = (int)(vice_ptr_to_int(param));

    if (activated) {
        new_disk_image_type = disk_type;
    } else {
        if (disk_type == new_disk_image_type) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(create_disk_image_callback)
{
    char *name = NULL;
    char *format_name;
    int overwrite = 1;
    int result;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Select diskimage name", FILEREQ_MODE_SAVE_FILE);
        if (name != NULL) {
            if (util_file_exists(name)) {
                if (message_box("VICE QUESTION", "File exists, do you want to overwrite?", MESSAGE_YESNO) != 0) {
                    overwrite = 0;
                }
            }
            if (overwrite == 1) {
                /* ask user for label,id of new disk */
                format_name = sdl_ui_text_input_dialog(
                        "Enter disk label,id:", NULL);
                if (!format_name) {
                    lib_free(name);
                    return NULL;
                }
                /* convert to PETSCII */
                charset_petconvstring((uint8_t *)format_name, 0);

                /* try to create the new image */
                if (vdrive_internal_create_format_disk_image(name, format_name,
                            new_disk_image_type) < 0) {
                    ui_error("Cannot create disk image");
                }
                result = message_box("Attach new image", "Select unit",
                        MESSAGE_UNIT_SELECT);
                /* 0-3 = unit #8 - unit #11, 4 = SKIP */
                if (result >= 0 && result <= 3) {
                    /* try to attach disk image */
                    if (file_system_attach_disk(result + 8, name) < 0) {
                        ui_error("Cannot attach disk image.");
                    }
                }

                lib_free(format_name);
            }
            lib_free(name);
        }
    }
    return NULL;
}

static const ui_menu_entry_t create_disk_image_type_menu[] = {
    { "D64",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D64 },
    { "D71",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D71 },
    { "D80",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D80 },
    { "D81",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D81 },
    { "D82",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D82 },
    { "D1M",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D1M },
    { "D2M",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D2M },
    { "D4M",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_D4M },
    { "G64",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_G64 },
    { "G71",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_G71 },
    { "P64",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_P64 },
    { "X64",
      MENU_ENTRY_RESOURCE_RADIO,
      set_disk_type_callback,
      (ui_callback_data_t)DISK_IMAGE_TYPE_X64 },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t create_disk_image_menu[] = {
    { "Image type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)create_disk_image_type_menu },
    { "Create",
      MENU_ENTRY_DIALOG,
      create_disk_image_callback,
      NULL },
    SDL_MENU_LIST_END
};

#define DRIVE_TYPE_ITEM(text, data) \
    { text,                         \
      MENU_ENTRY_OTHER_TOGGLE,      \
      set_drive_type_callback,      \
      (ui_callback_data_t)(data) },

#ifdef HAVE_OPENCBM
#define DRIVE_TYPE_ITEM_OPENCBM(text, data) DRIVE_TYPE_ITEM(text, data)
#else
#define DRIVE_TYPE_ITEM_OPENCBM(text, data)
#endif

#ifdef HAVE_RAWDRIVE
#define DRIVE_TYPE_ITEM_RAWDRIVE(text, data) DRIVE_TYPE_ITEM(text, data)
#else
#define DRIVE_TYPE_ITEM_RAWDRIVE(text, data)
#endif

#define DRIVE_TYPE_MENU(x)                                                      \
    static const ui_menu_entry_t drive_##x##_type_menu[] = {                    \
        DRIVE_TYPE_ITEM("None", x << 16)                                        \
        DRIVE_TYPE_ITEM("Directory", ATTACH_DEVICE_FS + (x << 16))              \
        DRIVE_TYPE_ITEM_OPENCBM("Real drive", ATTACH_DEVICE_REAL + (x << 16))   \
        DRIVE_TYPE_ITEM_RAWDRIVE("Block device", ATTACH_DEVICE_RAW + (x << 16)) \
        DRIVE_TYPE_ITEM("1540", DRIVE_TYPE_1540 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1541", DRIVE_TYPE_1541 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1541-II", DRIVE_TYPE_1541II + (x << 16))               \
        DRIVE_TYPE_ITEM("1551", DRIVE_TYPE_1551 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1570", DRIVE_TYPE_1570 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1571", DRIVE_TYPE_1571 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1571CR", DRIVE_TYPE_1571CR + (x << 16))                \
        DRIVE_TYPE_ITEM("1581", DRIVE_TYPE_1581 + (x << 16))                    \
        DRIVE_TYPE_ITEM("2000", DRIVE_TYPE_2000 + (x << 16))                    \
        DRIVE_TYPE_ITEM("4000", DRIVE_TYPE_4000 + (x << 16))                    \
        DRIVE_TYPE_ITEM("2031", DRIVE_TYPE_2031 + (x << 16))                    \
        DRIVE_TYPE_ITEM("2040", DRIVE_TYPE_2040 + (x << 16))                    \
        DRIVE_TYPE_ITEM("3040", DRIVE_TYPE_3040 + (x << 16))                    \
        DRIVE_TYPE_ITEM("4040", DRIVE_TYPE_4040 + (x << 16))                    \
        DRIVE_TYPE_ITEM("1001", DRIVE_TYPE_1001 + (x << 16))                    \
        DRIVE_TYPE_ITEM("8050", DRIVE_TYPE_8050 + (x << 16))                    \
        DRIVE_TYPE_ITEM("8250", DRIVE_TYPE_8250 + (x << 16))                    \
        SDL_MENU_LIST_END                                                       \
    };

DRIVE_TYPE_MENU(8)
DRIVE_TYPE_MENU(9)
DRIVE_TYPE_MENU(10)
DRIVE_TYPE_MENU(11)

#define DRIVE_FSDIR_MENU(x)                                   \
    static const ui_menu_entry_t drive_##x##_fsdir_menu[] = { \
        { "Choose directory",                                 \
          MENU_ENTRY_DIALOG,                                  \
          set_directory_callback,                             \
          (ui_callback_data_t)x },                            \
        { "Read P00 files",                                   \
          MENU_ENTRY_OTHER_TOGGLE,                            \
          set_read_p00_files_callback,                        \
          (ui_callback_data_t)x },                            \
        { "Write P00 files",                                  \
          MENU_ENTRY_OTHER_TOGGLE,                            \
          set_write_p00_files_callback,                       \
          (ui_callback_data_t)x },                            \
        { "Hide non-P00 files",                               \
          MENU_ENTRY_OTHER_TOGGLE,                            \
          set_hide_p00_files_callback,                        \
          (ui_callback_data_t)x },                            \
        SDL_MENU_LIST_END                                     \
    };

DRIVE_FSDIR_MENU(8)
DRIVE_FSDIR_MENU(9)
DRIVE_FSDIR_MENU(10)
DRIVE_FSDIR_MENU(11)

#define DRIVE_EXTEND_MENU(x)                                      \
    static const ui_menu_entry_t drive_##x##_extend_menu[] = {    \
        { "Never extend",                                         \
          MENU_ENTRY_OTHER_TOGGLE,                                \
          set_extend_callback,                                    \
          (ui_callback_data_t)(DRIVE_EXTEND_NEVER + (x << 8)) },  \
        { "Ask on extend",                                        \
          MENU_ENTRY_OTHER_TOGGLE,                                \
          set_extend_callback,                                    \
          (ui_callback_data_t)(DRIVE_EXTEND_ASK + (x << 8)) },    \
        { "Extend on access",                                     \
          MENU_ENTRY_OTHER_TOGGLE,                                \
          set_extend_callback,                                    \
          (ui_callback_data_t)(DRIVE_EXTEND_ACCESS + (x << 8)) }, \
        SDL_MENU_LIST_END                                         \
    };

DRIVE_EXTEND_MENU(8)
DRIVE_EXTEND_MENU(9)
DRIVE_EXTEND_MENU(10)
DRIVE_EXTEND_MENU(11)

#define DRIVE_EXPAND_MENU(x)                                   \
    static const ui_menu_entry_t drive_##x##_expand_menu[] = { \
        { "RAM at $2000-$3FFF",                                \
          MENU_ENTRY_OTHER_TOGGLE,                             \
          set_expand_callback,                                 \
          (ui_callback_data_t)(0x2000 + (x << 16)) },          \
        { "RAM at $4000-$5FFF",                                \
          MENU_ENTRY_OTHER_TOGGLE,                             \
          set_expand_callback,                                 \
          (ui_callback_data_t)(0x4000 + (x << 16)) },          \
        { "RAM at $6000-$7FFF",                                \
          MENU_ENTRY_OTHER_TOGGLE,                             \
          set_expand_callback,                                 \
          (ui_callback_data_t)(0x6000 + (x << 16)) },          \
        { "RAM at $8000-$9FFF",                                \
          MENU_ENTRY_OTHER_TOGGLE,                             \
          set_expand_callback,                                 \
          (ui_callback_data_t)(0x8000 + (x << 16)) },          \
        { "RAM at $A000-$BFFF",                                \
          MENU_ENTRY_OTHER_TOGGLE,                             \
          set_expand_callback,                                 \
          (ui_callback_data_t)(0xa000 + (x << 16)) },          \
        SDL_MENU_LIST_END                                      \
    };

DRIVE_EXPAND_MENU(8)
DRIVE_EXPAND_MENU(9)
DRIVE_EXPAND_MENU(10)
DRIVE_EXPAND_MENU(11)

UI_MENU_DEFINE_FILE_STRING(DriveProfDOS1571Name)
UI_MENU_DEFINE_FILE_STRING(DriveSuperCardName)

#define DRIVE_EXBOARD_MENU(x)                                          \
    static const ui_menu_entry_t drive_##x##_exboard_menu[] = {        \
        { "Professional DOS 1571",                                     \
          MENU_ENTRY_OTHER_TOGGLE,                                     \
          set_exboard_callback,                                        \
          (ui_callback_data_t)(0 + (x << 16)) },                       \
        { "Professional DOS 1571 ROM file",                            \
          MENU_ENTRY_DIALOG,                                           \
          file_string_DriveProfDOS1571Name_callback,                   \
          (ui_callback_data_t)"Set Professional DOS 1571 ROM image" }, \
        { "Supercard+",                                                \
          MENU_ENTRY_OTHER_TOGGLE,                                     \
          set_exboard_callback,                                        \
          (ui_callback_data_t)(1 + (x << 16)) },                       \
        { "Supercard+ ROM file",                                       \
          MENU_ENTRY_DIALOG,                                           \
          file_string_DriveSuperCardName_callback,                     \
          (ui_callback_data_t)"Set Supercard+ ROM image" },            \
        SDL_MENU_LIST_END                                              \
    };

DRIVE_EXBOARD_MENU(8)
DRIVE_EXBOARD_MENU(9)
DRIVE_EXBOARD_MENU(10)
DRIVE_EXBOARD_MENU(11)

#define DRIVE_IDLE_MENU(x)                                           \
    static const ui_menu_entry_t drive_##x##_idle_menu[] = {         \
        { "None",                                                    \
          MENU_ENTRY_OTHER_TOGGLE,                                   \
          set_idle_callback,                                         \
          (ui_callback_data_t)(DRIVE_IDLE_NO_IDLE + (x << 8)) },     \
        { "Trap idle",                                               \
          MENU_ENTRY_OTHER_TOGGLE,                                   \
          set_idle_callback,                                         \
          (ui_callback_data_t)(DRIVE_IDLE_SKIP_CYCLES + (x << 8)) }, \
        { "Skip cycles",                                             \
          MENU_ENTRY_OTHER_TOGGLE,                                   \
          set_idle_callback,                                         \
          (ui_callback_data_t)(DRIVE_IDLE_TRAP_IDLE + (x << 8)) },   \
        SDL_MENU_LIST_END                                            \
    };

DRIVE_IDLE_MENU(8)
DRIVE_IDLE_MENU(9)
DRIVE_IDLE_MENU(10)
DRIVE_IDLE_MENU(11)

UI_MENU_DEFINE_TOGGLE(Drive8RTCSave)
UI_MENU_DEFINE_TOGGLE(Drive9RTCSave)
UI_MENU_DEFINE_TOGGLE(Drive10RTCSave)
UI_MENU_DEFINE_TOGGLE(Drive11RTCSave)

UI_MENU_DEFINE_TOGGLE(AttachDevice8Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice9Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice10Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice11Readonly)

#ifdef HAVE_RAWDRIVE
UI_MENU_DEFINE_FILE_STRING(RawDriveDriver)
#endif

#ifdef HAVE_RAWDRIVE
#define DRIVE_MENU_RAWDRIVE_ITEM           \
    { "Blockdevice",                       \
      MENU_ENTRY_DIALOG,                   \
      file_string_RawDriveDriver_callback, \
      (ui_callback_data_t)"Select device file to use as drive" },
#else
#define DRIVE_MENU_RAWDRIVE_ITEM
#endif

#define DRIVE_MENU(x)                                           \
    static const ui_menu_entry_t drive_##x##_menu[] = {         \
        { "Drive " #x " type",                                  \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_type_callback,                       \
          (ui_callback_data_t)drive_##x##_type_menu },          \
        { "Drive " #x " dir settings",                          \
          MENU_ENTRY_SUBMENU,                                   \
          submenu_callback,                                     \
          (ui_callback_data_t)drive_##x##_fsdir_menu },         \
        { "Drive " #x " 40 track handling",                     \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_extend_callback,                     \
          (ui_callback_data_t)drive_##x##_extend_menu },        \
        { "Drive " #x " expansion memory",                      \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_expand_callback,                     \
          (ui_callback_data_t)drive_##x##_expand_menu },        \
        { "Drive " #x " expansion board",                       \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_exboard_callback,                    \
          (ui_callback_data_t)drive_##x##_exboard_menu },       \
        { "Drive " #x " idle method",                           \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_idle_callback,                       \
          (ui_callback_data_t)drive_##x##_idle_menu },          \
        { "Drive " #x " parallel cable",                        \
          MENU_ENTRY_SUBMENU,                                   \
          drive_##x##_show_parallel_callback,                   \
          (ui_callback_data_t)drive_##x##_parallel_menu },      \
        { "Drive " #x " RPM*100",                               \
          MENU_ENTRY_RESOURCE_INT,                              \
          slider_Drive##x##RPM_callback,                        \
          (ui_callback_data_t)"Set RPM (29500-30500)" },        \
        { "Drive " #x " wobble",                                \
          MENU_ENTRY_RESOURCE_INT,                              \
          slider_Drive##x##Wobble_callback,                     \
          (ui_callback_data_t)"Set Wobble (0-1000)" },          \
        SDL_MENU_ITEM_SEPARATOR,                                \
        { "Attach Drive " #x" read only",                       \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          toggle_AttachDevice##x##Readonly_callback,            \
          NULL },                                               \
        SDL_MENU_ITEM_SEPARATOR,                                \
        { "Save Drive " #x" FD2000/4000 RTC data",              \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          toggle_Drive##x##RTCSave_callback,                    \
          NULL },                                               \
        SDL_MENU_ITEM_SEPARATOR,                                \
        DRIVE_MENU_RAWDRIVE_ITEM                                \
        SDL_MENU_LIST_END                                       \
    };


DRIVE_MENU(8)
DRIVE_MENU(9)
DRIVE_MENU(10)
DRIVE_MENU(11)

static const ui_menu_entry_t fliplist_menu[] = {
    { "Add current image to fliplist",
      MENU_ENTRY_OTHER,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_ADD },
    { "Remove current image from fliplist",
      MENU_ENTRY_OTHER,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_REMOVE },
    { "Attach next image in fliplist",
      MENU_ENTRY_OTHER,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_NEXT },
    { "Attach previous image in fliplist",
      MENU_ENTRY_OTHER,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_PREVIOUS },
    { "Load fliplist",
      MENU_ENTRY_DIALOG,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_LOAD },
    { "Save fliplist",
      MENU_ENTRY_DIALOG,
      fliplist_callback,
      (ui_callback_data_t)UI_FLIP_SAVE },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(AutostartHandleTrueDriveEmulation)
UI_MENU_DEFINE_TOGGLE(AutostartWarp)
UI_MENU_DEFINE_TOGGLE(AutostartDelayRandom)
UI_MENU_DEFINE_TOGGLE(AutostartBasicLoad)
UI_MENU_DEFINE_TOGGLE(AutostartRunWithColon)
UI_MENU_DEFINE_RADIO(AutostartPrgMode)
UI_MENU_DEFINE_STRING(AutostartPrgDiskImage)


static UI_MENU_CALLBACK(custom_AutostartDelay_callback)
{
    static char buf[20];
    char *value;
    int previous;
    int new_value;

    resources_get_int("AutostartDelay", &previous);

    if (activated) {
        sprintf(buf, "%d", previous);
        value = sdl_ui_text_input_dialog(
                "Autostart delay in seconds (0 = default, max = 1000)", buf);
        if (value) {
            new_value = (int)strtol(value, NULL, 10);
            if (new_value < 0) {
                new_value = 0;
            } else if (new_value > 1000) {
                new_value = 1000;
            }
            if (new_value != previous) {
                resources_set_int("AutostartDelay", new_value);
            }
            lib_free(value);
        }
    } else {
        sprintf(buf, "%d seconds", previous);
        return buf;
    }
    return NULL;
}


static const ui_menu_entry_t autostart_settings_menu[] = {
    { "Handle TDE on autostart",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_AutostartHandleTrueDriveEmulation_callback,
      NULL },
    { "Autostart warp",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_AutostartWarp_callback,
      NULL },
    { "Autostart delay",
      MENU_ENTRY_RESOURCE_INT,
      custom_AutostartDelay_callback,
      NULL },
    { "Autostart random delay",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_AutostartDelayRandom_callback,
      NULL },
    { "Load to BASIC start (without ',1')",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_AutostartBasicLoad_callback,
      NULL },
    { "Use ':' with RUN",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_AutostartRunWithColon_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Autostart PRG mode"),
    { "VirtualFS",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_AutostartPrgMode_callback,
      (ui_callback_data_t)AUTOSTART_PRG_MODE_VFS },
    { "Inject",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_AutostartPrgMode_callback,
      (ui_callback_data_t)AUTOSTART_PRG_MODE_INJECT },
    { "Disk image",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_AutostartPrgMode_callback,
      (ui_callback_data_t)AUTOSTART_PRG_MODE_DISK },
    SDL_MENU_ITEM_SEPARATOR,
    { "Autostart disk image",
      MENU_ENTRY_RESOURCE_STRING,
      string_AutostartPrgDiskImage_callback,
      (ui_callback_data_t)"Disk image for autostarting PRG files" },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(custom_drive_volume_callback)
{
    static char buf[20];
    int previous, new_value;

    resources_get_int("DriveSoundEmulationVolume", &previous);

    if (activated) {
        new_value = sdl_ui_slider_input_dialog("Select volume", previous, 0, 4000);
        if (new_value != previous) {
            resources_set_int("DriveSoundEmulationVolume", new_value);
        }
    } else {
        sprintf(buf, "%3i%%", (previous * 100) / 4000);
        return buf;
    }
    return NULL;
}

const ui_menu_entry_t drive_menu[] = {
    { "Attach disk image to drive 8",
      MENU_ENTRY_DIALOG,
      attach_disk_callback,
      (ui_callback_data_t)8 },
    { "Attach disk image to drive 9",
      MENU_ENTRY_DIALOG,
      attach_disk_callback,
      (ui_callback_data_t)9 },
    { "Attach disk image to drive 10",
      MENU_ENTRY_DIALOG,
      attach_disk_callback,
      (ui_callback_data_t)10 },
    { "Attach disk image to drive 11",
      MENU_ENTRY_DIALOG,
      attach_disk_callback,
      (ui_callback_data_t)11 },
    { "Detach disk image from drive 8",
      MENU_ENTRY_OTHER,
      detach_disk_callback,
      (ui_callback_data_t)8 },
    { "Detach disk image from drive 9",
      MENU_ENTRY_OTHER,
      detach_disk_callback,
      (ui_callback_data_t)9 },
    { "Detach disk image from drive 10",
      MENU_ENTRY_OTHER,
      detach_disk_callback,
      (ui_callback_data_t)10 },
    { "Detach disk image from drive 11",
      MENU_ENTRY_OTHER,
      detach_disk_callback,
      (ui_callback_data_t)11 },
    { "Detach all disk images",
      MENU_ENTRY_OTHER,
      detach_disk_callback,
      (ui_callback_data_t)0 },
    { "Create new disk image",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)create_disk_image_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Drive 8 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_8_menu },
    { "Drive 9 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_9_menu },
    { "Drive 10 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_10_menu },
    { "Drive 11 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_11_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "True drive emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DriveTrueEmulation_callback,
      NULL },
    { "Drive sound emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DriveSoundEmulation_callback,
      NULL },
    { "Drive sound Volume",
      MENU_ENTRY_DIALOG,
      custom_drive_volume_callback,
      NULL },
    { "Virtual device traps",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VirtualDevices_callback,
      NULL },
    { "Autostart settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)autostart_settings_menu },
    { "Fliplist settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)fliplist_menu },
    SDL_MENU_LIST_END
};
