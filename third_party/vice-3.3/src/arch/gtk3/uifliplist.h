/** \file   uifliplist.h
 * \brief   Fliplist menu management - header
 *
 * \author  Michael C. Martin <mcmartin@gmail.com>
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

#ifndef VICE_UIFLIPLIST_H
#define VICE_UIFLIPLIST_H

#include <gtk/gtk.h>

void ui_populate_fliplist_menu(GtkWidget *menu, int unit, int separator_count);

void ui_fliplist_add_current_cb(GtkWidget *widget, gpointer data);
void ui_fliplist_remove_current_cb(GtkWidget *widget, gpointer data);
void ui_fliplist_next_cb(GtkWidget *widget, gpointer data);
void ui_fliplist_prev_cb(GtkWidget *widget, gpointer data);

void ui_fliplist_load_callback(GtkWidget *widget, gpointer data);
void ui_fliplist_save_callback(GtkWidget *widget, gpointer data);

#endif
