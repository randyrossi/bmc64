/*
 * uifliplist.c
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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
#include "drive.h"
#include "fliplist.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "uiapi.h"
#include "uifliplist.h"
#include "uidrive.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

struct cb_data_t {
    int unit;
    long data;                  /* should be enough for a pointer */
};

typedef enum {
    CBD_NEXT, CBD_PREV, CBD_ADD, CBD_REMOVE
} cbd_enum_t;

extern char last_attached_images[NUM_DRIVES][256];
extern ui_drive_enable_t enabled_drives;
extern UI_CALLBACK(attach_disk);
extern UI_CALLBACK(detach_disk);

static UI_CALLBACK(attach_from_fliplist3)
{
    fliplist_attach_head(8, vice_ptr_to_int(UI_MENU_CB_PARAM));
}

static UI_CALLBACK(add2fliplist)
{
    fliplist_add_image(8);
    uifliplist_update_menus(8, 8);
}

static UI_CALLBACK(remove_from_fliplist)
{
    fliplist_remove(8, NULL);
    uifliplist_update_menus(8, 8);
}

static char *load_save_fliplist_last_dir = NULL;

static UI_CALLBACK(load_save_fliplist)
{
    char *filename, *title;
    int what = vice_ptr_to_int(UI_MENU_CB_PARAM);
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_FLIPLIST, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();
    title = util_concat(what ? _("Load ") : _("Save"), _("Flip list file"), NULL);
    filename = ui_select_file(title, NULL, 0, load_save_fliplist_last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 1, NULL, what ? UI_FC_LOAD : UI_FC_SAVE);
    lib_free(title);
    switch (button) {
        case UI_BUTTON_OK:
            if (what) {
                if (fliplist_load_list(FLIPLIST_ALL_UNITS, filename, 0) == 0) {
                    ui_message(_("Successfully read `%s'."), filename);
                } else {
                    ui_error(_("Error reading `%s'."), filename);
                }
            } else {
                if (fliplist_save_list(FLIPLIST_ALL_UNITS, filename) == 0) {
                    ui_message(_("Successfully wrote `%s'"), filename);
                } else {
                    ui_error(_("Error writing `%s'."), filename);
                }
            }
            lib_free(load_save_fliplist_last_dir);
            util_fname_split(filename, &load_save_fliplist_last_dir, NULL);
            break;
        default:
            break;
    }
}

