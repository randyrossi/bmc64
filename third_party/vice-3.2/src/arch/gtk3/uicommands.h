/** \file   uicommands.h
 * \brief   Simple commands triggered from the menu - header
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

#ifndef VICE_UICOMMANDS_H
#define VICE_UICOMMANDS_H

#include "vice.h"
#include <gtk/gtk.h>

gboolean ui_toggle_resource(GtkWidget *widget, gpointer resource);

gboolean ui_swap_joysticks_callback(GtkWidget *widget, gpointer user_data);
gboolean ui_swap_userport_joysticks_callback(GtkWidget *widget,
                                             gpointer user_data);

void ui_machine_reset_callback(GtkWidget *widget, gpointer user_data);
void ui_drive_reset_callback(GtkWidget *widget, gpointer user_data);
void ui_close_callback(GtkWidget *widget, gpointer user_data);

gboolean ui_main_window_delete_event(GtkWidget *widget, GdkEvent *event,
                                     gpointer user_data);
void ui_main_window_destroy_callback(GtkWidget *widget, gpointer user_data);

gboolean ui_toggle_crt_controls(void);
gboolean ui_crt_controls_enabled(void);

void ui_open_manual_callback(GtkWidget *widget, gpointer user_data);
#endif
