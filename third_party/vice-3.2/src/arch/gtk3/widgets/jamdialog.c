/**
 * \brief   Gtk3 CPU jam dialog
 *
 * GTK3 CPU Jam dialog
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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
 */


#include "vice.h"
#include <gtk/gtk.h>

#include "debug_gtk3.h"
#include "uiapi.h"

#include "jamdialog.h"


enum {
    RESPONSE_NONE = 1,
    RESPONSE_RESET_SOFT,
    RESPONSE_RESET_HARD,
    RESPONSE_MONITOR,
};



ui_jam_action_t jam_dialog(GtkWidget *parent, const char *msg)
{
    GtkWidget *dialog;
    ui_jam_action_t result = UI_JAM_NONE;

    dialog = gtk_dialog_new_with_buttons("CPU JAM", GTK_WINDOW(parent),
            GTK_DIALOG_MODAL,
            "None", RESPONSE_NONE,
            "Soft reset", RESPONSE_RESET_SOFT,
            "Hard reset", RESPONSE_RESET_HARD,
            "Active monitor", RESPONSE_MONITOR,
            NULL);

    switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
        case RESPONSE_NONE:
        case GTK_RESPONSE_DELETE_EVENT:
            result = UI_JAM_NONE;
            break;
        case RESPONSE_RESET_SOFT:
            result = UI_JAM_RESET;
            break;
        case RESPONSE_RESET_HARD:
            result = UI_JAM_HARD_RESET;
            break;
        case RESPONSE_MONITOR:
            result = UI_JAM_MONITOR;
            break;
        default:
            /* shouldn't get here */
            break;
    }

    gtk_widget_destroy(dialog);
    return result;
}
