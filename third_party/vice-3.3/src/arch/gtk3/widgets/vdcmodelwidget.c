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


static void (*vdc_revision_func)(int);
static void (*vdc_ram_func)(int);


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

    debug_gtk3("GOT REV %d.", rev);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        debug_gtk3("setting VDCRevision to %d.", rev);
        resources_set_int("VDCRevision", rev);

        if (vdc_revision_func != NULL) {
            vdc_revision_func(rev);
        }
    }
}


/** brief   Extra event handler to update C128 model setting
 *
 * \param[in]   widget  64KB check button
 * \param[in]   data    extra event data (unused)
 */
static void on_64kb_ram_toggled(GtkWidget *widget, gpointer data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (vdc_ram_func != NULL) {
        debug_gtk3("calling vdc_ram_func(%d).", state);
        vdc_ram_func(state);
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
    g_signal_connect(extra_ram, "toggled", G_CALLBACK(on_64kb_ram_toggled),
            NULL);
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
    debug_gtk3("got VDCRevision %d.", rev);

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
    GtkWidget *grid;
    GtkWidget *radio;
    int i = 0;

    /* revisions grid is at (0,2) in the 'main' grid */
    grid = gtk_grid_get_child_at(GTK_GRID(widget), 0, 2);
    while ((radio = gtk_grid_get_child_at(
                    GTK_GRID(grid), 0, i)) != NULL) {
        debug_gtk3("Connection signsal handler %d", i);
        if (GTK_IS_RADIO_BUTTON(radio)) {
            debug_gtk3("IS RADIO");
            g_signal_connect(radio, "toggled", G_CALLBACK(on_revision_toggled),
                    GINT_TO_POINTER(vdc_revs[i].id));
        }
        i++;
    }
}


/** \brief  Set callback function to trigger on revision change
 *
 * \param[in,out]   widget  vdc model widget
 * \param[in]       func    callback
 */
void vdc_model_widget_set_revision_callback(GtkWidget *widget,
                                           void (*func)(int))
{
    vdc_revision_func = func;
}



void vdc_model_widget_set_ram_callback(GtkWidget *widget,
                                       void (*func)(int))
{
    vdc_ram_func = func;
}
