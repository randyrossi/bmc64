/** \file   settings_drive.c
 * \brief   Drive settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES DriveTrueEmulation          -vsid
 * $VICERES DriveSoundEmulation         -vsid
 * $VICERES DriveSoundEmulationVolume   -vsid
 *
 *  (for more, see used widgets)
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
#include "debug_gtk3.h"
#include "drive.h"
#include "drive-check.h"
#include "machine.h"
#include "resources.h"
#include "drivewidgethelpers.h"
#include "driveunitwidget.h"
#include "drivemodelwidget.h"
#include "driveextendpolicywidget.h"
#include "driveidlemethodwidget.h"
#include "driveparallelcablewidget.h"
#include "driverpmwidget.h"
#include "driveramwidget.h"
#include "drivedoswidget.h"
#include "drivefsdevicewidget.h"
#include "driveoptionswidget.h"

#include "settings_drive.h"

/* this needs documentation and the literal 4 sucks */
static GtkWidget *drive_model[4];
static GtkWidget *drive_options[4];
static GtkWidget *drive_extend[4];
static GtkWidget *drive_idle[4];
static GtkWidget *drive_parallel[4];
static GtkWidget *drive_rpm[4];
static GtkWidget *drive_ram[4];
static GtkWidget *drive_dos[4];
static GtkWidget *drive_fsdevice[4];


/** \brief  Callback for changes in the drive type widget
 *
 * XXX: I could use gtk_widget_get_parent() on the \a widget argument to get
 *      the stack child widget, but perhaps I'll need to wrap widgets into
 *      more GtkGrid's to get the layout right, and that would make a simple
 *      parent lookup fail (it would return the wrapper grid). Then again if
 *      I do use wrappers, this code will still fail.
 *      What did I get myself into, just to avoid globals and to make the
 *      glue logic simpler?     -- compyx
 *
 * TODO:    Change behaviour, depending on machine class
 *
 * \param[in,out]   widget  drive type widget
 * \param[in,out]   data    the child widget of the GtkStack
 */
static void stack_child_drive_type_callback(GtkWidget *widget, gpointer data)
{
#if 0
    GtkWidget *drive_extend;
    GtkWidget *drive_expand;
    GtkWidget *drive_parallel;
    int unit;
    int type;

    debug_gtk3("called.");

    unit = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "UnitNumber"));
    resources_get_int_sprintf("Drive%dType", &type, unit);

    /* determine if extend-policy is valid for current unit number and its
     * drive type */
    drive_extend = gtk_grid_get_child_at(GTK_GRID(data),
            CHILD_DRIVE_EXTEND_XPOS, CHILD_DRIVE_EXTEND_YPOS);
    if (drive_extend != NULL) {
        gtk_widget_set_sensitive(drive_extend,
                drive_check_extend_policy(type));
    }

    /* determine which expansions are valid for current unit number and its
     * drive type */
    drive_expand = gtk_grid_get_child_at(GTK_GRID(data),
            CHILD_DRIVE_EXPAND_XPOS, CHILD_DRIVE_EXPAND_YPOS);
    if (drive_expand != NULL) {
        drive_expansion_widget_update(drive_expand);
    }

    /* determine which parallel cables are valid for current unit number and
     * its drive type */
    drive_parallel = gtk_grid_get_child_at(GTK_GRID(data),
            CHILD_DRIVE_PARALLEL_XPOS, CHILD_DRIVE_PARALLEL_YPOS);
    if (drive_parallel != NULL) {
        drive_parallel_cable_widget_update(drive_parallel);
    }
#endif

}


/** \brief  Extra event handler for the drive model changes
 *
 * \param[in]   widget      drive type radio button
 * \param[in]   user_data   unit number
 */
