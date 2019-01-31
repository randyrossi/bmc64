/*
 * uisettings.c - Implementation of common UI settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archapi.h"
#include "debug.h"
#include "fliplist.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "types.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "uiperipheral.h"
#include "uirs232.h"
#include "uisettings.h"
#include "uisound.h"
#include "util.h"
#include "vsync.h"


static UI_CALLBACK(set_refresh_rate)
{
    int current_refresh_rate;

    resources_get_int("RefreshRate", &current_refresh_rate);

    if (!CHECK_MENUS) {
        if (current_refresh_rate != vice_ptr_to_int(UI_MENU_CB_PARAM)) {
            resources_set_int("RefreshRate", vice_ptr_to_int(UI_MENU_CB_PARAM));
            ui_update_menus();
        }
    } else {
        if (vice_ptr_to_int(UI_MENU_CB_PARAM) == current_refresh_rate) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
        if (UI_MENU_CB_PARAM == 0) {
            int speed;

            resources_get_int("Speed", &speed);
            if (speed == 0) {
                /* Cannot enable the `automatic' setting if a speed limit is
                   not specified. */
                ui_menu_set_sensitive(w, 0);
            } else {
                ui_menu_set_sensitive(w, 1);
            }
        }
    }
}

static UI_CALLBACK(set_custom_refresh_rate)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;
    int current_refresh_rate;

    resources_get_int("RefreshRate", &current_refresh_rate);

    if (!*input_string) {
        sprintf(input_string, "%d", current_refresh_rate);
    }

    if (CHECK_MENUS) {
        if (current_refresh_rate < 0 || current_refresh_rate > 10) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    } else {
        int current_speed;

        vsync_suspend_speed_eval();
        msg_string = lib_stralloc(_("Enter refresh rate"));
        button = ui_input_string(_("Refresh rate"), msg_string, input_string, 32);
        lib_free(msg_string);
        if (button == UI_BUTTON_OK) {
            i = atoi(input_string);
            resources_get_int("Speed", &current_speed);
            if (!(current_speed <= 0 && i <= 0) && i >= 0 && current_refresh_rate != i) {
                resources_set_int("RefreshRate", i);
                ui_update_menus();
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

static UI_CALLBACK(set_maximum_speed)
{
    int current_speed;

    resources_get_int("Speed", &current_speed);

    if (!CHECK_MENUS) {
        if (current_speed != vice_ptr_to_int(UI_MENU_CB_PARAM)) {
            resources_set_int("Speed", vice_ptr_to_int(UI_MENU_CB_PARAM));
            ui_update_menus();
        }
    } else {
        if (current_speed == vice_ptr_to_int(UI_MENU_CB_PARAM)) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
        if (UI_MENU_CB_PARAM == 0) {
            if (machine_class != VICE_MACHINE_VSID) {
                int current_refresh_rate;

                resources_get_int("RefreshRate", &current_refresh_rate);

                ui_menu_set_sensitive(w, current_refresh_rate != 0);
            }
        }
    }
}

static UI_CALLBACK(set_custom_maximum_speed)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int i;
    int current_speed;

    resources_get_int("Speed", &current_speed);
    if (!*input_string) {
        sprintf(input_string, "%d", current_speed);
    }

    if (CHECK_MENUS) {
        switch (current_speed) {
            case 200:
            case 100:
            case  50:
            case  20:
            case  10:
            case   0:
                ui_menu_set_tick(w, 0);
                break;
            default:
                ui_menu_set_tick(w, 1);
        }
    } else {
        vsync_suspend_speed_eval();
        msg_string = lib_stralloc(_("Enter speed"));
        button = ui_input_string(_("Maximum speed"), msg_string, input_string, 32);
        lib_free(msg_string);
        if (button == UI_BUTTON_OK) {
            i = atoi(input_string);
            if (machine_class != VICE_MACHINE_VSID) {
                int current_refresh_rate;

                resources_get_int("RefreshRate", &current_refresh_rate);

                if (!(current_refresh_rate <= 0 && i <= 0) && i >= 0 && current_speed != i) {
                    resources_set_int("Speed", i);
                    ui_update_menus();
                } else {
                    ui_error(_("Invalid speed value"));
                }
            } else {
                if (i > 0 && i != current_speed) {
                    resources_set_int("Speed", i);
                    ui_update_menus();
                } else {
                    ui_error(_("Invalid speed value"));
                }
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

static UI_CALLBACK(save_resources)
{
#if 0
    char *fname;
#endif

    vsync_suspend_speed_eval();
    if (resources_save(NULL) < 0) {
        ui_error(_("Cannot save settings."));
    }else {
        if (w != NULL) {
            ui_message(_("Settings saved successfully."));
        }
    }
#if 0
    fname = archdep_default_fliplist_file_name();
    fliplist_save_list((unsigned int) -1, fname);
    lib_free(fname);
#endif
    ui_update_menus();
}

static UI_CALLBACK(load_resources)
{
    int r;

    vsync_suspend_speed_eval();
    r = resources_load(NULL);

    if (r < 0) {
        if (r == RESERR_FILE_INVALID) {
            ui_error(_("Cannot load settings:\nresource file not valid."));
        } else {
            ui_error(_("Cannot load settings:\nresource file not found."));
        }
    }

    ui_update_menus();
}

static char *resources_last_dir = NULL;

static UI_CALLBACK(save_resources_file)
{
    char *filename;
    ui_button_t button;
    int len = 1024;

#ifndef HAVE_DIRNAME
    char *tmp;
#endif

    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    vsync_suspend_speed_eval();

    filename = lib_malloc(len + 1);
    strcpy(filename, "");

    filename = ui_select_file(_("File to save settings to"), NULL, 0, resources_last_dir, &filter, 1, &button, 0, NULL, UI_FC_SAVE);
    
    if (button == UI_BUTTON_OK && filename != NULL) {
        if (resources_save(filename) < 0) {
            ui_error(_("Cannot save settings."));
        } else {
            if (w != NULL) {
                ui_message(_("Settings saved successfully."));
            }
        }
        lib_free(resources_last_dir);
        util_fname_split(filename, &resources_last_dir, NULL);
    }

    lib_free(filename);
    ui_update_menus();
}

static UI_CALLBACK(load_resources_file)
{
    char *filename;
    ui_button_t button;
    int r;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    vsync_suspend_speed_eval();
    filename = ui_select_file(_("Resource file name"), NULL, 0, resources_last_dir, &filter, 1, &button, 0, NULL, UI_FC_LOAD);

    if (button == UI_BUTTON_OK && filename != NULL) {
        r = resources_load(filename);
        if (r < 0) {
            if (r == RESERR_FILE_INVALID) {
                ui_error(_("Cannot load settings:\nresource file not valid."));
            } else {
                ui_error(_("Cannot load settings:\nresource file not found."));
            }
        } else {
            lib_free(resources_last_dir);
            util_fname_split(filename, &resources_last_dir, NULL);
        }
    }

    lib_free(filename);
    ui_update_menus();
}

static UI_CALLBACK(set_default_resources)
{
    vsync_suspend_speed_eval();
    resources_set_defaults();
    ui_update_menus();
}

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_TOGGLE(SaveResourcesOnExit)
UI_MENU_DEFINE_TOGGLE(ConfirmOnExit)

/* ------------------------------------------------------------------------- */

/*
 * If you change the list of refresh rates,
 * also adjust set_custom_refresh_rate().
 */
static ui_menu_entry_t set_refresh_rate_submenu[] = {
    { N_("Auto"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/1", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/2", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/3", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)3, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/4", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)4, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/5", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)5, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/6", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)6, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/7", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)7, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/8", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)8, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/9", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)9, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1/10", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_refresh_rate, (ui_callback_data_t)10, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Custom"), UI_MENU_TYPE_TICKDOTS,
      (ui_callback_t)set_custom_refresh_rate, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/*
 * If you change the list of maximum speeds,
 * also adjust set_custom_maximum_speed().
 */
ui_menu_entry_t set_maximum_speed_submenu[] = {
    { "200%", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)200, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "100%", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)100, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "50%", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)50, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "20%", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)20, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "10%", UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)10, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("No limit"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_maximum_speed, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Custom"), UI_MENU_TYPE_TICKDOTS,
      (ui_callback_t)set_custom_maximum_speed, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

ui_menu_entry_t ui_performance_settings_menu[] = {
    { N_("Refresh rate"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_refresh_rate_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Maximum speed"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_maximum_speed_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_settings_settings_menu[] = {
    { N_("Save settings"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)save_resources, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load settings"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)load_resources, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save settings to file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)save_resources_file, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load settings from file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)load_resources_file, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Restore default settings"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)set_default_resources, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Save settings on exit"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SaveResourcesOnExit, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Confirm quitting VICE"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ConfirmOnExit, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

#ifdef DEBUG

static UI_CALLBACK(set_auto_playback_frames)
{
    static char input_string[32];
    char *msg_string;
    ui_button_t button;
    int autoplaybackframes;

    if (!CHECK_MENUS) {
        vsync_suspend_speed_eval();
        resources_get_int("AutoPlaybackFrames", &autoplaybackframes);
        if (!*input_string) {
            sprintf(input_string, "%d", autoplaybackframes);
        }
        msg_string = lib_stralloc(_("Enter auto playback frames"));
        button = ui_input_string(_("Auto playback frames"), msg_string, input_string, 32);
        lib_free(msg_string);
        if (button == UI_BUTTON_OK) {
            autoplaybackframes = atoi(input_string);
            resources_set_int("AutoPlaybackFrames", autoplaybackframes);
            ui_update_menus();
        }
    }
}

UI_MENU_DEFINE_RADIO(TraceMode)

ui_menu_entry_t debug_tracemode_submenu[] = {
    { N_("Normal"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TraceMode, (ui_callback_data_t)DEBUG_NORMAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Small"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TraceMode, (ui_callback_data_t)DEBUG_SMALL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("History"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TraceMode, (ui_callback_data_t)DEBUG_HISTORY, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Autoplay"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TraceMode, (ui_callback_data_t)DEBUG_AUTOPLAY, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(MainCPU_TRACE)
UI_MENU_DEFINE_TOGGLE(Drive0CPU_TRACE)
UI_MENU_DEFINE_TOGGLE(Drive1CPU_TRACE)
UI_MENU_DEFINE_TOGGLE(Drive2CPU_TRACE)
UI_MENU_DEFINE_TOGGLE(Drive3CPU_TRACE)
UI_MENU_DEFINE_TOGGLE(DoCoreDump)

ui_menu_entry_t debug_settings_submenu[] = {
    { N_("Trace Mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_tracemode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Main CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MainCPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Drive0 CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Drive0CPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive1 CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Drive1CPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive2 CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Drive2CPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Drive3 CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Drive3CPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR, /* replaced by extra items in XY_dynamic_menu_create() (eg DTV) */
    { N_("Autoplay playback frames"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_auto_playback_frames, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save core dump"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DoCoreDump, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t debug_settings_submenu_vsid[] = {
    { N_("Trace Mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_tracemode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Main CPU Trace"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MainCPU_TRACE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Autoplay playback frames"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_auto_playback_frames, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Save core dump"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DoCoreDump, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_debug_settings_menu[] = {
    { N_("Debug settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_debug_settings_menu_vsid[] = {
    { N_("Debug settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_settings_submenu_vsid,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif
