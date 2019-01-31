/** \file   vdcmodelwidget.c
 * \brief   VDC settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES VDCRevision     x128
 * $VICERES VDC64KB         x128
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

#include "basewidgets.h"
#include "resourcecheckbutton.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"

#include "vdcmodelwidget.h"

/** \brief  List of VDC revisions
 */
static const vice_gtk3_radiogroup_entry_t vdc_revs[] = {
    { "Revision 0", 0 },
    { "Revision 1", 1 },
    { "Revision 2", 2 },
    { NULL, -1 }
};


#if 0

/** \brief  Get index in revisions table of \a revision
 *
 * \param[in]   revision    revision number
 *
 * \return  index in revisions table or -1 on error
 *
 * \note    Looks a bit over-the-top for a simple sequence, but should revision
 *          numbers change somehow, this code will still work.
 */
static int get_revision_index(int revision)
{
    return uihelpers_radiogroup_get_index(vdc_revs, revision);
}
#endif

/** \brief  Handler for the "toggled" event of the revision radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   revision (`int`)
 */
static void on_revision_toggled(GtkWidget *widget, gpointer user_data)
{
    int rev = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        debug_gtk3("setting VDCRevision to %d\n", rev);
        resources_set_int("VDCRevision", rev);
    }
}


/** \brief  Create check button to toggle 64KB video ram
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_64kb_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new(
            "VDC64KB", "Enable 64KB video ram");
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


/** \brief  Create widget to manipulate VDC settings
 *
 * \return  GtkGrid
 */
GtkWidget *vdc_model_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    GtkWidget *extra_ram;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT,
            "VDC settings", 1);

    extra_ram = create_64kb_widget();
    group = vice_gtk3_resource_radiogroup_new("VDCRevision",
            vdc_revs, GTK_ORIENTATION_VERTICAL);
    g_object_set(extra_ram, "margin-left", 16, NULL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), extra_ram, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the VDC model widget
 *
 * \param[in,out]   widget  VDC model widget
 */
void vdc_model_widget_update(GtkWidget *widget)
{
    int rev;
    int index;

    resources_get_int("VDCRevision", &rev);
    index = vice_gtk3_radiogroup_get_list_index(vdc_revs, rev);
    debug_gtk3("got VDCRevision %d\n", rev);

    if (index >= 0) {
        int i = 0;
        GtkWidget *radio;

        /* +2: skip title & 64KB checkbox */
        while ((radio = gtk_grid_get_child_at(
                        GTK_GRID(widget), 0, i + 2)) != NULL) {
            if (GTK_IS_RADIO_BUTTON(radio)) {
                if (i == index) {
                    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio),
                            TRUE);
                    break;
                }
            }
            i++;
        }
    }
}


/** \brief  Connect signal handlers of the VDC widget
 *
 * \param[in,out]   widget  VDC model widget
 */
void vdc_model_widget_connect_signals(GtkWidget *widget)
{
    GtkWidget *radio;
    int i = 0;

    while ((radio = gtk_grid_get_child_at(
                    GTK_GRID(widget), 0, i + 2)) != NULL) {
        if (GTK_IS_RADIO_BUTTON(radio)) {
            g_signal_connect(radio, "toggled", G_CALLBACK(on_revision_toggled),
                    GINT_TO_POINTER(vdc_revs[i].id));
        }
        i++;
    }
}