static void on_model_changed(GtkWidget *widget, gpointer user_data)
{
    int unit = GPOINTER_TO_INT(user_data);
    int model = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "ModelID"));
    GtkWidget *option;

    debug_gtk3("called, unit is #%d, model ID = %d.", unit, model);

    option = drive_extend[unit - 8];
    if (option != NULL) {
        gtk_widget_set_sensitive(option, drive_check_extend_policy(model));
    }
    option = drive_parallel[unit - 8];
    if (option != NULL) {
        gtk_widget_set_sensitive(option, drive_check_parallel_cable(model));
    }
    option = drive_idle[unit - 8];
    if (option != NULL) {
        gtk_widget_set_sensitive(option, drive_check_idle_method(model));
    }

    /* RAM extensions */
    option = drive_ram[unit - 8];
    if (option != NULL) {
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 1),
                drive_check_expansion2000(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 2),
                drive_check_expansion4000(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 3),
                drive_check_expansion6000(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 4),
                drive_check_expansion8000(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 5),
                drive_check_expansionA000(model));
    }

    /* DOS extensions */
    option = drive_dos[unit - 8];
    if (option != NULL) {
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 1),
                drive_check_profdos(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 2),
                drive_check_stardos(model));
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(option), 0, 3),
                drive_check_supercard(model));
    }

    /* drive options widget */
    option = drive_options[unit - 8];
    if (option != NULL) {
        GtkWidget *iec = gtk_grid_get_child_at(GTK_GRID(option), 1, 0);
        GtkWidget *rtc = gtk_grid_get_child_at(GTK_GRID(option), 2, 0);

        if (iec != NULL) {
            gtk_widget_set_sensitive(iec, drive_check_iec(model));
        }
        if (rtc != NULL) {
            gtk_widget_set_sensitive(rtc, drive_check_rtc(model));
        }
    }
}


/** \brief  Format the drive emulation volume value
 *
 * Turns the volume scale into 0-100
 *
 * \param[in]   scale   drive volume widget
 * \param[in]   value   value of widget to format
 *
 * \return  heap-allocated string representing the current value
 *
 * \note    the string returned here appears to be deallocated by Gtk3. if
 *          I assume the code example of Gtk3 is correct (compyx)
 */
static gchar *on_drive_volume_format(GtkScale *scale, gdouble value)
{
    return g_strdup_printf("%d", (int)(value / 40));
}


/** \brief  Create slider to control drive sound emulation volume
 *
 * \return  GtkScale
 */
static GtkWidget *create_drive_volume_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("DriveSoundEmulationVolume",
            GTK_ORIENTATION_HORIZONTAL, 0, 4000, 100);
    gtk_widget_set_hexpand(scale, TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);
    g_signal_connect(scale, "format-value", G_CALLBACK(on_drive_volume_format),
            NULL);
    return scale;
}


/** \brief  Create layout for x64/x64sc/xscpu64 and x128
 *
 * \param[in]   grid    main widget grid
 * \param[in]   unit    unit number
 *
 * \return  \a grid
 */
static GtkWidget *create_vic20_layout(GtkWidget *grid, int unit)
{
    GtkWidget *wrapper;

    /* row 0 & 1, column 0 */

    wrapper = gtk_grid_new();

    drive_model[unit - 8] = drive_model_widget_create(unit);
    drive_model_widget_add_callback(drive_model[unit- 8],
            stack_child_drive_type_callback,
            (gpointer)(grid));
    gtk_grid_attach(GTK_GRID(wrapper), drive_model[unit - 8], 0, 0, 1, 1);
    drive_options[unit - 8] = drive_options_widget_create(unit);
    gtk_grid_attach(GTK_GRID(wrapper), drive_options[unit - 8], 0, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 0, 1, 2);

    /* row 0, column 1 */
    drive_ram[unit - 8] = drive_ram_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_ram[unit - 8], 1, 0, 1, 1);

    /* row 1, column 1 */
    /*    drive_dos = drive_dos_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_dos, 1, 1, 1, 1);
    */

    /* row 0 & 1, column 2 */
    wrapper = gtk_grid_new();
    drive_extend[unit - 8] = drive_extend_policy_widget_create(unit);
    drive_idle[unit - 8] = drive_idle_method_widget_create(unit);
    /*    drive_parallel = drive_parallel_cable_widget_create(unit); */
    gtk_grid_attach(GTK_GRID(wrapper), drive_extend[unit - 8], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), drive_idle[unit - 8], 0, 1, 1, 1);
    /*    gtk_grid_attach(GTK_GRID(wrapper), drive_parallel, 0, 2, 1, 1); */
    gtk_widget_show_all(wrapper);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 2, 0, 1, 2);

    /* row 2, column 0 */
    drive_rpm[unit- 8] = drive_rpm_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_rpm[unit - 8], 0, 2, 1, 1);

    /* row 2, column 1 & 2 */
    drive_fsdevice[unit - 8] = drive_fsdevice_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_fsdevice[unit - 8], 1, 2, 2, 1);
    drive_model_widget_add_callback(drive_model[unit - 8], on_model_changed,
            GINT_TO_POINTER(unit));
    return grid;
}


