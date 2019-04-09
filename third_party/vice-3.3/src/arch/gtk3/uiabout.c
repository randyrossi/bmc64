/** \file   uiabout.c
 * \brief   GTK3 about dialog
 *
 * \todo    Needs a proper logo, not the old, ugly, blue one. The logo from the
 *          pokefinder website will do nicely, I think.
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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
 */


#include "vice.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug_gtk3.h"
#include "info.h"
#include "lib.h"
#include "ui.h"
#include "version.h"
#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif
#include "uidata.h"

#include "uiabout.h"


/** \brief  List of current team members
 *
 * This list is allocated in create_current_team_list() and deallocated in
 * the "destroy" callback of the About dialog
 */
static char **authors;


/** \brief  Create list of current team members
 *
 * \return  heap-allocated list of strings
 */
static char **create_current_team_list(void)
{
    char **list;
    size_t i;

    /* get proper size of list (sizeof doesn't work here) */
    for (i = 0; core_team[i].name != NULL; i++) {
        /* NOP */
    }
    list = lib_malloc(sizeof *list * (i + 1));

#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s(): team members = %d\n", __func__, (int)i);
#endif

    /* create list of current team members */
    for (i = 0; core_team[i].name != NULL; i++) {
        list[i] = core_team[i].name;
    }
    list[i] = NULL;
    return list;
}


#if 0
static char **create_translators_list(void)
{
    char **list = lib_malloc(sizeof *list * 256);
    size_t i;

    while (trans_team[i].name != NULL) {
        char *member = lib_malloc(256);
        snprintf(member, 256, "%s - %s (%s)",
                trans_team[i].years,
                trans_team[i].name,
                trans_team[i].language);
        list[i++] = member;
    }
    list[i] = NULL;
    return list;
}
#endif


/** \brief  Deallocate current team list
 *
 * \param[in,out]   list    list of team member names
 */
static void destroy_current_team_list(char **list)
{
    lib_free(list);
}


/** \brief  Create VICE logo
 *
 * \return  GdkPixbuf instance
 */
static GdkPixbuf *get_vice_logo(void)
{
    return uidata_get_pixbuf("vice-logo-black.svg");
}

/** \brief  Handler for the "destroy" event
 *
 * \param[in,out]   widget      widget triggering the event (unused)
 * \param[in]       user_data   data for the event (unused)
 */
static void about_destroy_callback(GtkWidget *widget, gpointer user_data)
{
    destroy_current_team_list(authors);
    /* GdkPixbuf mentions setting refcount to 1, but it appears the about
     * dialog parent cleans it up somehow -- compyx */
#if 0
    g_object_unref(user_data);
#endif
}


/** \brief  Handler for the "response" event
 *
 * This handles the "response" event, which is triggered for various standard
 * buttons, although which buttons trigger this is a little unclear at the
 * moment.
 *
 * \param[in,out]   widget      widget triggering the event (the dialog)
 * \param[in]       response_id response ID
 * \param[in]       user_data   extra data (unused)
 */
static void about_response_callback(GtkWidget *widget, gint response_id,
                                    gpointer user_data)
{
#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s(): response id: %d\n", __func__, response_id);
#endif
    /* the GTK_RESPONSE_DELETE_EVENT is sent when the user clicks 'Close', but
     * also when the user clicks the 'X' */
    if (response_id == GTK_RESPONSE_DELETE_EVENT) {
#ifdef HAVE_DEBUG_GTK3UI
        g_print("[debug-gtk3ui] %s(): CLOSE button clicked\n", __func__);
#endif
        gtk_widget_destroy(widget);
    }
}


/** \brief  Callback to show the 'About' dialog
 *
 * \param[in,out]   widget      widget triggering the event
 * \param[in]       user_data   data for the event (unused)
 */
void ui_about_dialog_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *about = gtk_about_dialog_new();
    GdkPixbuf *logo = get_vice_logo();

#ifdef HAVE_DEBUG_GTK3UI
    g_print("[debug-gtk3ui] %s() called\n", __func__);
#endif

    /* set toplevel window, Gtk doesn't like dialogs without parents */
    gtk_window_set_transient_for(GTK_WINDOW(about), ui_get_active_window());

    /* generate team members list */
    authors = create_current_team_list();

    /* set window title */
    gtk_window_set_title(GTK_WINDOW(about), "About VICE");

    /* set version string */
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about),
#ifdef USE_SVN_REVISION
            VERSION " r" VICE_SVN_REV_STRING " (Gtk3)"
#else
            VERSION " (Gtk3)"
#endif
            );

    /* set license */
    gtk_about_dialog_set_license_type(GTK_ABOUT_DIALOG(about), GTK_LICENSE_GPL_2_0);
    /* set website link and title */
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about),
            "http://vice-emu.sourceforge.net/");
    gtk_about_dialog_set_website_label(GTK_ABOUT_DIALOG(about),
            "http://vice-emu.sourceforge.net/");
    /* set list of current team members */
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about), (const gchar **)authors);
    /* set copyright string */
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about),
            "Copyright 1996-2018 VICE TEAM");

    /* set logo */
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about), logo);

    /*
     * hook up event handlers
     */

    /* destroy callback, called when the dialog is closed through the 'X',
     * but NOT when clicking 'Close' */
    g_signal_connect(about, "destroy", G_CALLBACK(about_destroy_callback), (gpointer)logo);

    /* set up a generic handler for various buttons, this makes sure the
     * 'Close' button is handled properly */
    g_signal_connect(about, "response", G_CALLBACK(about_response_callback),
            NULL);

    /* make the about dialog modal */
    gtk_window_set_modal(GTK_WINDOW(about), TRUE);

    /* ... and show the dialog finally */
    gtk_widget_show(about);
}
