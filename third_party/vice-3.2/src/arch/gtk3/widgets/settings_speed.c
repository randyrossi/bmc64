/** \file   settings_speed.c
 * \brief   GTK3 speed control central widget for the settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES WarpMode        all
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
 */


#include "vice.h"

#include <gtk/gtk.h>

#include "lib.h"
#include "ui.h"
#include "resources.h"
#include "vsync.h"

#include "basewidgets.h"
#include "debug_gtk3.h"
#include "resourcecheckbutton.h"
#include "widgethelpers.h"
#include "refreshratewidget.h"
#include "speedwidget.h"

#include "settings_speed.h"


/** \brief  Checkbox indicating 'Pause'
 */
static GtkWidget *checkbox_pause = NULL;

/** \brief  Checkbox indicating 'Warp'
 */
static GtkWidget *checkbox_warp = NULL;



/** \brief  Event handler for the 'pause' checkbox
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void pause_callback(GtkWidget *widget, gpointer data)
{
    int pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    debug_gtk3("%spausing emulation", pause ? "" : "un");
    ui_pause_emulation(pause);
}


/** \brief  Generate the 'Warp mode' checkbox
 *
 * \return  Warp checkbox
 */
static GtkWidget *create_warp_checkbox(void)
{
    return vice_gtk3_resource_check_button_new("WarpMode", "Warp mode");
}


/** \brief  Generate the 'Pause emulation' checkbox
 *
 * \return  Pause checkbox
 */
static GtkWidget *create_pause_checkbox(void)
{
    GtkWidget *check;
    int paused;

    check = gtk_check_button_new_with_label("Pause emulation");
    paused = ui_emulation_is_paused() ? TRUE : FALSE;
    /* set widget state before connecting the event handler, otherwise the
     * event handler triggers an un-pause */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), paused);
    g_signal_connect(check, "toggled", G_CALLBACK(pause_callback), NULL);
    gtk_widget_show(check);
    return check;
}



/** \brief  Create the composite speed settings widget
 *
 * This will create a composite widget to control emulation speed, refresh
 * rate and warp/pause settings. To be used as the 'central widget' of the
 * settings dialog.
 *
 * \return  speed settings widget
 *
 * \todo    Add 'advance frame' button
 */
GtkWidget *settings_speed_widget_create(GtkWidget *widget)
{
    GtkWidget *layout;

    layout = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    checkbox_pause = create_pause_checkbox();
    checkbox_warp = create_warp_checkbox();

    /* create layout */
    gtk_grid_attach(GTK_GRID(layout), refreshrate_widget_create(), 0, 0, 1, 3);
    gtk_grid_attach(GTK_GRID(layout), speed_widget_create(), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), checkbox_warp, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), checkbox_pause, 1, 2, 1, 1);

    gtk_widget_show(layout);
    return layout;
}