/** \brief  Create layout for xvic
 *
 * \param[in]   grid    main widget grid
 * \param[in]   unit    unit number
 *
 * \return  \a grid
 */
static GtkWidget *create_c64_layout(GtkWidget *grid, int unit)
{
    GtkWidget *wrapper;

    /* row 0 & 1, column 0 */

    wrapper = gtk_grid_new();

    drive_model[unit - 8] = drive_model_widget_create(unit);
    drive_model_widget_add_callback(drive_model[unit - 8], stack_child_drive_type_callback,
            (gpointer)(grid));
    gtk_grid_attach(GTK_GRID(wrapper), drive_model[unit - 8], 0, 0, 1, 1);
    drive_options[unit - 8] = drive_options_widget_create(unit);
    gtk_grid_attach(GTK_GRID(wrapper), drive_options[unit - 8], 0, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 0, 1, 2);

    /* row 0, column 1 */
    drive_ram[unit - 8] = drive_ram_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_ram[unit - 8], 1, 0, 1, 1);

    /* row 1, column 1 */
    drive_dos[unit - 8] = drive_dos_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_dos[unit - 8], 1, 1, 1, 1);

    /* row 0 & 1, column 2 */
    wrapper = gtk_grid_new();
    drive_extend[unit - 8] = drive_extend_policy_widget_create(unit);
    drive_idle[unit - 8] = drive_idle_method_widget_create(unit);
    drive_parallel[unit - 8] = drive_parallel_cable_widget_create(unit);
    gtk_grid_attach(GTK_GRID(wrapper), drive_extend[unit - 8], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), drive_idle[unit - 8], 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), drive_parallel[unit - 8], 0, 2, 1, 1);
    gtk_widget_show_all(wrapper);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 2, 0, 1, 2);

    /* row 2, column 0 */
    drive_rpm[unit - 8] = drive_rpm_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_rpm[unit - 8], 0, 2, 1, 1);

    /* row 2, column 1 & 2 */
    drive_fsdevice[unit - 8] = drive_fsdevice_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_fsdevice[unit-8], 1, 2, 2, 1);

    drive_model_widget_add_callback(drive_model[unit - 8], on_model_changed,
            GINT_TO_POINTER(unit));
    return grid;
}


/** \brief  Create layout for xplus4
 *
 * \param[in]   grid    main widget grid
 * \param[in]   unit    unit number
 *
 * \return  \a grid
 */
static GtkWidget *create_plus4_layout(GtkWidget *grid, int unit)
{
    GtkWidget *wrapper;

    /* row 0 & 1, column 0 */

    wrapper = gtk_grid_new();

    drive_model[unit -8] = drive_model_widget_create(unit);
    drive_model_widget_add_callback(drive_model[unit - 8],
            stack_child_drive_type_callback,
            (gpointer)(grid));
    gtk_grid_attach(GTK_GRID(wrapper), drive_model[unit - 8], 0, 0, 1, 1);
    drive_options[unit - 8] = drive_options_widget_create(unit);
    gtk_grid_attach(GTK_GRID(wrapper), drive_options[unit - 8], 0, 1, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 0, 1, 2);

    /* row 0, column 1 */
#if 0
    drive_ram = drive_ram_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_ram, 1, 0, 1, 1);

    /* row 1, column 1 */
    drive_dos = drive_dos_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_dos, 1, 1, 1, 1);
