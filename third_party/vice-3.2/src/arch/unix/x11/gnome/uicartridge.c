/*
 * uicartridge.c - Cartridge save image dialog for the Xaw widget set.
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

#include "ui.h"
#include "uiarch.h"
#include "uiapi.h"
#include "cartridge.h"
#include "uifileentry.h"
#include "uilib.h"
#include "lib.h"

#include "uicartridge.h"


static GtkWidget *cartridge_dialog;

void ui_cartridge_save_dialog(int type)
{
    gint res;
    char *name;

    if (cartridge_dialog) {
        gdk_window_show(gtk_widget_get_window(cartridge_dialog));
        gdk_window_raise(gtk_widget_get_window(cartridge_dialog));
        gtk_widget_show(cartridge_dialog);
    } else {
        uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;
        cartridge_dialog = vice_file_entry(_("Save cartridge image"), NULL, NULL, &filter, 1, UI_FC_SAVE);
        g_signal_connect(G_OBJECT(cartridge_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &cartridge_dialog);
    }

    ui_popup(cartridge_dialog, _("Save cartridge image"), FALSE);
    res = gtk_dialog_run(GTK_DIALOG(cartridge_dialog));
    ui_popdown(cartridge_dialog);
    
    if (res != GTK_RESPONSE_ACCEPT) {
        return;
    }
    
    name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(cartridge_dialog));
    if (!name) {
        ui_error(_("Invalid filename"));
        return;
    }
	    
    if (cartridge_save_image (type, name) < 0) {
        ui_error(_("Cannot write cartridge image file\n`%s'\n"), name);
    } else {
	ui_message(_("Successfully wrote `%s'"), name);
    }
    lib_free(name);
}
