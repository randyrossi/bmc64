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
 *
 * XXX: Not sure this is needed anymore since any model changes/updates go
 *      through callbacks/event handlers.
 */
static GtkWidget *machine_widget = NULL;

static GtkWidget *cia1_group;
static GtkWidget *cia2_group;

static void (*cia_model_callback)(int, int);



/** \brief  Handler for the radiogroup callbacks for the CIA models
 *
 * Triggers the callback set with cia_model_widget_set_callback()
 *
 * \param[in]   widget  CIA radiogroup widget triggering the event
 * \param[in]   model   CIA model ID
 */
static void on_cia_model_callback_internal(GtkWidget *widget, int model)
{
    debug_gtk3("got value %d.", model);
    if (cia_model_callback != NULL) {
        cia_model_callback(widget == cia1_group ? 1 : 2, model);
    }
}




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
    vice_gtk3_resource_radiogroup_add_callback(radio_group,
            on_cia_model_callback_internal);
    if (num == 1) {
        cia1_group = radio_group;
    } else {
        cia2_group = radio_group;
    }
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
    GtkWidget *cia1_widget;
    GtkWidget *cia2_widget;

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


/** \brief  Synchronize CIA model widgets with their current resources
 *
 * Only updates the widget if the widget is out of sync with the resources,
 * this will avoid having the machine model widget and the CIA model widget
 * triggering each others' event handlers.
 *
 * \param[in,out]   widget  CIA model widget
 */
void cia_model_widget_sync(GtkWidget *widget)
{
    if (cia1_group != NULL) {
        vice_gtk3_resource_radiogroup_sync(cia1_group);
    }
    if (cia2_group != NULL) {
        vice_gtk3_resource_radiogroup_sync(cia2_group);
    }
}


void cia_model_widget_set_callback(GtkWidget *widget,
                                   void (*func)(int cia_num, int cia_model))
{
    cia_model_callback = func;
}
