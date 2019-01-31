/*
 * uiattach.c - Attach menu commands and settings.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "attach.h"
#include "autostart.h"
#include "autostart-prg.h"
#include "diskimage.h"
#include "fliplist.h"
#include "imagecontents.h"
#include "diskcontents.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "ui.h"
#include "uiattach.h"
#include "util.h"
#include "vdrive-internal.h"

static char *file_name = NULL;
static char *format_name = NULL;
static unsigned int file_type = 0;

static char *image_type_name[] = { ".d64", ".d67", ".d71", ".d80", ".d81", ".d82", ".g64", ".p64", ".x64", ".d1m", ".d2m", ".d4m" };
static char *image_type_name_for_user[] = { "D64", "D67", "D71", "D80" ,"D81", "D82", "G64", "P64", "X64", "D1M", "D2M", "D4M" };
static int image_type[] = {
    DISK_IMAGE_TYPE_D64,
    DISK_IMAGE_TYPE_D67,
    DISK_IMAGE_TYPE_D71,
    DISK_IMAGE_TYPE_D80,
    DISK_IMAGE_TYPE_D81,
    DISK_IMAGE_TYPE_D82,
    DISK_IMAGE_TYPE_G64,
    DISK_IMAGE_TYPE_P64,
    DISK_IMAGE_TYPE_X64,
    DISK_IMAGE_TYPE_D1M,
    DISK_IMAGE_TYPE_D2M,
    DISK_IMAGE_TYPE_D4M
};

static TUI_MENU_CALLBACK(attach_disk_callback);
static TUI_MENU_CALLBACK(autostart_callback);
static TUI_MENU_CALLBACK(create_disk_image_name_callback);
static TUI_MENU_CALLBACK(create_format_name_callback);
static TUI_MENU_CALLBACK(create_disk_image_callback);
static TUI_MENU_CALLBACK(create_set_disk_image_type_callback);
static TUI_MENU_CALLBACK(create_disk_image_type_submenu_callback);
static TUI_MENU_CALLBACK(flip_add_callback);
static TUI_MENU_CALLBACK(flip_remove_callback);
static TUI_MENU_CALLBACK(flip_next_callback);
static TUI_MENU_CALLBACK(flip_previous_callback);

static tui_menu_item_def_t disk_image_type_submenu[] = {
    { "D_64", "Create D64 disk image", create_set_disk_image_type_callback,
      (void *)0, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_D67", "Create D67 disk image", create_set_disk_image_type_callback,
      (void *)1, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D_71", "Create D71 disk image", create_set_disk_image_type_callback,
      (void *)2, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D8_0", "Create D80 disk image", create_set_disk_image_type_callback,
      (void *)3, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D_81", "Create D81 disk image", create_set_disk_image_type_callback,
      (void *)4, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D8_2", "Create D82 disk image", create_set_disk_image_type_callback,
      (void *)5, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_G64", "Create GCR disk image", create_set_disk_image_type_callback,
      (void *)6, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_P64", "Create P64 disk image", create_set_disk_image_type_callback,
      (void *)7, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_X64", "Create X64 disk image", create_set_disk_image_type_callback,
      (void *)8, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D_1M", "Create D1M disk image", create_set_disk_image_type_callback,
      (void *)9, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D2_M", "Create D2M disk image", create_set_disk_image_type_callback,
      (void *)10, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "D_4M", "Create D4M disk image", create_set_disk_image_type_callback,
      (void *)11, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_create_disk_menu_def[] = {
    { "Disk Image _Type",
      "Specify the type of the disk image",
      create_disk_image_type_submenu_callback, NULL, 26,
      TUI_MENU_BEH_CONTINUE, disk_image_type_submenu, "Disk image type" },
    { "Disk Image _Name:",
      "Specify the name of the disk image",
      create_disk_image_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Format name",
      "Specify name and ID used to format the disk",
      create_format_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "_Create disk!",
      "Create a blank disk and format it with the specified parameters",
      create_disk_image_callback, (void *)0, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "Create disk and attach to drive #_8",
      "Create a blank disk, format and attach it to drive #8",
      create_disk_image_callback, (void *)8, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Create disk and attach to drive #_9",
      "Create a blank disk, format and attach it to drive #9",
      create_disk_image_callback, (void *)9, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Create disk and attach to drive #1_0",
      "Create a blank disk, format and attach it to drive #10",
      create_disk_image_callback, (void *)10, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Create disk and attach to drive #1_1",
      "Create a blank disk, format and attach it to drive #11",
      create_disk_image_callback, (void *)11, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ui_flip_menu_def[] = {
    { "_Add current image",
      "Add current disk image to flip list (ALT-F3)",
      flip_add_callback, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Remove current image",
      "Remove current disk image from flip list (ALT-F4)",
      flip_remove_callback, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach _next image",
      "Attach next disk image from flip list (ALT-F1)",
      flip_next_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Attach _previous image",
      "Attach previous disk image from flip list (ALT-F2)",
      flip_previous_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(AutostartHandleTrueDriveEmulation)
TUI_MENU_DEFINE_TOGGLE(AutostartWarp)
TUI_MENU_DEFINE_TOGGLE(AutostartRunWithColon)
TUI_MENU_DEFINE_TOGGLE(AutostartBasicLoad)
TUI_MENU_DEFINE_TOGGLE(AutostartDelayRandom)
TUI_MENU_DEFINE_RADIO(AutostartPrgMode)

static TUI_MENU_CALLBACK(ui_set_AutostartDelay_callback)
{
    if (been_activated) {
        int delay, value;
        char buf[10];

        resources_get_int("AutostartDelay", &delay);
        sprintf(buf, "%d", delay);

        if (tui_input_string("Autostar delay", "Enter autostart delay to use:", buf, 10) == 0) {
            value = atoi(buf);
            if (value > 1000) {
                value = 1000;
            } else if (value < 0) {
                value = 0;
            }
            resources_set_int("AutostartDelay", value);
            tui_message("Autostart delay set to : %d", value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(autostart_prg_mode_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("AutostartPrgMode", &value);
    switch (value) {
        default:
        case AUTOSTART_PRG_MODE_VFS:
            s = "Virtual FS";
            break;
        case AUTOSTART_PRG_MODE_INJECT:
            s = "Inject";
            break;
        case AUTOSTART_PRG_MODE_DISK:
            s = "Disk Image";
            break;
    }
    return s;
}

static tui_menu_item_def_t autostart_prg_mode_submenu[] = {
    { "Virtual FS", NULL, radio_AutostartPrgMode_callback,
      (void *)AUTOSTART_PRG_MODE_VFS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Inject", NULL, radio_AutostartPrgMode_callback,
      (void *)AUTOSTART_PRG_MODE_INJECT, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Disk Image", NULL, radio_AutostartPrgMode_callback,
      (void *)AUTOSTART_PRG_MODE_DISK, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(autostart_prg_disk_image_file_callback)
{
    char s[256];
    const char *v;

    if (been_activated) {

        *s = '\0';

        if (tui_input_string("Change autostart PRG disk image name", "New image name:", s, 255) == -1) {
            return NULL;
        }

        if (*s == '\0') {
            return NULL;
        }

        resources_set_string("AutostartPrgDiskImage", s);
    }

    resources_get_string("AutostartPrgDiskImage", &v);

    return v;
}

static tui_menu_item_def_t ui_autostart_menu_def[] = {
    { "Handle TDE on autostart:", "Handle TDE on autostart",
      toggle_AutostartHandleTrueDriveEmulation_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Warp on autostart:", "Warp on autostart",
      toggle_AutostartWarp_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Use ':' with RUN:", "Use ':' with RUN",
      toggle_AutostartRunWithColon_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load to BASIC start", "Load without ,1",
      toggle_AutostartBasicLoad_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Delay (in secs)",
      "Set the delay to use",
      ui_set_AutostartDelay_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Random delay", "Random delay",
      toggle_AutostartDelayRandom_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Autostart PRG mode:", "Select the mode of the autostart PRG",
      autostart_prg_mode_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, autostart_prg_mode_submenu, "Autostart PRG mode" },
    { "Autostart PRG disk image file:", "Select the autostart PRG disk image file",
      autostart_prg_disk_image_file_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t ui_attach_menu_def[] = {
    { "Drive #_8:",
      "Attach disk image for disk drive #8",
      attach_disk_callback, (void *)8, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #_9:",
      "Attach disk image for disk drive #9",
      attach_disk_callback, (void *)9, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #1_0:",
      "Attach disk image for disk drive #10",
      attach_disk_callback, (void *)10, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #1_1:",
      "Attach disk image for disk drive #11",
      attach_disk_callback, (void *)11, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Autostart _Settings...",
      "Autostart Settings", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_autostart_menu_def, NULL },
    { "Autostart _Drive #8",
      "Reset the emulator and run the first program in the disk in drive 8",
      autostart_callback, (void *)8, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Create _blank disk & attach",
      "Create a blank disk image and attach it to drive #8",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_create_disk_menu_def, NULL },
    { "_Fliplist for Drive #8...",
      "Select, add or remove disk images from the flip list", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_flip_menu_def, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(attach_disk_callback)
{
    const char *s;

    if (been_activated) {
        char *default_item, *directory;
        char *name, *file;
        unsigned int file_number = 0;

        s = file_system_get_disk_name((unsigned int)param);
        util_fname_split(s, &directory, &default_item);

        name = tui_file_selector("Attach a disk image", directory, 
                                 "*.d64;*.d71;*.d81;*.g64;*.g71;*.g41;*.x64;*.p64;*.d80;*.d82;*.d67;*.d1m;*.d2m;*.d4m;"
                                 "*.d6z;*.d7z;*.d8z;*.g6z;*.g4z;*.x6z;*.zip;*.gz;*.lzh",
                                 default_item, diskcontents_filesystem_read, &file,
                                 &file_number);
        if (file_number > 0) {
            if (autostart_disk(name, NULL, file_number, AUTOSTART_MODE_RUN) < 0) {
                tui_error("Cannot autostart disk image.");
            } else {
                *behavior = TUI_MENU_BEH_RESUME;
            }
        } else if (name != NULL && (s == NULL || strcasecmp(name, s) != 0) && file_system_attach_disk((int)param, name) < 0) {
            tui_error("Invalid disk image.");
        }

        lib_free(file);

        ui_update_menus();

        lib_free(directory);
        lib_free(default_item);
        lib_free(name);
    }

    s = file_system_get_disk_name((unsigned int)param);

    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static TUI_MENU_CALLBACK(autostart_callback)
{
    if (been_activated) {
        if (autostart_device((int)param) < 0) {
            tui_error("Cannot autostart device #%d", (int)param);
        }
    }

    return NULL;
}

static TUI_MENU_CALLBACK(create_disk_image_type_submenu_callback)
{
    return image_type_name_for_user[file_type];
}

static TUI_MENU_CALLBACK(create_set_disk_image_type_callback)
{
    if (been_activated) {
        file_type = (unsigned int)param;
    }

    return NULL;
}

static TUI_MENU_CALLBACK(create_disk_image_name_callback)
{
    if (been_activated) {
        char new_file_name[PATH_MAX];

        if (file_name == NULL) {
            memset(new_file_name, 0, PATH_MAX);
        } else {
            strcpy(new_file_name, file_name);
        }

        while (tui_input_string("Create disk image", "Enter file name:", new_file_name, PATH_MAX) != -1) {
            util_remove_spaces(new_file_name);
            if (*new_file_name == 0) {
                char *tmp;

                tmp = tui_file_selector("Create disk image", NULL, "*.d64", NULL, NULL, NULL, NULL);
                if (tmp != NULL) {
                    strcpy(new_file_name, tmp);
                    lib_free(tmp);
                }
            } else {
                char *extension;
                char *last_dot;

                last_dot = strrchr(new_file_name, '.');

                if (last_dot == NULL) {
                    extension = image_type_name[file_type];
                } else {
                    char *last_slash, *last_backslash, *last_path_separator;

                    last_slash = strrchr(new_file_name, '/');
                    last_backslash = strrchr(new_file_name, '\\');

                    if (last_slash == NULL) {
                        last_path_separator = last_backslash;
                    } else if (last_backslash == NULL) {
                        last_path_separator = last_slash;
                    } else if (last_backslash < last_slash) {
                        last_path_separator = last_slash;
                    } else {
                        last_path_separator = last_backslash;
                    }

                    if (last_path_separator == NULL || last_path_separator < last_dot) {
                        extension = "";
                    } else {
                        extension = image_type_name[file_type];
                    }
                }

                if (file_name == NULL) {
                    file_name = util_concat(new_file_name, extension, NULL);
                } else {
                    int len = strlen(new_file_name);

                    file_name = lib_realloc(file_name, len + strlen(extension) + 1);
                    memcpy(file_name, new_file_name, len);
                    strcpy(file_name + len, extension);
                }
                break;
            }
        }
    }

    return file_name;
}

static TUI_MENU_CALLBACK(create_format_name_callback)
{
    if (been_activated) {
        char new_format_name[20];

        memset(new_format_name, 0, 20);
        tui_input_string("Format disk image", "Name and ID:", new_format_name, 19);
        lib_free(format_name);
        format_name = lib_stralloc(new_format_name);
    }

    return format_name;
}

static TUI_MENU_CALLBACK(create_disk_image_callback)
{
    if (been_activated) {
        unsigned int drive;

        drive = (unsigned int)param;

        if (file_name == NULL) {
            tui_error("Specify a disk image name.");
            return NULL;
        }

        if (vdrive_internal_create_format_disk_image(file_name, format_name, image_type[file_type]) < 0) {
            tui_error("Could not create disk image!");
            return NULL;
        }

        if (drive > 0) {
            if (file_system_attach_disk(drive, file_name) < 0) {
                tui_error("Could not attach disk image!");
                return NULL;
            }
        }

        ui_update_menus();
    }

    return NULL;
}

static TUI_MENU_CALLBACK(flip_add_callback)
{
    if (been_activated) {
        fliplist_add_image(8);
    }
    return NULL;
}

static TUI_MENU_CALLBACK(flip_remove_callback)
{
    if (been_activated) {
        fliplist_remove(-1, NULL);
    }
    return NULL;
}

static TUI_MENU_CALLBACK(flip_next_callback)
{
    if (been_activated) {
        fliplist_attach_head(8, 1);
    }
    return NULL;
}

static TUI_MENU_CALLBACK(flip_previous_callback)
{
    if (been_activated) {
        fliplist_attach_head(8, 0);
    }
    return NULL;
}
