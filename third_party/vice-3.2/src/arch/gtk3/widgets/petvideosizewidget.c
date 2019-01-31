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


/** \brief  Look up the index of \a size int the video sizes array
 *
 * \param[in]   size    video size value (0, 40 or 80)
 *
 * \return  index in the array or -1 when \a size is not found
 */
static int get_video_size_index(int size)
{
    return vice_gtk3_radiogroup_get_list_index(video_sizes, size);
}


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * Sets the VideoSize resource when it has been changed.
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   value for the resource (`int`)
 */
static void on_video_size_toggled(GtkWidget *widget, gpointer user_data)
{
    int old_val;
    int new_val;

    resources_get_int("VideoSize", &old_val);
    new_val = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))
            && (new_val != old_val)) {
        debug_gtk3("setting VideoSize to %d\n", new_val);
        resources_set_int("VideoSize", new_val);
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
    int size;
    int index;

    resources_get_int("VideoSize", &size);
    index = get_video_size_index(size);

    grid = uihelpers_radiogroup_create(
            "Video size",
            video_sizes,
            on_video_size_toggled,
            index);
    gtk_widget_show_all(grid);
    return grid;
}
