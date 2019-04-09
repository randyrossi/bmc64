/** \file   settings_autostart.c
 * \brief   GTK3 autostart settings central widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES AutostartDelay                      -vsid
 * $VICERES AutostartDelayRandom                -visd
 * $VICERES AutostartPrgMode                    -vsid
 * $VICERES AutostartPrgDiskImage               -vsid
 * $VICERES AutostartRunWithColon               -vsid
 * $VICERES AutostartBasicLoad                  -vsid
 * $VICERES AutostartWarp                       -vsid
 * $VICERES AutostartHandleTrueDriveEmulation   -vsid
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
#include "machine.h"
#include "resources.h"
#include "autostart-prg.h"
#include "resourcewidgetmanager.h"
#include "uisettings.h"

#include "settings_autostart.h"


/** \brief  Autostart modes for PRG files
 */
static const vice_gtk3_radiogroup_entry_t autostart_modes[] = {
    { "Virtual FS",         AUTOSTART_PRG_MODE_VFS /* 0 */ },
    { "Inject into RAM",    AUTOSTART_PRG_MODE_INJECT /* 1 */ },
    { "Copy to D64",        AUTOSTART_PRG_MODE_DISK /* 2 */ },
   { NULL, -1 }
};


/** \brief  Keeps track of resources in the UI
 */
static resource_widget_manager_t manager;



/*
 * Event handlers
 */


/** \brief  Handler for the "destroy" event of the main widget
 *
 * Cleans up resource used by the resource widget manager
 *
 * \param[in]   widget  widget triggering the event (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer data)
{
    debug_gtk3("calling vice_resource_wiget_manager_exit()");
    vice_resource_widget_manager_exit(&manager);
}


/*
 * Widget helpers
 */


/** \brief  Create widget to control "AutostartDelay" resource
 *
 * \return  grid
 */
static GtkWidget *create_fixed_delay_widget(void)
{
    GtkWidget *layout;
    GtkWidget *label;
    GtkWidget *spin;
    GtkWidget *help;

    layout = vice_gtk3_grid_new_spaced(16, 0);

    label  = gtk_label_new("Autostart fixed delay:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin", 8, NULL);

    spin = vice_gtk3_resource_spin_int_new("AutostartDelay",
            0, 1000, 1);
    /* register with manager */
    vice_resource_widget_manager_add_widget(&manager, spin, NULL, NULL, NULL, NULL);

    gtk_grid_attach(GTK_GRID(layout), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), spin, 1, 0, 1, 1);

    label = gtk_label_new("seconds");
    gtk_grid_attach(GTK_GRID(layout), label, 2, 0, 1, 1);

    help = gtk_label_new("0 = machine-specific delay for KERNAL boot");
    gtk_grid_attach(GTK_GRID(layout), help, 0, 2, 3, 1);

    gtk_widget_show_all(layout);
    return layout;
}



/** \brief  Create widget to control "AutostartDelay[Random]" resources
 *
 * \return  grid
 */
static GtkWidget *create_delay_widget(void)
{
    GtkWidget *grid;
    GtkWidget *rnd_delay;
    GtkWidget *fix_delay;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Delay settings", 3);
    g_object_set(grid, "margin-top", 8, NULL);

    rnd_delay = vice_gtk3_resource_check_button_new( "AutostartDelayRandom",
            "Add random delay");
    vice_resource_widget_manager_add_widget(&manager, rnd_delay,
            NULL, NULL, NULL, NULL);
    g_object_set(rnd_delay, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rnd_delay, 0, 2, 1, 1);

    fix_delay = create_fixed_delay_widget();
    g_object_set(fix_delay, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), fix_delay, 0, 3, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control "AutostartPrgDiskImage"
 *
 * \return grid
 */
static GtkWidget *create_prg_diskimage_widget(void)
{
    GtkWidget *grid;
    GtkWidget *image;
    const char *patterns[] = { "*.d64", NULL };

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Autostart disk image", 1);

    image = vice_gtk3_resource_browser_new("AutostartPrgDiskImage",
            patterns, "D64 disk images", "Select D64 image",
            "autostart disk image", NULL);
    vice_resource_widget_manager_add_widget(&manager, image,
            NULL, NULL, NULL, NULL);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control PRG-related autostart resources
 *
 * \return  grid
 */
static GtkWidget *create_prg_widget(void)
{
    GtkWidget *grid;
    GtkWidget *colon;
    GtkWidget *basic;
    GtkWidget *mode;
    GtkWidget *group;
    GtkWidget *image;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "PRG settings", 3);
    g_object_set(grid, "margin-top", 8, NULL);

    colon = vice_gtk3_resource_check_button_new("AutostartRunWithColon",
            "Use ':' with RUN");
    vice_resource_widget_manager_add_widget(&manager, colon,
            NULL, NULL, NULL, NULL);
    g_object_set(colon, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), colon, 0, 1, 1, 1);

    basic = vice_gtk3_resource_check_button_new("AutostartBasicLoad",
            "Load to BASIC start");
    vice_resource_widget_manager_add_widget(&manager, basic,
            NULL, NULL, NULL, NULL);
    g_object_set(basic, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), basic, 0, 2, 1, 1);

    mode = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Autostart PRG mode", 1);
    group = vice_gtk3_resource_radiogroup_new("AutostartPrgMode",
            autostart_modes, GTK_ORIENTATION_VERTICAL);
    vice_resource_widget_manager_add_widget(&manager, group,
            NULL, NULL, NULL, NULL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(mode), group, 0, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), mode, 0, 3, 1, 1);

    image = create_prg_diskimage_widget();
    gtk_grid_attach(GTK_GRID(grid), image, 0, 4, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to use in the settings dialog for autostart resources
 *
 * \param[in[   parent  parent widget (unused)
 *
 * \return  grid
 */
GtkWidget *settings_autostart_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *tde;
    GtkWidget *warp;

    /* initialize resource widget manager and register with uisettings */
    debug_gtk3("Initializing resource widget manager and registering it with"
            " uisettings.c");
    vice_resource_widget_manager_init(&manager);
    ui_settings_set_resource_widget_manager(&manager);

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    g_object_set(grid, "margin", 8, NULL);

    tde = vice_gtk3_resource_check_button_new(
            "AutostartHandleTrueDriveEmulation",
            "Handle True Drive Emulation on autostart");
    gtk_grid_attach(GTK_GRID(grid), tde, 0, 0, 1, 1);
    vice_resource_widget_manager_add_widget(&manager, tde, NULL, NULL, NULL, NULL);

    warp = vice_gtk3_resource_check_button_new("AutostartWarp",
            "Warp on autostart");
    gtk_grid_attach(GTK_GRID(grid), warp, 0, 1, 1, 1);
    vice_resource_widget_manager_add_widget(&manager, warp, NULL, NULL, NULL, NULL);

    gtk_grid_attach(GTK_GRID(grid), create_delay_widget(),
            0, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_prg_widget(),
            0, 3, 1, 1);


    g_signal_connect(grid, "destroy", G_CALLBACK(on_destroy), NULL);

    vice_resource_widget_manager_dump(&manager);
    gtk_widget_show_all(grid);


    return grid;
}
