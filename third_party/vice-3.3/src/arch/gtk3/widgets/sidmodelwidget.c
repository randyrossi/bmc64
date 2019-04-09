/** \file   sidmodelwidget.c
 * \brief   Widget to select SID model
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SidModel    all
 *  (all = if a SidCart is installed)
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
#include <glib/gstdio.h>

#include "basewidgets.h"
#include "lib.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"
#include "machinemodelwidget.h"
#include "mixerwidget.h"

#include "sidmodelwidget.h"


/** \brief  Empty list of SID models
 */
static const vice_gtk3_radiogroup_entry_t sid_models_none[] = {
    { NULL, -1 }
};


#if 0
/** \brief  All SID models
 */
static const vice_gtk3_radiogroup_entry_t sid_models_all[] = {
    { "6581", 0 },
    { "8580", 1 },
    { "8580D", 2 },
    { "6581R4", 3 },
    { "DTVSID", 4 },
    { NULL, -1 }
};
#endif

/** \brief  SID models used in the C64/C64SCPU, C128 and expanders for PET,
 *          VIC-20 and Plus/4
 */
static const vice_gtk3_radiogroup_entry_t sid_models_c64[] = {
    { "6581", 0 },
    { "8580", 1 },
    { "8580D", 2 },
    { NULL, -1 }
};


/** \brief  SID models used in the C64DTV
 */
static const vice_gtk3_radiogroup_entry_t sid_models_c64dtv[] = {
    { "DTVSID (ReSID-DTV)", 4 },
    { "6581", 0 },
    { "8580", 1 },
    { "8580D", 2 },
    { NULL, -1 }
};


/** \brief  SID models used in the CBM-II 510/520 models
 */
static const vice_gtk3_radiogroup_entry_t sid_models_cbm5x0[] = {
    { "6581", 0 },
    { "8580", 1 },
    { "8580D", 2 },
    { NULL, -1 }
};

/** \brief  Handler for the "toggled" event of the SID type radio buttons
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   sid type
 */
static void on_sid_model_toggled(GtkWidget *widget, int user_data)
{
    GtkWidget *parent;
    void (*callback)(int);


    if (widget == NULL) {
        debug_gtk3("WIDGET IS NUL!!!!!\n\n\n\n\n");
    }

    /* sync mixer widget */
    mixer_widget_sid_type_changed();

    parent = gtk_widget_get_parent(widget);
    callback = g_object_get_data(G_OBJECT(parent), "ExtraCallback");
    if (callback != NULL) {
        debug_gtk3("calling extra callback");
        callback(user_data);
    } else {
        debug_gtk3("No ExtraCallback!");
    }
}

/** \brief  Reference to the machine model widget
 *
 * Used to update the widget when the SID model changes
 */
static GtkWidget *machine_widget = NULL;


/** \brief  Create SID model widget
 *
 * Creates a SID model widget, depending on `machine_class`. Also sets a
 * callback to force an update of the 'machine model' widget.
 *
 * \param[in,out]   machine_model_widget    reference to machine model widget
 *
 * \return  GtkGrid
 */
GtkWidget *sid_model_widget_create(GtkWidget *machine_model_widget)
{
    GtkWidget *grid;
    GtkWidget *group;
    const vice_gtk3_radiogroup_entry_t *models;

    machine_widget = machine_model_widget;

    switch (machine_class) {

        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_VSID:
            models = sid_models_c64;
            break;

        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_CBM6x0:
            models = sid_models_cbm5x0;
            break;

        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_VIC20:
            models = sid_models_c64;
            break;

        case VICE_MACHINE_C64DTV:
            models = sid_models_c64dtv;
            break;

        default:
            /* shouldn't get here */
            models = sid_models_none;
    }

    grid = vice_gtk3_grid_new_spaced_with_label(VICE_GTK3_DEFAULT,
            VICE_GTK3_DEFAULT, "SID model", 1);
    group = vice_gtk3_resource_radiogroup_new(
            "SidModel", models, GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    vice_gtk3_resource_radiogroup_add_callback(group, on_sid_model_toggled);

    /* SID cards for the Plus4, PET or VIC20:
     *
     *  plus4: http://plus4world.powweb.com/hardware.php?ht=11
     *  pet  : http://www.cbmhardware.de/show.php?r=14&id=71/PETSID
     *  vic20: c64 cart adapter with any c64 sid cart
     *
     * check if Plus4/PET/VIC20 has a SID cart active:
     */
    if (machine_class == VICE_MACHINE_PLUS4
            || machine_class == VICE_MACHINE_PET
            || machine_class == VICE_MACHINE_VIC20) {
        int sidcart;

        /* make SID widget sensitive, depending on if a SID cart is active */
        resources_get_int("SidCart", &sidcart);
        gtk_widget_set_sensitive(grid, sidcart);
    }

    /*
     * Fix layout issues
     *
     * Not the proper place: since uihelpers_create_grid_with_label() should
     * be deprecated and replaced with something more robust, this is a
     * temporary fix.
     *
     * -- compyx, 2018-03-07
     */
    g_object_set(G_OBJECT(grid), "margin", 8, NULL);

    return grid;
}


/** \brief  Update the SID model widget
 *
 * \param[in,out]   widget      SID model widget
 * \param[in]       model       SID model ID
 */
void sid_model_widget_update(GtkWidget *widget, int model)
{
    vice_gtk3_radiogroup_set_index(widget, model);
    mixer_widget_sid_type_changed();
}



void sid_model_widget_sync(GtkWidget *widget)
{
    int model;

    if (resources_get_int("SidModel", &model) < 0) {
        debug_gtk3("failed to get SidModel resource");
        return;
    }
    sid_model_widget_update(widget, model);
}



/** \brief  Set extra callback to trigger when the model changes
 *
 * \param[in]   widget      the SID model widget
 * \param[in]   callback    function to call on model change
 */
void sid_model_widget_set_callback(GtkWidget *widget, void (*callback)(int))
{
    g_object_set_data(G_OBJECT(widget), "ExtraCallback", (gpointer)callback);
}
