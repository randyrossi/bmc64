/*
 * menudefs.c - Definition of menu commands and settings.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <conio.h>
#include <dir.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "archdep.h"
#include "attach.h"
#include "autostart.h"
#include "datasette.h"
#include "drive.h"
#include "gfxoutput.h"
#include "imagecontents.h"
#include "tapecontents.h"
#include "info.h"
#include "interrupt.h"
#include "ioutil.h"
#include "kbd.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "mem.h"
#include "menudefs.h"
#include "monitor.h"
#include "resources.h"
#include "tape.h"
#include "tui.h"
#include "tui_backend.h"
#include "tuifs.h"
#include "tuiview.h"
#include "ui.h"
#include "uiattach.h"
#include "uidrive.h"
#include "uiffmpeg.h"
#include "uijoyport.h"
#include "uijoystick.h"
#include "uikeymap.h"

#ifdef HAVE_NETWORK
#include "uinetplay.h"
#endif

#include "uiperipherial.h"
#include "uiprinter.h"
#include "uisampler.h"
#include "uiscreenshot.h"
#include "uisnapshot.h"
#include "uisound.h"
#include "util.h"
#include "version.h"
#include "vicefeatures.h"
#include "video.h"
#include "videoarch.h"

#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif

/* ------------------------------------------------------------------------- */

tui_menu_t ui_attach_submenu;
tui_menu_t ui_datasette_submenu;
tui_menu_t ui_datasette_settings_submenu;
tui_menu_t ui_detach_submenu;
tui_menu_t ui_info_submenu;
tui_menu_t ui_main_menu;
tui_menu_t ui_quit_submenu;
tui_menu_t ui_reset_submenu;
tui_menu_t ui_jamaction_submenu;
tui_menu_t ui_rom_submenu;
tui_menu_t ui_screenshot_submenu;
tui_menu_t ui_settings_submenu;
tui_menu_t ui_snapshot_submenu;
tui_menu_t ui_special_submenu;
tui_menu_t ui_video_submenu;

#ifdef HAVE_NETWORK
tui_menu_t ui_netplay_submenu;
#endif

/* ------------------------------------------------------------------------ */

