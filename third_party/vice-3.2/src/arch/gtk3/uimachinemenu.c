/** \file   uimachinemenu.c
 * \brief   Native GTK3 menus for machine emulators (not vsid)
 *
 * \author  Marcus Sutton <loggedoubt@gmail.com>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES WarpMode        all
 * $VICERES KeySetEnable    -vsid
 * $VICERES Mouse           -vsid
 * $VICERES DtvBlitterLog   x64dtv
 * $VICERES DtvDMALog       x64dtv
 * $VICERES DtvFlashLog     x64dtv
 */

/*
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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "datasette.h"
#include "debug.h"
#include "machine.h"
#include "ui.h"
#include "uiabout.h"
#include "uicart.h"
#include "uicmdline.h"
#include "uicommands.h"
#include "uicompiletimefeatures.h"
#include "uidatasette.h"
#include "uidebug.h"
#include "uidiskattach.h"
#include "uiedit.h"
#include "uifliplist.h"
#include "uimachinemenu.h"
#include "uimedia.h"
#include "uimenu.h"
#include "uimonarch.h"
#include "uidiskcreate.h"
#include "uitapecreate.h"

#ifdef HAVE_NETWORK
# include "uinetplay.h"
# include "uinetplay_new.h"
#endif

#include "uisettings.h"
#include "uismartattach.h"
#include "uisnapshot.h"
#include "uitapeattach.h"

/*
 * The following are translation unit local so we can create functions that
 * modify menu contents or even functions that alter the top bar itself.
 */


/** \brief  Main menu bar widget
 *
 * Contains the submenus on the menu main bar
 *
 * This one lives until ui_exit() or thereabouts
 */
static GtkWidget *main_menu_bar = NULL;


/** \brief  File submenu
 */
static GtkWidget *file_submenu = NULL;


/** \brief  Edit submenu
 */
static GtkWidget *edit_submenu = NULL;


/** \brief  Snapshot submenu
 */
static GtkWidget *snapshot_submenu = NULL;


/** \brief  Settings submenu
 */
static GtkWidget *settings_submenu = NULL;


#ifdef DEBUG
/** \brief  Debug submenu, only available when --enable-debug was specified
 */
static GtkWidget *debug_submenu = NULL;
#endif


/** \brief  Help submenu
 */
static GtkWidget *help_submenu = NULL;


/** \brief  File->Detach disk submenu
 */
