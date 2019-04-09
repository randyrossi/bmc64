/** \file   uicompiletimefeatures.c
 * \brief   Dialog to display compile time features
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
#include "vicefeatures.h"
#include "ui.h"

#include "uicompiletimefeatures.h"


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


/** \brief  Create tree model with compile time features
 *
 * \return  GtkTreeStore
 */
static GtkTreeStore *create_store(void)
{
    GtkTreeStore *store;
    GtkTreeIter iter;
    feature_list_t *list;

    store = gtk_tree_store_new(3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    list = vice_get_feature_list();

    while (list->symbol != NULL) {
        gtk_tree_store_append(store, &iter, NULL);
        gtk_tree_store_set(store, &iter,
                0, list->descr,
                1, list->symbol,
                2, list->isdefined ? "yes" : "no",
                -1);
        list++;
    }
    return store;
}


/** \brief  Create listview with compile time features
 *
 * \return  GtkScrolledWindow
 */
static GtkWidget *create_content_widget(void)
{
    GtkWidget *view;
    GtkWidget *scrolled;
    GtkTreeStore *store;
    GtkCellRenderer *text_renderer;
    GtkTreeViewColumn *column_desc;
    GtkTreeViewColumn *column_macro;
    GtkTreeViewColumn *column_state;


    scrolled = gtk_scrolled_window_new(NULL, NULL);
   store = create_store();
    view =  gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

    text_renderer = gtk_cell_renderer_text_new();

    column_desc = gtk_tree_view_column_new_with_attributes(
            "Description", text_renderer,
            "text", 0,
            NULL);
    column_macro = gtk_tree_view_column_new_with_attributes(
            "Macro", text_renderer,
            "text", 1,
            NULL);
    column_state = gtk_tree_view_column_new_with_attributes(
            "Enabled", text_renderer,
            "text", 2,
            NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column_desc);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column_macro);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), column_state);

    gtk_widget_set_size_request(scrolled, 800, 600);
    gtk_container_add(GTK_CONTAINER(scrolled), view);

    gtk_widget_show_all(scrolled);
    return scrolled;
}


/** \brief  Show list of compile time features
 *
 * \param[in]   widget      parent widget
 * \param[in]   user_data   extra data (unused)
 */
void uicompiletimefeatures_dialog_show(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *content;
    gchar title[256];

    g_snprintf(title, 256, "%s compile time features", machine_name);

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
