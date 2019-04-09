/** \file   vsidmainwidget.c
 * \brief   GTK3 main widget for VSD
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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "debug.h"
#include "machine.h"
#include "lib.h"
#include "log.h"
#include "uivsidmenu.h"
#include "vsidtuneinfowidget.h"
#include "vsidcontrolwidget.h"
#include "vsidmixerwidget.h"
#include "vsidplaylistwidget.h"
#include "hvscstilwidget.h"

#include "vsidmainwidget.h"

static GtkWidget *main_widget;

static GtkWidget *tune_info_widget;
static GtkWidget *control_widget;
static GtkWidget *mixer_widget;
static GtkWidget *stil_widget;

/*
 * Disable unfinished playlist widget
 *
 * I probably won't be able to work this into a proper functioning widget
 * before the code freeze in about a week (2018-12-03), so until 3.3 is
 * released this will be disabled. The Wiki contains items about "playlist"
 * and "HVSC browser" support, so this should not be forgotten. And I also
 * would like to properly implement those things, soon.
 *
 * --compyx
 */
#if 0
static GtkWidget *playlist_widget;
#endif


/** \brief  Create VSID main widget
 *
 * \return  GtkGrid
 */
GtkWidget *vsid_main_widget_create(void)
{
    GtkWidget *grid;

    grid = vice_gtk3_grid_new_spaced(32, 32);
    g_object_set(G_OBJECT(grid),
            "margin-left", 16,
            "margin-right", 16,
            "margin-top", 16,
            "margin-bottom", 16,
            NULL);

    tune_info_widget = vsid_tune_info_widget_create();
    gtk_grid_attach(GTK_GRID(grid), tune_info_widget, 0, 0, 1, 1);

    control_widget = vsid_control_widget_create();
    gtk_grid_attach(GTK_GRID(grid), control_widget, 0, 1, 1, 1);

    mixer_widget = vsid_mixer_widget_create();
    gtk_grid_attach(GTK_GRID(grid), mixer_widget, 0, 2, 1, 1);

    stil_widget = hvsc_stil_widget_create();
    gtk_widget_set_vexpand(stil_widget, TRUE);
    gtk_grid_attach(GTK_GRID(grid), stil_widget, 1, 0, 1, 4);

#if 0
    playlist_widget = vsid_playlist_widget_create();
    gtk_grid_attach(GTK_GRID(grid), playlist_widget, 2, 0, 1, 4);
#endif
    gtk_widget_show_all(grid);
    main_widget = grid;
    return grid;
}


/** \brief  Set number of tunes
 *
 * \param[in]   n   tune count
 */
void vsid_main_widget_set_tune_count(int n)
{
    vsid_control_widget_set_tune_count(n);
    vsid_tune_info_widget_set_tune_count(n);
    ui_vsid_tune_menu_set_tune_count(n);
}

/** \brief  Set current tune
 *
 * \param[in]   n   tune number
 */
void vsid_main_widget_set_tune_current(int n)
{
    vsid_control_widget_set_tune_current(n);
    vsid_tune_info_widget_set_tune_current(n);
    ui_vsid_tune_set_tune_current(n);

    /* update mixer widget to use the SID model of the current tune */
    if (mixer_widget != NULL) {
        gtk_widget_destroy(mixer_widget);
        mixer_widget = vsid_mixer_widget_create();
        gtk_grid_attach(GTK_GRID(main_widget), mixer_widget, 0, 2, 1, 1);
    }
}


/** \brief  Set default tune
 *
 * \param[in]   n   tune number
 */
void vsid_main_widget_set_tune_default(int n)
{
    vsid_control_widget_set_tune_default(n);
    vsid_tune_info_widget_set_tune_default(n);
}
