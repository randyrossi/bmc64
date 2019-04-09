/** \file   uivsidmenu.c
 * \brief   Native GTK3 menus for the SID player, vsid
 *
 * \author  Marcus Sutton <loggedoubt@gmail.com>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES PSIDKeepEnv     vsid
 * $VICERES MainCPU_TRACE   vsid
 * $VICERES DoCoreDump      vsid
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

#include "debug.h"
#include "lib.h"
#include "machine.h"
#include "psid.h"
#include "ui.h"
#include "uiabout.h"
#include "uicmdline.h"
#include "uicommands.h"
#include "uicompiletimefeatures.h"
#include "uidebug.h"
#include "uimedia.h"
#include "uimenu.h"
#include "uimonarch.h"
#include "uisettings.h"
#include "uisidattach.h"
#include "uismartattach.h"
#include "uivsidmenu.h"


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


/** \brief  Tune submenu
 */
static GtkWidget *tune_submenu = NULL;


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


/** \brief  List of items in the tune submenu
 */
static GSList *tune_submenu_group = NULL;


/** \brief  File->Reset submenu
 */
static ui_menu_item_t reset_submenu[] = {
    { "Soft reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-soft", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_SOFT),
        GDK_KEY_F9, VICE_MOD_MASK },
    { "Hard reset", UI_MENU_TYPE_ITEM_ACTION,
        "reset-hard", ui_machine_reset_callback, GINT_TO_POINTER(MACHINE_RESET_MODE_HARD),
        GDK_KEY_F12, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'File' menu
 */
static ui_menu_item_t file_menu[] = {
    { "Load PSID file ...", UI_MENU_TYPE_ITEM_ACTION,
        "load-psid", uisidattach_show_dialog, NULL,
        GDK_KEY_L, VICE_MOD_MASK },

    UI_MENU_SEPARATOR,

    /* XXX: this item might need its own dialog that only
     *      contains sound recording options
     */
    { "Record sound file ...", UI_MENU_TYPE_ITEM_ACTION,
        "sound-save", uimedia_dialog_show, NULL,
        GDK_KEY_R, VICE_MOD_MASK | GDK_SHIFT_MASK },

    { "Stop sound recording", UI_MENU_TYPE_ITEM_ACTION,
        "sound-stop", (void *)uimedia_stop_recording, NULL,
        GDK_KEY_S, VICE_MOD_MASK | GDK_SHIFT_MASK },

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

    { "Reset", UI_MENU_TYPE_SUBMENU,
        NULL, NULL, reset_submenu,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Exit player", UI_MENU_TYPE_ITEM_ACTION,
        "exit", ui_close_callback, NULL,
        GDK_KEY_Q, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Tune' menu
 */
#if 0
static ui_menu_item_t tune_menu[] = {

    UI_MENU_TERMINATOR
};
#endif


/** \brief  'Settings' menu
 */
static ui_menu_item_t settings_menu[] = {
    /* XXX: this item should perhaps be removed and its functionality
     *      added to the settings dialog
     */
    { "Override PSID settings", UI_MENU_TYPE_ITEM_CHECK,
        "psid-keep-env", (void *)(ui_toggle_resource), (void *)"PSIDKeepEnv",
        0, 0 },

    UI_MENU_SEPARATOR,

    /* the settings dialog */
    { "Settings ...", UI_MENU_TYPE_ITEM_ACTION,
        "settings", (void *)ui_settings_dialog_create, NULL,
        GDK_KEY_O, VICE_MOD_MASK },

    UI_MENU_TERMINATOR
};


/** \brief  'Debug' menu items
 */
#ifdef DEBUG
static ui_menu_item_t debug_menu[] = {
    { "Trace mode ...", UI_MENU_TYPE_ITEM_ACTION,
        "tracemode", uidebug_trace_mode_callback, NULL,
        0, 0 },

    UI_MENU_SEPARATOR,

    { "Main CPU trace", UI_MENU_TYPE_ITEM_CHECK,
        "trace-maincpu", (void *)(ui_toggle_resource), (void *)"MainCPU_TRACE",
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

/** \brief  Play a tune when selected with the Tune menu
 *
 * \return  void
 */
static void select_tune_from_menu(GtkMenuItem *menuitem,
                                  gpointer     user_data) {
    int tune;

    if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) {
        return;
    }
    tune = GPOINTER_TO_INT(user_data);
    psid_init_driver();
    machine_play_psid(tune);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
}


/** \brief  Remove each of the old menu items before adding the new ones
 *
 * \return  void
 */
static void remove_item_from_menu (GtkWidget *widget, gpointer data) {
    gtk_widget_destroy(widget);
}

/** \brief  Create the tune menu when a new PSID is loaded
 *
 * \return  void
 */
void ui_vsid_tune_menu_set_tune_count(int count) {
    GtkWidget *item = NULL;
    long i;
    char *buf;

    gtk_container_foreach(GTK_CONTAINER(tune_submenu), remove_item_from_menu, NULL);
    for (i = count; i >0; i--) {
        buf = lib_msprintf("Tune %s%d", i < 10 ? "_" :"", i);
        item = gtk_radio_menu_item_new_with_mnemonic_from_widget (GTK_RADIO_MENU_ITEM(item), buf);
        lib_free(buf);
        gtk_widget_show(item);
        g_signal_connect(
            item,
            "activate",
            G_CALLBACK(select_tune_from_menu),
            GINT_TO_POINTER(i));
        gtk_menu_shell_prepend(GTK_MENU_SHELL(tune_submenu), item);
    }
    tune_submenu_group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
}


/** \brief  Ensure the correct menu element is selected when the current tune is changed by anything other than the menu
 *
 * \return  void
 */
void ui_vsid_tune_set_tune_current(int count) {
    if (tune_submenu_group) {
        gpointer nth_item = g_slist_nth_data(tune_submenu_group, (guint)count - 1);
        if (nth_item) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM (nth_item), TRUE);
        }
    }
}

/** \brief  Create the top menu bar with standard submenus
 *
 * \return  GtkMenuBar
 */
GtkWidget *ui_vsid_menu_bar_create(void)
{
    GtkWidget *menu_bar;

    /* create the top menu bar */
    menu_bar = gtk_menu_bar_new();

    /* create the top-level 'File' menu */
    file_submenu = ui_menu_submenu_create(menu_bar, "File");

    /* create the top-level 'Tune' menu */
    tune_submenu = ui_menu_submenu_create(menu_bar, "Tune");

    /* create the top-level 'Settings' menu */
    settings_submenu = ui_menu_submenu_create(menu_bar, "Settings");

#ifdef DEBUG
    /* create the top-level 'Debug' menu (when --enable-debug is used) */
    debug_submenu = ui_menu_submenu_create(menu_bar, "Debug");
#endif

    /* create the top-level 'Help' menu */
    help_submenu = ui_menu_submenu_create(menu_bar, "Help");


    /* add items to the File menu */
    ui_menu_add(file_submenu, file_menu);

#if 0
    /* TODO: add items to the Tune menu */
    ui_menu_add(tune_submenu, tune_menu);
#endif

    /* add items to the Settings menu */
    ui_menu_add(settings_submenu, settings_menu);

#ifdef DEBUG
    /* add items to the Debug menu */
    ui_menu_add(debug_submenu, debug_menu);
#endif

    /* add items to the Help menu */
    ui_menu_add(help_submenu, help_menu);

    main_menu_bar = menu_bar;    /* XXX: do I need g_object_ref()/g_object_unref()
                                         for this */
    return menu_bar;
}
