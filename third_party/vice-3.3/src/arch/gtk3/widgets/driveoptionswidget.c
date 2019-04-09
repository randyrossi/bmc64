/** \file   driveoptionswidget.c
 * \brief   Drive options widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IECDevice8              -vsid
 * $VICERES IECDevice9              -vsid
 * $VICERES IECDevice10             -vsid
 * $VICERES IECDevice11             -vsid
 * $VICERES AttachDevice8Readonly   -vsid
 * $VICERES AttachDevice9Readonly   -vsid
 * $VICERES AttachDevice10Readonly  -vsid
 * $VICERES AttachDevice11Readonly  -vsid
 * $VICERES Drive8RTCSave           -vsid -xcbm5x0 -xcbm2 -xpet
 * $VICERES Drive9RTCSave           -vsid -xcbm5x0 -xcbm2 -xpet
 * $VICERES Drive10RTCSave          -vsid -xcbm5x0 -xcbm2 -xpet
 * $VICERES Drive11RTCSave          -vsid -xcbm5x0 -xcbm2 -xpet
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

#include "debug_gtk3.h"
#include "drive.h"
#include "drive-check.h"
#include "machine.h"
#include "resources.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "drivewidgethelpers.h"

#include "driveoptionswidget.h"



static GtkWidget *create_iec_check_button(int unit)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new_sprintf(
            "IECDevice%d", "IEC Device", unit);
    return check;
}


static GtkWidget *create_readonly_check_button(int unit)
{
    return vice_gtk3_resource_check_button_new_sprintf("AttachDevice%dReadonly",
            "Read Only", unit);
}



static GtkWidget *create_rtc_check_button(int unit)
{
    GtkWidget *check;
    int drive_type;

    check = vice_gtk3_resource_check_button_new_sprintf("Drive%dRTCSave",
            "RTC Save", unit);

    drive_type = ui_get_drive_type(unit);
    gtk_widget_set_sensitive(check,
            drive_type == DRIVE_TYPE_2000 || drive_type == DRIVE_TYPE_4000);
    return check;
}



/** \brief  Create drive options widget for \a unit
 *
 * \param[in]   unit    drive unit number
 *
 * \return GtkGrid
 */
GtkWidget *drive_options_widget_create(int unit)
{
    GtkWidget *grid;
    GtkWidget *iec_widget = NULL;
    GtkWidget *readonly_widget;
    GtkWidget *rtc_widget = NULL;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));
    g_object_set(grid, "margin-left", 16, NULL);

    readonly_widget = create_readonly_check_button(unit);
    gtk_grid_attach(GTK_GRID(grid), readonly_widget, 0, 0, 1, 1);

    switch (machine_class) {
        /* these machines have IEC */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PLUS4:
            iec_widget = create_iec_check_button(unit);
            gtk_grid_attach(GTK_GRID(grid), iec_widget, 1, 0, 1, 1);
            rtc_widget = create_rtc_check_button(unit);
            gtk_grid_attach(GTK_GRID(grid), rtc_widget, 2, 0, 1, 1);
           break;
        default:
            break;
    }
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the options widget using the resources for \a unit
 *
 * \param[in,out]   widget  drive options widget
 * \param[in]       unit    drive unit number (8-11)
 */
void drive_options_widget_update(GtkWidget *widget, int unit)
{

    /* XXX: what was this suppposed to do? Probably deprecated */

#if 0
    unit_number = unit;

    int value;
    char res_name[256];
    GtkWidget *spin;

    uihelpers_get_drive_resource_from_check_button(
            iec_widget, "IECDevice%d", unit);
    uihelpers_get_drive_resource_from_check_button(
            readonly_widget, "AttachDevice%dReadonly", unit);

    /* RPM */
    g_snprintf(res_name, 256, "Drive%dRPM", unit);
    resources_get_int(res_name, &value);
    spin = gtk_grid_get_child_at(GTK_GRID(rpm_widget), 1, 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), (gdouble)(value / 100));

    /* Wobble */
    g_snprintf(res_name, 256, "Drive%dWobble", unit);
    resources_get_int(res_name, &value);
    spin = gtk_grid_get_child_at(GTK_GRID(rpm_widget), 1, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), (gdouble)(value / 100));

    /* RTC */
    if (drive_check_rtc(ui_get_drive_type(unit))) {
        /* supported */
        int state;

        resources_get_int_sprintf("Drive%dRTCSave", &state, unit);
        gtk_widget_set_sensitive(rtc_widget, TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rtc_widget), state);
    } else {
        gtk_widget_set_sensitive(rtc_widget, FALSE);
    }
#endif
}
