/*
 * menu_snapshot.c - Implementation of the snapshot settings menu for the SDL UI.
 *
 * Written by
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_snapshot.h"
#include "resources.h"
#include "snapshot.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "util.h"
#include "vice-event.h"

static int save_disks = 1;
static int save_roms = 0;

UI_MENU_DEFINE_RADIO(EventStartMode)

static UI_MENU_CALLBACK(toggle_save_disk_images_callback)
{
    if (activated) {
        save_disks = !save_disks;
    } else {
        if (save_disks) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(toggle_save_rom_images_callback)
{
    if (activated) {
        save_roms = !save_roms;
    } else {
        if (save_roms) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(save_snapshot_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose snapshot file to save", FILEREQ_MODE_SAVE_FILE);
        if (name != NULL) {
            util_add_extension(&name, "vsf");
            if (machine_write_snapshot(name, save_roms, save_disks, 0) < 0) {
                snapshot_display_error();
            }
            lib_free(name);
        }
    }
    return NULL;
}

#if 0
/* FIXME */
static UI_MENU_CALLBACK(save_snapshot_slot_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_slot_selection_dialog("Choose snapshot slot to save", SLOTREQ_MODE_SAVE_SLOT);
        if (name != NULL) {
            util_add_extension(&name, "vsf");
            if (machine_write_snapshot(name, save_roms, save_disks, 0) < 0) {
                snapshot_display_error();
            }
            lib_free(name);
        }
    }
    return NULL;
}
#endif

static UI_MENU_CALLBACK(quickload_snapshot_callback)
{
    if (activated) {
        if (machine_read_snapshot("snapshot.vsf", 0) < 0) {
           snapshot_display_error();
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(quicksave_snapshot_callback)
{
    if (activated) {
        if (machine_write_snapshot("snapshot.vsf", save_roms, save_disks, 0) < 0) {
            snapshot_display_error();
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(start_stop_recording_history_callback)
{
    int recording_new;

    recording_new = (event_record_active() ? 0 : 1);
    if (activated) {
        if (recording_new) {
            event_record_start();
        } else {
            event_record_stop();
        }
        return sdl_menu_text_exit_ui;
    } else {
        if (!recording_new) {
            return "(recording)";
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(start_stop_playback_history_callback)
{
    int playback_new;

    playback_new = (event_playback_active() ? 0 : 1);
    if (activated) {
        if (playback_new) {
            event_playback_start();
        } else {
            event_playback_stop();
        }
        return sdl_menu_text_exit_ui;
    } else {
        if (!playback_new) {
            return "(playing)";
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(load_snapshot_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose snapshot file to load", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (machine_read_snapshot(name, 0) < 0) {
                snapshot_display_error();
            }
            lib_free(name);
        }
    }
    return NULL;
}

#if 0
/* FIXME */
static UI_MENU_CALLBACK(load_snapshot_slot_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_slot_selection_dialog("Choose snapshot slot to load", SLOTREQ_MODE_CHOOSE_SLOT);
        if (name != NULL) {
            if (machine_read_snapshot(name, 0) < 0) {
                snapshot_display_error();
            }
            lib_free(name);
        }
    }
    return NULL;
}
#endif

static UI_MENU_CALLBACK(set_milestone_callback)
{
    if (activated) {
        event_record_set_milestone();
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(return_to_milestone_callback)
{
    if (activated) {
        event_record_reset_milestone();
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(select_history_files_callback)
{
    char *name;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Select event history directory", FILEREQ_MODE_CHOOSE_DIR);
        if (name != NULL) {
            resources_set_string("EventSnapshotDir", name);
        }
    }
    return NULL;
}

static const ui_menu_entry_t save_snapshot_menu[] = {
    { "Save currently attached disk images",
      MENU_ENTRY_OTHER,
      toggle_save_disk_images_callback,
      NULL },
    { "Save currently attached ROM images",
      MENU_ENTRY_OTHER,
      toggle_save_rom_images_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Select filename and save snapshot",
      MENU_ENTRY_DIALOG,
      save_snapshot_callback,
      NULL },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t snapshot_menu[] = {
    { "Load snapshot image",
      MENU_ENTRY_DIALOG,
      load_snapshot_callback,
      NULL },
    { "Save snapshot image",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)save_snapshot_menu },
    { "Quickload snapshot.vsf",
      MENU_ENTRY_DIALOG,
      quickload_snapshot_callback,
      NULL },
    { "Quicksave snapshot.vsf",
      MENU_ENTRY_DIALOG,
      quicksave_snapshot_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Start/stop recording history",
      MENU_ENTRY_OTHER,
      start_stop_recording_history_callback,
      NULL },
    { "Start/stop playback history",
      MENU_ENTRY_OTHER,
      start_stop_playback_history_callback,
      NULL },
    { "Set recording milestone",
      MENU_ENTRY_OTHER,
      set_milestone_callback,
      NULL },
    { "Return to milestone",
      MENU_ENTRY_OTHER,
      return_to_milestone_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Record start mode"),
    { "Save new snapshot",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_EventStartMode_callback,
      (ui_callback_data_t)EVENT_START_MODE_FILE_SAVE },
    { "Load existing snapshot",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_EventStartMode_callback,
      (ui_callback_data_t)EVENT_START_MODE_FILE_LOAD },
    { "Start with reset",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_EventStartMode_callback,
      (ui_callback_data_t)EVENT_START_MODE_RESET },
    { "Overwrite playback",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_EventStartMode_callback,
      (ui_callback_data_t)EVENT_START_MODE_PLAYBACK },
    SDL_MENU_ITEM_SEPARATOR,
    { "Select history files/directory",
      MENU_ENTRY_DIALOG,
      select_history_files_callback,
      NULL },
    SDL_MENU_LIST_END
};

#ifdef ANDROID_COMPILE
void loader_load_snapshot(char *name)
{
    if (machine_read_snapshot(name, 0) < 0) {
        snapshot_display_error();
    }
}

void loader_save_snapshot(char *name)
{
    if (machine_write_snapshot(name, save_roms, save_disks, 0) < 0) {
        snapshot_display_error();
    }
}
#endif
