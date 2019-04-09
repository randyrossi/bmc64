/** \file   settings_soundchip.c
 * \brief   Sound chip settings main widget for the settings UI
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include "vice_gtk3.h"
#include "lib.h"
#include "resources.h"
#include "sidsoundwidget.h"

#include "settings_soundchip.h"


/** \brief  Create sound chip settings widget
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_soundchip_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *sid_widget = NULL;

    layout = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(layout), 8);
    gtk_grid_set_row_spacing(GTK_GRID(layout), 8);
    g_object_set(layout, "margin", 16, NULL);

    sid_widget = sid_sound_widget_create(layout);

    gtk_grid_attach(GTK_GRID(layout), sid_widget, 0, 0, 1, 1);

    gtk_widget_show_all(layout);
    return layout;
}


