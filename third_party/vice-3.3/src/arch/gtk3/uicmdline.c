/** \file   uicmdline.c
 * \brief   Dialog to display command line options
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
 *
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "machine.h"
#include "cmdline.h"
#include "ui.h"

#include "uicmdline.h"


/** \brief  Handler for the 'response' event of the dialog
 *
 * \param[in,out]   dialog      dialog triggering the event
 * \param[in]       response_id response ID
 * \param[in]       user_data   extra event data (unused)
 */
static void on_response(GtkDialog *dialog,
                        gint response_id,
                        gpointer user_data)
{
    if (response_id == GTK_RESPONSE_CLOSE) {
        gtk_widget_destroy(GTK_WIDGET(dialog));
    }
}


/** \brief  Create textview with scrollbars
 *
 * \return  GtkScrolledWindow
 */
static GtkWidget *create_content_widget(void)
{
    GtkWidget *view;
    GtkWidget *scrolled;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextTag *name_tag;
    GtkTextTag *inv_tag;
    GtkTextTag *desc_tag;
    int num_options = cmdline_get_num_options();
    int i;

    view = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(view), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(view), GTK_WRAP_WORD_CHAR);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(view),16);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(view),16);


    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));

    /* name: bold (for -option) */
    name_tag = gtk_text_buffer_create_tag(buffer, "name_tag",
            "weight", PANGO_WEIGHT_BOLD, NULL);
    /* name: bold and red (for +option) */
    inv_tag = gtk_text_buffer_create_tag(buffer, "inv_tag",
            "weight", PANGO_WEIGHT_BOLD,
            "foreground", "red", NULL);
    /* description: indented and sans-serif */
    desc_tag = gtk_text_buffer_create_tag(buffer, "desc_tag",
            "left-margin", 48,
            "family", "sans-serif", NULL);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, 800, 600);
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);

    for (i = 0; i < num_options; i++) {
        char *name;
        char *param;
        char *desc;

        name = cmdline_options_get_name(i);
        param = cmdline_options_get_param(i);
        desc = cmdline_options_get_description(i);

        if (name[0] == '-') {
            gtk_text_buffer_insert_with_tags(buffer, &iter, name, -1,
                    name_tag, NULL);
        } else {
            gtk_text_buffer_insert_with_tags(buffer, &iter, name, -1,
                    inv_tag, NULL);
        }

        if (param != NULL) {
            gtk_text_buffer_insert(buffer, &iter, " ", -1);
            gtk_text_buffer_insert(buffer, &iter, param, -1);
        }
        gtk_text_buffer_insert(buffer, &iter, "\n", -1);

        if (desc == NULL) {
            fprintf(stderr, "no desc for '%s'\n", name);
            desc = "[DESCRIPTION MISSING]";
        }
        gtk_text_buffer_insert_with_tags(buffer, &iter, desc, -1,
                desc_tag, NULL);
        gtk_text_buffer_insert(buffer, &iter, "\n\n", -1);
    }

    gtk_widget_show_all(scrolled);
    return scrolled;
}


/** \brief  Show list of command line options
 *
 * \param[in]   widget      parent widget (unused)
 * \param[in]   user_data   extra data (unused)
 */
void uicmdline_dialog_show(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *content;
    gchar title[256];

    g_snprintf(title, 256, "%s command line options", machine_name);

    dialog = gtk_dialog_new_with_buttons(title,
            ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Close", GTK_RESPONSE_CLOSE,
            NULL);

    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_box_pack_start(GTK_BOX(content), create_content_widget(),
            TRUE, TRUE, 0);

    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);
    gtk_widget_show_all(dialog);
}
