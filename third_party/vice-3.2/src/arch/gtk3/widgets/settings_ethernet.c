/** \file   settings_ethernet.c
 * \brief   GTK3 ethernet settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES ETHERNET_INTERFACE  x64 x64sc xscpu64 x128 xvic
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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "resources.h"
#include "lib.h"
#include "log.h"
#include "machine.h"

#ifdef HAVE_RAWNET
# include "rawnet.h"
#endif

#include "settings_ethernet.h"



#ifdef HAVE_RAWNET

/** \brief  Handler for the 'changed' event of the combo box
 *
 * This function attempts to set the ETHERNET_INTERFACE resource.
 *
 * \param[in]   widget  combo box
 * \param[in]   data    extra event data (unused)
 */
static void on_device_combo_changed(GtkWidget *widget, gpointer data)
{
    const char *iface = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));

    if (iface != NULL) {
        if (resources_set_string("ETHERNET_INTERFACE", iface) < 0) {
            log_error(LOG_ERR, "failed to set ETHERNET_INTERFACE to '%s'\n",
                    iface);
        }
    }
}


/** \brief  Create combo box to select the ethernet interface
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_device_combo(void)
{
    GtkWidget *combo;
    const char *iface = NULL;

    /* get current interface */
    if (resources_get_string("ETHERNET_INTERFACE", &iface) < 0) {
        log_error(LOG_ERR, "failed to retrieve ETHERNET_INTERFACE resource, "
                "defaulting to eth0");
        iface = "eth0";
    }

    /* build combo box with a list of interfaces */
    combo = gtk_combo_box_text_new();
    if (rawnet_enumadapter_open()) {
        int i = 0;

        char *if_name;
        char *if_desc;

        while (rawnet_enumadapter(&if_name, &if_desc)) {
            char *display;

            if (if_desc != NULL) {
                display = lib_msprintf("%s (%s)", if_name, if_desc);
            } else {
                display = lib_stralloc(if_name);
            }

            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                    if_name /* ID */, display /* text*/);

            if (strcmp(if_name, iface) == 0) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
            }

            i++;

            lib_free(display);
            lib_free(if_name);
            if (if_desc != NULL) {
                lib_free(if_desc);
            }
        }
        rawnet_enumadapter_close();
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_device_combo_changed),
            NULL);

    return combo;
}
#endif


/** \brief  Create Ethernet settings widget for the settings UI
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_ethernet_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    char *text;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    switch (machine_class) {
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_CBM6x0:   /* fall through */
        case VICE_MACHINE_VSID:

            text = lib_msprintf(
                    "<b>Error</b>: Ethernet not supported for <b>%s</b>, "
                    "please fix the code that calls this code!",
                    machine_name);
            label = gtk_label_new(NULL);
            gtk_label_set_markup(GTK_LABEL(label), text);
            gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
            gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
            lib_free(text);
            gtk_widget_show_all(grid);
            return grid;
        default:
            break;
    }

#ifdef HAVE_RAWNET
    label = gtk_label_new("Ethernet device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_device_combo(), 1, 0, 1, 1);
#else
    label = gtk_label_new("Ethernet not supported, please compile with "
            "--enable-ethernet.");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
#endif

    gtk_widget_show_all(grid);
    return grid;
}
