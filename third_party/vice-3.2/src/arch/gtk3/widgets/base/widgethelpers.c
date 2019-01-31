/**
 * \brief   Helpers for creating Gtk3 widgets
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * This file is supposed to contain some helper functions for boiler plate
 * code, such as creating layout widgets, creating lists of radio or check
 * boxes, etc.
 *
 * \todo    turn the margin/padding values into defines and move into a file
 *          like uidefs.h (partially done, \a see vice_gtk3_settings.h)
 *
 * \todo    rename/replace functions (partially done)
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

#include "lib.h"
#include "resources.h"
#include "vsync.h"

#include "vice_gtk3_settings.h"
#include "debug_gtk3.h"

#include "widgethelpers.h"


/** \brief  Size of the buffer used for snprintf() calls to generate labels
 */
#define LABEL_BUFFER_SIZE   256


/** \brief  Create a GtkGrid with a bold GtkLabel as its first widget
 *
 * This creates a GtkGrid with a left-aligned, bold label, optionally spread
 * over multiple columns. If you don't know what pass as the \a columns
 * argument, just pass 1.
 *
 * \note    Deprecated in favour of vice_gtk3_grid_new_spaced_with_label()
 *
 * \param[in]   text    label text
 * \param[in]   columns number of columns in the grid the label should span
 *
 * \return  GtkGrid with a label
 */
GtkWidget *uihelpers_create_grid_with_label(const gchar *text, gint columns)
{
    GtkWidget *grid;
    GtkWidget *label;
    gchar buffer[LABEL_BUFFER_SIZE];

    /* sanitize columns input */
    if (columns < 1) {
        columns = 1;
    }

    /* use HTML-ish markup to make the label bold */
    g_snprintf(buffer, LABEL_BUFFER_SIZE, "<b>%s</b>", text);

    grid = gtk_grid_new();
    g_object_set(grid, "margin", 8, NULL);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), buffer);
    gtk_widget_set_halign(label, GTK_ALIGN_START);    /* align left */
    g_object_set(label, "margin-bottom", 8, NULL);  /* add 8 units of margin
                                                       to the bottom */
    gtk_widget_show(label);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, columns, 1);
    return grid;
}

/** \brief  Create a GtkGrid with a label radio buttons with text/id pairs
 *
 * \param[in]   label       label text
 * \param[in]   data        text/value pairs for the radio buttons
 * \param[in]   callback    optional callback to trigger when a radio button
 *                          is toggled (`NULL` == no callback)
 *
 * \note    keep in mind that the callback is also triggered when a radio button
 *          is deactivated, so use gtk_toggle_button_get_active() if you only
 *          want to respond to the currently activated radio button
 *
 * \return  GtkGrid
 */
GtkWidget *uihelpers_radiogroup_create(
        const gchar *label,
        const vice_gtk3_radiogroup_entry_t *data,
        void (*callback)(GtkWidget *, gpointer),
        int active)
{
    GtkWidget *grid;
    GtkRadioButton *last;
    GSList *group = NULL;
    size_t i;

    debug_gtk3("DEPRECATED in favour of vice_gtk3_resource_radiogroup!\n");
    grid = uihelpers_create_grid_with_label(label, 1);

    last = NULL;
    for (i = 0; data[i].name != NULL; i++) {
        GtkWidget *radio = gtk_radio_button_new_with_label(group, data[i].name);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
        gtk_grid_attach(GTK_GRID(grid), radio, 0, i + 1, 1, 1);
        g_object_set(radio, "margin-left", 16, NULL);  /* indent 16 units */

        if (active == i) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
        }

        if (callback != NULL) {
            g_signal_connect(radio, "toggled", G_CALLBACK(callback),
                    GINT_TO_POINTER(data[i].id));
        }
        gtk_widget_show(radio);
        last = GTK_RADIO_BUTTON(radio);
    }
    g_object_set(grid, "margin", 8, NULL);
    gtk_widget_show(grid);
    return grid;
}


