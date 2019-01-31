/** \file   settings_keyboard.c
 * \brief   GTK3 keyboard settings main widget
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

#include "lib.h"
#include "ui.h"
#include "resources.h"
#include "vsync.h"
#include "basewidgets.h"
#include "widgethelpers.h"

#include "kbdmappingwidget.h"
#include "kbdlayoutwidget.h"

#include "settings_keyboard.h"



/** \brief  Create keyboard settings widget
 *
 * \param[in]   widget  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_keyboard_widget_create(GtkWidget *widget)
{
    GtkWidget *layout;
    GtkWidget *mapping_widget;
    GtkWidget *layout_widget;

    layout = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    mapping_widget = kbdmapping_widget_create(widget);
    gtk_grid_attach(GTK_GRID(layout), mapping_widget, 0, 0, 1, 1);

    layout_widget = kbdlayout_widget_create();

    gtk_grid_attach(GTK_GRID(layout), layout_widget, 0, 1, 1, 1);

    gtk_widget_show_all(layout);
    return layout;
}
