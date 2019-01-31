/** \file   petmiscwidget.c
 * \brief   Widget to set the PET Crtc and EoiBlank resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Crtc        xpet
 *  (enable/disable CRTC 6546 emulation for models 40xx and above)
 * $VICERES EoiBlank    xpet
 *  (enable/disable "blank screen on EOI" (oldest PET 2001))
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

#include "petmiscwidget.h"


static GtkWidget *crtc_widget = NULL;
static GtkWidget *blank_widget = NULL;


/** \brief  Handler for the "toggled" event of the CRTC check button
 *
 * Sets the Crtc resource when it has been changed.
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   extra data (unused)
 */
static void on_crtc_toggled(GtkWidget *widget, gpointer user_data)
{
    int old_val;
    int new_val;

    resources_get_int("Crtc", &old_val);
    new_val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    if (new_val != old_val) {
        debug_gtk3("setting Crtc to %s\n", new_val ? "ON" : "OFF");
        resources_set_int("Crtc", new_val);
    }
}


/** \brief  Handler for the "toggled" event of the EOI Blank check button
 *
 * Sets the EoiBlank resource when it has been changed.
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   extra data (unused)
 */
static void on_blank_toggled(GtkWidget *widget, gpointer user_data)
{
    int old_val;
    int new_val;

    resources_get_int("EoiBlank", &old_val);
    new_val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    if (new_val != old_val) {
        debug_gtk3("setting EoiBlank to %s\n", new_val ? "ON" : "OFF");
        resources_set_int("EoiBlank", new_val);
    }
}


/** \brief  Create PET miscellaneous settings widget
 *
 * Adds check buttons for the Crtc and EoiBlank resources.
 *
 * \return  GtkGrid
 */
GtkWidget *pet_misc_widget_create(void)
{
    GtkWidget *grid;
    int crtc;
    int blank;

    resources_get_int("Crtc", &crtc);
    resources_get_int("EoiBlank", &blank);

    grid = uihelpers_create_grid_with_label("Miscellaneous", 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    crtc_widget = gtk_check_button_new_with_label("CRTC chip enable");
    g_object_set(crtc_widget, "margin-left", 16, NULL);
    blank_widget = gtk_check_button_new_with_label("EOI blanks screen");
    gtk_grid_attach(GTK_GRID(grid), crtc_widget, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), blank_widget, 1, 1, 1, 1);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(crtc_widget), crtc);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(blank_widget), blank);

    g_signal_connect(crtc_widget, "toggled", G_CALLBACK(on_crtc_toggled), NULL);
    g_signal_connect(blank_widget, "toggled", G_CALLBACK(on_blank_toggled), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
