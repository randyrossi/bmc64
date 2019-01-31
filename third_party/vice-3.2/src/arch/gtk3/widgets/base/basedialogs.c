/**
 * \brief   Gtk3 basic dialogs (Info, Yes/No, etc)
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
#include <stdbool.h>

#include "lib.h"
#include "debug_gtk3.h"
#include "ui.h"

#include "basedialogs.h"


/** \brief  Handler for the 'destroy' event of a dialog
 *
 * Destroys the temporary parent widget \a data
 *
 * \param[in]   dialog  unused
 * \param[in]   data    temporary parent widget
 */
static void on_dialog_destroy(GtkWidget *dialog, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_destroy(window);
}


/** \brief  Create a GtkMessageDialog
 *
 * \param[in]   type        message type
 * \param[in]   buttons     buttons to use
 * \param[in]   title       dialog title
 * \param[in]   text        dialog text, optional marked-up with Pango
 *
 * \return  GtkMessageDialog
 */
static GtkWidget *create_dialog(GtkMessageType type, GtkButtonsType buttons,
                                const char *title, const char *text)
{
    GtkWidget *dialog;
    GtkWindow *parent = ui_get_active_window();
    bool no_parent = false;

    if (parent == NULL) {
        /* set up a temporary parent to avoid Gtk warnings */
        no_parent = true;
        parent = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    }

    dialog = gtk_message_dialog_new(
            parent,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            type, buttons, NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), text);

    /* set up signal handler to destroy the temporary parent window */
    if (no_parent) {
        g_signal_connect(dialog, "destroy", G_CALLBACK(on_dialog_destroy),
                (gpointer)parent);
    }

    return dialog;
}


/* XXX: All the following functions use the same function signature, even if
 *      returning a `gboolean` doesn't really make sense, that way they can
 *      be used with a generic function-pointer, should we require that.
 */


/** \brief  Create 'info' dialog
 *
 * \param[in[   title       dialog title
 * \param[in]   fmt         message format string and arguments
 *
 * \return  `TRUE`
 */
gboolean vice_gtk3_message_info(const char *title, const char *fmt, ...)
{
    GtkWidget *dialog;
    va_list args;
    char *buffer;

    va_start(args, fmt);
    buffer = lib_mvsprintf(fmt, args);
    va_end(args);

    dialog = create_dialog(GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, title, buffer);
    gtk_dialog_run(GTK_DIALOG(dialog));
    lib_free(buffer);
    gtk_widget_destroy(dialog);
    return TRUE;
}


/** \brief  Create 'confirm' dialog
 *
 * \param[in[   title       dialog title
 * \param[in]   fmt         message format string and arguments
 *
 * \return  `TRUE` on OK/Yes, `FALSE` otherwise
 */
gboolean vice_gtk3_message_confirm(const char *title, const char *fmt, ...)
{
    GtkWidget *dialog;
    va_list args;
    char *buffer;
    int result;

    va_start(args, fmt);
    buffer = lib_mvsprintf(fmt, args);
    va_end(args);

    dialog = create_dialog(GTK_MESSAGE_QUESTION, GTK_BUTTONS_OK_CANCEL,
            title, buffer);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    lib_free(buffer);
    gtk_widget_destroy(dialog);
    debug_gtk3("got response ID %d\n", result);
    if (result == GTK_RESPONSE_OK) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/** \brief  Create 'error' dialog
 *
 * \param[in]   widget      parent widget
 * \param[in[   title       dialog title
 * \param[in]   fmt         message format string and arguments
 *
 * \return  `TRUE`
 */
gboolean vice_gtk3_message_error(const char *title, const char *fmt, ...)
{
    GtkWidget *dialog;
    va_list args;
    char *buffer;

    va_start(args, fmt);
    buffer = lib_mvsprintf(fmt, args);
    va_end(args);

    dialog = create_dialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, title, buffer);
    gtk_dialog_run(GTK_DIALOG(dialog));
    lib_free(buffer);
    gtk_widget_destroy(dialog);
    return TRUE;
}
