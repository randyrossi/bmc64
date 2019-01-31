/*
 * uijoystickkeys.c
 *
 * Written by
 *  Fabrizio Gennari <fabrizio.ge@tiscali.it>
 *
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

#include "uiarch.h"
#include "uimenu.h"
#include "resources.h"

#include "uijoystickkeys.h"


static void ui_keybutton_pressed(GtkButton *button, gpointer user_data)
{
    int i;
    GtkWidget **buttons = user_data;

    for (i = 0; i < 18; i++) {
        if (button != GTK_BUTTON(buttons[i])) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(buttons[i]), FALSE);
        }
    }
}

struct keysbuttons {
    GtkWidget **buttons;
    guint *keys;
};

static gboolean ui_change_key(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    int i;
    struct keysbuttons *k = user_data;
    guint key = event->keyval;

    if (key == KEYSYM_Alt_L || key == KEYSYM_Alt_R) {
        return FALSE;
    }
    if (key == KEYSYM_Escape) {
        key = 0;
    }

    for (i = 0; i < 18; i++) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(k->buttons[i]))) {
            k->keys[i] = key;
            gtk_button_set_label(GTK_BUTTON(k->buttons[i]), key != 0 ? gdk_keyval_name(key) : "None");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(k->buttons[i]), FALSE);
            return TRUE;
        }
    }
    return FALSE;
}

UI_CALLBACK(ui_keyset_dialog)
{
    GtkWidget *keyset_dialog,
    *keyset1, *keyset2,
    *button[18], *label[18], *box[18], *vb;
    GtkWidget *f;
    guint keys[18];
    int i;
    struct keysbuttons k = {button, keys};
    gint res;

    keyset_dialog = gtk_dialog_new_with_buttons ("Configure keysets", 0, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(keyset_dialog), FALSE);

    resources_get_int("KeySet1SouthWest", (void *)&keys[0]);
    resources_get_int("KeySet1South", (void *)&keys[1]);
    resources_get_int("KeySet1SouthEast", (void *)&keys[2]);
    resources_get_int("KeySet1West", (void *)&keys[3]);
    resources_get_int("KeySet1Fire", (void *)&keys[4]);
    resources_get_int("KeySet1East", (void *)&keys[5]);
    resources_get_int("KeySet1NorthWest", (void *)&keys[6]);
    resources_get_int("KeySet1North", (void *)&keys[7]);
    resources_get_int("KeySet1NorthEast", (void *)&keys[8]);
    resources_get_int("KeySet2SouthWest", (void *)&keys[9]);
    resources_get_int("KeySet2South", (void *)&keys[10]);
    resources_get_int("KeySet2SouthEast", (void *)&keys[11]);
    resources_get_int("KeySet2West", (void *)&keys[12]);
    resources_get_int("KeySet2Fire", (void *)&keys[13]);
    resources_get_int("KeySet2East", (void *)&keys[14]);
    resources_get_int("KeySet2NorthWest", (void *)&keys[15]);
    resources_get_int("KeySet2North", (void *)&keys[16]);
    resources_get_int("KeySet2NorthEast", (void *)&keys[17]);

    vb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    keyset1 = gtk_table_new(3, 3, TRUE);
    keyset2 = gtk_table_new(3, 3, TRUE);
    label[0] = gtk_label_new("Southwest");
    label[1] = gtk_label_new("South");
    label[2] = gtk_label_new("Southeast");
    label[3] = gtk_label_new("West");
    label[4] = gtk_label_new("Fire");
    label[5] = gtk_label_new("East");
    label[6] = gtk_label_new("Northwest");
    label[7] = gtk_label_new("North");
    label[8] = gtk_label_new("Northeast");
    label[9] = gtk_label_new("Southwest");
    label[10] = gtk_label_new("South");
    label[11] = gtk_label_new("Southeast");
    label[12] = gtk_label_new("West");
    label[13] = gtk_label_new("Fire");
    label[14] = gtk_label_new("East");
    label[15] = gtk_label_new("Northwest");
    label[16] = gtk_label_new("North");
    label[17] = gtk_label_new("Northeast");

    for (i = 0;i < 18; i++) {
        char *keylabel = (keys[i] != 0) ? gdk_keyval_name(keys[i]) : "None";

        box[i] = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        button[i] = gtk_toggle_button_new();
        gtk_button_set_label(GTK_BUTTON(button[i]), keylabel);
        gtk_box_pack_start(GTK_BOX(box[i]), button[i], TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(box[i]), label[i], FALSE, FALSE, 0);
        gtk_widget_show(box[i]);
        gtk_widget_show(button[i]);
        gtk_widget_show(label[i]);
        g_signal_connect(G_OBJECT(button[i]), "pressed", G_CALLBACK(ui_keybutton_pressed), (gpointer) button);
    }

    f = gtk_frame_new("Keyset 1");
    gtk_frame_set_label_align(GTK_FRAME(f), 0, 0);
    gtk_container_add(GTK_CONTAINER(f), keyset1);
    gtk_widget_show(f);
    gtk_box_pack_start(GTK_BOX(vb), f, FALSE, FALSE, 5);

    f = gtk_frame_new("Keyset 2");
    gtk_frame_set_label_align(GTK_FRAME(f), 0, 0);
    gtk_container_add(GTK_CONTAINER(f), keyset2);
    gtk_widget_show(f);
    gtk_box_pack_start(GTK_BOX(vb), f, FALSE, FALSE, 5);

    gtk_widget_show(vb);
    gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(keyset_dialog))), vb);

    gtk_table_attach (GTK_TABLE(keyset1), box[0], 0, 1, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[1], 1, 2, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[2], 2, 3, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[3], 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[4], 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[5], 2, 3, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[6], 0, 1, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[7], 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset1), box[8], 2, 3, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_widget_show(keyset1);
    
    gtk_table_attach (GTK_TABLE(keyset2), box[9], 0, 1, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[10], 1, 2, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[11], 2, 3, 2, 3, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[12], 0, 1, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[13], 1, 2, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[14], 2, 3, 1, 2, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[15], 0, 1, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[16], 1, 2, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_table_attach (GTK_TABLE(keyset2), box[17], 2, 3, 0, 1, GTK_EXPAND|GTK_FILL, 0, 0, 0);
    gtk_widget_show(keyset2);

    g_signal_connect(G_OBJECT(keyset_dialog), "key-press-event", G_CALLBACK(ui_change_key), (gpointer)&k);
    res = gtk_dialog_run(GTK_DIALOG(keyset_dialog));
    if (res == GTK_RESPONSE_OK) {
        resources_set_int("KeySet1SouthWest", keys[0]);
        resources_set_int("KeySet1South", keys[1]);
        resources_set_int("KeySet1SouthEast", keys[2]);
        resources_set_int("KeySet1West", keys[3]);
        resources_set_int("KeySet1Fire", keys[4]);
        resources_set_int("KeySet1East", keys[5]);
        resources_set_int("KeySet1NorthWest", keys[6]);
        resources_set_int("KeySet1North", keys[7]);
        resources_set_int("KeySet1NorthEast", keys[8]);
        resources_set_int("KeySet2SouthWest", keys[9]);
        resources_set_int("KeySet2South", keys[10]);
        resources_set_int("KeySet2SouthEast", keys[11]);
        resources_set_int("KeySet2West", keys[12]);
        resources_set_int("KeySet2Fire", keys[13]);
        resources_set_int("KeySet2East", keys[14]);
        resources_set_int("KeySet2NorthWest", keys[15]);
        resources_set_int("KeySet2North", keys[16]);
        resources_set_int("KeySet2NorthEast", keys[17]);
    }
    gtk_widget_destroy(keyset_dialog);
}
