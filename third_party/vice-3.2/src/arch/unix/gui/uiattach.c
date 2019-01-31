/*
 * uiattach.c
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
#include <stdlib.h>
#include <string.h>

#include "attach.h"
#include "autostart.h"
#include "imagecontents.h"
#include "tapecontents.h"
#include "diskcontents.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "tape.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uiautostart.h"
#include "uiedisk.h"
#include "uifliplist.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"
#include "ioutil.h"
#include "cbmimage.h"
#include "diskimage.h"

static int selection_from_image = 0;

void ui_set_selected_file(int num)
{
    selection_from_image = num;
}

static char *attach_disk_last_dir = NULL;

static read_contents_func_type funcs[] = {
    diskcontents_read_unit8,
    diskcontents_read_unit9,
    diskcontents_read_unit10,
    diskcontents_read_unit11
};

UI_CALLBACK(attach_disk)
{
    int unit = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *filename, *title;
    ui_button_t button;
    int attach_wp, old_attach_wp;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_DISK, UILIB_FILTER_COMPRESSED, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    resources_get_int_sprintf("AttachDevice%dReadonly", &attach_wp, unit);
    old_attach_wp = attach_wp;

    title = lib_msprintf(_("Attach Disk Image as unit #%d"), unit);
    filename = ui_select_file(title, funcs[unit - 8], unit == 8 ? 1 : 0, attach_disk_last_dir,
                              filter, sizeof(filter) / sizeof(*filter), &button,  1, &attach_wp, UI_FC_LOAD);

    lib_free(title);

    if ((button == UI_BUTTON_OK) || (button == UI_BUTTON_AUTOSTART)) {
        log_message(LOG_DEFAULT, "Attaching '%s' %s...", filename, attach_wp ? "read only" : "writable");
        resources_set_int_sprintf("AttachDevice%dReadonly", attach_wp, unit);
    }

    switch (button) {
        case UI_BUTTON_OK:
            if (file_system_attach_disk(unit, filename) < 0) {
                ui_error(_("Invalid Disk Image"));
                resources_set_int_sprintf("AttachDevice%dReadonly", old_attach_wp, unit);
            }
            lib_free(attach_disk_last_dir);
            util_fname_split(filename, &attach_disk_last_dir, NULL);
            break;
        case UI_BUTTON_AUTOSTART:
            if (autostart_disk(filename, NULL, selection_from_image, AUTOSTART_MODE_RUN) < 0) {
                ui_error(_("Invalid Disk Image or Filename"));
                resources_set_int_sprintf("AttachDevice%dReadonly", old_attach_wp, unit);
            }
            lib_free(attach_disk_last_dir);
            util_fname_split(filename, &attach_disk_last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }

    lib_free(filename);
}

static ui_menu_entry_t attach_disk_image_submenu[] = {
    { N_("Unit #8"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_disk, (ui_callback_data_t)8, NULL,
      KEYSYM_8, UI_HOTMOD_META },
    { N_("Unit #9"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_disk, (ui_callback_data_t)9, NULL,
      KEYSYM_9, UI_HOTMOD_META },
    { N_("Unit #10"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_disk, (ui_callback_data_t)10, NULL,
      KEYSYM_0, UI_HOTMOD_META },
    { N_("Unit #11"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_disk, (ui_callback_data_t)11, NULL,
      KEYSYM_1, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(attach_empty_disk)
{
    int unit = vice_ptr_to_int(UI_MENU_CB_PARAM);

    /* Where does the 1024 come from?  */
    char filename[1024];

    vsync_suspend_speed_eval();

    /* The following code depends on a zeroed filename.  */
    memset(filename, 0, 1024);

    if (ui_empty_disk_dialog(filename) < 0) {
        return;
    }

    if (file_system_attach_disk(unit, filename) < 0) {
        ui_error(_("Invalid Disk Image"));
    }
}