static ui_menu_item_t detach_submenu[] = {
    { "Drive #8", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive8", ui_disk_detach_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Drive #9", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive9", ui_disk_detach_callback, GINT_TO_POINTER(9),
        0, 0 },
    { "Drive #10", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive10", ui_disk_detach_callback, GINT_TO_POINTER(10),
        0, 0 },
    { "Drive #11", UI_MENU_TYPE_ITEM_ACTION,
        "detach-drive11", ui_disk_detach_callback, GINT_TO_POINTER(11),
        0, 0 },

    { "Detach all", UI_MENU_TYPE_ITEM_ACTION,
        "detach-all", ui_disk_detach_all_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  File->Attach disk submenu
 */
static ui_menu_item_t attach_submenu[] = {
    { "Drive #8", UI_MENU_TYPE_ITEM_ACTION,
        "attach-drive8", ui_disk_attach_callback, GINT_TO_POINTER(8),
        GDK_KEY_8, VICE_MOD_MASK },
    { "Drive #9", UI_MENU_TYPE_ITEM_ACTION,
        "attach-drive9", ui_disk_attach_callback, GINT_TO_POINTER(9),
        GDK_KEY_9, VICE_MOD_MASK },
    { "Drive #10", UI_MENU_TYPE_ITEM_ACTION,
        "attach-drive10", ui_disk_attach_callback, GINT_TO_POINTER(10),
        GDK_KEY_0, VICE_MOD_MASK },
    { "Drive #11", UI_MENU_TYPE_ITEM_ACTION,
        "attach-drive11", ui_disk_attach_callback, GINT_TO_POINTER(11),
        GDK_KEY_1, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};



/** \brief  File->Flip list submenu
 */
static ui_menu_item_t fliplist_submenu[] = {
    { "Add current image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-add", ui_fliplist_add_current_cb, GINT_TO_POINTER(8),
        GDK_KEY_I, VICE_MOD_MASK },
    { "Remove current image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
       "fliplist-remove", ui_fliplist_remove_current_cb, GINT_TO_POINTER(8),
        GDK_KEY_K, VICE_MOD_MASK },
    { "Attach next image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-next", ui_fliplist_next_cb, GINT_TO_POINTER(8),
        GDK_KEY_N, VICE_MOD_MASK },
    { "Attach previous image (Unit #8)", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-prev", ui_fliplist_prev_cb, GINT_TO_POINTER(8),
        GDK_KEY_N, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Load flip list file...", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-load", ui_fliplist_load_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Save flip list file...", UI_MENU_TYPE_ITEM_ACTION,
        "fliplist-save", ui_fliplist_save_callback, GINT_TO_POINTER(8),
        0, 0 },

    UI_MENU_TERMINATOR
};

/** \brief  File->Datasette control submenu
 */

static ui_menu_item_t datasette_control_submenu[] = {
    { "Stop", UI_MENU_TYPE_ITEM_ACTION,
        "tape-stop", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_STOP),
        0, 0 },
    { "Start", UI_MENU_TYPE_ITEM_ACTION,
        "tape-start", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_START),
        0, 0 },
    { "Forward", UI_MENU_TYPE_ITEM_ACTION,
        "tape-ff", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_FORWARD),
        0, 0 },
    { "Rewind", UI_MENU_TYPE_ITEM_ACTION,
        "tape-rew", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_REWIND),
        0, 0 },
    { "Record", UI_MENU_TYPE_ITEM_ACTION,
        "tape-record", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RECORD),
        0, 0 },
    { "Reset", UI_MENU_TYPE_ITEM_ACTION,
        "tape-reset", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RESET),
        0, 0 },
    { "Reset Counter", UI_MENU_TYPE_ITEM_ACTION,
        "tape-counter", ui_datasette_tape_action_cb, GINT_TO_POINTER(DATASETTE_CONTROL_RESET_COUNTER),
        0, 0 },
    UI_MENU_TERMINATOR
};

#if 0
/** \brief  'File->Cartridge attach' submenu
 */
static ui_menu_item_t cart_attach_submenu[] = {
    { "Smart attach cart image ... ", UI_MENU_TYPE_ITEM_ACTION,
        "attach-cart", (void *)uicart_smart_attach_dialog, NULL,
        GDK_KEY_C, VICE_MOD_MASK },
    UI_MENU_TERMINATOR
};
#endif


/** \brief  File->Reset submenu
 */
