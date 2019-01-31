/** \file   uicommands.c
 * \brief   Simple commands triggered from the menu
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 */

/*
 * $VICERES WarpMode        all
 * $VICERES JoyDevice1      -vsid
 * $VICERES JoyDevice2      -vsid
 * $VICERES JoyDevice3      -vsid
 * $VICERES JoyDevice4      -vsid
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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>
#include <stdbool.h>

#include "archdep.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basedialogs.h"
#include "drive.h"
#include "machine.h"
#include "util.h"
#include "vsync.h"

#include "ui.h"
#include "uicommands.h"


static bool crt_controls_enable = false;


/** \brief  Swap joysticks
 *
 * \param[in]   widget      widget triggering the event (invalid)
 * \param[in]   user_data   extra data for event (unused)
 *
 * \return  TRUE
 */
gboolean ui_swap_joysticks_callback(GtkWidget *widget, gpointer user_data)
{
    int joy1;
    int joy2;

    resources_get_int("JoyDevice1", &joy1);
    resources_get_int("JoyDevice2", &joy2);
    resources_set_int("JoyDevice1", joy2);
    resources_set_int("JoyDevice2", joy1);

    return TRUE;
}


/** \brief  Swap userport joysticks
 *
 * \param[in]   widget      widget triggering the event (invalid)
 * \param[in]   user_data   extra data for event (unused)
 *
 * \return  TRUE
 */
gboolean ui_swap_userport_joysticks_callback(GtkWidget *widget,
                                             gpointer user_data)
{
    int joy3;
    int joy4;

    resources_get_int("JoyDevice3", &joy3);
    resources_get_int("JoyDevice4", &joy4);
    resources_set_int("JoyDevice3", joy4);
    resources_set_int("JoyDevice4", joy3);

    return TRUE;
}


/** \brief  Callback for the soft/hard reset items
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   MACHINE_RESET_MODE_SOFT/MACHINE_RESET_MODE_HARD
 */
void ui_machine_reset_callback(GtkWidget *widget, gpointer user_data)
{
    vsync_suspend_speed_eval();
    machine_trigger_reset(GPOINTER_TO_INT(user_data));
}


/** \brief  Callback for the drive reset items
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   drive unit number (8-11) (int)
 */
void ui_drive_reset_callback(GtkWidget *widget, gpointer user_data)
{
    vsync_suspend_speed_eval();
    drive_cpu_trigger_reset(GPOINTER_TO_INT(user_data) - 8);
}


/** \brief  Ask the user to confirm to exit the emulator if ConfirmOnExit is set
 *
 * \return  TRUE if the emulator should be exited, FALSE if not
 */
static gboolean confirm_exit(void)
{
    int confirm;

    resources_get_int("ConfirmOnExit", &confirm);
    if (!confirm) {
        return TRUE;
    }

    if (vice_gtk3_message_confirm("Exit VICE",
                                  "Do you really wish to exit VICE?")) {
        return TRUE;
    }
    return FALSE;
}


/** \brief  Callback for the File->Exit menu item
 *
 * \param[in]   widget      menu item triggering the event (unused)
 * \param[in]   user_data   unused
 */
void ui_close_callback(GtkWidget *widget, gpointer user_data)
{
    if (confirm_exit()) {
        ui_exit();
    }
}


/** \brief  Handler for the "delete-event" of a main window
 *
 * \param[in]   widget      window triggering the event
 * \param[in]   event       event details (unused)
 * \param[in]   user_data   extra data for the event (unused)
 *
 * \return  TRUE, if the function returns at all
 */
gboolean ui_main_window_delete_event(GtkWidget *widget, GdkEvent *event,
                                     gpointer user_data)
{
    if (confirm_exit()) {
        /* if we reach this point, the function doesn't return */
        ui_exit();
    }
    return TRUE;
}


/** \brief  Callback for the "destroy" event of a main window
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   extra data for the callback (unused)
 */
void ui_main_window_destroy_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *grid;

    debug_gtk3("WINDOW DESTROY called on %p\n", widget);

    /*
     * This should not be needed, destroying a GtkWindow should trigger
     * destruction of all widgets it contains.
     */
    debug_gtk3("Manually calling destroy() on the CRT widgets. This should not"
            " be necesarry, but right now it is\n");
    grid = gtk_bin_get_child(GTK_BIN(widget));
    if (grid != NULL) {
        GtkWidget *crt = gtk_grid_get_child_at(GTK_GRID(grid), 0, 2);
        if (crt != NULL) {
            gtk_widget_destroy(crt);
        }
    }
}


/** \brief  Toggle boolean resource from the menu
 *
 * Toggles \a resource when a valid.
 *
 * \param[in]   widget      menu item triggering the event
 * \param[in]   resource    resource name
 *
 * \return  TRUE if succesful, FALSE otherwise
 */
gboolean ui_toggle_resource(GtkWidget *widget, gpointer resource)
{
    const char *res = (const char *)resource;

    if (res != NULL) {
        int new_state;

        /* attempt to toggle resource */
        if (resources_toggle(res, &new_state) < 0) {
            debug_gtk3("toggling resource %s failed\n", res);
            return FALSE;
        }
        debug_gtk3("resource %s toggled to %s\n",
                   res, new_state ? "True" : "False");
        return TRUE;
    }
    return FALSE;
}


gboolean ui_toggle_crt_controls(void)
{
    crt_controls_enable = !crt_controls_enable;

    ui_enable_crt_controls(crt_controls_enable);
    return TRUE;
}


gboolean ui_crt_controls_enabled(void)
{
    return crt_controls_enable;
}

/** \brief  Open the Manual
 *
 * \return  TRUE if succesful, FALSE otherwise
 */
void ui_open_manual_callback(GtkWidget *widget, gpointer user_data)
{
    GError *error = NULL;
    gboolean res;
    char *uri;
    const char *path;
#if defined(WIN32_COMPILE)
    const char *tpath;
#endif

#ifdef MACOSX_BUNDLE
    /* On Macs the manual path is relative to the bundle. */
    path = util_concat(archdep_boot_path(), "/../doc/", NULL);
#elif defined(WIN32_COMPILE)
    /* On Windows the manual path is relative to the .exe */
    tpath = util_concat("/", archdep_boot_path(), "/doc/", NULL);
    /* we need forward slashes in the uri */
    path = util_subst(tpath, "\\", "/");
    lib_free(tpath);
    debug_gtk3("doc path: %s\n", path);
#else
    path = util_concat(DOCDIR, "/", NULL);
#endif

    /* first try opening the pdf */
    uri = util_concat("file://", path, "vice.pdf", NULL);
    debug_gtk3("pdf uri: %s\n", uri);
    res = gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    lib_free(uri);
    g_clear_error(&error);
    if (res) {
        lib_free(path);
        return;
    }
    /* try opening the html doc */
#if defined(WIN32_COMPILE)
    /* HACK: on windows the html files are in a seperate directory */
    uri = util_concat("file://", path, "../html/vice_toc.html", NULL);
#else
    uri = util_concat("file://", path, "vice_toc.html", NULL);
#endif
    debug_gtk3("html uri: %s\n", uri);
    res = gtk_show_uri_on_window(NULL, uri, GDK_CURRENT_TIME, &error);
    lib_free(uri);
    g_clear_error(&error);
    /* if (res) {
        lib_free(path);
        return;
    } */
    lib_free(path);
    return;
}
