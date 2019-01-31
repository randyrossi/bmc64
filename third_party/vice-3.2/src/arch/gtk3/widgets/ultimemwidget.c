/** \file   ultimemwidget.c
 * \brief   VIC-20 UltiMem widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES UltiMemWriteBack    xvic
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

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "widgethelpers.h"
#include "basewidgets.h"

#include "ultimemwidget.h"


/** \brief  Create widget to control VIC-20 Ultimem resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *ultimem_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_resource_check_button_new("UltiMemWriteBack",
                "Enable UltiMem image write back"),
            0, 0, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
