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
#include "resourcewidgetmanager.h"
#include "uisettings.h"

#include "settings_ethernet.h"


#ifdef HAVE_RAWNET
static void clean_iface_list(void);
#endif


/** \brief  Resource widget manager instance
 */
static resource_widget_manager_t manager;


/** \brief  Handler for the 'destroy' event of the main widget
 *
 * \param[in]   widget  main widget (grid)
 * \param[in]   data    extra event data (unused)
 */
static void on_settings_ethernet_destroy(GtkWidget *widget, gpointer data)
{
#ifdef HAVE_RAWNET
    clean_iface_list();
#endif
    vice_resource_widget_manager_exit(&manager);
}



#ifdef HAVE_RAWNET

/** \brief  List of available interfaces
 *
 * This list is dynamically generated and destroyed when the main widget
 * is destroyed.
 */
static vice_gtk3_combo_entry_str_t *iface_list;


/** \brief  Build interface list for the combo box
 *
 * \return  bool
 */
static gboolean build_iface_list(void)
{
    int num = 0;
    char *if_name;
    char *if_desc;

    /* get number of adapters */
    if (!rawnet_enumadapter_open()) {
        return FALSE;
    }
    while (rawnet_enumadapter(&if_name, &if_desc)) {
        lib_free(if_name);
        if (if_desc != NULL) {
            lib_free(if_desc);
        }
        num++;
    }
    rawnet_enumadapter_close();

    /* allocate memory for list */
    iface_list = lib_malloc((size_t)(num + 1) * sizeof *iface_list);

    /* now add the list items */
    if (!rawnet_enumadapter_open()) {
        lib_free(iface_list);
        iface_list = NULL;
        return FALSE;
    }

    num = 0;
    while (rawnet_enumadapter(&if_name, &if_desc)) {
        iface_list[num].id = lib_stralloc(if_name);
        /*
         * On Windows, the description string seems to be always present, on
         * Unix this isn't the case and NULL can be returned.
         */
        if (if_desc == NULL) {
            iface_list[num].name = lib_stralloc(if_name);
        } else {
            iface_list[num].name = lib_msprintf("%s (%s)", if_name, if_desc);
        }
        lib_free(if_name);
        if (if_desc != NULL) {
            lib_free(if_desc);
        }

        num++;
    }
    iface_list[num].id = NULL;
    iface_list[num].name = NULL;
    rawnet_enumadapter_close();
    return TRUE;
}


/** \brief  Free memory used by the interface list
 */
static void clean_iface_list(void)
{
    if (iface_list != NULL) {
        int num = 0;
        while (iface_list[num].id != NULL) {
            lib_free(iface_list[num].id);
            lib_free(iface_list[num].name);
            num++;
        }
        lib_free(iface_list);
        iface_list = NULL;
    }
}



/** \brief  Create combo box to select the ethernet interface
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_device_combo(void)
{
    GtkWidget *combo;

    if (build_iface_list()) {
        combo = vice_gtk3_resource_combo_box_str_new("ETHERNET_INTERFACE",
                iface_list);
    } else {
        combo = gtk_combo_box_text_new();
    }

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
#ifdef HAVE_RAWNET
    GtkWidget *combo;
#endif

    /* initialize and register resource widget manager */
    vice_resource_widget_manager_init(&manager);
    ui_settings_set_resource_widget_manager(&manager);


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

    combo = create_device_combo();
    vice_resource_widget_manager_add_widget(&manager, combo, NULL,
            NULL, NULL, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 0, 1, 1);

#else
    label = gtk_label_new("Ethernet not supported, please compile with "
            "--enable-ethernet.");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
#endif

    g_signal_connect(grid, "destroy", G_CALLBACK(on_settings_ethernet_destroy),
            NULL);


    gtk_widget_show_all(grid);
    return grid;
}
