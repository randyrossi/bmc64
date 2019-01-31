/*
 * uilib.c
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

#include <stdio.h>
#include <string.h>

#include "ioutil.h"
#include "lib.h"
#include "resources.h"
#include "uiapi.h"
#include "uicommands.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

/* prompt for a string and write it to a resource */
void uilib_select_string(const char *resname, const char *title, const char *name)
{
    ui_button_t button;
    const char *value;
    char *new_value;
    int len;

    vsync_suspend_speed_eval();

    resources_get_string(resname, &value);

    if (value == NULL) {
        value = "";
    }

    len = strlen(value) * 2;
    if (len < 255) {
        len = 255;
    }

    new_value = lib_calloc(1, len + 1);
    strncpy(new_value, value, len);

    button = ui_input_string(title, name, new_value, len);

    if (button == UI_BUTTON_OK) {
        resources_set_string(resname, new_value);
    }

    lib_free(new_value);
}

/* prompt for a file(name) and write it to a resource */
void uilib_select_file(const char *resname, const char *title, uilib_file_filter_enum_t filter)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;

    vsync_suspend_speed_eval();

    filename = ui_select_file(title, NULL, 0, last_dir, &filter, 1, &button, 0, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            if (resources_set_string(resname, filename) < 0) {
                ui_error(_("Could not select file\n'%s'"), filename);
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    ui_update_menus();
    lib_free(filename);
}

/* prompt for a device (name) and write it to a resource */
void uilib_select_dev(const char *resource, const char *title, uilib_file_filter_enum_t filter)
{
    char *filename;
    ui_button_t button;

    vsync_suspend_speed_eval();

    filename = ui_select_file(title, NULL, 0, "/dev", &filter, 1, &button, 0, NULL, UI_FC_LOAD);
    switch (button) {
        case UI_BUTTON_OK:
            resources_set_string(resource, filename);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

/* prompt for a filename, must allow to enter a non existing file */
ui_button_t uilib_input_file(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    vsync_suspend_speed_eval();

    filename = ui_select_file(title, NULL, 0, last_dir, &filter, 1, &button, 0, NULL, UI_FC_SAVE);

    switch (button) {
        case UI_BUTTON_OK:
            strncpy(buf, filename, buflen);
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    ui_update_menus();
    lib_free(filename);
    return button;
}

/* prompt for a pathname, must allow to enter a non existing path */
ui_button_t uilib_change_dir(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    return ui_change_dir(title, prompt, buf, buflen);
}

/* prompt for a path (name) and write it to a resource */
void uilib_select_dir(const char *resname, const char *title, const char *name)
{
    char *wd;
    int len;
    ui_button_t button;

    len = ioutil_maxpathlen();
    wd = lib_malloc(len);

    button = uilib_change_dir(title, _("Path:"), wd, len);

    switch (button) {
        case UI_BUTTON_OK:
            if (resources_set_string(resname, wd) < 0) {
                ui_error(_("Could not select dir\n'%s'"), wd);
            }
            break;
        default:
            /* Do nothing special.  */
            break;
    }

    ui_update_menus();
    lib_free(wd);
}
