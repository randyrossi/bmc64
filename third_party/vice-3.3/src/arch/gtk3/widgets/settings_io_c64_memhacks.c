/** \file   settings_io_c64_memhacks.c
 * \brief   Widget to control C64 memory expansion hacks
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MemoryHack          x64 x64sc
 * $VICERES C64_256Kbase        x64 x64sc
 * $VICERES C64_256Kfilename    x64 x64sc
 * $VICERES PLUS60Kbase         x64 x64sc
 * $VICERES PLUS60Kfilename     x64 x64sc
 * $VICERES PLUS256Kfilename    x64 x64sc
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
 * FIXME; the enabling/disabling of the various memory hack widgets can be done
 *        a little less hackish -- compyx
 */

#include "vice.h"
#include <gtk/gtk.h>

#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "openfiledialog.h"
#include "c64-memory-hacks.h"
#include "resourcewidgetmanager.h"
#include "uisettings.h"

#include "settings_io_c64_memhacks.h"


/** \brief  List of C64 memory hack devices
 */
static const vice_gtk3_radiogroup_entry_t mem_hack_devices[] = {
    { "None",       MEMORY_HACK_NONE },
    { "C64 256K",   MEMORY_HACK_C64_256K },
    { "+60K",       MEMORY_HACK_PLUS60K },
    { "+256K",      MEMORY_HACK_PLUS256K },
    { NULL, -1 }
};

/**\brief   List of I/O base addresses for the C64_256K memory hack
 */
static const vice_gtk3_radiogroup_entry_t c64_256k_base_addresses[] = {
    { "$DE00-$DE7F", 0xde00 },
    { "$DE80-$DEFF", 0xde80 },
    { "$DF00-$DF7F", 0xdf00 },
    { "$DF80-$DFFF", 0xdf80 },
    { NULL, -1 }
};

/**\brief   List of I/O base addresses for the +60K memory hack
 */
static const vice_gtk3_radiogroup_entry_t plus_60k_base_addresses[] = {
    { "$D040", 0xd040 },
    { "$D100", 0xd100 },
    { NULL, -1 }
};


/*
 * References to widget that need to be enabled/disabled, depending on the 
 * memory expansion hack selected
 */
static GtkWidget *c64_256k_base = NULL;
static GtkWidget *c64_256k_image = NULL;
static GtkWidget *plus_60k_base = NULL;
static GtkWidget *plus_60k_image = NULL;
static GtkWidget *plus_256k_image = NULL;


/** \brief  Resource widget manager instance
 */
static resource_widget_manager_t manager;


/** \brief  Clean up resources used by the main widget
 *
 * \param[in]   widget  main widget (unused)
 * \param[in]   data    extra event data (unused
 */
static void on_main_widget_destroy(GtkWidget *widget, gpointer data)
{
    vice_resource_widget_manager_exit(&manager);
}


/** \brief  Handler for the "clicked" event of the browse button for C64_256K
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_256k_image_browse_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Select 256K image file", NULL,
            NULL, NULL);
    if (filename != NULL) {
        GtkWidget *grid = gtk_widget_get_parent(button);
        GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Handler for the "clicked" event of the browse button for +60K
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_plus60k_image_browse_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Select +60K image file", NULL,
            NULL, NULL);
    if (filename != NULL) {
        GtkWidget *grid = gtk_widget_get_parent(button);
        GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Handler for the "clicked" event of the browse button for +256K
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_plus256k_image_browse_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Select +256K image file", NULL,
            NULL, NULL);
    if (filename != NULL) {
        GtkWidget *grid = gtk_widget_get_parent(button);
        GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Extra handler for the "toggled" event of the mem hack radio buttons
 *
 * This handler enables/disables the widgets related to the memory hack selected
 *
 * \param[in]   widget      radio button of the select hack
 * \param[in]   user_data   hack ID
 */
static void on_hack_toggled(GtkWidget *widget, gpointer user_data)
{
    int hack_id = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        debug_gtk3("hack %d selected.", hack_id);

        switch (hack_id) {
            case MEMORY_HACK_NONE:
                gtk_widget_set_sensitive(c64_256k_base, FALSE);
                gtk_widget_set_sensitive(c64_256k_image, FALSE);
                gtk_widget_set_sensitive(plus_60k_base, FALSE);
                gtk_widget_set_sensitive(plus_60k_image, FALSE);
                gtk_widget_set_sensitive(plus_256k_image, FALSE);
                break;
            case MEMORY_HACK_C64_256K:
                gtk_widget_set_sensitive(c64_256k_base, TRUE);
                gtk_widget_set_sensitive(c64_256k_image, TRUE);
                gtk_widget_set_sensitive(plus_60k_base, FALSE);
                gtk_widget_set_sensitive(plus_60k_image, FALSE);
                gtk_widget_set_sensitive(plus_256k_image, FALSE);
                break;
            case MEMORY_HACK_PLUS60K:
                gtk_widget_set_sensitive(c64_256k_base, FALSE);
                gtk_widget_set_sensitive(c64_256k_image, FALSE);
                gtk_widget_set_sensitive(plus_60k_base, TRUE);
                gtk_widget_set_sensitive(plus_60k_image, TRUE);
                gtk_widget_set_sensitive(plus_256k_image, FALSE);
                break;
            case MEMORY_HACK_PLUS256K:
                gtk_widget_set_sensitive(c64_256k_base, FALSE);
                gtk_widget_set_sensitive(c64_256k_image, FALSE);
                gtk_widget_set_sensitive(plus_60k_base, FALSE);
                gtk_widget_set_sensitive(plus_60k_image, FALSE);
                gtk_widget_set_sensitive(plus_256k_image, TRUE);
                break;
            default:
                /* shouldn't get here */
                break;
        }
    }
}


