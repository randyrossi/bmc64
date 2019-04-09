/** \file   uinetplay_new.c
 * \brief   GTK3 Netplay UI
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES NetworkServerName           -vsid
 * $VICERES NetworkServerPort           -vsid
 * $VICERES NetworkServerBindAddress    -vsid
 *
 * Not sure about this one (TODO):
 * $VICERES NetworkControl              -vsid
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
#include <stdio.h>
#include <stdlib.h>

#include "basewidgets.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "network.h"
#include "ui.h"
#include "widgethelpers.h"

#include "uinetplay_new.h"


static GtkWidget *server_addr = NULL;
static GtkWidget *server_port = NULL;
static GtkWidget *bind_enable = NULL;
static GtkWidget *status_widget = NULL;


static const char *net_modes[] = {
    "Idle",
    "Server listening",
    "Connected server",
    "Connected client"
};


static GtkWidget *create_indented_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    return label;
}


static void on_server_enable_toggled(GtkSwitch *widget, gpointer data)
{
    int state = gtk_switch_get_active(widget);
    debug_gtk3("Server %s requested.", state ? "ENABLE" : "DISABLE");

    gtk_widget_set_sensitive(bind_enable, state ? FALSE : TRUE);
    gtk_widget_set_sensitive(server_addr, state ? FALSE : TRUE);
    gtk_widget_set_sensitive(server_port, state ? FALSE : TRUE);
}


static void on_client_enable_toggled(GtkSwitch *widget, gpointer data)
{
#ifdef HAVE_DEBUG_GTK3UI
    int state = gtk_switch_get_active(widget);
#endif
    debug_gtk3("Client %s requested.", state ? "CONNECT" : "DISCONNECT");
}



static GtkWidget *create_server_enable_widget(void)
{
    GtkWidget *widget;

    widget = gtk_switch_new();
    gtk_widget_set_halign(widget, GTK_ALIGN_START);
    g_signal_connect(widget, "notify::active",
            G_CALLBACK(on_server_enable_toggled), NULL);
    return widget;
}


static GtkWidget *create_client_enable_widget(void)
{
    GtkWidget *widget;

    widget = gtk_switch_new();
    gtk_widget_set_halign(widget, GTK_ALIGN_START);
    g_signal_connect(widget, "notify::active",
            G_CALLBACK(on_client_enable_toggled), NULL);
    return widget;
}



/** \brief  Handler for the "response" event of the dialog
 *
 * \param[in,out]   dialog      dialog
 * \param[in]       response_id response ID
 * \param[in]       extra event data (unused)
 */
static void on_response(GtkWidget *dialog, gint response_id, gpointer data)
{
    switch (response_id) {
        case GTK_RESPONSE_DELETE_EVENT:
            gtk_widget_destroy(dialog);
            break;
        default:
            debug_gtk3("unknown response ID %d, ignoring.", response_id);
            break;
    }
}


static void netplay_update_status(void)
{
    const char *text = NULL;
    char *temp;
    int mode = network_get_mode();

    if (mode < 0 || mode >= (int)(sizeof net_modes / sizeof net_modes[0])) {
        text = "invalid";
    } else {
        text = net_modes[mode];
    }

    temp = lib_msprintf("<b>%s</b>", text);
    gtk_label_set_markup(GTK_LABEL(status_widget), temp);
    lib_free(temp);
}



/** \brief  Create the 'content' widget for the dialog
 *
 * This creates the content of the dialog, holding all controls
 *
 * \return  GtkGrid
 */
static GtkWidget *create_content_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *server_enable;
    GtkWidget *bind_addr;
    int row;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    row = 0;

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Server</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 4, 1);
    row++;

    label = create_indented_label("Server IP address");
    server_addr = vice_gtk3_resource_entry_full_new("NetworkServerName");
    gtk_widget_set_hexpand(server_addr, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), server_addr, 1, row, 1, 1);

    server_port = vice_gtk3_resource_spin_int_new("NetworkServerPort",
            1, 65535, 1);
    gtk_grid_attach(GTK_GRID(grid), server_port, 2, row, 1, 1);
    server_enable = create_server_enable_widget();

    gtk_grid_attach(GTK_GRID(grid), server_enable, 3, row, 1, 1);
    row++;

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Client</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 4, 1);
    row++;

    label = create_indented_label("Bind IP address");
    bind_addr = vice_gtk3_resource_entry_full_new(
            "NetworkServerBindAddress");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bind_addr, 1, row, 1, 1);

    bind_enable = create_client_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), bind_enable, 3, row, 2, 1);
    row++;

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Network status</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 4, 1);

    row++;
    status_widget = create_indented_label("");
    gtk_grid_attach(GTK_GRID(grid), status_widget, 0, row, 4, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Show Netplay dialog
 *
 * \param[in]   parent  parent widget
 * \param[in]   data    extra data (unused)
 */
void ui_netplay_dialog_new(GtkWidget *parent, gpointer data)
{
    GtkWidget *dialog;
    GtkWidget *content;

    dialog = gtk_dialog_new_with_buttons("Netplay",
            ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Close", GTK_RESPONSE_DELETE_EVENT,
            NULL);

    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content), create_content_widget());
    netplay_update_status();

    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    gtk_widget_show_all(dialog);
}
