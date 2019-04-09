/** \file   settings_sampler.c
 * \brief   Widget to control sampler settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SamplerDevice
 * $VICERES SamplerGain
 * $VICERES SampleName
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
#include "resources.h"
#include "sampler.h"
#include "openfiledialog.h"

#include "settings_sampler.h"


/** \brief  Function to retrieve the list of sampler input devices
 */
static sampler_device_t *(*devices_getter)(void) = NULL;


/** \brief  Reference to the text entry
 *
 * Used by the "browse" button callback to set the new file name and trigger
 * a resource update
 */
static GtkWidget *entry_widget = NULL;


/** \brief  Reference to the "browse" button
 */
static GtkWidget *browse_button = NULL;


/** \brief  Handler for the "changed" event of the devices combo box
 *
 * \param[in]   combo       combo box with devices
 * \param[in]   user_data   extra data (unused)
 */
static void on_device_changed(GtkComboBoxText *combo, gpointer user_data)
{
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

    debug_gtk3("setting SamplerDevice to %d.", index);
    resources_set_int("SamplerDevice", index);

    /* this assumes the "media file input" is always first in the list */
    gtk_widget_set_sensitive(entry_widget, index == 0);
    gtk_widget_set_sensitive(browse_button, index == 0);
}


/** \brief  Handler for the "value-changed" event of the gain slider
 *
 * \param[in]   scale       gain slider
 * \param[in]   user_data   extra data (unused)
 */
static void on_gain_changed(GtkScale *scale, gpointer user_data)
{
    int value = (int)gtk_range_get_value(GTK_RANGE(scale));

    debug_gtk3("setting SamplerGain to %d.", value);
    resources_set_int("SamplerGain", value);
}


/** \brief  Handler for the "changed" event of the text entry box
 *
 * \param[in]   entry       input file text box
 * \param[in]   user_data   extra data (unused)
 */
static void on_entry_changed(GtkEntry *entry, gpointer user_data)
{
    const char *text;

    text = gtk_entry_get_text(entry);
    debug_gtk3("setting SampleName to '%s'.", text);
    resources_set_string("SampleName", text);
}


/** \brief  Handler for the "clicked" event of the "browse" button
 *
 * \param[in]   widget      browse button
 * \param[in]   user_data   extra data (unused)
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Select input file", NULL, NULL, NULL);
    if (filename != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry_widget), filename);
        g_free(filename);
    }
}


/** \brief  Create combo box for the devices list
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_device_widget(void)
{
    GtkWidget *combo;
    sampler_device_t *devices;
    int current;
    int i;

    resources_get_int("SamplerDevice", &current);

    combo = gtk_combo_box_text_new();
    if (devices_getter != NULL) {
        devices = devices_getter();
    } else {
        return combo;
    }
    for (i = 0; devices[i].name != NULL; i++) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                devices[i].name, devices[i].name);
        if (i ==  current) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
        }
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_device_changed), NULL);
    return combo;
}


/** \brief  Create slider for the gain
 *
 * \return  GtkScale
 */
static GtkWidget *create_gain_widget(void)
{
    GtkWidget *scale;
    GtkWidget *label;
    int value;
    int i;

    label = gtk_label_new("Sampler gain");
    g_object_set(label, "margin-left",16, NULL);

    scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,
            0.0, 200.0, 25.0);
    gtk_scale_set_digits(GTK_SCALE(scale), 0);

    /* add tick marks */
    for (i = 0; i < 200; i += 25) {
        gtk_scale_add_mark(GTK_SCALE(scale), (gdouble)i, GTK_POS_BOTTOM, NULL);
    }


    if (resources_get_int("SamplerGain", &value) >= 0) {
        gtk_range_set_value(GTK_RANGE(scale), (gdouble)value);
    } else {
        gtk_range_set_value(GTK_RANGE(scale), 100.0);
    }

    g_signal_connect(scale, "value-changed", G_CALLBACK(on_gain_changed), NULL);

    gtk_widget_show_all(scale);
    return scale;
}


/** \brief  Create text entry for the input file name
 *
 * \return  GtkEntry
 */
static GtkWidget *create_input_entry(void)
{
    GtkWidget *entry;
    const char *text;

    resources_get_string("SampleName", &text);

    entry = gtk_entry_new();
    if (text != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), text);
    }

    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);
    return entry;
}


/** \brief  Create the "browse" button
 *
 * \return  GtkButton
 */
static GtkWidget *create_input_button(void)
{
    GtkWidget *button;

    button = gtk_button_new_with_label("Browse ...");
    g_signal_connect(button, "clicked", G_CALLBACK(on_browse_clicked), NULL);
    return button;
}


/** \brief  Set the function to retrieve the input devices list
 *
 * \param[in]   func    pointer to function to retrieve devices list
 */
void settings_sampler_set_devices_getter(sampler_device_t *(func)(void))
{
    devices_getter = func;
}


/** \brief  Create widget to control sampler settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_sampler_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *combo;
    int index;

    grid = uihelpers_create_grid_with_label("Sampler settings", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    /* sampler device list */
    label = gtk_label_new("Sampler device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    combo = create_device_widget();
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 1, 2, 1);

    /* sampler gain */
    label = gtk_label_new("Sampler gain");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_gain_widget(), 1, 2, 2, 1);

    /* sampler input file text entry and browse button */
    label = gtk_label_new("Sampler media file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    entry_widget = create_input_entry();
    gtk_widget_set_hexpand(entry_widget, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry_widget, 1, 3, 1, 1);
    browse_button = create_input_button();
    gtk_grid_attach(GTK_GRID(grid), browse_button, 2, 3, 1, 1);

    /* update sensitivity of entry and button */
    index = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));

    gtk_widget_set_sensitive(entry_widget, index == 0);
    gtk_widget_set_sensitive(browse_button, index == 0);

    gtk_widget_show_all(grid);
    return grid;
}
