/*
 * dragdrop.c - GTK only, drag and drop handling
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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
 
/* #define DEBUG_X11UI */

#include "vice.h"

#include <string.h>

#include "lib.h"
#include "machine.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "videoarch.h"

#include "dragdrop.h"


#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/*******************************************************************************
 * Drag and Drop support
 *******************************************************************************/

/* Define a list of data types called "targets" that a destination widget will
 * accept. The string type is arbitrary, and negotiated between DnD widgets by
 * the developer. An enum or GQuark can serve as the integer target id. */
enum {
    TARGET_STRING,
};

#if 0
/* datatype (string), restrictions on DnD (GtkTargetFlags), datatype (int) */
static GtkTargetEntry target_list[] = {
        { "STRING",     0, TARGET_STRING },
        { "text/plain", 0, TARGET_STRING },
};
#endif

static int dropdata = 0;
static int (*drop_cb)(char*) = NULL; 

/* Emitted when the user releases (drops) the selection. It should check that
 * the drop is over a valid part of the widget (if its a complex widget), and
 * itself to return true if the operation should continue. Next choose the
 * target type it wishes to ask the source for. Finally call gtk_drag_get_data
 * which will emit "drag-data-get" on the source. */
static gboolean drag_drop_handler(GtkWidget *widget, GdkDragContext *context, 
    gint x, gint y, guint time, gpointer user_data)
{
    GdkAtom target_type;
    GList *targets = gdk_drag_context_list_targets(context);

    DBG(("drag_drop_handler"));

    /* If the source offers a target */
    if (targets) {
        /* Choose the best target type */
        target_type = GDK_POINTER_TO_ATOM(g_list_nth_data (targets, TARGET_STRING));

        dropdata = 1;
        /* Request the data from the source. */
        gtk_drag_get_data(
            widget,         /* will receive 'drag-data-received' signal */
            context,        /* represents the current state of the DnD */
            target_type,    /* the target type we want */
            time            /* time stamp */
        );
        return TRUE;
    }
    /* No target offered by source => error */
    return FALSE;
}

/* Emitted when the data has been received from the source. It should check
 * the GtkSelectionData sent by the source, and do something with it. Finally
 * it needs to finish the operation by calling gtk_drag_finish, which will emit
 * the "data-delete" signal if told to. */
static void drag_data_received_handler(GtkWidget *widget, GdkDragContext *context, 
    gint x, gint y, GtkSelectionData *selection_data, guint target_type, guint time,
    gpointer data)
{
    char *filename, *p, *lfn;
    gboolean dnd_success = FALSE;
    gboolean delete_selection_data = FALSE;
    DBG(("drag_data_received_handler"));

    /* Deal with what we are given from source */
    if(dropdata && (selection_data != NULL) && (gtk_selection_data_get_length (selection_data) >= 0))
    {
        dropdata = 0;
        if (gdk_drag_context_get_selected_action(context) == GDK_ACTION_MOVE) {
            delete_selection_data = TRUE;
        }

        /* FIXME; Check that we got a format we can use */
        filename = (char*)gtk_selection_data_get_data(selection_data);
        DBG(("DnD got string: %s", filename));
        dnd_success = TRUE;
        /* incase we got a list of files, terminate the list after the first
           file */
        p = filename;
        while (*p) {
            if ((*p == '\n') || (*p == '\r')) {
                *p = 0;
                break;
            }
            p++;
        }
        lfn = g_filename_from_uri(filename, NULL, NULL);
        if (!lfn) {
            log_error(ui_log, "URI conversion failed: %s", filename);
        }

        DBG(("DnD using filename: '%s'", lfn));
        /* finally call the drop callback set by the individual ui */
        if (drop_cb) {
            drop_cb(lfn);
        }
        DBG(("DnD done"));
        if (lfn) {
            free(lfn);
        }
    }

    if (dnd_success == FALSE) {
        DBG(("DnD data transfer failed!"));
    }

    gtk_drag_finish (context, dnd_success, delete_selection_data, time);
}


void set_drop_target_widget(GtkWidget *w)
{
    gtk_drag_dest_set(w, 
        GTK_DEST_DEFAULT_ALL, 
        NULL, /* set targets to NULL */
        0, 
        GDK_ACTION_COPY | GDK_ACTION_MOVE /* must be copy AND move or it won't 
                                             work with all WMs / Filemanagers */
    );
    gtk_drag_dest_add_text_targets(w); /* add text targets */
    gtk_drag_dest_add_uri_targets(w); /* add uri targets, to eg include nautilus list view drops */

    g_signal_connect (G_OBJECT(w), "drag-data-received", G_CALLBACK(drag_data_received_handler), NULL);
    g_signal_connect (G_OBJECT(w), "drag-drop", G_CALLBACK(drag_drop_handler), NULL);
}

void ui_set_drop_callback(void *cb)
{
    drop_cb = cb;
}
