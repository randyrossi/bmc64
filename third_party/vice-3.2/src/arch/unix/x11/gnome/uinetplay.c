/*
 * uinetplay.c - UI controls for netplay
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

#ifdef HAVE_NETWORK

#include "lib.h"
#include "log.h"
#include "network.h"
#include "resources.h"
#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uinetplay.h"
#include "util.h"

static GtkWidget *netplay_dialog, *current_mode, *dcb, *ctrls, *np_server, *np_server_bind, *np_port;
static log_t np_log = LOG_ERR;

typedef struct np_control_s {
    char *name;
    GtkWidget *s_cb;
    GtkWidget *c_cb;
    unsigned int s_mask;
    unsigned int c_mask;
} np_control_t;

#define NR_NPCONROLS 5
static np_control_t np_controls[] = {
    { N_("Keyboard"), NULL, NULL,
      NETWORK_CONTROL_KEYB,
      NETWORK_CONTROL_KEYB << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Joystick 1"), NULL, NULL,
      NETWORK_CONTROL_JOY1,
      NETWORK_CONTROL_JOY1 << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Joystick 2"), NULL, NULL,
      NETWORK_CONTROL_JOY2,
      NETWORK_CONTROL_JOY2 << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Devices"), NULL, NULL,
      NETWORK_CONTROL_DEVC,
      NETWORK_CONTROL_DEVC << NETWORK_CONTROL_CLIENTOFFSET },
    { N_("Settings"), NULL, NULL,
      NETWORK_CONTROL_RSRC,
      NETWORK_CONTROL_RSRC << NETWORK_CONTROL_CLIENTOFFSET },
    { NULL, NULL, NULL, 0, 0 }
};

static void netplay_update_control_res(GtkWidget *w, gpointer data)
{
    unsigned int control, mask;

    g_return_if_fail(GTK_IS_CHECK_BUTTON(w));
    g_return_if_fail(data != 0);

    mask = *((unsigned int *)data);
    resources_get_int("NetworkControl", (int *)&control);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w))) {
        control |= mask;
    } else {
        control &= ~mask;
    }

    resources_set_int("NetworkControl", (int)control);
}

static void netplay_update_control_gui(void)
{
    int i;
    unsigned int control;

    resources_get_int("NetworkControl", (int *)&control);
    for (i = 0; i < NR_NPCONROLS; i++)
    {
        if (control & np_controls[i].s_mask) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(np_controls[i].s_cb), TRUE);
        }
        if (control & np_controls[i].c_mask) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(np_controls[i].c_cb), TRUE);
        }
    }
}

static void netplay_update_resources(void)
{
    const gchar *server_name;
    const gchar *server_bind_address;
    char p[256];
    long port;

    strncpy(p, gtk_entry_get_text(GTK_ENTRY(np_port)), 256);
    server_name = gtk_entry_get_text(GTK_ENTRY(np_server));
    server_bind_address = gtk_entry_get_text(GTK_ENTRY(np_server_bind));
    util_string_to_long(p, NULL, 10, &port);
    if (port < 1 || port > 0xFFFF) {
        ui_error(_("Invalid port number"));
        return;
    }
    resources_set_int("NetworkServerPort", (int)port);
    resources_set_string("NetworkServerName", server_name);
    resources_set_string("NetworkServerBindAddress", server_bind_address);
}

static void netplay_update_status(void)
{
    gchar *text = NULL;
    const char *server_name;
    const char *server_bind_address;
    int port;
    char st[256];

    switch(network_get_mode()) {
        case NETWORK_IDLE:
            gtk_widget_set_sensitive(GTK_WIDGET(dcb), FALSE);
            gtk_widget_set_sensitive(GTK_WIDGET(ctrls), TRUE);
            text = _("Idle");
            break;
        case NETWORK_SERVER:
            gtk_widget_set_sensitive(GTK_WIDGET(dcb), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(ctrls), FALSE);
            text = _("Server listening");
            break;
        case NETWORK_SERVER_CONNECTED:
            gtk_widget_set_sensitive(GTK_WIDGET(dcb), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(ctrls), FALSE);
            text = _("Connected server");
            break;
        case NETWORK_CLIENT:
            gtk_widget_set_sensitive(GTK_WIDGET(dcb), TRUE);
            gtk_widget_set_sensitive(GTK_WIDGET(ctrls), FALSE);
            text = _("Connected client");
            break;
        default:
            break;
    }
    gtk_label_set_text(GTK_LABEL(current_mode), text);

    resources_get_int("NetworkServerPort", &port);
    resources_get_string("NetworkServerName", &server_name);
    resources_get_string("NetworkServerBindAddress", &server_bind_address);
    snprintf(st, 256, "%d", port);
    gtk_entry_set_text(GTK_ENTRY(np_port), st);
    if (server_name[0] == 0) {
        server_name = "127.0.0.1";
    }
    gtk_entry_set_text(GTK_ENTRY(np_server), server_name);

    if (server_bind_address[0] == 0) {
        server_bind_address = "127.0.0.1";
    }
    gtk_entry_set_text(GTK_ENTRY(np_server_bind), server_bind_address);

    log_message(np_log, "Status: %s, Server: %s, Port: %d; server bind address: %s", text, server_name, port, server_bind_address);
    netplay_update_control_gui();
}

static void netplay_start_server(GtkWidget *w, gpointer data)
{
    netplay_update_resources();
    if (network_start_server() < 0) {
        ui_error(_("Couldn't start netplay server."));
    }
    netplay_update_status();
    gtk_dialog_response(GTK_DIALOG(netplay_dialog), GTK_RESPONSE_CANCEL);
}

static void netplay_connect(GtkWidget *w, gpointer data)
{
    netplay_update_resources();
    if (network_connect_client() < 0) {
        ui_error(_("Couldn't connect client."));
    }
    netplay_update_status();
    gtk_dialog_response(GTK_DIALOG(netplay_dialog), GTK_RESPONSE_CANCEL);
}

static void netplay_disconnect(GtkWidget *w, gpointer data)
{
    netplay_update_resources();
    network_disconnect();
    netplay_update_status();
    gtk_dialog_response(GTK_DIALOG(netplay_dialog), GTK_RESPONSE_CANCEL);
}

static GtkWidget *build_netplay_dialog(void)
{
    GtkWidget *d, *f, *b, *hb, *rb, *l, *entry, *h, *v;
    char *unknown = util_concat("<", _("Unknown"), ">", NULL);
    char *connect_to = util_concat(_("Connect to"), " ", NULL);
    char *current_mode_text = util_concat(_("Current mode"), ": ", NULL);
    char *ip = util_concat(_("IP"), ": ", NULL);
    char *port = util_concat(_("Port"), ": ", NULL);
    char *padding = util_concat("      ", NULL);

    d = gtk_dialog_new_with_buttons(_("Netplay Settings"), NULL, GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_resizable(GTK_WINDOW(d), FALSE);

    f = gtk_frame_new(_("Netplay Settings"));

    h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    ctrls = b = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* start row */
    gtk_box_pack_start(GTK_BOX(b), hb, FALSE, FALSE, 5);
    gtk_widget_show(hb);
    hb = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    /* button "start server" */
    rb = gtk_button_new_with_label(_("Start server"));
    gtk_box_pack_start(GTK_BOX(hb), rb, FALSE, FALSE, 10);
    g_signal_connect(G_OBJECT(rb), "clicked", G_CALLBACK(netplay_start_server), rb);
    gtk_widget_set_can_focus(rb, 0);
    gtk_widget_show(rb);

    /* label "IP" */
    l = gtk_label_new(ip);
    gtk_container_add(GTK_CONTAINER(hb), l);
    gtk_widget_show(l);
    /*lib_free(ip);*/

    /* entry IP server bind address */
    np_server_bind = entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hb), entry, FALSE, FALSE, 10);
    gtk_widget_set_size_request(entry, 100, -1);
    gtk_widget_show(entry);

    /* label "Port" */
    l = gtk_label_new(port);
    gtk_container_add(GTK_CONTAINER(hb), l);
    gtk_widget_show(l);
    lib_free(port);

    /* entry port */
    np_port = entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hb), entry, FALSE, FALSE, 10);
    gtk_widget_set_size_request(entry, 50, -1);
    gtk_widget_show(entry);

    /* start row */
    gtk_box_pack_start(GTK_BOX(b), hb, FALSE, FALSE, 10);
    gtk_widget_show(hb);
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* button "connect to server" */
    rb = gtk_button_new_with_label(connect_to);
    gtk_box_pack_start(GTK_BOX(hb), rb, FALSE, FALSE, 10);
    g_signal_connect(G_OBJECT(rb), "clicked", G_CALLBACK(netplay_connect), rb);
    gtk_widget_set_can_focus(rb, 0);
    gtk_widget_show(rb);
    lib_free(connect_to);

    /* label "IP" */
    l = gtk_label_new(ip);
    gtk_container_add(GTK_CONTAINER(hb), l);
    gtk_widget_show(l);
    lib_free(ip);

    /* entry "remote IP" */
    np_server = entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hb), entry, FALSE, FALSE, 10);
    gtk_widget_set_size_request(entry, 100, -1);
    gtk_widget_show(entry);

    /* padding label */
    l = gtk_label_new(padding);
    gtk_container_add(GTK_CONTAINER(hb), l);
    gtk_widget_show(l);

    l = gtk_label_new(padding);
    gtk_container_add(GTK_CONTAINER(hb), l);
    gtk_widget_set_size_request(l, 50, -1);
    gtk_widget_show(l);
    lib_free(padding);

    /* start row */
    gtk_box_pack_start(GTK_BOX(b), hb, FALSE, FALSE, 5);
    gtk_widget_show(hb);
    hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    /* start row */
    gtk_box_pack_start(GTK_BOX(b), hb, FALSE, FALSE, 5);
    gtk_widget_show(hb);

    gtk_container_add(GTK_CONTAINER(f), h);
    gtk_widget_show(h);
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), f, TRUE, TRUE, 10);
    gtk_widget_show(f);
    gtk_box_pack_start(GTK_BOX(h), b, FALSE, FALSE, 5);
    gtk_widget_show(b);

    /* Control widgets */
    {
        GtkWidget *cf, *tmp, *table;
        int i;

        cf = gtk_frame_new(_("Control"));
        h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        v = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(h), v);
        gtk_widget_show(v);
        gtk_container_add(GTK_CONTAINER(cf), h);
        gtk_widget_show(h);
        gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), cf, FALSE, FALSE, 10);
        gtk_widget_show(cf);

        /* content of the "control" frame */
        table = gtk_table_new(NR_NPCONROLS + 1, 3, FALSE);
        tmp = gtk_label_new(_("Server"));
        gtk_table_attach(GTK_TABLE(table), tmp, 1, 2, 0, 1, 0, 0, 5, 0);
        gtk_widget_show(tmp);
        tmp = gtk_label_new(_("Client"));
        gtk_table_attach(GTK_TABLE(table), tmp, 2, 3, 0, 1, 0, 0, 5, 0);
        gtk_widget_show(tmp);

        /*gtk_table_set_homogeneous(table, TRUE);*/

        for (i = 0; i < NR_NPCONROLS; i++) {
            tmp = gtk_label_new(_(np_controls[i].name));
            gtk_table_attach_defaults(GTK_TABLE(table), tmp, 0, 1, i + 1, i + 2);
            gtk_widget_show(tmp);
            np_controls[i].s_cb = gtk_check_button_new();
            gtk_table_attach_defaults(GTK_TABLE(table), np_controls[i].s_cb, 1, 2, i + 1, i + 2);
            g_signal_connect(G_OBJECT(np_controls[i].s_cb), "toggled", G_CALLBACK(netplay_update_control_res), (gpointer)&np_controls[i].s_mask);
            gtk_widget_show(np_controls[i].s_cb);
            np_controls[i].c_cb = gtk_check_button_new();
            gtk_table_attach_defaults(GTK_TABLE(table), np_controls[i].c_cb, 2, 3, i + 1, i + 2);
            g_signal_connect(G_OBJECT(np_controls[i].c_cb), "toggled", G_CALLBACK(netplay_update_control_res), (gpointer)&np_controls[i].c_mask);
            gtk_widget_show(np_controls[i].c_cb);
        }
        gtk_box_pack_start(GTK_BOX(v), table, FALSE, FALSE, 10);
        gtk_widget_show(table);
    }

    /* disconnect button */
    dcb = rb = gtk_button_new_with_label(_("Disconnect"));
    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), rb, FALSE, FALSE, 10);
    g_signal_connect(G_OBJECT(rb), "clicked", G_CALLBACK(netplay_disconnect), rb);
    gtk_widget_set_can_focus(rb, 0);
    gtk_widget_show(rb);

    h = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    /* label "current mode */
    l = gtk_label_new(current_mode_text);
    gtk_container_add(GTK_CONTAINER(h), l);
    gtk_widget_show(l);
    /* actual label used for the status */
    current_mode = gtk_label_new(unknown);
    lib_free(unknown);
    lib_free(current_mode_text);
    gtk_container_add(GTK_CONTAINER(h), current_mode);
    gtk_widget_show(current_mode);

    gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(d))), h, FALSE, FALSE, 10);
    gtk_widget_show(h);

    netplay_update_status();

    /* gtk_dialog_close_hides(GTK_DIALOG(d), TRUE); */
    return d;
}

void ui_netplay_dialog(void)
{
    if (netplay_dialog) {
        gdk_window_show(gtk_widget_get_window(netplay_dialog));
        gdk_window_raise(gtk_widget_get_window(netplay_dialog));
        gtk_widget_show(netplay_dialog);
        netplay_update_status();
    } else {
        np_log = log_open("Netplay");
        netplay_dialog = build_netplay_dialog();
        g_signal_connect(G_OBJECT(netplay_dialog), "destroy", G_CALLBACK(gtk_widget_destroyed), &netplay_dialog);
    }
    ui_popup(netplay_dialog, "Netplay Dialog", FALSE);
    (void) gtk_dialog_run(GTK_DIALOG(netplay_dialog));
    ui_popdown(netplay_dialog);
}

#endif /* HAVE_NETWORK */
