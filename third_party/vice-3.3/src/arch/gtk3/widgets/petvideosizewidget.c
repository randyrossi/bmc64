/** \file   petvideosizewidget.c
 * \brief   Widget to set the PET video size
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES VideoSize   xpet
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
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"

#include "petvideosizewidget.h"


/** \brief  Data for the radio buttons group
 */
static const vice_gtk3_radiogroup_entry_t video_sizes[] = {
    { "Auto (from ROM)", 0 },
    { "40 Columns", 40 },
    { "80 Columns", 80 },
    { NULL, -1 }
};


/** \brief  User-defined extra callback
 */
static void (*user_callback)(int) = NULL;


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * Sets the VideoSize resource when it has been changed.
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   new_val     value for the resource (`int`)
 */
static void on_video_size_toggled(GtkWidget *widget, int new_val)
{
    if (user_callback != NULL) {
        debug_gtk3("call user_callback with %d.", new_val);
        user_callback(new_val);
    }
}


/** \brief  Create PET video size widget
 *
 * Creates a widget to control the PET's video display number of columns
 *
 * \return  GtkGrid
 */
GtkWidget *pet_video_size_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    g_object_set(G_OBJECT(grid), "margin-left", 8, NULL);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Display width</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    group = vice_gtk3_resource_radiogroup_new(
            "VideoSize",
            video_sizes,
            GTK_ORIENTATION_VERTICAL);
    /* TODO: should probably be 'set_callback` since I only support a single
     *       callback, 'add' indicates it would be possible to add multiple
     *       callbacks, which isn't possible (not yet anyway, and shouldn't
     *       be required)
     */
    vice_gtk3_resource_radiogroup_add_callback(group, on_video_size_toggled);
    g_object_set(G_OBJECT(group), "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);


    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set user-defined callback to be triggered when the widget changes
 *
 * \param[in]   widget  PET video size widget
 * \param[in]   func    user-defined callback
 */
void pet_video_size_widget_set_callback(GtkWidget *widget,
                                        void (*func)(int))
{
    user_callback = func;
}


/** \brief  Synchronize \a widget with its current resource value
 *
 * \param[in,out]   widget  PET video size widget
 */
void pet_video_size_widget_sync(GtkWidget *widget)
{
    GtkWidget *group;

    group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (group != NULL && GTK_IS_GRID(group)) {
        vice_gtk3_resource_radiogroup_sync(group);
    }
}
