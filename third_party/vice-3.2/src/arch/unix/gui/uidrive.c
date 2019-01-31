/*
 * uidrive.c
 *
 * Written by
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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

#include "drive.h"
#include "resources.h"
#include "uiapi.h"
#include "uidrive.h"
#include "uimenu.h"
#include "util.h"

UI_MENU_DEFINE_TOGGLE(DriveTrueEmulation)
UI_MENU_DEFINE_TOGGLE(DriveSoundEmulation)
UI_MENU_DEFINE_TOGGLE(FlashTrueFS)
UI_MENU_DEFINE_RADIO(Drive8ExtendImagePolicy)
UI_MENU_DEFINE_RADIO(Drive9ExtendImagePolicy)
UI_MENU_DEFINE_RADIO(Drive10ExtendImagePolicy)
UI_MENU_DEFINE_RADIO(Drive11ExtendImagePolicy)

UI_CALLBACK(uidrive_extend_policy_control)
{
    if (!CHECK_MENUS) {
        ui_update_menus();
    } else {
        int type;

        resources_get_int_sprintf("Drive%iType", &type, vice_ptr_to_int(UI_MENU_CB_PARAM) + 8);

        if (drive_check_extend_policy(type)) {
            ui_menu_set_sensitive(w, 1);
        } else {
            ui_menu_set_sensitive(w, 0);
        }
    }
}

#define SET_DRIVE_EXTEND_IMAGE_POLICY_MENU(x, y)                                                       \
ui_menu_entry_t set_drive##x##_extend_image_policy_submenu[] = {                                       \
    { N_("Never extend"), UI_MENU_TYPE_TICK,                                                           \
      (ui_callback_t)radio_Drive##y##ExtendImagePolicy, (ui_callback_data_t)DRIVE_EXTEND_NEVER, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                       \
    { N_("Ask on extend"), UI_MENU_TYPE_TICK,                                                          \
      (ui_callback_t)radio_Drive##y##ExtendImagePolicy, (ui_callback_data_t)DRIVE_EXTEND_ASK, NULL,    \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                       \
    { N_("Extend on access"), UI_MENU_TYPE_TICK,                                                       \
      (ui_callback_t)radio_Drive##y##ExtendImagePolicy, (ui_callback_data_t)DRIVE_EXTEND_ACCESS, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                                       \
    UI_MENU_ENTRY_LIST_END                                                                             \
}

SET_DRIVE_EXTEND_IMAGE_POLICY_MENU(0, 8);
SET_DRIVE_EXTEND_IMAGE_POLICY_MENU(1, 9);
SET_DRIVE_EXTEND_IMAGE_POLICY_MENU(2, 10);
SET_DRIVE_EXTEND_IMAGE_POLICY_MENU(3, 11);

#define UI_RADIO_DRIVETYPE_CALLBACK(x)                                              \
UI_CALLBACK(radio_Drive##x##Type)                                                   \
{                                                                                   \
    int current_value;                                                              \
                                                                                    \
    resources_get_int("Drive" #x "Type", &current_value);                            \
    if (!CHECK_MENUS) {                                                             \
        if (current_value != vice_ptr_to_int(UI_MENU_CB_PARAM)) {                   \
            resources_set_int("Drive" #x "Type", vice_ptr_to_int(UI_MENU_CB_PARAM)); \
            ui_update_menus();                                                      \
        }                                                                           \
    } else {                                                                        \
        ui_menu_set_tick(w, current_value == vice_ptr_to_int(UI_MENU_CB_PARAM));    \
        if (drive_check_type(vice_ptr_to_int(UI_MENU_CB_PARAM), 0)) {               \
            ui_menu_set_sensitive(w, 1);                                            \
        } else {                                                                    \
            ui_menu_set_sensitive(w, 0);                                            \
        }                                                                           \
    }                                                                               \
}

UI_RADIO_DRIVETYPE_CALLBACK(8)
UI_RADIO_DRIVETYPE_CALLBACK(9)
UI_RADIO_DRIVETYPE_CALLBACK(10)
UI_RADIO_DRIVETYPE_CALLBACK(11)

UI_CALLBACK(drive_rpm_settings)
{
    int what = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *prompt, *title, resource[14];
    char buf[50];
    ui_button_t button;
    int current;
    long res;

    prompt = title = _("Drive RPM");
    sprintf(resource, "Drive%dRPM", what);

    resources_get_int(resource, &current);

    sprintf(buf, "%d", current);
    button = ui_input_string(title, prompt, buf, 50);
    switch (button) {
        case UI_BUTTON_OK:
            if (util_string_to_long(buf, NULL, 10, &res) != 0) {
                ui_error(_("Invalid value: %s"), buf);
                return;
            }
            resources_set_int(resource, (int)res);
            break;
        default:
            break;
    }
}

UI_CALLBACK(drive_wobble_settings)
{
    int what = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *prompt, *title, resource[14];
    char buf[50];
    ui_button_t button;
    int current;
    long res;

    prompt = title = _("Drive wobble");
    sprintf(resource, "Drive%dWobble", what);

    resources_get_int(resource, &current);

    sprintf(buf, "%d", current);
    button = ui_input_string(title, prompt, buf, 50);
    switch (button) {
        case UI_BUTTON_OK:
            if (util_string_to_long(buf, NULL, 10, &res) != 0) {
                ui_error(_("Invalid value: %s"), buf);
                return;
            }
            resources_set_int(resource, (int)res);
            break;
        default:
            break;
    }
}

ui_menu_entry_t ui_drive_options_submenu[] = {
    { N_("True drive emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveTrueEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive sound emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DriveSoundEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(flash_select_dir)
{
    uilib_select_dir("FSFlashDir", _("Select Flash filesystem directory"), _("Path:"));
}

ui_menu_entry_t ui_flash_options_submenu[] = {
    { N_("Enable true hardware flash file system"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_FlashTrueFS, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Set Flash filesystem directory"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)flash_select_dir, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
