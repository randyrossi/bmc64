/*
 * vsidui.c - Implementation of the VSID-specific part of the UI.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
 * based on c64ui.c written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#define VSIDUI 1 /* WTH ? */

#include <stdio.h>
#include <stdlib.h>

#include "attach.h"
#include "c64mem.h"
#include "c64ui.h"
#include "debug.h"
#include "icon.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "psid.h"
#include "resources.h"
#include "uiapi.h"
#include "uicommands.h"
#include "uilib.h"
#include "uimenu.h"
#include "uisettings.h"
#include "uisid.h"
#include "uisound.h"
#include "util.h"
#include "vicii.h"
#include "videoarch.h"
#include "vsync.h"
#include "vsidui.h"
#include "vsiduiunix.h"

#define VSID_WINDOW_MINW     (400)
#define VSID_WINDOW_MINH     (300)

/* FIXME: sid menus need to be handled differently, but still need to 
   figure out the right way. */

UI_MENU_DEFINE_RADIO(SidStereoAddressStart)
UI_MENU_DEFINE_RADIO(SidTripleAddressStart)

#define SET_SID_ADDRESS_D400_MENU(x, y)                                            \
static ui_menu_entry_t set_sid_##x##_address_d4xx_submenu[] = {                    \
    { "$D420", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd420, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D440", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd440, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D460", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd460, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D480", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd480, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D4A0", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd4a0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D4C0", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd4c0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    { "$D4E0", UI_MENU_TYPE_TICK,                                                  \
      (ui_callback_t)radio_Sid##y##AddressStart, (ui_callback_data_t)0xd4e0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                   \
    UI_MENU_ENTRY_LIST_END                                                         \
}

#define SET_SID_ADDRESS_RANGE_MENU(x, y, z)                                           \
static ui_menu_entry_t set_sid_##y##_address_##x##xx_submenu[] = {                    \
    { "$"#x"00", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##00, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"20", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##20, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"40", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##40, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"60", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##60, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"80", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##80, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"A0", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##a0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"C0", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##c0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    { "$"#x"E0", UI_MENU_TYPE_TICK,                                                   \
      (ui_callback_t)radio_Sid##z##AddressStart, (ui_callback_data_t)0x##x##e0, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                      \
    UI_MENU_ENTRY_LIST_END                                                            \
}

#define SET_SID_RANGE_MENU(x)                              \
static ui_menu_entry_t set_sid_##x##_address_submenu[] = { \
    { "$D4xx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_d4xx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    { "$D5xx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_d5xx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    { "$D6xx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_d6xx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    { "$D7xx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_d7xx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    { "$DExx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_dexx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    { "$DFxx", UI_MENU_TYPE_NORMAL,                        \
      NULL, NULL, set_sid_##x##_address_dfxx_submenu,      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },           \
    UI_MENU_ENTRY_LIST_END                                 \
}

SET_SID_ADDRESS_D400_MENU(stereo, Stereo);
SET_SID_ADDRESS_RANGE_MENU(d5, stereo, Stereo);
SET_SID_ADDRESS_RANGE_MENU(d6, stereo, Stereo);
SET_SID_ADDRESS_RANGE_MENU(d7, stereo, Stereo);
SET_SID_ADDRESS_RANGE_MENU(de, stereo, Stereo);
SET_SID_ADDRESS_RANGE_MENU(df, stereo, Stereo);

SET_SID_RANGE_MENU(stereo);

SET_SID_ADDRESS_D400_MENU(triple, Triple);
SET_SID_ADDRESS_RANGE_MENU(d5, triple, Triple);
SET_SID_ADDRESS_RANGE_MENU(d6, triple, Triple);
SET_SID_ADDRESS_RANGE_MENU(d7, triple, Triple);
SET_SID_ADDRESS_RANGE_MENU(de, triple, Triple);
SET_SID_ADDRESS_RANGE_MENU(df, triple, Triple);

SET_SID_RANGE_MENU(triple);

UI_MENU_DEFINE_TOGGLE(SidFilters)

static ui_menu_entry_t sid_submenu[] = {
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Extra SIDs"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_extra_sids_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Second SID base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Third SID base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("SID filters"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SidFilters, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RESID
    UI_MENU_ENTRY_SEPERATOR,
    { N_("reSID sampling method"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_resid_sampling_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if !defined(USE_GNOMEUI)
    { N_("reSID resampling passband"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_sid_resid_passband, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#endif
    UI_MENU_ENTRY_LIST_END
};

static log_t vsid_log = LOG_ERR;
static void vsid_create_menus(void);

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(PSIDTune)

static ui_menu_entry_t ui_tune_menu[] = {
    { N_("Tunes"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static char *psidpath = NULL;

static int vsid_ui_load_psid(char *filename)
{
    vsync_suspend_speed_eval();
    if (machine_autodetect_psid(filename) < 0) {
        log_error(vsid_log, "`%s' is not a valid PSID file.", filename);
        return -1;
    }
    lib_free(psidpath);
    util_fname_split(filename, &psidpath, NULL);
    psid_init_driver();
    machine_play_psid(0);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    vsid_create_menus();
    return 0;
}

static UI_CALLBACK(psid_load)
{
    char *filename;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_SID, UILIB_FILTER_ALL };

    filename = ui_select_file(_("Load PSID file"), NULL, 0, psidpath, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);

    vsync_suspend_speed_eval();

    switch (button) {
        case UI_BUTTON_OK:
            vsid_ui_load_psid(filename);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

static ui_menu_entry_t ui_load_commands_menu[] = {
    { N_("Load PSID file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)psid_load, NULL, NULL,
      KEYSYM_l, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};


/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static ui_menu_entry_t set_video_standard_submenu_vsid[] = {
    { "PAL-G", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "NTSC-M", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Old NTSC-M"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_NTSCOLD, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PAL-N", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_PALN, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* FIXME: find a better way that lets us use the global reset menu */
static UI_CALLBACK(reset)
{
    vsync_suspend_speed_eval();
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
}

static UI_CALLBACK(powerup_reset)
{
    vsync_suspend_speed_eval();
    machine_trigger_reset(MACHINE_RESET_MODE_HARD);
}

static ui_menu_entry_t reset_submenu[] = {
    { N_("Soft"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)reset, NULL, NULL,
      KEYSYM_F9, UI_HOTMOD_META },
    { N_("Hard"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)powerup_reset, NULL, NULL,
      KEYSYM_F12, UI_HOTMOD_META },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vsid_run_commands_menu[] = {
    { N_("Reset"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, reset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(PSIDKeepEnv)

static ui_menu_entry_t psid_menu[] = {
    { N_("Override PSID settings"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_PSIDKeepEnv, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Video standard"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_video_standard_submenu_vsid,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#ifndef USE_GNOMEUI
static ui_menu_entry_t vsidui_left_menu[] = {
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_load_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_tune_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_record_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, vsid_run_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_tool_commands_monitor_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_exit_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vsidui_right_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, psid_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_settings_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_help_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef DEBUG
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_debug_settings_menu_vsid,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

#endif

#ifdef USE_GNOMEUI
static ui_menu_entry_t vsidui_file_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_load_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_record_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, vsid_run_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_tool_commands_monitor_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_exit_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vsidui_settings_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, psid_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Maximum speed", UI_MENU_TYPE_NORMAL,
       NULL, NULL, set_maximum_speed_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_settings_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vsidui_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vsidui_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_tune_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vsidui_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef DEBUG
    { N_("Debug"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_settings_submenu_vsid,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    /* Translators: RJ means right justify and should be
        saved in your tranlation! e.g. german "RJHilfe" */
    { N_("RJHelp"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_help_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vsidui_speed_menu[] = {
    { N_("Reset"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, reset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif  /* USE_GNOMEUI */

/* ------------------------------------------------------------------------- */

extern int num_checkmark_menu_items;

static void vsid_create_menus(void)
{
    static ui_menu_entry_t tune_menu[256];
    static int tunes = 0;
    int default_tune;
    int i;
    char *buf;

    buf = lib_stralloc(_("Default Tune"));

    /* Free previously allocated memory. */
    for (i = 0; i <= tunes; i++) {
        lib_free(tune_menu[i].string);
    }

#ifdef USE_GNOMEUI
    ui_menu_shutdown();
#endif

    /* Get number of tunes in current PSID. */
    tunes = psid_tunes(&default_tune);

    /* Build tune menu. */
    for (i = 0; i <= tunes; i++) {
        tune_menu[i].string = (ui_callback_data_t)lib_stralloc(buf);
        tune_menu[i].type = UI_MENU_TYPE_TICK;
        tune_menu[i].callback = (ui_callback_t) radio_PSIDTune;
        tune_menu[i].callback_data = (ui_callback_data_t)(long)i;
        tune_menu[i].sub_menu = NULL;
        tune_menu[i].hotkey_keysym = i < 10 ? KEYSYM_0 + i : 0;
        tune_menu[i].hotkey_modifier = (ui_hotkey_modifier_t) i < 10 ? UI_HOTMOD_META : 0;
        lib_free(buf);
        buf = lib_msprintf(_("Tune %d"), i + 1);
    }

    lib_free(buf);

    /* last entry in menu */
    tune_menu[i].string = (ui_callback_data_t) NULL;

    num_checkmark_menu_items = 0;
    ui_tune_menu[0].sub_menu = tune_menu;

#ifndef USE_GNOMEUI
    ui_set_left_menu(vsidui_left_menu);
    ui_set_right_menu(vsidui_right_menu);
#endif

#ifdef USE_GNOMEUI
    ui_set_topmenu(vsidui_top_menu);
    ui_set_speedmenu(vsidui_speed_menu);
#endif
}

static void vsid_close_menus(void)
{
    unsigned int i;
    for (i = 0; i < 256; i++) {
        if (ui_tune_menu[0].sub_menu[i].string == NULL) {
            break;
        }
        lib_free(ui_tune_menu[0].sub_menu[i].string);
    }
}

int vsid_ui_init(void)
{
    int res;
    video_canvas_t *canvas = vicii_get_canvas();

    res = ui_open_canvas_window(canvas, _("VSID: The SID emulator"), VSID_WINDOW_MINW, VSID_WINDOW_MINH, 0);
    if (res < 0) {
        return -1;
    }

    /* FIXME: There might be a separte vsid icon.  */
    ui_set_application_icon(c64_icon_data);
    uisound_menu_create();
    uisid_model_menu_create();

    ui_set_drop_callback(vsid_ui_load_psid);

    vsid_create_menus();
    return 0;
}

/* void vsid_ui_shutdown(void) */
void vsid_ui_close(void) /* FIXME: bad name */
{
    if (psidpath != NULL) {
        lib_free(psidpath);
        psidpath = NULL;
    }

    uisound_menu_shutdown();
    uisid_model_menu_shutdown();
    vsid_close_menus();
#ifndef USE_GNOMEUI
    ui_vsid_control_shutdown();
#endif
}

/******************************************************************************/

void vsid_ui_display_name(const char *name)
{
    log_message(LOG_DEFAULT, "VSIDUI: Name: %s", name);
    ui_vsid_setpsid(name);
}

void vsid_ui_display_author(const char *author)
{
    log_message(LOG_DEFAULT, "VSIDUI: Author: %s", author);
    ui_vsid_setauthor(author);
}

void vsid_ui_display_copyright(const char *copyright)
{
    log_message(LOG_DEFAULT, "VSIDUI: Copyright: %s", copyright);
    ui_vsid_setcopyright(copyright);
}

void vsid_ui_display_sync(int sync)
{
    char buf[50];
    sprintf(buf, "Using %s sync", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
    log_message(LOG_DEFAULT, "VSIDUI: %s", buf);
    ui_vsid_setsync(buf);
}

void vsid_ui_display_sid_model(int model)
{
    log_message(LOG_DEFAULT, "VSIDUI: Using %s emulation", model == 0 ? "MOS6581" : "MOS8580");
    ui_vsid_setmodel(model == 0 ? "MOS6581" : "MOS8580");
}

void vsid_ui_set_default_tune(int nr)
{
    log_message(LOG_DEFAULT, "VSIDUI: Default tune: %i", nr);
    ui_vsid_setdeftune(nr);
}

void vsid_ui_display_tune_nr(int nr)
{
    log_message(LOG_DEFAULT, "VSIDUI: Playing tune: %i", nr);
    ui_vsid_settune(nr);
}

void vsid_ui_display_nr_of_tunes(int count)
{
    log_message(LOG_DEFAULT, "VSIDUI: Number of tunes: %i", count);
    ui_vsid_setnumtunes(count);
}

void vsid_ui_display_time(unsigned int sec)
{
    /* log_message(LOG_DEFAULT, "VSIDUI: Time: %i", sec); */
    ui_vsid_settime(sec);
}

void vsid_ui_display_irqtype(const char *irq)
{
    log_message(LOG_DEFAULT, "VSIDUI: Using %s interrupt", irq);
    ui_vsid_setirq(irq);
}

void vsid_ui_setdrv(char* driver_info_text)
{
    log_message(LOG_DEFAULT, "VSIDUI: Driver info: %s", driver_info_text);
    ui_vsid_setdrv(driver_info_text);
}


/** \brief  Set driver address
 *
 * \param[in]   addr    driver address
 */
void vsid_ui_set_driver_addr(uint16_t addr)
{
}


/** \brief  Set load address
 *
 * \param[in]   addr    load address
 */
void vsid_ui_set_load_addr(uint16_t addr)
{
}


/** \brief  Set init routine address
 *
 * \param[in]   addr    init routine address
 */
void vsid_ui_set_init_addr(uint16_t addr)
{
}


/** \brief  Set play routine address
 *
 * \param[in]   addr    play routine address
 */
void vsid_ui_set_play_addr(uint16_t addr)
{
}


/** \brief  Set size of SID on actual machine
 *
 * \param[in]   size    size of SID
 */
void vsid_ui_set_data_size(uint16_t size)
{
}