/** \brief  Create widget to select the memory hacks device
 *
 * \return  GtkGrid
 */
static GtkWidget *memory_hacks_device_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    size_t i = 0;

    grid = uihelpers_create_grid_with_label(
            "C64 memory expansion hack device", 1);
    group = vice_gtk3_resource_radiogroup_new("MemoryHack", mem_hack_devices,
            GTK_ORIENTATION_HORIZONTAL);
    vice_resource_widget_manager_add_widget(&manager, group, NULL,
            NULL, NULL, NULL);

    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);

    /* now set up extra event handlers on the radio buttons to be able to
     * enable/disable widgets */
    while (i < (sizeof mem_hack_devices / sizeof mem_hack_devices[0]) - 1) {
        GtkWidget *radio = gtk_grid_get_child_at(GTK_GRID(group), i, 0);
        if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)) {
            g_signal_connect(radio, "toggled", G_CALLBACK(on_hack_toggled),
                    GINT_TO_POINTER(mem_hack_devices[i].id));
        }
        i++;
    }

    return grid;
}


/** \brief  Create widget to set the I/O base of the C64_256K expansion
 *
 * \return  GtkGrid
 */
static GtkWidget *c64_256k_base_address_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = uihelpers_create_grid_with_label(
            "C64 256K base addresss", 1);
    group = vice_gtk3_resource_radiogroup_new("C64_256Kbase",
            c64_256k_base_addresses,
            GTK_ORIENTATION_HORIZONTAL);
    vice_resource_widget_manager_add_widget(&manager, group, NULL,
            NULL, NULL, NULL);

    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to set the I/O base of the +60k expansion
 *
 * \return  GtkGrid
 */
static GtkWidget *plus_60k_base_address_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = uihelpers_create_grid_with_label(
            "+60K base addresss", 1);
    group = vice_gtk3_resource_radiogroup_new(
            "PLUS60Kbase", plus_60k_base_addresses,
            GTK_ORIENTATION_HORIZONTAL);
    vice_resource_widget_manager_add_widget(&manager, group, NULL,
            NULL, NULL, NULL);

    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to set the C64_256K image file
 *
 * \return  GtkGrid
 */
static GtkWidget *c64_256k_image_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;

    grid = uihelpers_create_grid_with_label("C64 256K image file", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("filename");
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new("C64_256Kfilename");
    vice_resource_widget_manager_add_widget(&manager, entry, NULL,
            NULL, NULL, NULL);
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(browse, "clicked",
            G_CALLBACK(on_256k_image_browse_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to set the +60K image file
 *
 * \return  GtkGrid
 */
static GtkWidget *plus_60k_image_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;

    grid = uihelpers_create_grid_with_label("+60K image file", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("filename");
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new("PLUS60Kfilename");
    vice_resource_widget_manager_add_widget(&manager, entry, NULL,
            NULL, NULL, NULL);
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(browse, "clicked",
            G_CALLBACK(on_plus60k_image_browse_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to set the +256K image file
 *
 * \return  GtkGrid
 */
static GtkWidget *plus_256k_image_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;

    grid = uihelpers_create_grid_with_label("+256K image file", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("filename");
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new("PLUS256Kfilename");
    vice_resource_widget_manager_add_widget(&manager, entry, NULL,
            NULL, NULL, NULL);

    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(browse, "clicked",
            G_CALLBACK(on_plus256k_image_browse_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget controlling C64 memory hacks
 *
 * \param[in]   parent  parent widhet, used for dialog
 *
 * \return  GtkGrid
 */
GtkWidget *settings_io_c64_memhacks_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *hack;
    size_t i = 0;

    vice_resource_widget_manager_init(&manager);
    ui_settings_set_resource_widget_manager(&manager);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    hack = memory_hacks_device_widget_create();
    gtk_grid_attach(GTK_GRID(grid), hack, 0, 1, 1, 1);

    c64_256k_base = c64_256k_base_address_widget_create();
    gtk_grid_attach(GTK_GRID(grid), c64_256k_base, 0, 2, 1, 1);
    c64_256k_image = c64_256k_image_widget_create();
    gtk_grid_attach(GTK_GRID(grid), c64_256k_image, 0, 3, 1, 1);
    plus_60k_base = plus_60k_base_address_widget_create();
    gtk_grid_attach(GTK_GRID(grid), plus_60k_base, 0, 4, 1, 1);
    plus_60k_image = plus_60k_image_widget_create();
    gtk_grid_attach(GTK_GRID(grid), plus_60k_image, 0, 5, 1, 1);
    plus_256k_image = plus_256k_image_widget_create();
    gtk_grid_attach(GTK_GRID(grid), plus_256k_image, 0, 6, 1, 1);

    /* enable/disable the widgets depending on the hack selected */
    while (i < (sizeof mem_hack_devices / sizeof mem_hack_devices[0]) - 1) {
        GtkWidget *group = gtk_grid_get_child_at(GTK_GRID(hack), 0, 1);
        GtkWidget *radio = gtk_grid_get_child_at(GTK_GRID(group), i, 0);
        if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)
                && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
            /* trigger the extra event handler to set the proper widgets */
            on_hack_toggled(radio, GINT_TO_POINTER(mem_hack_devices[i].id));
            break;
        }
        i++;
    }

    g_signal_connect(grid, "destroy", G_CALLBACK(on_main_widget_destroy),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}
