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

#include "archdep.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basedialogs.h"
#include "drive.h"
#include "log.h"
#include "machine.h"
#include "util.h"
#include "vsync.h"

#if 0
#ifdef WIN32_COMPILE
# include <windows.h>
#endif
#endif

#include "ui.h"
#include "uicommands.h"
#include "uimachinewindow.h"


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
    ui_set_ignore_mouse_hide(FALSE);
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

    debug_gtk3("WINDOW DESTROY called on %p.", widget);

    /*
     * This should not be needed, destroying a GtkWindow should trigger
     * destruction of all widgets it contains.
     */
    debug_gtk3("Manually calling destroy() on the CRT widgets. This should not"
            " be necesarry, but right now it is.");
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
            debug_gtk3("toggling resource %s failed.", res);
            return FALSE;
        }
        debug_gtk3("resource %s toggled to %s.",
                   res, new_state ? "True" : "False");
        return TRUE;
    }
    return FALSE;
}


/** \brief  Open the Manual
 */
void ui_open_manual_callback(GtkWidget *widget, gpointer user_data)
{
    GError *error = NULL;
    gboolean res;
    char *uri;
    const char *path;
    gchar *final_uri;

    /*
     * Gget arch-dependent documentation dir (doesn't contain the HTML docs
     * on Windows, but that's an other issue to fix.
     */
    path = archdep_get_vice_docsdir();

    /* first try opening the pdf */
    uri = archdep_join_paths(path, "vice.pdf", NULL);

    debug_gtk3("URI before GTK3: %s", uri);

    /*
     * This should not be used, but rather a helper tool provided by Gtk:
     * gspawn-winXX-helper-console.exe needs to be installed by the bindist
     * script.
     */
#if 0
#ifdef WIN32_COMPILE
    /* Windows: the Gtk/GLib stuff fails whatever I do, so let's use actual
     *          Windows code. --compyx
     */
    ShellExecuteA(NULL, "open", uri, NULL, NULL, SW_SHOW);
    /* that's right: no error checking and no fallback to HTML */
    return;
#endif
#endif

    final_uri = g_filename_to_uri(uri, NULL, &error);
    debug_gtk3("final URI (pdf): %s", final_uri);
    if (final_uri == NULL) {
        /*
         * This is a fatal error, if a proper URI can't be built something is
         * wrong and should be looked at. This is different from failing to
         * load the PDF or not having a program to show the PDF
         */
        log_error(LOG_ERR, "failed to construct a proper URI from '%s',"
                " not trying the HTML fallback, this is an error that"
                " should not happen.",
                uri);
        g_clear_error(&error);
        lib_free(uri);
        lib_free(path);
        return;
    }

    debug_gtk3("pdf uri: '%s'.", final_uri);
    res = gtk_show_uri_on_window(NULL, final_uri, GDK_CURRENT_TIME, &error);
    if (!res) {
        vice_gtk3_message_error(
                "Failed to load PDF: %s.",
                error != NULL ? error->message : "<no message>");
    }
    lib_free(uri);
    g_free(final_uri);
    g_clear_error(&error);
    if (res) {
        /* We succesfully managed to open the PDF application, but there's no
         * way to determine if actually loading the PDF in that application
         * worked. So we simply exit here to avoid also opening a HTML browser
         * which on Windows at least seems to completely ignore the default and
         * always starts fucking Internet Explorer.
         *
         * Also how do we close the PDF application if we could determine it
         * failed to load the PDF? We don't get any reference to the application
         * to be able to terminate it. Gtk3 is awesome!
         *
         * -- compyx
         */
        lib_free(path);
        return;
    }

    /* try opening the html doc */
#if defined(WIN32_COMPILE)
    /* HACK: on windows the html files are in a separate directory */
    uri = archdep_join_paths(path, "..", "html", "vice_toc.html", NULL);
#else
    uri = archdep_join_paths(path, "vice_toc.html", NULL);
#endif

    final_uri = g_filename_to_uri(uri, NULL, &error);
    if (final_uri == NULL) {
        /*
         * This is a fatal error, if a proper URI can't be built something is
         * wrong and should be looked at. This is different from failing to
         * load the PDF or not having a program to show the PDF
         */
        log_error(LOG_ERR,
                "failed to construct a proper URI from '%s',"
                " this is an error that should not happen.",
                uri);
        g_free(final_uri);
        lib_free(uri);
        lib_free(path);
        return;
    }

    /*
     * On Windows this does not respect the user's preferred browser. That is,
     * it didn't respect my Firefox but decided to use Internet Explorer,
     * which is an unspeakable act of cruelty.
     */
    debug_gtk3("html uri: '%s'.", final_uri);
    res = gtk_show_uri_on_window(NULL, final_uri, GDK_CURRENT_TIME, &error);
    if (!res && error != NULL) {
        vice_gtk3_message_error("Failed to show URI", error->message);
    }
    lib_free(uri);
    g_free(final_uri);
    g_clear_error(&error);
    lib_free(path);
}
