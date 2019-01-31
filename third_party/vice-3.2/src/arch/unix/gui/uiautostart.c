/*
 * uiautostart.c
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "autostart-prg.h"
#include "lib.h"
#include "resources.h"
#include "uiautostart.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

UI_MENU_DEFINE_TOGGLE(AutostartHandleTrueDriveEmulation)
UI_MENU_DEFINE_TOGGLE(AutostartWarp)
UI_MENU_DEFINE_TOGGLE(AutostartRunWithColon)
UI_MENU_DEFINE_TOGGLE(AutostartBasicLoad)
UI_MENU_DEFINE_TOGGLE(AutostartDelayRandom)
UI_MENU_DEFINE_RADIO(AutostartPrgMode)

static UI_CALLBACK(set_autostart_image_name)
{
    char *name = util_concat(_("Name"), ":", NULL);

    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Disk image for PRG autostart"), name);
    lib_free(name);
}

static UI_CALLBACK(set_autostart_delay)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;

    vsync_suspend_speed_eval();

    resources_get_int("AutostartDelay", &i);

    sprintf(input_string, "%d", i);
    msg_string = lib_stralloc(_("Enter autostart delay in seconds\n(max. 1000, 0 means default)"));
    button = ui_input_string(_("Autostart delay"), msg_string, input_string, 32);
    lib_free(msg_string);
    if (button == UI_BUTTON_OK) {
        i = atoi(input_string);
        if (i < 0) {
            i = 0;
        } else {
            if (i > 1000) {
                i = 1000;
            }
        }
        resources_set_int("AutostartDelay", i);
        ui_update_menus();
    }
}

static ui_menu_entry_t uiautostart_prg_mode_submenu[] = {
    { N_("Virtual FS"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_AutostartPrgMode, (ui_callback_data_t)AUTOSTART_PRG_MODE_VFS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Inject to RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_AutostartPrgMode, (ui_callback_data_t)AUTOSTART_PRG_MODE_INJECT, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Disk image"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_AutostartPrgMode, (ui_callback_data_t)AUTOSTART_PRG_MODE_DISK, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t uiautostart_settings_submenu[] = {
    { N_("Handle True Drive Emulation on autostart"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AutostartHandleTrueDriveEmulation, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Autostart warp"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AutostartWarp, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Autostart random delay"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AutostartDelayRandom, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Autostart fixed delay"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_autostart_delay, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Use ':' with RUN"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AutostartRunWithColon, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load to BASIC start"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AutostartBasicLoad, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PRG autostart mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uiautostart_prg_mode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PRG autostart disk image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_autostart_image_name, (ui_callback_data_t)"AutostartPrgDiskImage", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
