/** \file   uisnapshot.h
 * \brief   Snapshot dialogs and menu item handlers - header
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

#ifndef VICE_UISNAPSHOT_H
#define VICE_UISNAPSHOT_H

#include "vice.h"
#include <gtk/gtk.h>


void uisnapshot_open_file(GtkWidget *parent, gpointer user_data);
void uisnapshot_save_file(GtkWidget *parent, gpointer user_data);

void uisnapshot_quickload_snapshot(GtkWidget *parent, gpointer user_data);
void uisnapshot_quicksave_snapshot(GtkWidget *parent, gpointer user_data);

void uisnapshot_history_select_dir(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_record_start(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_record_stop(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_playback_start(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_playback_stop(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_milestone_set(GtkWidget *parent, gpointer user_data);
void uisnapshot_history_milestone_reset(GtkWidget *parent, gpointer user_data);

#endif