#endif
    /* row 0 & 1, column 2 */
    wrapper = gtk_grid_new();
    drive_extend[unit - 8] = drive_extend_policy_widget_create(unit);
    drive_idle[unit - 8]  = drive_idle_method_widget_create(unit);

    /* FIXME: vice.texi mentions parallel support for Plus4, the Gtk2 UI does
     *        not provide this
     */
#if 0
    drive_parallel = drive_parallel_cable_widget_create(unit);
#endif
    gtk_grid_attach(GTK_GRID(wrapper), drive_extend[unit - 8], 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), drive_idle[unit - 8], 0, 1, 1, 1);
#if 0
    gtk_grid_attach(GTK_GRID(wrapper), drive_parallel, 0, 2, 1, 1);
#endif
    gtk_widget_show_all(wrapper);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 2, 0, 1, 2);

    /* row 2, column 0 */
    drive_rpm[unit - 8] = drive_rpm_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_rpm[unit - 8], 0, 2, 1, 1);

    /* row 2, column 1 & 2 */
    drive_fsdevice[unit - 8] = drive_fsdevice_widget_create(unit);
    gtk_grid_attach(GTK_GRID(grid), drive_fsdevice[unit - 8], 1, 2, 2, 1);

    drive_model_widget_add_callback(drive_model[unit - 8], on_model_changed,
            GINT_TO_POINTER(unit));
    return grid;
}



/** \brief  Create a composite widget with settings for drive \a unit
 *
 * \param[in]   unit    unit number
 *
 * \return  GtkGrid
 */
static GtkWidget *create_stack_child_widget(int unit)
{
    GtkWidget *grid;
    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    g_object_set(grid, "margin-left", 8, NULL);
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:
            create_c64_layout(grid, unit);
            break;
        case VICE_MACHINE_VIC20:
            create_vic20_layout(grid, unit);
            break;
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_CBM6x0:   /* fall through */
        case VICE_MACHINE_C64DTV:
            create_plus4_layout(grid, unit);
            break;
        default:
            break;
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create main drive settings widget
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_drive_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *wrapper;
    GtkWidget *tde;
    GtkWidget *sound;
    GtkWidget *stack;
    GtkWidget *switcher;
    GtkWidget *volume;
    GtkWidget *label;
    GtkWidget *volume_wrapper;
    int unit;

    /* three column wide grid */
    layout = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(layout), 16);
    gtk_grid_set_row_spacing(GTK_GRID(layout), 16);

    wrapper = gtk_grid_new();
    g_object_set(wrapper, "margin-left", 16, NULL);
    gtk_grid_set_column_spacing(GTK_GRID(wrapper), 16);
    tde = vice_gtk3_resource_check_button_new("DriveTrueEmulation",
            "True drive emulation");
    gtk_grid_attach(GTK_GRID(wrapper), tde, 0, 0, 1, 1);
    sound = vice_gtk3_resource_check_button_new("DriveSoundEmulation",
            "Drive sound emulation");
    gtk_grid_attach(GTK_GRID(wrapper), sound, 1, 0, 1, 1);

    volume_wrapper = gtk_grid_new();
    label = gtk_label_new("Drive volume:");
    volume = create_drive_volume_widget();
    g_object_set(volume, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(volume_wrapper), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(volume_wrapper), volume, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), volume_wrapper, 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(layout), wrapper, 0, 0, 1, 1);

    stack = gtk_stack_new();
    for (unit = 8; unit < 12; unit++) {
        char title[256];

        g_snprintf(title, 256, "Drive %d", unit);
        gtk_stack_add_titled(GTK_STACK(stack),
                create_stack_child_widget(unit),
                title, title);
    }
    gtk_stack_set_transition_type(GTK_STACK(stack),
            GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);

    switcher = gtk_stack_switcher_new();
    gtk_widget_set_halign(switcher, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(switcher, TRUE);
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(switcher),
            GTK_STACK(stack));

    gtk_widget_show_all(stack);
    gtk_widget_show_all(switcher);

    gtk_grid_attach(GTK_GRID(layout), switcher, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(layout), stack, 0, 2, 3, 1);

    gtk_widget_show_all(layout);
    return layout;
}
