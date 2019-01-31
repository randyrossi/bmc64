/** \file   ciamodelwidget.c
 * \brief   Widget to set the CIA model
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES CIA1Model   x64 x64sc xscpu64 x64dtv x128 xcbm5x0 xcbm2 vsid
 * $VICERES CIA2Model   x64 x64sc xscpu64 x64dtv x128 vsid
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
#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "cia.h"
#include "machine.h"
#include "machinemodelwidget.h"

#include "ciamodelwidget.h"


static const vice_gtk3_radiogroup_entry_t cia_models[] = {
    { "6526 (old)", CIA_MODEL_6526 },
    { "8521 (new)", CIA_MODEL_6526A },
    { NULL, -1 }
};

/** \brief  Reference to machine model widget
 *
 * Used to update the machine model widget when a CIA model is changed
 */
static GtkWidget *machine_widget = NULL;


/** \brief  CIA1 model widget
 */
static GtkWidget *cia1_widget = NULL;


/** \brief  CIA2 model widget
 */
static GtkWidget *cia2_widget = NULL;


/** \brief  Create a widget for CIA \a num
 *
 * \param[in]   num     CIA number (1 or 2)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cia_widget(int num)
{
    GtkWidget *grid;
    GtkWidget *radio_group;
    GtkWidget *label;
    char buffer[256];

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    g_snprintf(buffer, 256, "CIA%d", num);
    label = gtk_label_new(buffer);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new_sprintf(
            "CIA%dModel", cia_models, GTK_ORIENTATION_HORIZONTAL, num);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create CIA model(s) widget
 *
 * Creates a CIA model widget for either one or two CIA's.
 * The \a machine_model_widget argument is used to update the machine model
 * widget when a CIA model changes.
 *
 * \param[in,out]   machine_model_widget    machine model widget
 * \param[in]       count                   number of CIA's (1 or 2)
 *
 * \return  GtkGrid
 */
GtkWidget *cia_model_widget_create(GtkWidget *machine_model_widget, int count)
{
    GtkWidget *grid;

    machine_widget = machine_model_widget;

    grid = uihelpers_create_grid_with_label("CIA Model", 1);

    cia1_widget = create_cia_widget(1);
    gtk_grid_attach(GTK_GRID(grid), cia1_widget, 0, 1, 1, 1);
    if (count > 1) {
        cia2_widget = create_cia_widget(2);
        gtk_grid_attach(GTK_GRID(grid), cia2_widget, 0, 2, 1, 1);
    }
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update CIA model widget
 *
 * Only updates the widget if the widget is out of sync with the resources,
 * this will avoid having the machine model widget and the CIA model widget
 * triggering each others' event handlers.
 *
 * \note    This will break if the CIA_MODEL_6526(A) defines are changed
 *
 * \param[in,out]   widget  CIA model widget
 */
void cia_model_widget_update(GtkWidget *widget)
{
    int old_model;
    int new_model;
    GtkWidget *radio;

    /* CIA1 */
    radio = gtk_grid_get_child_at(GTK_GRID(cia1_widget), 1, 0);
    old_model = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))
        ? CIA_MODEL_6526 : CIA_MODEL_6526A;
    resources_get_int("CIA1Model", &new_model);
    if (old_model != new_model) {
        radio = gtk_grid_get_child_at(GTK_GRID(cia1_widget), 1 + new_model, 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
    }

    if (cia2_widget == NULL) {
        return;
    }

    /* CIA2 */
    radio = gtk_grid_get_child_at(GTK_GRID(cia2_widget), 1, 0);
    old_model = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))
        ? CIA_MODEL_6526 : CIA_MODEL_6526A;
    resources_get_int("CIA2Model", &new_model);
    if (old_model != new_model) {
        radio = gtk_grid_get_child_at(GTK_GRID(cia2_widget), 1 + new_model, 0);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
    }
}
