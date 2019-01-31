/** \file   soundsyncwidget.c
 * \brief   GTK3 sound synchronization mode widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SoundSpeedAdjustment    all
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

#include "basewidgets.h"
#include "lib.h"
#include "ui.h"
#include "resources.h"
#include "vsync.h"
#include "sound.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"

#include "soundsyncwidget.h"


/** \brief  List of sound synchronization modes
 */
static const vice_gtk3_radiogroup_entry_t sync_modes[] = {
    { "Flexible", SOUND_ADJUST_FLEXIBLE },
    { "Adjusting", SOUND_ADJUST_ADJUSTING },
    { "Exact", SOUND_ADJUST_EXACT },
    { NULL, -1 }
};


/** \brief  Create widget for "Sound output mode"
 *
 * A simple list of radio buttons for sound output mode: System, Mono or Stereo
 *
 * \return  grid
 */
GtkWidget *sound_sync_mode_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT,
            "Synchronization mode", 1);
    group = vice_gtk3_resource_radiogroup_new(
            "SoundSpeedAdjustment", sync_modes, GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