static TUI_MENU_CALLBACK(attach_tape_callback)
{
    const char *s;

    if (been_activated) {
        char *directory, *default_item;
        char *name, *file;

        s = tape_get_file_name();
        util_fname_split(s, &directory, &default_item);

        name = tui_file_selector("Attach a tape image", directory, "*.t64;*.tap;*.t6z;*.taz;*.zip;*.gz;*.lzh", default_item, tapecontents_read, &file, NULL);

        if (file != NULL) {
            if (autostart_tape(name, file, 0, AUTOSTART_MODE_RUN) < 0) {
                tui_error("Cannot autostart tape image.");
            } else {
                *behavior = TUI_MENU_BEH_RESUME;
            }
            lib_free(file);
        } else if (name != NULL && (s == NULL || strcasecmp(s, name) != 0) && tape_image_attach(1, name) < 0) {
            tui_error("Invalid tape image.");
        }
        ui_update_menus();
        lib_free(directory);
        lib_free(default_item);
        lib_free(name);
    }

    s = tape_get_file_name();
    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static TUI_MENU_CALLBACK(datasette_callback)
{
    if (been_activated) {
        datasette_control((int)param);
    }
    return NULL;
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

static TUI_MENU_CALLBACK(detach_disk_callback)
{
    const char *s;

    if (been_activated) {
        file_system_detach_disk((int)param);
        ui_update_menus();
    }

    s = file_system_get_disk_name((unsigned int)param);
    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static TUI_MENU_CALLBACK(detach_tape_callback)
{
    const char *s;

    if (been_activated) {
        tape_image_detach(1);
        ui_update_menus();
    }

    s = tape_get_file_name();
    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

/* ------------------------------------------------------------------------ */

static TUI_MENU_CALLBACK(change_workdir_callback)
{
    char s[256];

    if (!been_activated) {
        return NULL;
    }

    *s = '\0';

    if (tui_input_string("Change working directory", "New directory:", s, 255) == -1) {
        return NULL;
    }

    util_remove_spaces(s);
    if (*s == '\0') {
        return NULL;
    }

    if (ioutil_chdir(s) == -1) {
        tui_error("Invalid directory.");
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(resolution_submenu_callback)
{
    int mode;

    resources_get_int("VGAMode", &mode);
    return vga_modes[mode].description;
}

TUI_MENU_DEFINE_RADIO(VGAMode)

static TUI_MENU_CALLBACK(refresh_rate_submenu_callback)
{
    int v;
    static char s[256];

    resources_get_int("RefreshRate", &v);

    if (v == 0) {
        return "Auto";
    } else {
        sprintf(s, "1/%d", v);
        return s;
    }
}

TUI_MENU_DEFINE_RADIO(RefreshRate)

#ifndef USE_MIDAS_SOUND
TUI_MENU_DEFINE_TOGGLE(TripleBuffering)
#endif

/* ------------------------------------------------------------------------- */
/* Datasette settings */

TUI_MENU_DEFINE_TOGGLE(DatasetteResetWithCPU)
TUI_MENU_DEFINE_RADIO(DatasetteSpeedTuning)
TUI_MENU_DEFINE_RADIO(DatasetteZeroGapDelay)

static TUI_MENU_CALLBACK(datasette_speedtuning_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("DatasetteSpeedTuning", &value);
    sprintf(s, "%d",value);
    return s;
}

static TUI_MENU_CALLBACK(datasette_zerogapdelay_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("DatasetteZeroGapDelay", &value);
    sprintf(s, "%8d",value);
    return s;
}

static tui_menu_item_def_t datasette_speedtuning_submenu[] = {
    { "_0 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)0, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_1 cycle",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)1, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)2, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)3, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_4 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)4, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_5 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)5, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_6 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)6, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_7 cycles",
      NULL,
      radio_DatasetteSpeedTuning_callback, (void *)7, 5,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t datasette_zerogapdelay_submenu[] = {
    { "_1:   1000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)1000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_2:   2000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)2000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3:   5000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)5000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_4:  10000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)10000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_5:  20000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)20000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_6:  50000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)50000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_7: 100000 cycles",
      NULL,
      radio_DatasetteZeroGapDelay_callback, (void *)100000, 8,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(ui_set_tape_wobble_callback)
{
    if (been_activated) {
        int current_wobble, value;
        char buf[10];

        resources_get_int("DatasetteTapeWobble", &current_wobble);
        sprintf(buf, "%d", current_wobble);

        if (tui_input_string("Random tape wobble", "Enter the random tape wobble:", buf, 10) == 0) {
            value = atoi(buf);
            resources_set_int("DatasetteTapeWobble", value);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static tui_menu_item_def_t datasette_settings_submenu[] = {
    { "_Reset Datasette with CPU:",
      "Reset the datasette when main CPU resets",
      toggle_DatasetteResetWithCPU_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cycles _delay each trigger:",
      "Finetuning for the speed of datasette",
      datasette_speedtuning_submenu_callback, NULL, 2,
      TUI_MENU_BEH_CONTINUE, datasette_speedtuning_submenu,
      "Delay each trigger:" },
    { "A _zero in the tap is:",
      "How many cyles delay are represented by a zero in the tap?",
      datasette_zerogapdelay_submenu_callback, NULL, 8,
      TUI_MENU_BEH_CONTINUE, datasette_zerogapdelay_submenu,
      "A zero in tap is..." },
    { "Random tape wobble",
      "Set random tape wobble",
      ui_set_tape_wobble_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

/* CPU JAM actions */

TUI_MENU_DEFINE_RADIO(JAMAction)

static tui_menu_item_def_t cpu_jam_actions_submenu[] = {
    { "_Ask", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_DIALOG, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Continue", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_CONTINUE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Start _Monitor", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_MONITOR, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Reset", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_RESET, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Hard reset", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_HARD_RESET, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Quit emulator", NULL, radio_JAMAction_callback,
      (void *)MACHINE_JAM_ACTION_QUIT, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(set_romset_callback)
{
    if (been_activated) {
        if (machine_romset_file_load((char *)param) < 0) {
            tui_error("Could not load ROM set '%s'", param);
        } else {
            tui_message("ROM set loaded successfully.");
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(load_romset_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load custom ROM set definition", NULL, "*.vrs", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (machine_romset_file_load(name) < 0) {
                ui_error("Could not load ROM set file '%s'", name);
            }
            lib_free(name);
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(dump_romset_callback)
{
    if (been_activated) {
        char name[PATH_MAX];
        memset(name, 0, PATH_MAX);

        tui_input_string("Dump ROM set definition", "Enter file name:", name, PATH_MAX);
        util_remove_spaces(name);

        machine_romset_file_save(name);
    }
    return NULL;
}

static tui_menu_item_def_t rom_submenu[] = {
    { "_Default ROM set",
      "Load default ROM set file",
      set_romset_callback, "default.vrs", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "C_ustom ROM set",
      "Load custom ROM set from a *.vrs file",
      load_romset_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Save ROM set",
      "Save ROM set definition to a *.vrs file",
      dump_romset_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(save_settings_callback)
{
    if (been_activated) {
        if (resources_save(NULL) < 0) {
            tui_error("Cannot save settings.");
        } else {
            tui_message("Settings saved successfully.");
        }
    }

    return NULL;
}

static TUI_MENU_CALLBACK(load_settings_callback)
{
    if (been_activated) {
        if (resources_load(NULL) < 0) {
            tui_error("Cannot load settings.");
        } else {
            tui_message("Settings loaded successfully.");
            ui_update_menus();
        }
    }

    return NULL;
}

static TUI_MENU_CALLBACK(restore_default_settings_callback)
{
    if (been_activated) {
        resources_set_defaults();
        tui_message("Default settings restored.");
        ui_update_menus();
    }

    return NULL;
}

static TUI_MENU_CALLBACK(quit_callback)
{
    if (been_activated) {
        _setcursortype(_NORMALCURSOR);
        normvideo();
        clrscr();
        file_system_detach_disk(-1);
        exit(0);
    }

    return NULL;
}

static tui_menu_item_def_t quit_submenu[] = {
    { "_Not really!",
      "Go back to the menu",
      NULL, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Yes, exit emulator",
      "Leave the emulator completely",
      quit_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------ */

static TUI_MENU_CALLBACK(monitor_callback)
{
    if (been_activated) {
        monitor_startup(e_default_space);
    }

    return NULL;
}

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(soft_reset_callback)
{
    if (been_activated) {
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;

    return NULL;
}

static TUI_MENU_CALLBACK(hard_reset_callback)
{
    if (been_activated) {
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
    }

    *become_default = 0;

    return NULL;
}

static TUI_MENU_CALLBACK(reset_drive_callback)
{
    if (been_activated) {
        drive_cpu_trigger_reset((unsigned int)param);
    }

    *become_default = 0;

    return NULL;
}

static tui_menu_item_def_t reset_submenu[] = {
    { "_Not Really!", "Go back to the menu",
      NULL, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Do a _Soft Reset", "Do a soft reset without resetting the memory",
      soft_reset_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Do a _Hard Reset", "Clear memory and reset as after a power-up",
      hard_reset_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Reset drive #_8", "Reset drive #8 separately",
      reset_drive_callback, (void *)0, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Reset drive #_9", "Reset drive #9 separately",
      reset_drive_callback, (void *)1, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Reset drive #1_0", "Reset drive #10 separately",
      reset_drive_callback, (void *)2, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "Reset drive #_11", "Reset drive #11 separately",
      reset_drive_callback, (void *)3, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

static char *center_text_70(char *text)
{
    char *retval = NULL;
    char *spaces;
    int space_size = (70 - strlen(text)) / 2;

    spaces = lib_malloc(space_size + 1);
    memset(spaces, 32, space_size);
    spaces[space_size] = 0;

    retval = util_concat(spaces, text, NULL);
    lib_free(spaces);

    return retval;
}

static char *authors_start[] = {
    "V I C E",
#ifdef USE_SVN_REVISION
    "Version " VERSION "rev " VICE_SVN_REV_STRING,
#else
    "Version " VERSION,
#endif
#ifdef UNSTABLE
    "(unstable)",
#endif
    NULL
};

static char *authors_end[] = {
    "Official VICE homepage:",
    "http://vice-emu.sourceforge.net/",
    NULL
};

#ifdef UNSTABLE
static char *authors_unstable[] = {
    "WARNING: this is an *unstable* test version!",
    "Please check out the homepage for the latest updates.",
    NULL
};
#endif

/* this is getting too big for a static dialog, so it's been
   turned into a scrolling text view. */
static TUI_MENU_CALLBACK(show_copyright_callback)
{
    if (been_activated) {
        char *str_list;
        char *tmp1, tmp2;
        int i;
        
        str_list = lib_stralloc("\n");
        for (i = 0; authors_start[i]; i++) {
            tmp1 = center_text_70(authors_start[i]);
            tmp2 = util_concat(str_list, tmp1, "\n", NULL);
            lib_free(tmp1);
            lib_free(str_list);
            str_list = tmp2;
        }
        tmp1 = util_concat(str_list, "\n", NULL);
        lib_free(str_list);
        str_list = tmp1;
        for (i = 0; core_team[i].name; i++) {
            tmp1 = util_concat("Copyright (c) ", core_team[i].years, " ", core_team[i].name, NULL);
            tmp2 = center_text_70(tmp1);
            lib_free(tmp1);
            tmp1 = util_concat(str_list, tmp2, "\n", NULL);
            lib_free(str_list);
            str_list = tmp1;
        }
        tmp1 = util_concat(str_list, "\n", NULL);
        lib_free(str_list);
        str_list = tmp1;
        for (i = 0; authors_end[i]; i++) {
            tmp1 = center_text_70(authors_end[i]);
            tmp2 = util_concat(str_list, tmp1, "\n", NULL);
            lib_free(tmp1);
            lib_free(str_list);
            str_list = tmp2;
        }
#ifdef UNSTABLE
        tmp1 = util_concat(str_list, "\n", NULL);
        lib_free(str_list);
        str_list = tmp1;
        for (i = 0; authors_unstable[i]; i++) {
            tmp1 = center_text_70(authors_unstable[i]);
            tmp2 = util_concat(str_list, tmp1, "\n", NULL);
            lib_free(tmp1);
            lib_free(str_list);
            str_list = tmp2;
        }
#endif
        tmp1 = util_concat(str_list, "\n", NULL);
        lib_free(str_list);
        str_list = tmp1;
        tui_view_text(70, 20, NULL, str_list);
        lib_free(str_list);
    }
    return NULL;
}

static TUI_MENU_CALLBACK(show_features_callback)
{
    if (been_activated) {
        feature_list_t *list;
        char *str, *lstr;
        unsigned int len = 0;

        list = vice_get_feature_list();
        while (list->symbol) {
            len += strlen(list->descr) + strlen(list->symbol) + (15);
            ++list;
        }
        str = lib_malloc(len);
        lstr = str;
        list = vice_get_feature_list();
        while (list->symbol) {
            sprintf(lstr, "%s\n%s\n%s\n\n", list->isdefined ? "yes " : "no  ", list->descr, list->symbol);
            lstr += strlen(lstr);
            ++list;
        }
        tui_view_text(70, 20, NULL, str);
        lib_free(str);
    }
    return NULL;
}

static TUI_MENU_CALLBACK(show_info_callback)
{
    if (been_activated) {
        tui_view_text(70, 20, NULL, (const char *)param);
    }
    return NULL;
}

static tui_menu_item_def_t info_submenu[] = {
    { "_Copyright",
      "VICE copyright information",
      show_copyright_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Who made what?",
      "VICE contributors",
      show_info_callback, (void *)info_contrib_text, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_License",
      "VICE license (GNU General Public License)",
      show_info_callback, (void *)info_license_text, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_No warranty!",
      "VICE is distributed WITHOUT ANY WARRANTY!",
      show_info_callback, (void *)info_warranty_text, 0,
      TUI_MENU_BEH_CONTINUE },
    { "Compile time features",
      "VICE compile time features",
      show_features_callback, NULL, 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

/* This is a bit of a hack, but I prefer this way instead of writing 1,000
   menu entries...  */
static void create_ui_video_submenu(void)
{
    static tui_menu_t refresh_rate_submenu, vga_mode_submenu;
    int i;

    refresh_rate_submenu = tui_menu_create("Refresh", 1);

    for (i = 1; i <= 10; i++) {
        char *label, *desc;
        if (i != 10) {
            label = lib_msprintf("1/_%d", i);
        } else {
            label = lib_stralloc("1/1_0");
        }
        if (i == 1) {
            desc = lib_msprintf("Set refresh rate to 1/%d (update every frame)", i);
        } else {
            desc = lib_msprintf("Set refresh rate to 1/%d (update once every %d frames)", i, i);
        }
        tui_menu_add_item(refresh_rate_submenu, label,
                          desc,
                          radio_RefreshRate_callback,
                          (void *)i, 0,
                          TUI_MENU_BEH_CLOSE);
        lib_free(label);
        lib_free(desc);
    }

    tui_menu_add_separator(refresh_rate_submenu);
    tui_menu_add_item(refresh_rate_submenu, "_Automatic",
                      "Let the emulator select an appropriate refresh rate automagically",
                      radio_RefreshRate_callback,
                      NULL, 0,
                      TUI_MENU_BEH_CLOSE);

    vga_mode_submenu = tui_menu_create("VGA Resolution", 1);

    for (i = 0; i < NUM_VGA_MODES; i++) {
        char *s1, *s2;

        /* FIXME: hotkeys work only for less than 11 elements. */
        s1 = lib_msprintf("Mode %s%d: %s",(i<10?" _":""), i, vga_modes[i].description);
        s2 = lib_msprintf("Set VGA resolution to %s", vga_modes[i].description);
        tui_menu_add_item(vga_mode_submenu, s1,
                          s2,
                          radio_VGAMode_callback,
                          (void *)i, 0,
                          TUI_MENU_BEH_CLOSE);
        lib_free(s1);
        lib_free(s2);
    }

    ui_video_submenu = tui_menu_create("Video Settings", 1);

    tui_menu_add_submenu(ui_video_submenu, "_VGA Resolution:",
                         "Choose screen resolution for video emulation",
                         vga_mode_submenu,
                         resolution_submenu_callback,
                         NULL, 15);

    tui_menu_add_submenu(ui_video_submenu, "_Refresh Rate:",
                         "Choose frequency of screen refresh",
                         refresh_rate_submenu,
                         refresh_rate_submenu_callback,
                         NULL, 4);

#ifndef USE_MIDAS_SOUND
    tui_menu_add_item(ui_video_submenu, "_Triple Buffering:",
                      "Enable triple buffering for smoother animations (when available)",
                      toggle_TripleBuffering_callback,
                      NULL, 3,
                      TUI_MENU_BEH_CONTINUE);
#endif

    tui_menu_add_separator(ui_video_submenu);

    ui_screenshot_submenu = tui_menu_create("Screenshot Commands", 1);
    switch (machine_class) {
        default:
            tui_menu_add(ui_screenshot_submenu, ui_screenshot_menu_def_vic_vicii_vdc);
            break;
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM6x0:
            tui_menu_add(ui_screenshot_submenu, ui_screenshot_menu_def_crtc);
            break;
        case VICE_MACHINE_PLUS4:
            tui_menu_add(ui_screenshot_submenu, ui_screenshot_menu_def_ted);
            break;
    }

    tui_menu_add_submenu(ui_video_submenu, "_Screenshot Commands...",
                         "Commands for saving screenshots",
                         ui_screenshot_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

#ifdef HAVE_FFMPEG
    uiffmpeg_init(ui_screenshot_submenu);
#endif
}

/* ------------------------------------------------------------------------- */

TUI_MENU_DEFINE_TOGGLE(VirtualDevices)

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(speed_submenu_callback)
{
    static char s[1024];
    int value;

    resources_get_int("Speed", &value);
    if (value) {
        sprintf(s, "%d%%", value);
        return s;
    } else {
        return "None";
    }
}

static TUI_MENU_CALLBACK(speed_callback)
{
    if (been_activated) {
        int value = (int)param;

        if (value < 0) {
            char buf[25];

            *buf = '\0';
            if (tui_input_string("Maximum Speed", "Enter maximum speed (%%):", buf, 25) == 0) {
                value = atoi(buf);
                if (value > 1000) {
                    value = 1000;
                } else if (value < 0) {
                    value = 0;
                }
            } else {
                return NULL;
            }
        }

        resources_set_int("Speed", value);
    }
    return NULL;
}

TUI_MENU_DEFINE_TOGGLE(WarpMode)
TUI_MENU_DEFINE_TOGGLE(UseLeds)

static TUI_MENU_CALLBACK(toggle_ShowStatusbar_callback)
{
    int value;

    resources_get_int("ShowStatusbar", &value);

    if (been_activated) {
        if (value == STATUSBAR_MODE_OFF) {
            value = STATUSBAR_MODE_ON;
        } else if (value == STATUSBAR_MODE_ON) {
            value = STATUSBAR_MODE_AUTO;
        } else {
            value = STATUSBAR_MODE_OFF;
        }

        resources_set_int("ShowStatusbar", value);
    }

    switch (value) {
        case STATUSBAR_MODE_OFF:
            return "Off";
        case STATUSBAR_MODE_ON:
            return "On";
        case STATUSBAR_MODE_AUTO:
            return "Auto";
        default:
            return "(Custom)";
    }
}


static void create_speed_limit_submenu(void)
{
    int i;
    int speed[4] = { 100, 50, 20, 10 };
    char *s1, *s2;
    static tui_menu_t speed_submenu;

    ui_special_submenu = tui_menu_create("Other Settings", 1);

    speed_submenu = tui_menu_create("Speed Limit", 1);
    for (i = 0; i < 4; i++) {
        if (speed[i] == 100) {
            s1 = lib_msprintf("Limit speed to the one of the real %s", machine_name);
        } else {
            s1 = lib_msprintf("Limit speed to %d%% of the real %s", speed[i], machine_name);
        }
        s2 = lib_msprintf("_%d%%", speed[i]);
        tui_menu_add_item(speed_submenu, s2,
                          s1,
                          speed_callback,
                          (void *)speed[i], 5,
                          TUI_MENU_BEH_CLOSE);
        lib_free(s1);
        lib_free(s2);
    }
    tui_menu_add_item(speed_submenu, "_No Limit",
                      "Run the emulator as fast as possible",
                      speed_callback,
                      (void *)0, 5,
                      TUI_MENU_BEH_CLOSE);
    tui_menu_add_separator(speed_submenu);
    tui_menu_add_item(speed_submenu, "_Custom...",
                      "Specify a custom relative speed value",
                       speed_callback,
                       (void *)-1, 5,
                       TUI_MENU_BEH_CLOSE);

    tui_menu_add_submenu(ui_special_submenu, "_Speed Limit:",
                         "Specify a custom speed limit",
                         speed_submenu, speed_submenu_callback,
                         NULL, 5);
}

static void create_special_submenu(int has_serial_traps)
{
    create_speed_limit_submenu();

    tui_menu_add_item(ui_special_submenu, "Enable _Warp Mode:",
                      "Make the emulator run as fast as possible skipping lots of frames",
                      toggle_WarpMode_callback,
                      NULL, 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_separator(ui_special_submenu);
    uiperipherial_init(ui_special_submenu);
    uiprinter_init(ui_special_submenu);

    if (has_serial_traps)
        tui_menu_add_item(ui_special_submenu, "Enable virtual device _traps:",
                          "Enable virtual device traps for fast tape and drive emulation",
                          toggle_VirtualDevices_callback,
                          NULL, 4,
                          TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(ui_special_submenu, "_Change Working Directory...",
                      "Change the current working directory",
                      change_workdir_callback,
                      NULL, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_separator(ui_special_submenu);
    tui_menu_add_item(ui_special_submenu,
                      "Use _Keyboard LEDs:",
                      "Use PC keyboard LEDs for the disk drive and Warp Mode",
                      toggle_UseLeds_callback,
                      NULL, 4,
                      TUI_MENU_BEH_CONTINUE);
    tui_menu_add_item(ui_special_submenu,
                      "Show Status_bar:",
                      "Statusbar to display Speed, Tape and Drive properties; toggle with ALT-F5",
                      toggle_ShowStatusbar_callback,
                      NULL, 10,
                      TUI_MENU_BEH_CONTINUE);
}

/* ------------------------------------------------------------------------- */

void ui_create_main_menu(int has_tape, int has_drive, int has_serial_traps, int number_joysticks, int has_datasette, const tui_menu_item_def_t *d)
{
    int port1 = (number_joysticks & 16) >> 4;
    int port2 = (number_joysticks & 8) >> 3;
    int port3 = (number_joysticks & 4) >> 2;
    int port4 = (number_joysticks & 2) >> 1;
    int port5 = number_joysticks & 1;

    /* Main menu. */
    ui_main_menu = tui_menu_create(NULL, 1);

    ui_attach_submenu = tui_menu_create("Attach Images", 1);
    tui_menu_add(ui_attach_submenu, ui_attach_menu_def);

    if (has_tape) {
        tui_menu_add_separator(ui_attach_submenu);
        tui_menu_add_item(ui_attach_submenu, "_Tape:",
                          "Attach tape image for cassette player (device #1)",
                          attach_tape_callback,
                          NULL, 30,
                          TUI_MENU_BEH_CONTINUE);
        tui_menu_add_item(ui_attach_submenu, "Autostart Ta_pe",
                          "Reset the emulator and run the first program on the tape image",
                          autostart_callback,
                          (void *)1, 0,
                          TUI_MENU_BEH_RESUME);
    }

    ui_detach_submenu = tui_menu_create("Detach Images", 1);
    tui_menu_add_item(ui_detach_submenu, "Drive #_8:",
                      "Remove disk from disk drive #8",
                      detach_disk_callback,
                      (void *)8, 30,
                      TUI_MENU_BEH_CONTINUE);
    tui_menu_add_item(ui_detach_submenu,"Drive #_9:",
                      "Remove disk from disk drive #9",
                      detach_disk_callback,
                      (void *)9, 30,
                      TUI_MENU_BEH_CONTINUE);
    tui_menu_add_item(ui_detach_submenu, "Drive #1_0:",
                      "Remove disk from disk drive #10",
                      detach_disk_callback,
                      (void *)10, 30,
                      TUI_MENU_BEH_CONTINUE);
    tui_menu_add_item(ui_detach_submenu, "Drive #1_1:",
                      "Remove disk from disk drive #11",
                      detach_disk_callback,
                      (void *)11, 30,
                      TUI_MENU_BEH_CONTINUE);

    if (has_tape) {
        tui_menu_add_separator(ui_detach_submenu);
        tui_menu_add_item(ui_detach_submenu, "_Tape:",
                          "Remove tape from cassette player (device #1)",
                          detach_tape_callback,
                          NULL, 30,
                          TUI_MENU_BEH_CONTINUE);
    }

    tui_menu_add_submenu(ui_main_menu, "_Attach Image...",
                         "Insert virtual disks, tapes or cartridges in the emulated machine",
                         ui_attach_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
    tui_menu_add_submenu(ui_main_menu, "_Detach Image...",
                         "Remove virtual disks, tapes or cartridges from the emulated machine",
                         ui_detach_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    if (has_datasette) {
        ui_datasette_submenu = tui_menu_create("Datasette control", 1);
        tui_menu_add_item(ui_datasette_submenu, "S_top",
                          "Press the STOP key of the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_STOP, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "_Start",
                          "Press the START key of the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_START, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "_Forward",
                          "Press the FORWARD key of the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_FORWARD, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "_Rewind",
                          "Press the REWIND key of the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_REWIND, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "Re_cord",
                          "Press the RECORD key of the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_RECORD, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "R_eset",
                          "Rewind the tape and stop the datasette",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_RESET, 0,
                          TUI_MENU_BEH_RESUME);
        tui_menu_add_item(ui_datasette_submenu, "Reset C_ounter",
                          "Set the datasette counter to '000'",
                          datasette_callback,
                          (void *)DATASETTE_CONTROL_RESET_COUNTER, 0,
                          TUI_MENU_BEH_RESUME);

        tui_menu_add_submenu(ui_main_menu, "Datassett_e Control...",
                             "Press some buttons on the emulated datasette",
                             ui_datasette_submenu,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);
    }

    tui_menu_add_separator(ui_main_menu);

    create_ui_video_submenu();
    tui_menu_add_submenu(ui_main_menu, "_Video Settings...",
                         "Screen parameters",
                         ui_video_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    if (has_drive) {
        uidrive_init(ui_main_menu, d);
    }

    if (has_datasette) {
        ui_datasette_settings_submenu = tui_menu_create("Datasette Settings", 1);
        tui_menu_add(ui_datasette_settings_submenu,datasette_settings_submenu);
        tui_menu_add_submenu(ui_main_menu, "Da_tasette Settings...",
                             "Datasette settings",
                             ui_datasette_settings_submenu,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);
    }

    uikeymap_init(ui_main_menu);

    uisound_init(ui_main_menu);
    uisampler_init(ui_main_menu);

    uijoyport_init(ui_main_menu, port1, port2, port3, port4, port5);

    ui_rom_submenu = tui_menu_create("Firmware ROM Settings", 1);
    tui_menu_add(ui_rom_submenu, rom_submenu);
    tui_menu_add_submenu(ui_main_menu, "Firm_ware ROM Settings...",
                         "Firmware ROMs the emulator is using",
                         ui_rom_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    create_special_submenu(has_serial_traps);

    tui_menu_add_submenu(ui_main_menu, "_Other Settings...",
                         "Extra emulation features",
                         ui_special_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    tui_menu_add_separator(ui_main_menu);

    ui_snapshot_submenu = tui_menu_create("Freeze Commands", 1);
    tui_menu_add(ui_snapshot_submenu, ui_snapshot_menu_def);

    tui_menu_add_submenu(ui_main_menu, "_Freeze Commands...",
                         "Commands for loading/saving the machine state",
                         ui_snapshot_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

#ifdef HAVE_NETWORK
    ui_netplay_submenu = tui_menu_create("Netplay Commands", 1);
    tui_menu_add(ui_netplay_submenu, ui_netplay_menu_def);

    tui_menu_add_submenu(ui_main_menu, "_Netplay Commands...",
                         "Commands for netplay between computers",
                         ui_netplay_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
#endif

    tui_menu_add_separator(ui_main_menu);

    ui_settings_submenu = tui_menu_create("Configuration Commands", 1);

    tui_menu_add_item(ui_settings_submenu, "_Write Configuration",
                      "Save current settings as default for next session",
                      save_settings_callback,
                      NULL, 0,
                      TUI_MENU_BEH_CLOSE);
    tui_menu_add_item(ui_settings_submenu, "_Load Configuration",
                      "Load saved settings from previous session",
                      load_settings_callback,
                      NULL, 0,
                      TUI_MENU_BEH_CLOSE);
    tui_menu_add_item(ui_settings_submenu, "Restore _Factory Defaults",
                      "Set default settings",
                      restore_default_settings_callback,
                      NULL, 0,
                      TUI_MENU_BEH_CLOSE);

    tui_menu_add_submenu(ui_main_menu, "_Configuration Commands...",
                         "Commands to save, retrieve and restore settings",
                         ui_settings_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    tui_menu_add_separator(ui_main_menu);

    tui_menu_add_item(ui_main_menu, "_Monitor",
                      "Enter the built-in machine language monitor",
                      monitor_callback,
                      NULL, 0,
                      TUI_MENU_BEH_RESUME);

    ui_jamaction_submenu = tui_menu_create("Default CPU JAM action", 1);
    tui_menu_add(ui_jamaction_submenu, cpu_jam_actions_submenu);
    tui_menu_add_submenu(ui_main_menu, "_JAM action ",
                         "Default CPU JAM action",
                         ui_jamaction_submenu,
                         NULL, NULL, 0);

    ui_reset_submenu = tui_menu_create("Reset?", 1);
    tui_menu_add(ui_reset_submenu, reset_submenu);
    tui_menu_add_submenu(ui_main_menu, "_Reset ",
                         "Reset the machine",
                         ui_reset_submenu,
                         NULL, NULL, 0);

    ui_quit_submenu = tui_menu_create("Quit", 1);
    tui_menu_add(ui_quit_submenu, quit_submenu);
    tui_menu_add_submenu(ui_main_menu, "_Quit",
                         "Quit emulator",
                         ui_quit_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
    tui_menu_add_separator(ui_main_menu);

    ui_info_submenu = tui_menu_create("Info", 1);
    tui_menu_add(ui_info_submenu, info_submenu);
    tui_menu_add_submenu(ui_main_menu, "VICE _Info...",
                         "VICE is Free Software distributed under the GNU General Public License!",
                         ui_info_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
