/** \file   romsetmanagerwidget.c
 * \brief   GTK3 ROM set manager widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * This widget is used to manage rom sets.
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

#include "vice_gtk3.h"
#include "debug_gtk3.h"
#include "machine.h"
#include "diskimage.h"
#include "romset.h"
#include "lib.h"
#include "ui.h"

#include "romsetmanagerwidget.h"


/** \brief  Object to hold button data
 */
typedef struct romset_button_s {
    const char *label;  /**< button label */
    void (*callback)(GtkWidget *, gpointer);    /**< callback for the 'clicked'
                                                     event */
} romset_button_t;


/*
 * Forward declarations of functions
 */
static void not_implemented(GtkWidget *widget, gpointer data);
static void on_load_archive(GtkWidget *widget, gpointer data);
static GtkTreeStore *create_tree_model(void);


/** \brief  List of buttons to manipulate the ROM sets
 */
static const romset_button_t romset_buttons[] = {
    { "Load set",       on_load_archive },
    { "Save set",       not_implemented },
    { "Select item",    not_implemented },
    { "Add item",       not_implemented },
    { "Delete item",    not_implemented },
    { NULL, NULL }
};


/*
 * References to widgets
 */
static GtkWidget *romset_predefs = NULL;
static GtkWidget *romset_listing = NULL;
static GtkTreeStore *romset_model = NULL;
static GtkWidget *romset_view = NULL;


/** \brief  Temporary event handler for buttons
 */
static void not_implemented(GtkWidget *widget, gpointer data)
{
    vice_gtk3_message_info("Whoops!", "Sorry, not implemented yet.");
}


/** \brief  Handler for the 'show current' button
 *
 * \param[in]   widget  button
 * \param[in]   data    unused
 */
static void on_show_current_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *view;
    GtkTextBuffer *buffer;
    char *list;

    view = gtk_bin_get_child(GTK_BIN(romset_listing));
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    list = machine_romset_file_list();

    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer), list, -1);

    lib_free(list);

}

/** \brief  Handler for the 'load' button
 *
 * \param[in]   widget  button
 * \param[in]   data    unused
 */
static void on_load_archive(GtkWidget *widget, gpointer data)
{
    gchar *filename;
    const char *patterns[] = { "*.vra", NULL };

    filename = vice_gtk3_open_file_dialog("Load ROM set archive",
            "ROM sets", patterns, NULL);
    if (filename != NULL) {
        if (romset_archive_load(filename, 0) < 0) {
            vice_gtk3_message_error("VICE core",
                    "Loading rom set '%s' failed", filename);
        } else {
            /* (re)create tree model) */
            romset_model = create_tree_model();
            gtk_tree_view_set_model(GTK_TREE_VIEW(romset_view),
                    GTK_TREE_MODEL(romset_model));
        }
        g_free(filename);
    }
}


/** \brief  Handler for the 'changed' event of the predefined ROM sets combo
 *
 * \param[in]   widget  combo box
 * \param[in]   data    extra event data (unused)
 */
static void on_predef_changed(GtkWidget *widget, gpointer data)
{
    const gchar *id;

    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
    if (id != NULL) {
        debug_gtk3("Setting ROM set to '%s'.", id);
        machine_romset_file_load(id);

        /* TODO: use a callback to update the ROM widgets when loading has
         *       succeeded
         */
    }
}


/** \brief  Create combo box with predefined ROM sets
 *
 * \param[in]   sets    list of predefined ROM sets
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_predef_romsets_widget(
        const vice_gtk3_combo_entry_str_t *sets)
{
    GtkWidget *combo;

    combo = gtk_combo_box_text_new();
    if (sets == NULL) {
        gtk_widget_set_sensitive(combo, FALSE);
    } else {
        /* add predefined sets */
        int i;

        for (i = 0; sets[i].name != NULL; i++) {
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                    sets[i].id, sets[i].name);
        }

        g_signal_connect(combo, "changed", G_CALLBACK(on_predef_changed), NULL);
    }
    return combo;
}


/** \brief  Create widget to show a ROM set listing
 *
 * \todo    Maybe later turn into a list view, so the resource names and their
 *          values can be displayed a little nicer.
 *
 * \return  GtkScrolledWindow with a GtkTextView inside
 */