static ui_menu_entry_t attach_empty_disk_image_submenu[] = {
    { N_("Unit #8"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_empty_disk, (ui_callback_data_t)8, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #9"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_empty_disk, (ui_callback_data_t)9, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #10"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_empty_disk, (ui_callback_data_t)10, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #11"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_empty_disk, (ui_callback_data_t)11, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_CALLBACK(detach_disk)
{
    int unit = vice_ptr_to_int(UI_MENU_CB_PARAM);

    vsync_suspend_speed_eval();
    file_system_detach_disk(unit);
}

static ui_menu_entry_t detach_disk_image_submenu[] = {
    { N_("Unit #8"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_disk, (ui_callback_data_t)8, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #9"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_disk, (ui_callback_data_t)9, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #10"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_disk, (ui_callback_data_t)10, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unit #11"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_disk, (ui_callback_data_t)11, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("All"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_disk, (ui_callback_data_t)-1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiattach_disk_menu[] = {
    { N_("Attach disk image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_disk_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Create and attach an empty disk"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_empty_disk_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Detach disk image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, detach_disk_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Flip list"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, fliplist_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static char *attach_tape_last_dir = NULL;

static UI_CALLBACK(create_new_tape)
{
    char *filename;
    int overwrite = 1;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_TAPE, UILIB_FILTER_COMPRESSED, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Create a new tape image"), tapecontents_read, 0, attach_tape_last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 1, NULL, UI_FC_SAVE);

    switch (button) {
        case UI_BUTTON_OK:
            if (util_file_exists(filename)) {
                if (ui_ask_yesno(_("File exists"),
                                        _("Do you want to overwrite it?")) !=
                        UI_BUTTON_OK) {
                    overwrite = 0;
                }
            }
            if (overwrite != 0) {
                if (cbmimage_create_image(filename, DISK_IMAGE_TYPE_TAP)) {
                    ui_error("Cannot create tape image");
                }
            }
            lib_free(attach_tape_last_dir);
            util_fname_split(filename, &attach_tape_last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

static UI_CALLBACK(attach_tape)
{
    char *filename;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_TAPE, UILIB_FILTER_COMPRESSED, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Attach tape image"), tapecontents_read, 1, attach_tape_last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 1, NULL, UI_FC_LOAD);

    switch (button) {
        case UI_BUTTON_OK:
            if (tape_image_attach(1, filename) < 0) {
                ui_error(_("Invalid Tape Image"));
            }
            lib_free(attach_tape_last_dir);
            util_fname_split(filename, &attach_tape_last_dir, NULL);
            break;
        case UI_BUTTON_AUTOSTART:
            if (autostart_tape(filename, NULL, selection_from_image, AUTOSTART_MODE_RUN) < 0) {
                ui_error(_("Invalid Tape Image"));
            }
            lib_free(attach_tape_last_dir);
            util_fname_split(filename, &attach_tape_last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

static UI_CALLBACK(detach_tape)
{
    tape_image_detach(1);
}

ui_menu_entry_t uiattach_tape_menu[] = {
    { N_("Create a new tape image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)create_new_tape, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach tape image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_tape, NULL, NULL,
      KEYSYM_t, UI_HOTMOD_META },
    { N_("Detach tape image"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_tape, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static image_contents_t *read_disk_or_tape_image_contents(const char *fname)
{
    image_contents_t *tmp;

    tmp = diskcontents_filesystem_read(fname);
    if (tmp) {
        return tmp;
    }
    return tapecontents_read(fname);
}

static char *smart_attach_last_dir = NULL;

int uiattach_autostart_file(char *filename)
{
    return autostart_autodetect(filename, NULL, 0, AUTOSTART_MODE_RUN);
}

static UI_CALLBACK(smart_attach)
{
    char *filename;
    ui_button_t button;
    int do_free_dir;
    char *dir;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_ALL, UILIB_FILTER_TAPE, UILIB_FILTER_DISK, UILIB_FILTER_PRGP00, UILIB_FILTER_COMPRESSED };

    vsync_suspend_speed_eval();

    if (smart_attach_last_dir) {
        dir = smart_attach_last_dir;
        do_free_dir = 0;
    } else {
        dir = ioutil_current_dir();
        do_free_dir = 1;
    }
    filename = ui_select_file(_("Smart-attach a file"), read_disk_or_tape_image_contents, 1, dir, filter, sizeof(filter) / sizeof(*filter), &button, 1, NULL, UI_FC_LOAD);
    if (do_free_dir) {
        lib_free(dir);
    }

    switch (button) {
        case UI_BUTTON_OK:
            if (file_system_attach_disk(8, filename) < 0 && tape_image_attach(1, filename) < 0 && autostart_snapshot(filename, NULL) < 0 && autostart_prg(filename, AUTOSTART_MODE_LOAD) < 0) {
                ui_error(_("Unknown image type"));
            }
            lib_free(smart_attach_last_dir);
            util_fname_split(filename, &smart_attach_last_dir, NULL);
            break;
        case UI_BUTTON_AUTOSTART:
            if (autostart_autodetect(filename, NULL, selection_from_image, AUTOSTART_MODE_RUN) < 0) {
                ui_error(_("Unknown image type"));
            }
            lib_free(smart_attach_last_dir);
            util_fname_split(filename, &smart_attach_last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

ui_menu_entry_t uiattach_smart_attach_menu[] = {
    { N_("Smart-attach disk/tape"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)smart_attach, NULL, NULL,
      KEYSYM_a, UI_HOTMOD_META },
    { N_("Autostart settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uiautostart_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

void uiattach_shutdown(void)
{
    lib_free(attach_disk_last_dir);
    lib_free(attach_tape_last_dir);
    lib_free(smart_attach_last_dir);
}
