/** \file   settings_crt.c
 * \brief   GTK3 CRT settings UI
 *
 * Settings UI widget to control CRT settings.
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
#include "debug_gtk3.h"
#include "resources.h"
#include "lib.h"
#include "machine.h"
#include "crtcontrolwidget.h"
#include "resourcewidgetmanager.h"
#include "uisettings.h"

#include "settings_crt.h"


/** \brief  Resource widget manager object
 */
static resource_widget_manager_t manager;


/** \brief  Clean up memory used by the widget manager
 *
 * \param[in]   widget  widget triggering the event (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer data)
{
    vice_resource_widget_manager_exit(&manager);
}


/** \brief  Create a CRT settings widget for the settings UI
 *
 * Creates sliders to control CRT settings. In the case of the C128 two widgets
 * are created, one for the VICII and for the VDC.
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_crt_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *chip1 = NULL;
    GtkWidget *chip2 = NULL;

    vice_resource_widget_manager_init(&manager);
    ui_settings_set_resource_widget_manager(&manager);

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_CBM5x0:
            chip1 = crt_control_widget_create(NULL, "VICII", FALSE);
            break;
        case VICE_MACHINE_VIC20:
            chip1 = crt_control_widget_create(NULL, "VIC", FALSE);
            break;
        case VICE_MACHINE_PLUS4:
            chip1 = crt_control_widget_create(NULL, "TED", FALSE);
            break;
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            chip1 = crt_control_widget_create(NULL, "CRTC", FALSE);
            break;
        case VICE_MACHINE_C128:
            chip1 = crt_control_widget_create(NULL, "VICII", FALSE);
            chip2 = crt_control_widget_create(NULL, "VDC", FALSE);
            break;
        default:
            debug_gtk3("shouldn't get here, VSID doesn't support CRTs.");
            break;
    }

    /* add VIC/VICII/TED/CRTC */
    if (chip1 != NULL) {
        gtk_grid_attach(GTK_GRID(grid), chip1, 0, 0, 1, 1);
        vice_resource_widget_manager_add_widget(&manager, chip1, NULL,
                crt_control_widget_reset, NULL, NULL);
    }
    /* add VDC in case of c128 */
    if (chip2 != NULL) {
        gtk_grid_attach(GTK_GRID(grid), chip2, 0, 1, 1, 1);
        vice_resource_widget_manager_add_widget(&manager, chip2, NULL,
                crt_control_widget_reset, NULL, NULL);
    }

    g_signal_connect(grid, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
