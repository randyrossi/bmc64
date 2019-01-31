/*
 * uiedisk.c - emptydisk dialog for the Gnome widget set.
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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
#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "charset.h"
#include "lib.h"
#include "uiedisk.h"
#include "ui.h"
#include "uiarch.h"
#include "uiapi.h"
#include "diskimage.h"
#include "vdrive/vdrive-internal.h"
#include "util.h"
#include "uilib.h"
#include "uifileentry.h"
#include "lib.h"

static struct {
    char *label;
    GtkWidget *w;
    int type;
} type_radio[] = {
    { "d64", NULL, DISK_IMAGE_TYPE_D64 },
    { "d67", NULL, DISK_IMAGE_TYPE_D67 },
    { "d71", NULL, DISK_IMAGE_TYPE_D71 },
    { "d80", NULL, DISK_IMAGE_TYPE_D80 },
    { "d81", NULL, DISK_IMAGE_TYPE_D81 },
    { "d82", NULL, DISK_IMAGE_TYPE_D82 },
    { "d1m", NULL, DISK_IMAGE_TYPE_D1M },
    { "d2m", NULL, DISK_IMAGE_TYPE_D2M },
    { "d4m", NULL, DISK_IMAGE_TYPE_D4M },
    { "g64", NULL, DISK_IMAGE_TYPE_G64 },
    { "g71", NULL, DISK_IMAGE_TYPE_G71 },
    { "p64", NULL, DISK_IMAGE_TYPE_P64 },
    { "x64", NULL, DISK_IMAGE_TYPE_X64 },
    { NULL, NULL, 0 }
};

static GtkWidget *edisk_dialog, *diskname, *diskid;

static GtkWidget *build_empty_disk_dialog(void)
{
    GtkWidget *d, *box, *hbox, *tmp, *frame;
    int i;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_DISK, UILIB_FILTER_ALL };

    d = vice_file_entry(_("Create empty disk"), NULL, NULL, filter, sizeof(filter) / sizeof(*filter), UI_FC_SAVE);
    gtk_dialog_set_default_response(GTK_DIALOG(d), GTK_RESPONSE_ACCEPT);

    frame = gtk_frame_new(_("Disk options"));
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* Diskname */
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    tmp = gtk_label_new(_("Disk name:"));
    gtk_box_pack_start(GTK_BOX(hbox), tmp, FALSE, FALSE, 0);
    gtk_widget_show(tmp);

    diskname = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(diskname), 16);
    gtk_editable_set_editable(GTK_EDITABLE(diskname), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), diskname, FALSE, FALSE, 0);
    gtk_widget_show(diskname);

    tmp = gtk_label_new("ID: ");
    gtk_box_pack_start(GTK_BOX(hbox), tmp, FALSE, FALSE, 0);
    gtk_widget_show(tmp);
    diskid = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(diskid), 2);
    gtk_editable_set_editable(GTK_EDITABLE(diskid), TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), diskid, FALSE, FALSE, 0);
    gtk_widget_set_size_request(diskid, 25, 22);
    gtk_widget_show(diskid);

    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    for (i = 0; type_radio[i].label; i++) {
        if (i == 0) {
            type_radio[i].w = gtk_radio_button_new_with_label(NULL, type_radio[i].label);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(type_radio[i].w), TRUE);
        } else {
            type_radio[i].w = gtk_radio_button_new_with_label(gtk_radio_button_get_group(GTK_RADIO_BUTTON(type_radio[i - 1].w)), type_radio[i].label);
        }
        gtk_box_pack_start(GTK_BOX(hbox), type_radio[i].w, FALSE, FALSE, 0);
        gtk_widget_show(type_radio[i].w);
    }

    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_widget_show(box);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), frame, FALSE, FALSE, 0);
    gtk_widget_show(frame);
    return d;
}

int ui_empty_disk_dialog(char *name)
{
    gint res;
    char *format_text;
    char *fname;
    const char *dname, *id;
    char dname2[16+1], id2[2+1];
    int i, type = 0, ret = 0;

    if (edisk_dialog) {
        gdk_window_show(gtk_widget_get_window(edisk_dialog));
        gdk_window_raise(gtk_widget_get_window(edisk_dialog));
        gtk_widget_show(edisk_dialog);
    } else {
        edisk_dialog = build_empty_disk_dialog();
        g_signal_connect(G_OBJECT(edisk_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &edisk_dialog);
    }

    ui_popup(edisk_dialog, _("Create empty disk"), FALSE);
    res = gtk_dialog_run(GTK_DIALOG(edisk_dialog));
    ui_popdown(edisk_dialog);

    if (res != GTK_RESPONSE_ACCEPT) {
        return -1;
    }

    /* filename */
    fname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(edisk_dialog));
    if (!fname) {
        return -1;
    }

    strcpy(name, fname);
    g_free(fname);

    /* format label */
    dname = gtk_entry_get_text(GTK_ENTRY(diskname));
    if (!dname) {
        dname2[0] = 0;
    } else {
        strcpy(dname2, dname);
        charset_petconvstring((unsigned char*)dname2, 0);
    }
    /* disk ID */
    id = gtk_entry_get_text(GTK_ENTRY(diskid));
    if (!id) {
        strcpy(id2, "00");
    } else {
        strcpy(id2, id);
        charset_petconvstring((unsigned char*)id2, 0);
    }
    format_text = util_concat(dname2, ",", id2, NULL);

    /* type radio button */
    for (i = 0; type_radio[i].label; i++) {
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(type_radio[i].w))) {
            type = type_radio[i].type;
            break;
        }
    }

    if (vdrive_internal_create_format_disk_image(name, format_text, type) < 0) {
        ui_error(_("Can't create image `%s'."));
        ret = -1;
    }
    lib_free(format_text);

    return ret;
}
