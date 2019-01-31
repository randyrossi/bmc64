/*
 * uisnapshot.c - Snapshot save dialog for the MS-DOS version of VICE.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include "lib.h"
#include "machine.h"
#include "snapshot.h"
#include "tui.h"
#include "tuimenu.h"
#include "tuifs.h"
#include "ui.h"
#include "uisnapshot.h"
#include "util.h"

#define SNAPSHOT_EXTENSION      "vsf"

static char *snapshot_selector(const char *title);
static TUI_MENU_CALLBACK(file_name_callback);
static TUI_MENU_CALLBACK(toggle_callback);
static TUI_MENU_CALLBACK(write_snapshot_callback);
static TUI_MENU_CALLBACK(load_snapshot_callback);

static char *file_name = NULL;
static int save_roms_flag;
static int save_disks_flag;

static tui_menu_item_def_t write_snapshot_menu_def[] = {
    { "_File name:",
      "Specify snapshot file name",
      file_name_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Save _Disks:",
      "Save current disk images in the snapshot file",
      toggle_callback, (void *) &save_disks_flag, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save _ROMs:",
      "Save current ROMs in the snapshot file",
      toggle_callback, (void *) &save_roms_flag, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "_Do it!",
      "Save snapshot with the specified parameters",
      write_snapshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t ui_snapshot_menu_def[] = {
    { "_Write Snapshot",
      "Write a snapshot file",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, write_snapshot_menu_def, NULL },
    { "_Load Snapshot",
      "Load a snapshot file",
      load_snapshot_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static char *snapshot_selector(const char *title)
{
    return tui_file_selector(title, NULL, "*.vsf", NULL, NULL, NULL, NULL);
}

static TUI_MENU_CALLBACK(file_name_callback)
{
    if (been_activated) {
        char new_file_name[PATH_MAX];

        if (file_name == NULL) {
            memset(new_file_name, 0, PATH_MAX);
        } else {
            strcpy(new_file_name, file_name);
        }

        while (tui_input_string("Save snapshot", "Enter file name:", new_file_name, PATH_MAX) != -1) {
            util_remove_spaces(new_file_name);
            if (*new_file_name == 0) {
                char *tmp;

                tmp = snapshot_selector("Save snapshot file");
                if (tmp != NULL) {
                    strcpy(new_file_name, tmp);
                    lib_free(tmp);
                }
            } else {
                char *extension;
                char *last_dot;

                last_dot = strrchr(new_file_name, '.');

                if (last_dot == NULL) {
                    extension = SNAPSHOT_EXTENSION;
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
                        extension = SNAPSHOT_EXTENSION;
                    }
                }
                
                if (file_name == NULL) {
                    file_name = util_concat(new_file_name, ".", extension, NULL);
                } else {
                    lib_free(file_name);
                    file_name = lib_stralloc(new_file_name);
                    util_add_extension(&file_name, extension);
                }
                break;
            }
        }
    }

    return file_name;
}

static TUI_MENU_CALLBACK(toggle_callback)
{
    int *p = (int *) param;

    if (been_activated) {
        *p = ! *p;
    }

    return *p ? "On" : "Off";
}

static TUI_MENU_CALLBACK(write_snapshot_callback)
{
    if (been_activated) {
        if (file_name == NULL || *file_name == 0) {
            tui_error("Specify a file name first.");
            return NULL;
        }

        if (!util_file_exists(file_name) || tui_ask_confirmation("The specified file already exists.  Replace?  (Y/N)")) {
            if (machine_write_snapshot(file_name, save_roms_flag, save_disks_flag, 0) < 0) {
                snapshot_display_error();
            } else {
                tui_message("Snapshot saved successfully.");
            }
        }
    }

    return NULL;
}

static TUI_MENU_CALLBACK(load_snapshot_callback)
{
    if (been_activated) {
        char *name = snapshot_selector("Load snapshot file");

        if (name != NULL) {
            if (machine_read_snapshot(name, 0) < 0) {
                snapshot_display_error();
            } else {
                *behavior = TUI_MENU_BEH_RESUME;
            }
            lib_free(name);
        }
    }

    return NULL;
}