static GtkWidget *create_listing_widget(void)
{
    GtkWidget *scroll;
    GtkWidget *view;

    view = gtk_text_view_new();
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(view), TRUE);

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll), view);
    gtk_widget_show_all(scroll);

    return scroll;
}


/** \brief  (Re)create tree model
 *
 * \return  GtkTreeStore
 */
static GtkTreeStore *create_tree_model(void)
{
    GtkTreeStore *model;
    int i;
    int num;

    model = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_STRING);

    /* populate if ROM sets are already defined */
    num = romset_archive_get_number();
    for (i = 0; i < num; i++) {
        const char *name;
        GtkTreeIter iter;

        name = romset_archive_get_item(i);
        gtk_tree_store_append(model, &iter, NULL);
        gtk_tree_store_set(model, &iter, 0, name, -1);

        /* TODO: add child nodes
         *
         * This either needs parsing a string (no!), or adding some
         * functionality to romset.c.
         */
    }

    return model;
}


/** \brief  Create the tree view for the ROM sets
 *
 * \param   [in]    model   tree model
 *
 * \return  GtkTreeView
 */
static GtkWidget *create_tree_view(GtkTreeStore *model)
{
    GtkWidget *view;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *res_column;
    GtkTreeViewColumn *val_column;

    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));

    renderer = gtk_cell_renderer_text_new();

    res_column = gtk_tree_view_column_new_with_attributes(
            "item-name",
            renderer,
            "text", 0,
            NULL);
    val_column = gtk_tree_view_column_new_with_attributes(
            "item-value",
            renderer,
            "text", 1,
            NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), res_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), val_column);

    return view;
}


/** \brief  Create a grid with buttons
 *
 * \return  GtkGrid
 */
static GtkWidget *create_buttons(void)
{
    GtkWidget *grid;
    int i;

    grid = gtk_grid_new();

    for (i = 0; romset_buttons[i].label != NULL; i++) {
        GtkWidget *button = gtk_button_new_with_label(romset_buttons[i].label);
        if (romset_buttons[i].callback != NULL) {
            g_signal_connect(button, "clicked",
                    G_CALLBACK(romset_buttons[i].callback), NULL);
        }
        gtk_grid_attach(GTK_GRID(grid), button, 0, i, 1, 1);
    }
    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create the romset manager widget
 *
 * \param[in]   predefs list of predefined ROM sets (optional)
 *
 * \return  GtkGrid
 */
GtkWidget *romset_manager_widget_create(
        const vice_gtk3_combo_entry_str_t *predefs)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *scroll;
    GtkWidget *buttons;
    GtkWidget *current_button;
    int row = 0;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Predefined ROM sets</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
       gtk_grid_attach(GTK_GRID(grid), label, 0, row, 2, 1);
    row++;

    romset_predefs = create_predef_romsets_widget(predefs);
    g_object_set(romset_predefs, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), romset_predefs, 0, row, 2, 1);
    row++;

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>ROM set listing</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 2, 1);
    row++;

    romset_listing = create_listing_widget();
    gtk_widget_set_size_request(romset_listing, -1, 150);
    gtk_widget_set_hexpand(romset_listing, TRUE);
    g_object_set(romset_listing, "margin-left", 16, NULL);

    gtk_grid_attach(GTK_GRID(grid), romset_listing, 0, row, 1, 1);

    current_button = gtk_button_new_with_label("Show current");
    gtk_widget_set_valign(current_button, GTK_ALIGN_START);
    gtk_widget_set_hexpand(current_button, FALSE);
    g_signal_connect(current_button, "clicked",
            G_CALLBACK(on_show_current_clicked), NULL);
    gtk_grid_attach(GTK_GRID(grid), current_button, 1, row, 1, 1);
    row++;

    romset_model = create_tree_model();
    romset_view = create_tree_view(romset_model);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>ROM set management</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 2, 1);
    row++;

    /* pack the tree view into a GtkScrolledWindow */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    g_object_set(scroll, "margin-left", 16, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);

    gtk_container_add(GTK_CONTAINER(scroll), romset_view);
    gtk_widget_show_all(scroll);

    gtk_grid_attach(GTK_GRID(grid), scroll, 0, row, 1, 1);

    buttons = create_buttons();
    gtk_grid_attach(GTK_GRID(grid), buttons, 1, row, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