static ui_menu_item_t reset_submenu[] = {
    { "Soft reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-soft", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_SOFT),
        GDK_KEY_F9, VICE_MOD_MASK },
    { "Hard reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-hard", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_HARD),
        GDK_KEY_F12, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    { "Reset drive #8", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive8", ui_drive_reset_callback, GINT_TO_POINTER(8),
        0, 0 },
    { "Reset drive #9", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive9", ui_drive_reset_callback, GINT_TO_POINTER(9),
        0, 0 },
    { "Reset drive #10", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive10", ui_drive_reset_callback, GINT_TO_POINTER(10),
        0, 0 },
    { "Reset drive #11", UI_MENU_TYPE_ITEM_ACTION,
        "reset-drive11", ui_drive_reset_callback, GINT_TO_POINTER(11),
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - head section
 */
static ui_menu_item_t file_menu_head[] = {
    { "Smart attach disk/tape ...", UI_MENU_TYPE_ITEM_ACTION,
        "smart-attach", ui_smart_attach_callback, NULL,
        GDK_KEY_A, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    /* disk */
    { "Attach disk image", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, attach_submenu,
        GDK_KEY_8, VICE_MOD_MASK },
    { "Create and attach an empty disk image ...", UI_MENU_TYPE_ITEM_ACTION,
        "create-disk", uidiskcreate_dialog_show, GINT_TO_POINTER(8),
        0, 0 },
    { "Detach disk image", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, detach_submenu,
        0, 0 },
    { "Flip list", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, fliplist_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - tape section pointer
 *
 * Set by ...
 */
static ui_menu_item_t *file_menu_tape_section = NULL;


/** \brief  'File' menu - tape section
 */
static ui_menu_item_t file_menu_tape[] = {
    { "Attach tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        "attach-tape", ui_tape_attach_callback, NULL,
        GDK_KEY_T, VICE_MOD_MASK },
    { "Create and attach an empty tape image ...", UI_MENU_TYPE_ITEM_ACTION,
        "create-tape", uitapecreate_dialog_show, NULL,
        0, 0 },
    { "Detach tape image", UI_MENU_TYPE_ITEM_ACTION,
        "detach-tape", ui_tape_detach_callback, NULL,
        0, 0 },
    { "Datasette controls", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, datasette_control_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu - tail section
 */
static ui_menu_item_t file_menu_tail[] = {
    /* cart */
    { "Attach cartridge image ...", UI_MENU_TYPE_ITEM_ACTION,
        "cart-attach", uicart_show_dialog, NULL,
        GDK_KEY_C, VICE_MOD_MASK },
    { "Detach cartridge image(s)", UI_MENU_TYPE_ITEM_ACTION,
        "cart-detach", (void *)uicart_detach, NULL,
        0, 0 },
    { "Cartridge freeze", UI_MENU_TYPE_ITEM_ACTION,
        "cart-freeze", (void *)uicart_trigger_freeze, NULL,
        GDK_KEY_Z, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    /* monitor */
    { "Activate monitor", UI_MENU_TYPE_ITEM_ACTION,
        "monitor", ui_monitor_activate_callback, NULL,
#ifdef MACOSX_SUPPORT
        /* use Command-Option-M on Mac */
        GDK_KEY_M, VICE_MOD_MASK | GDK_MOD1_MASK
#else
        GDK_KEY_H, VICE_MOD_MASK
#endif
    },

    UI_MENU_SEPARATOR,

#ifdef HAVE_NETWORK
    { "Netplay ...", UI_MENU_TYPE_ITEM_ACTION,
#if 0
        "netplay", ui_netplay_dialog_new, NULL,
#else
        "netplay", ui_netplay_dialog, NULL,
#endif
        0, 0 },

    UI_MENU_SEPARATOR,
#endif

    { "Reset", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, reset_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Exit emulator", UI_MENU_TYPE_ITEM_ACTION,
        "exit", ui_close_callback, NULL,
        GDK_KEY_Q, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Edit' menu
 */
static ui_menu_item_t edit_menu[] = {
    { "Copy", UI_MENU_TYPE_ITEM_ACTION,
        "copy", (void *)ui_copy_callback, NULL,
        GDK_KEY_Delete, VICE_MOD_MASK },
    { "Paste", UI_MENU_TYPE_ITEM_ACTION,
        "paste", (void *)ui_paste_callback, NULL,
        GDK_KEY_Insert, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Snapshot' menu
 */
static ui_menu_item_t snapshot_menu[] = {
    { "Load snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-load", uisnapshot_open_file, NULL,
        GDK_KEY_L, VICE_MOD_MASK },
    { "Save snapshot image ...", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-save", uisnapshot_save_file, NULL,
        GDK_KEY_S, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    { "Quickload snapshot", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-quickload", uisnapshot_quickload_snapshot, NULL,
        GDK_KEY_F10, VICE_MOD_MASK },   /* Shortcut doesn't work in MATE, key
                                         is mapped to Maximize Window. Using
                                         the menu to active this item does
                                         work though -- compyx */
    { "Quicksave snapshot", UI_MENU_TYPE_ITEM_ACTION,
        "snapshot-quicksave", uisnapshot_quicksave_snapshot, NULL,
        GDK_KEY_F11, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,
#if 0
    { "Select history directory ...", UI_MENU_TYPE_ITEM_ACTION,
        "history-select-dir", uisnapshot_history_select_dir, "0:3",
        0, 0 },
#endif
    { "Start recording events", UI_MENU_TYPE_ITEM_ACTION,
        "history-record-start", uisnapshot_history_record_start, NULL,
        0, 0 },
    { "Stop recording events", UI_MENU_TYPE_ITEM_ACTION,
        "history-record-stop", uisnapshot_history_record_stop, NULL,
        0, 0 },
    { "Start playing back events", UI_MENU_TYPE_ITEM_ACTION,
        "history-playback-start", uisnapshot_history_playback_start, NULL,
        0, 0 },
    { "Stop playing back events", UI_MENU_TYPE_ITEM_ACTION,
        "history-playback-stop", uisnapshot_history_playback_stop, NULL,
        0, 0 },
    { "Set recording milestone", UI_MENU_TYPE_ITEM_ACTION,
        "history-milestone-set", uisnapshot_history_milestone_set, NULL,
        GDK_KEY_E, VICE_MOD_MASK },
    { "Return to milestone", UI_MENU_TYPE_ITEM_ACTION,
        "history-milestone-reset", uisnapshot_history_milestone_reset, NULL,
        GDK_KEY_U, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,
#if 0
    { "Recording start mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "history-recording-start-mode", ui_settings_dialog_create, "20,0",
        0, 0 },

    UI_MENU_SEPARATOR,
#endif

    { "Save media file ...", UI_MENU_TYPE_ITEM_ACTION,
        "media-save", uimedia_dialog_show, NULL,
        GDK_KEY_R, VICE_MOD_MASK | GDK_SHIFT_MASK },

    { "Stop media recording", UI_MENU_TYPE_ITEM_ACTION,
        "media-stop", (void *)uimedia_stop_recording, NULL,
        GDK_KEY_S, VICE_MOD_MASK | GDK_SHIFT_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Settings' menu - head section
 */
static ui_menu_item_t settings_menu_head[] = {
    { "Toggle fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen", ui_fullscreen_callback, NULL,
        GDK_KEY_D, VICE_MOD_MASK },
#if 1
    { "Show menu/status in fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen-widgets", ui_fullscreen_decorations_callback, NULL,
        GDK_KEY_B, VICE_MOD_MASK },
#else
    /* Mac menubar version */
    { "Show statusbar in fullscreen", UI_MENU_TYPE_ITEM_ACTION,
        "fullscreen-widgets", ui_fullscreen_decorations_callback, NULL,
        GDK_KEY_B, VICE_MOD_MASK },
#endif

    UI_MENU_SEPARATOR,

    { "Toggle warp mode", UI_MENU_TYPE_ITEM_CHECK,
        "warp", (void *)(ui_toggle_resource), (void *)"WarpMode",
        GDK_KEY_W, VICE_MOD_MASK },
    { "Pause emulation", UI_MENU_TYPE_ITEM_CHECK,
        "pause", (void *)(ui_toggle_pause), NULL,
        GDK_KEY_P, VICE_MOD_MASK },
    { "Advance frame", UI_MENU_TYPE_ITEM_ACTION,
        "frame-advance", (void *)(ui_advance_frame), NULL,
        GDK_KEY_P, VICE_MOD_MASK | GDK_SHIFT_MASK },

    { "Toggle CRT controls", UI_MENU_TYPE_ITEM_ACTION,
        "crt-controes", (void *)ui_toggle_crt_controls, NULL,
        0, 0, },

    UI_MENU_SEPARATOR,

    UI_MENU_TERMINATOR
};


/** \brief  'Settings' menu - joystick section pointer
 *
 * Set by ...
 */
static ui_menu_item_t *settings_menu_joy_section = NULL;


/** \brief  'Settings' menu - all joystick items
 *
 * Only valid for x64/x64sc/xscpu64/x128/xplus4
 */
static ui_menu_item_t settings_menu_all_joy[] = {

    { "Swap joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "joystick-swap", (void *)(ui_swap_joysticks_callback), NULL,
        GDK_KEY_J, VICE_MOD_MASK },
    { "Swap userport joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "userportjoy-swap", (void *)(ui_swap_userport_joysticks_callback), NULL,
        GDK_KEY_U, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu - control port joystick items
 *
 * Only valid for x64dtv/xcbm5x0
 */
static ui_menu_item_t settings_menu_cbm5x0_joy[] = {

    { "Swap joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "joystick-swap", (void *)(ui_swap_joysticks_callback), NULL,
        GDK_KEY_J, VICE_MOD_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu - userport joystick items
 *
 * Only valid for xvic/xpet/xcbm2
 */
static ui_menu_item_t settings_menu_userport_joy[] = {
    { "Swap userport joysticks", UI_MENU_TYPE_ITEM_ACTION,
        "userportjoy-swap", (void *)(ui_swap_userport_joysticks_callback), NULL,
        GDK_KEY_U, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Allow keyset joystick", UI_MENU_TYPE_ITEM_CHECK,
        "keyset", (void *)(ui_toggle_resource), (void *)"KeySetEnable",
        GDK_KEY_J, VICE_MOD_MASK | GDK_SHIFT_MASK },
    { "Enable mouse grab", UI_MENU_TYPE_ITEM_CHECK,
        "mouse", (void *)(ui_toggle_resource), (void *)"Mouse",
        GDK_KEY_M, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};

/** \brief  'Settings' menu tail section
 */
static ui_menu_item_t settings_menu_tail[] = {

    UI_MENU_SEPARATOR,

    /* the settings dialog */
    { "Settings ...", UI_MENU_TYPE_ITEM_ACTION,
        "settings", ui_settings_dialog_create, NULL,
        0, 0 },
    UI_MENU_TERMINATOR
};


#ifdef DEBUG

/** \brief  'Debug' menu items for emu's except x64dtv
 */
static ui_menu_item_t debug_menu[] = {
    { "Trace mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "tracemode", uidebug_trace_mode_callback, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Main CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-maincpu", (void *)(ui_toggle_resource), (void *)"MainCPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Drive #8 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive8", (void *)(ui_toggle_resource), (void *)"Drive0CPU_TRACE",
        0, 0 },
    { "Drive #9 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive9", (void *)(ui_toggle_resource), (void *)"Drive1CPU_TRACE",
        0, 0 },
    { "Drive #10 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive10", (void *)(ui_toggle_resource), (void *)"Drive2CPU_TRACE",
        0, 0 },
    { "Drive #11 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive11", (void *)(ui_toggle_resource), (void *)"Drive3CPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Autoplay playback frames ...", UI_MENU_TYPE_ITEM_ACTION,
        "playframes", uidebug_playback_frames_callback, NULL,
        0, 0 },
    { "Save core dump", UI_MENU_TYPE_ITEM_CHECK,
        "coredump", (void *)(ui_toggle_resource), (void *)"DoCoreDump",
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  'Debug' menu items for x64dtv
 */
static ui_menu_item_t debug_menu_c64dtv[] = {
    { "Trace mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "tracemode", uidebug_trace_mode_callback, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Main CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-maincpu", (void *)(ui_toggle_resource), (void *)"MainCPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Drive #8 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive8", (void *)(ui_toggle_resource), (void *)"Drive0CPU_TRACE",
        0, 0 },
    { "Drive #9 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive9", (void *)(ui_toggle_resource), (void *)"Drive1CPU_TRACE",
        0, 0 },
    { "Drive #10 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive10", (void *)(ui_toggle_resource), (void *)"Drive2CPU_TRACE",
        0, 0 },
    { "Drive #11 CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-drive11", (void *)(ui_toggle_resource), (void *)"Drive3CPU_TRACE",
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Blitter log", UI_MENU_TYPE_ITEM_CHECK,
      "blitter-log", (void *)ui_toggle_resource, (void *)"DtvBlitterLog",
      0, 0 },
    { "DMA log", UI_MENU_TYPE_ITEM_CHECK,
      "dma-log", (void *)ui_toggle_resource, (void *)"DtvDMALog",
      0, 0 },
    { "Flash log", UI_MENU_TYPE_ITEM_CHECK,
      "flash-log", (void *)ui_toggle_resource, (void*)"DtvFlashLog",
      0, 0 },

    UI_MENU_SEPARATOR,

    { "Autoplay playback frames ...", UI_MENU_TYPE_ITEM_ACTION,
        "playframes", uidebug_playback_frames_callback, NULL,
        0, 0 },
    { "Save core dump", UI_MENU_TYPE_ITEM_CHECK,
        "coredump", (void *)(ui_toggle_resource), (void *)"DoCoreDump",
        0, 0 },

    UI_MENU_TERMINATOR
};

#endif


/** \brief  'Help' menu items
 */
static ui_menu_item_t help_menu[] = {
    { "Browse manual", UI_MENU_TYPE_ITEM_ACTION,
        "manual", ui_open_manual_callback, NULL,
        0, 0 },
    { "Command line options ...", UI_MENU_TYPE_ITEM_ACTION,
        "cmdline", uicmdline_dialog_show, NULL,
        0, 0 },
    { "Compile time features ...", UI_MENU_TYPE_ITEM_ACTION,
        "features", uicompiletimefeatures_dialog_show, NULL,
        0, 0 },
    { "About VICE", UI_MENU_TYPE_ITEM_ACTION,
        "about", ui_about_dialog_callback, NULL,
        0, 0 },

    UI_MENU_TERMINATOR
};


/** \brief  Create the top menu bar with standard submenus
 *
 * \return  GtkMenuBar
 */
GtkWidget *ui_machine_menu_bar_create(void)
{
    GtkWidget *menu_bar;

    /* create the top menu bar */
    menu_bar = gtk_menu_bar_new();

    /* create the top-level 'File' menu */
    file_submenu = ui_menu_submenu_create(menu_bar, "File");

    /* create the top-level 'Edit' menu */
    edit_submenu = ui_menu_submenu_create(menu_bar, "Edit");

    /* create the top-level 'Snapshot' menu */
    snapshot_submenu = ui_menu_submenu_create(menu_bar, "Snapshot");

    /* create the top-level 'Settings' menu */
    settings_submenu = ui_menu_submenu_create(menu_bar, "Settings");

#ifdef DEBUG
    /* create the top-level 'Debug' menu (when --enable-debug is used) */
    debug_submenu = ui_menu_submenu_create(menu_bar, "Debug");
#endif

    /* create the top-level 'Help' menu */
    help_submenu = ui_menu_submenu_create(menu_bar, "Help");

    /* determine which joystick swap menu items should be added */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_PLUS4:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* fall through */
        case VICE_MACHINE_SCPU64:
            /* add both swap-joy and swap-userport-joy */
            settings_menu_joy_section = settings_menu_all_joy;
            break;
        case VICE_MACHINE_CBM5x0:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* fall through */
        case VICE_MACHINE_C64DTV:
            /* only add swap-joy */
            settings_menu_joy_section = settings_menu_cbm5x0_joy;
            break;
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_CBM6x0:
            /* add tape section */
            file_menu_tape_section = file_menu_tape;
            /* only add swap-userport-joy */
            settings_menu_joy_section = settings_menu_userport_joy;
            break;
        case VICE_MACHINE_VSID:
            exit(1);
            break;
        default:
            break;
    }

    /* add items to the File menu */
    ui_menu_add(file_submenu, file_menu_head);
    if (file_menu_tape_section != NULL) {
        ui_menu_add(file_submenu, file_menu_tape_section);
    }
    ui_menu_add(file_submenu, file_menu_tail);

    /* add items to the Edit menu */
    ui_menu_add(edit_submenu, edit_menu);
    /* add items to the Snapshot menu */
    ui_menu_add(snapshot_submenu, snapshot_menu);

    /* add items to the Settings menu */
    ui_menu_add(settings_submenu, settings_menu_head);
    if (settings_menu_joy_section != NULL) {
        ui_menu_add(settings_submenu, settings_menu_joy_section);
    }
    ui_menu_add(settings_submenu, settings_menu_tail);

#ifdef DEBUG
    /* add items to the Debug menu */
    if (machine_class == VICE_MACHINE_C64DTV) {
        ui_menu_add(debug_submenu, debug_menu_c64dtv);
    } else {
        ui_menu_add(debug_submenu, debug_menu);
    }
#endif

    /* add items to the Help menu */
    ui_menu_add(help_submenu, help_menu);

    main_menu_bar = menu_bar;    /* XXX: do I need g_object_ref()/g_object_unref()
                                         for this */
    return menu_bar;
}