ui_menu_entry_t fliplist_submenu[] = {
    { N_("Add current image (Unit 8)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)add2fliplist, (ui_callback_data_t)0, NULL,
      KEYSYM_i, UI_HOTMOD_META },
    { N_("Remove current image (Unit 8)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)remove_from_fliplist, (ui_callback_data_t)0, NULL,
      KEYSYM_k, UI_HOTMOD_META },
    { N_("Attach next image (Unit 8)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)attach_from_fliplist3, (ui_callback_data_t)1, NULL,
      KEYSYM_n, UI_HOTMOD_META },
    { N_("Attach previous image (Unit 8)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)attach_from_fliplist3, (ui_callback_data_t)0, NULL,
      KEYSYM_N, UI_HOTMOD_META | UI_HOTMOD_SHIFT },
    { N_("Load flip list file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)load_save_fliplist, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save flip list file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)load_save_fliplist, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(attach_from_fliplist2)
{
    file_system_attach_disk(fliplist_get_unit((void *)UI_MENU_CB_PARAM), fliplist_get_image((void *)UI_MENU_CB_PARAM));
}

static UI_CALLBACK(remove_from_fliplist2)
{
    fliplist_remove(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit, (char *)((struct cb_data_t *)UI_MENU_CB_PARAM)->data);
    uifliplist_update_menus(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit, ((struct cb_data_t *)UI_MENU_CB_PARAM)->unit);
}

static UI_CALLBACK(add2fliplist2)
{
    fliplist_set_current(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit, (char *)((struct cb_data_t *)UI_MENU_CB_PARAM)->data);
    fliplist_add_image(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit);
    uifliplist_update_menus(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit, ((struct cb_data_t *)UI_MENU_CB_PARAM)->unit);
}

static UI_CALLBACK(attach_from_fliplist)
{
    fliplist_attach_head(((struct cb_data_t *)UI_MENU_CB_PARAM)->unit, (int)((struct cb_data_t *)UI_MENU_CB_PARAM)->data);
}

UI_MENU_DEFINE_TOGGLE(AttachDevice8Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice9Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice10Readonly)
UI_MENU_DEFINE_TOGGLE(AttachDevice11Readonly)

#define FLIPLIST_MENU_LIMIT 256

void uifliplist_update_menus(int from_unit, int to_unit)
{
    /* Yick, allocate dynamically */
    static ui_menu_entry_t flipmenu[NUM_DRIVES][FLIPLIST_MENU_LIMIT];
    static struct cb_data_t cb_data[NUM_DRIVES][sizeof(cbd_enum_t)];

    char *image = NULL, *t0 = NULL, *t1 = NULL, *t2 = NULL, *t3 = NULL;
    char *t4 = NULL, *t5 = NULL;
    void *fl_iterator;
    int i, drive, true_emu, fliplist_start = 0;

    ui_callback_t callback = NULL;

    resources_get_int("DriveTrueEmulation", &true_emu);

    for (drive = from_unit - 8; (drive <= to_unit - 8) && (drive < NUM_DRIVES); drive++) {
        i = 0;
        t0 = t1 = t2 = t3 = t4 = t5 = NULL;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        t0 = lib_msprintf(_("Attach #%d"), drive + 8);
        flipmenu[drive][i].string = t0;
        flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
        flipmenu[drive][i].callback = (ui_callback_t)attach_disk;
        flipmenu[drive][i].callback_data = (ui_callback_data_t)(long)(drive + 8);
        i++;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        t5 = lib_msprintf(_("Detach #%d"), drive + 8);
        flipmenu[drive][i].string = t5;
        flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
        flipmenu[drive][i].callback = (ui_callback_t)detach_disk;
        flipmenu[drive][i].callback_data = (ui_callback_data_t)(long)(drive + 8);
        i++;

        /* drivesettings */
        memcpy(&flipmenu[drive][i], (const char *)ui_drive_options_submenu, sizeof(ui_menu_entry_t));
        i++;
        /* Write protext UI controll */
        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        flipmenu[drive][i].string = _("Read-only");
        flipmenu[drive][i].type = UI_MENU_TYPE_TICK;
        switch (drive) {
        case 0:
            callback = (ui_callback_t)toggle_AttachDevice8Readonly;
            break;
        case 1:
            callback = (ui_callback_t)toggle_AttachDevice9Readonly;
            break;
        case 2:
            callback = (ui_callback_t)toggle_AttachDevice10Readonly;
            break;
        case 3:
            callback = (ui_callback_t)toggle_AttachDevice11Readonly;
            break;
        }
        flipmenu[drive][i].callback = callback;
        i++;

        fliplist_start = i;     /* if we take the goto don't free anything */

        /* don't update menu deeply when drive has not been enabled
           or nothing has been attached */
        if (true_emu) {
            if (!((1 << drive) & enabled_drives)) {
                goto update_menu;
            }
        } else {
            if (drive_get_disk_drive_type(drive) == DRIVE_TYPE_NONE) {
                goto update_menu;
            }
        }

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        flipmenu[drive][i].string = "--";
        flipmenu[drive][i].type = UI_MENU_TYPE_SEPARATOR;
        i++;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        util_fname_split(fliplist_get_next(drive + 8), NULL, &image);
        if (image) {
            t1 = util_concat(_("Next: "), image, NULL);
        } else {
            t1 = util_concat(_("Next: "), "<", _("empty"), ">", NULL);
        }
        flipmenu[drive][i].string = t1;
        flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
        flipmenu[drive][i].callback = (ui_callback_t)attach_from_fliplist;
        cb_data[drive][CBD_NEXT].unit = drive + 8;
        cb_data[drive][CBD_NEXT].data = 1;
        flipmenu[drive][i].callback_data = (ui_callback_data_t)&(cb_data[drive][CBD_NEXT]);
        lib_free(image);
        image = NULL;
        i++;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        util_fname_split(fliplist_get_prev(drive + 8), NULL, &image);
        if (image) {
            t2 = util_concat(_("Previous: "), image, NULL);
        } else {
            t2 = util_concat(_("Previous: "), "<", _("empty"), ">", NULL);
        }
        flipmenu[drive][i].string = t2;
        flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
        flipmenu[drive][i].callback = (ui_callback_t)attach_from_fliplist;
        cb_data[drive][CBD_PREV].unit = drive + 8;
        cb_data[drive][CBD_PREV].data = 0;
        flipmenu[drive][i].callback_data = (ui_callback_data_t)&(cb_data[drive][CBD_PREV]);
        lib_free(image);
        image = NULL;
        i++;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        util_fname_split(last_attached_images[drive], NULL, &image);
        if (image[0]) {
            t3 = util_concat(_("Add: "), image, NULL);
            flipmenu[drive][i].string = t3;
            flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
            flipmenu[drive][i].callback = (ui_callback_t)add2fliplist2;
            cb_data[drive][CBD_ADD].unit = drive + 8;
            cb_data[drive][CBD_ADD].data = (long) last_attached_images[drive];
            flipmenu[drive][i].callback_data = (ui_callback_data_t)&(cb_data[drive][CBD_ADD]);
            i++;
        }
        lib_free(image);
        image = NULL;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        util_fname_split(last_attached_images[drive], NULL, &image);
        if (image[0]) {
            t4 = util_concat(_("Remove: "), image, NULL);
            flipmenu[drive][i].string = t4;
            flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
            flipmenu[drive][i].callback = (ui_callback_t)remove_from_fliplist2;
            cb_data[drive][CBD_REMOVE].unit = drive + 8;
            cb_data[drive][CBD_REMOVE].data = (long) last_attached_images[drive];
            flipmenu[drive][i].callback_data = (ui_callback_data_t)&(cb_data[drive][CBD_REMOVE]);
            i++;
        }
        lib_free(image);
        image = NULL;

        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
        flipmenu[drive][i].string = "--";
        flipmenu[drive][i].type = UI_MENU_TYPE_SEPARATOR;
        i++;

        /* Now collect current fliplist */
        fl_iterator = fliplist_init_iterate(drive + 8);
        fliplist_start = i;
        while (fl_iterator) {
            memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));
            util_fname_split(fliplist_get_image(fl_iterator), NULL, &image);
            flipmenu[drive][i].string = util_concat(NO_TRANS, image, NULL);
            flipmenu[drive][i].type = UI_MENU_TYPE_NORMAL;
            flipmenu[drive][i].callback = (ui_callback_t)attach_from_fliplist2;
            flipmenu[drive][i].callback_data = (ui_callback_data_t)fl_iterator;

            fl_iterator = fliplist_next_iterate(drive + 8);
            lib_free(image);
            image = NULL;
            i++;
            if (i >= (FLIPLIST_MENU_LIMIT - 1)) {
                /* the end delimitor must fit */
                log_warning(LOG_DEFAULT, "Number of fliplist menu entries exceeded. Cutting after %d entries.", i);
                break;
            }
        }

    update_menu:
        /* make sure the menu is well terminated */
        memset(&(flipmenu[drive][i]), 0, sizeof(ui_menu_entry_t));

        ui_destroy_drive_menu(drive);
        ui_set_drive_menu(drive, flipmenu[drive]);

        lib_free(t0);
        lib_free(t1);
        lib_free(t2);
        lib_free(t3);
        lib_free(t4);
        lib_free(t5);
        while (fliplist_start < i) {
            lib_free(flipmenu[drive][fliplist_start].string);
            flipmenu[drive][fliplist_start].string = NULL;
            fliplist_start++;
        }
    }
    /* Update the checkmarks */
    ui_menu_update_all();
}

void uifliplist_shutdown(void)
{
    lib_free(load_save_fliplist_last_dir);
}
