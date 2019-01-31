/** \file   settings_crt.c
 * \brief   GTK3 CRT settings UI
 *
 * Settings UI widget to control CRT settings. For testing, the CRT settings
 * widget should be available from the status bar.
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
 *
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "resources.h"
#include "lib.h"
#include "machine.h"
#include "crtcontrolwidget.h"

#include "settings_crt.h"


GtkWidget *settings_crt_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *chip1;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    chip1 = crt_control_widget_create(grid, "VICII");
    gtk_grid_attach(GTK_GRID(grid), chip1, 0, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
