/** \file   videorenderfilterwidget.c
 * \brief   GTK3 widget to select renderer filter
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES CrtcFilter      xpet xcbm2
 * $VICERES TEDFilter       xplus4
 * $VICERES VDCFilter       x128
 * $VICERES VICFilter       xvic
 * $VICERES VICIIFilter     x64 x64sc xscpu64 xdtv64 x128 xcbm5x0
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
#include "debug_gtk3.h"
#include "widgethelpers.h"
#include "resources.h"
#include "video.h"

#include "videorenderfilterwidget.h"


/** \brief  Video chip prefix, used in the resource getting/setting
 */
static const char *chip_prefix;


/** \brief  List of radio buttons
 */
static const vice_gtk3_radiogroup_entry_t filters[] = {
    { "Unfiltered",     VIDEO_FILTER_NONE },
    { "CRT emulation",  VIDEO_FILTER_CRT },
    { "Scale2x",        VIDEO_FILTER_SCALE2X },
    { NULL, -1 }
};


#if 0
/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   new value for render filter (`int1)
 */
static void on_render_filter_toggled(GtkWidget *widget, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        int value = GPOINTER_TO_INT(user_data);

        debug_gtk3("setting %sFilter to %d\n", chip_prefix, value);
        resources_set_int_sprintf("%sFilter", value, chip_prefix);
    }
}
#endif


/** \brief  Create widget to control render filter resources
 *
 * \param[in]   chip    video chip prefix
 *
 * \return  GtkGrid
 */
GtkWidget *video_render_filter_widget_create(const char *chip)
{
    GtkWidget *grid;
    GtkWidget *render_widget;

    chip_prefix = chip;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Render filter", 1);
    render_widget = vice_gtk3_resource_radiogroup_new_sprintf(
            "%sFilter", filters, GTK_ORIENTATION_VERTICAL, chip);
    g_object_set(render_widget, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), render_widget, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
