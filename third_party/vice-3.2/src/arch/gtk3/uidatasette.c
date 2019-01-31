/** \file   uidatasette.c
 * \brief   Create independent datasette control widgets
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
 *
 */

#include "vice.h"
#include "uidatasette.h"
#include "datasette.h"
#include "uitapeattach.h"

#include <stdio.h>

void ui_datasette_tape_action_cb(GtkWidget *widget, gpointer data)
{
    int val = GPOINTER_TO_INT(data);
    if (val >= DATASETTE_CONTROL_STOP && val <= DATASETTE_CONTROL_RESET_COUNTER) {
        datasette_control(val);
    } else {
        fprintf(stderr, "Got an impossible Datasette Control action, code %ld (valid range %d-%d)\n", (long)val, DATASETTE_CONTROL_STOP, DATASETTE_CONTROL_RESET_COUNTER);
    }
}

GtkWidget *ui_create_datasette_control_menu(void)
{
    GtkWidget *menu, *item, *menu_items[DATASETTE_CONTROL_RESET_COUNTER+1];
    int i;

    menu = gtk_menu_new();
    item = gtk_menu_item_new_with_label(_("Attach tape image..."));
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(ui_tape_attach_callback), NULL);
    item = gtk_menu_item_new_with_label(_("Detach tape image"));
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(ui_tape_detach_callback), NULL);
    gtk_container_add(GTK_CONTAINER(menu), gtk_separator_menu_item_new());
    menu_items[0] = gtk_menu_item_new_with_label(_("Stop"));
    menu_items[1] = gtk_menu_item_new_with_label(_("Start"));
    menu_items[2] = gtk_menu_item_new_with_label(_("Forward"));
    menu_items[3] = gtk_menu_item_new_with_label(_("Rewind"));
    menu_items[4] = gtk_menu_item_new_with_label(_("Record"));
    menu_items[5] = gtk_menu_item_new_with_label(_("Reset"));
    menu_items[6] = gtk_menu_item_new_with_label(_("Reset Counter"));
    for (i = 0; i <= DATASETTE_CONTROL_RESET_COUNTER; ++i) {
        gtk_container_add(GTK_CONTAINER(menu), menu_items[i]);
        g_signal_connect(menu_items[i], "activate", G_CALLBACK(ui_datasette_tape_action_cb), GINT_TO_POINTER(i));
    }
    gtk_widget_show_all(menu);
    return menu;
}
