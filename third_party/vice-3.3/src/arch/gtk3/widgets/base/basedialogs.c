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
#include <string.h>

#include "lib.h"
#include "debug_gtk3.h"
#include "uimachinewindow.h"
#include "ui.h"

#include "basedialogs.h"


/** \brief  Handler for the 'destroy' event of a dialog
 *
 * Destroys the temporary parent widget \a data
 *
 * \param[in]   dialog      unused
 * \param[in]   data        temporary parent widget
 */
static void on_dialog_destroy(GtkWidget *dialog, gpointer data)
{

    debug_gtk3("RESTORE MOUSE HIDE");
    ui_set_ignore_mouse_hide(FALSE);
#if 0
    int pause_state;

    pause_state = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dialog),
                "OldPauseState"));
#endif
    GtkWidget *window = GTK_WIDGET(data);
    gtk_widget_destroy(window);
#if 0
    if (!pause_state) {
        /* old state was unpaused: restore */
        ui_pause_emulation(0);
    }
#endif
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
    gboolean no_parent = FALSE;
#if 0
    int pause_state = ui_emulation_is_paused();
    /* pause emulation if not paused already */
    if (!pause_state) {
        ui_pause_emulation(1);
    }
#endif

    ui_set_ignore_mouse_hide(TRUE);

    if (parent == NULL) {
        /* set up a temporary parent to avoid Gtk warnings */
        no_parent = TRUE;
        parent = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));
    }

    dialog = gtk_message_dialog_new(
            parent,
            GTK_DIALOG_DESTROY_WITH_PARENT,
            type, buttons, NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_message_dialog_set_markup(GTK_MESSAGE_DIALOG(dialog), text);

#if 0
    /* store old pause state */
    debug_gtk3("OldPauseState = %s.", pause_state ? "paused" : "unpaused");
    g_object_set_data(G_OBJECT(dialog),
            "OldPauseState", GINT_TO_POINTER(pause_state));
#endif
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
    debug_gtk3("got response ID %d.", result);
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
#if 0
    size_t len = strlen(buffer);
    unsigned int x = 0;
    unsigned int y = 0;
    for (y = 0; y < 256 && x + y < (int)len; y += 16) {

        char textbuf[17];

        printf("%02x:  ", y);
        memset(textbuf, 0, 17);
        for (x = 0; x < 16 && x + y < (int)len; x++) {
            printf("%02x ", buffer[x + y]);
            textbuf[x] = buffer[x + y];
        }
        puts("");
    }
#endif

    dialog = create_dialog(GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE, title, buffer);
    gtk_dialog_run(GTK_DIALOG(dialog));
    lib_free(buffer);
    gtk_widget_destroy(dialog);
    return TRUE;
}


/** \brief  Try to convert the text in \a entry into an integer value
 *
 * \param[in]   entry   GtkTextEntry
 * \param[out]  value   object to store integer result
 *
 * \return  TRUE when conversion succeeded, FALSE otherwise
 *
 * \note    When FALSE is returned, the value pointed at by \a value is
 *          unchanged
 */
static gboolean entry_get_int(GtkWidget *entry, int *value)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    char *endptr;
    long tmp;

    tmp = strtol(text, &endptr, 0);
    if (*endptr == '\0') {
        *value = (int)tmp;
        return TRUE;
    }
    return FALSE;
}


/** \brief  Handler for the key-press event of the integer input box
 *
 * Signals ACCEPT to the dialog.
 *
 * \param[in]   entry   entry box
 * \param[in]   event   event object
 * \param[in]   data    dialog reference
 *
 * \return  TRUE if Enter was pushed, FALSE otherwise (makes the pushed key
 *          propagate to the entry
 */
static gboolean on_integer_key_press_event(GtkEntry *entry,
                                           GdkEvent *event,
                                           gpointer data)
{
    GtkWidget *dialog = data;
    GdkEventKey *keyev = (GdkEventKey *)event;

    if (keyev->type == GDK_KEY_PRESS && keyev->keyval == GDK_KEY_Return) {
        /* got Enter */
        g_signal_emit_by_name(dialog, "response", GTK_RESPONSE_ACCEPT, NULL);
    }
    return FALSE;
}


/** \brief  Create a dialog to enter an integer value
 *
 * \param[in]   title       dialog title
 * \param[in]   message     dialog body text
 * \param[in]   old_value   current value of whatever needs to be changed
 * \param[out]  new_value   object to store new value on success
 * \param[in]   min         minimal valid value
 * \param[in]   max         maximum valid value
 *
 * \return  TRUE when a valid value was entered, FALSE otherwise
 *
 * TODO: check input while entering (marking any invalid value red or so)
 */
gboolean vice_gtk3_integer_input_box(
        const char *title, const char *message,
        int old_value, int *new_value,
        int min, int max)
{
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    char *text;
    gint response;
    char buffer[1024];

    dialog = gtk_dialog_new_with_buttons(title, ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Accept", GTK_RESPONSE_ACCEPT,
            "Cancel", GTK_RESPONSE_REJECT,
            NULL);
    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 16);
    g_object_set(G_OBJECT(grid), "margin-left", 16, "margin-right", 16, NULL);

    /* add body message text */
    label = gtk_label_new(message);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_label_set_line_wrap_mode(GTK_LABEL(label), PANGO_WRAP_WORD);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 2, 1);

    /* add info on limits */
    text = lib_msprintf("(enter a number between %d and %d)", min, max);
    label = gtk_label_new(text);
    lib_free(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 2, 1);

    label = gtk_label_new("Enter new value:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, FALSE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);

    /* add the text entry */
    entry = gtk_entry_new();
    g_snprintf(buffer, 1024, "%d", old_value);
    gtk_entry_set_text(GTK_ENTRY(entry), buffer);

    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 2, 1 ,1);

    gtk_widget_show_all(grid);
    gtk_box_pack_start(GTK_BOX(content), grid, TRUE, TRUE, 8);

    g_signal_connect(dialog, "key-press-event",
            G_CALLBACK(on_integer_key_press_event), (gpointer)dialog);

    response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        /* TODO set *new_value */
        if (entry_get_int(entry, new_value)) {
            gtk_widget_destroy(dialog);
            if (*new_value >= min && *new_value <= max) {
                return TRUE;
            } else {
                debug_gtk3("value entered out of bounds (%d-%d): %d.",
                        min, max, *new_value);
                return FALSE;
            }
        }
    }
    gtk_widget_destroy(dialog);
    return FALSE;
}