/** \brief  Get index of \a value in \a list
 *
 * Get the index in \a list for \a value. This function is required for custom
 * radiogroups that add 'unknown' options or something similar.
 *
 * \param[in]   list    radio button group array
 * \param[in]   value   value to find in \a list
 *
 * \return  index of \a value or -1 when not found
 */
int vice_gtk3_radiogroup_get_list_index(
        const vice_gtk3_radiogroup_entry_t *list,
        int value)
{
    int i;

    for (i = 0; list[i].name != NULL; i++) {
        if (list[i].id == value) {
            return i;
        }
    }
    return -1;
}


/** \brief  Set a radio button to active in a GktGrid
 *
 * This function only checks for radio buttons in the first row of the \a grid,
 * so it works fine with widgets created through
 * uihelpers_uihelpers_create_int_radiogroup_with_label(), but not much else.
 * So it might need some refactoring
 *
 * \param[in]   grid    GtkGrid containing radio buttons
 * \param[in[   index   index of the radio button (the actual index of the
 *                      radio button, other widgets are skipped)
 */
void vice_gtk3_radiogroup_set_index(GtkWidget *grid, int index)
{
    GtkWidget *radio;
    int row = 0;
    int radio_index = 0;


    debug_gtk3("Looking for index %d\n", index);

    if (index < 0) {
        return;
    }

    do {
        radio = gtk_grid_get_child_at(GTK_GRID(grid), 0, row);
        if (GTK_IS_TOGGLE_BUTTON(radio)) {
            debug_gtk3("got toggle button at row %d\n", row);
            if (radio_index == index) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
                return;
            }
            radio_index++;
        }
        row++;
    } while (radio != NULL);
}


/** \brief  Create a left-aligned, 16 units indented label
 *
 * XXX: This function is of little use an should probably be removed in favour
 *      of something a little more flexible.
 *
 * \param[in]   text    label text
 *
 * \return  label
  */
GtkWidget *vice_gtk3_create_indented_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    return label;
}


/** \brief  Create a new `GtkGrid`, setting column and row spacing
 *
 * \param[in]   column_spacing  column spacing (< 0 to use default)
 * \param[in]   row_spacing     row spacing (< 0 to use default)
 *
 * \return  new `GtkGrid` instance
 */
GtkWidget *vice_gtk3_grid_new_spaced(int column_spacing, int row_spacing)
{
    GtkWidget *grid = gtk_grid_new();

    gtk_grid_set_column_spacing(GTK_GRID(grid),
            column_spacing < 0 ? VICE_GTK3_GRID_COLUMN_SPACING : column_spacing);
    gtk_grid_set_row_spacing(GTK_GRID(grid),
            row_spacing < 0 ? VICE_GTK3_GRID_ROW_SPACING : row_spacing);
    return grid;
}


/** \brief  Create a new `GtkGrid` with label, setting column and row spacing
 *
 * \param[in]   column_spacing  column spacing (< 0 to use default)
 * \param[in]   row_spacing     row spacing (< 0 to use default)
 * \param[in]   label           label text
 * \param[in]   span            number of columns for the \a label to span
 *
 * \return  new `GtkGrid` instance
 */
GtkWidget *vice_gtk3_grid_new_spaced_with_label(int column_spacing,
                                                int row_spacing,
                                                const char *label,
                                                int span)
{
    GtkWidget *grid = vice_gtk3_grid_new_spaced(column_spacing, row_spacing);
    GtkWidget *lbl = gtk_label_new(NULL);
    char *temp;

    if (span <= 0) {
        span = 1;
    }

    /* create left-indented bold label */
    temp = lib_msprintf("<b>%s</b>", label);
    gtk_label_set_markup(GTK_LABEL(lbl), temp);
    gtk_widget_set_halign(lbl, GTK_ALIGN_START);
    /* g_object_set(lbl, "margin-bottom", 8, NULL); */
    lib_free(temp);

    /* attach label */
    gtk_grid_attach(GTK_GRID(grid), lbl, 0, 0, span, 1);
    gtk_widget_show(grid);
    return grid;
}



