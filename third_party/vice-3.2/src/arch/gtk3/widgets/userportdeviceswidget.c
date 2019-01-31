/** \file   userportdeviceswidget.c
 * \brief   Widget to select userport devices
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Userport4bitSampler x64 x64sc xscpu64 x128 xcbm2
 * $VICERES Userport8BSS        x64 x64sc xscpu64 x128 xcbm2
 * $VICERES UserportDAC         x64 x64sc xscpu64 x128 xcbm2 xvic xpet xplus4
 * $VICERES UserportDIGIMAX     x64 x64sc xscpu64 x128 xcbm2
 * $VICERES Userport58321a      x64 x64sc xscpu64 x128 xcbm2 xvic xpet
 * $VICERES Userport58321aSave  x64 x64sc xscpu64 x128 xcbm2 xvic xpet
 * $VICERES UserportDS1307      x64 x64sc xscpu64 x128 xcbm2 xvic xpet
 * $VICERES UserportDS1307Save  x64 x64sc xscpu64 x128 xcbm2 xvic xpet
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

#include <gtk/gtk.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lib.h"
#include "util.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "machine.h"
#include "resources.h"

#include "userportdeviceswidget.h"


/*
 * Used for the event handlers
 */
static GtkWidget *rtc_58321a = NULL;
static GtkWidget *rtc_58321a_save = NULL;
static GtkWidget *rtc_ds1307 = NULL;
static GtkWidget *rtc_ds1307_save = NULL;


/** \brief  Handler for the "toggled" event of the RTC58321a widget
 *
 * Enables/disables the related Save widget, depending on the state of the
 * RTC58321a widget.
 *
 * \param[in]   widget      RTC58321a widget
 * \param[in]   user_data   unused
 */
static void on_58321a_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(rtc_58321a_save,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


/** \brief  Handler for the "toggled" event of the RTCDS1307 widget
 *
 * Enables/disables the related Save widget, depending on the state of the
 * RTCDS1307 widget
 *
 * \param[in]   widget      RTCDS1307 widget
 * \param[in]   user_data   unused
 */
static void on_ds1307_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(rtc_ds1307_save,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


/** \brief  Create widget for the "Userport4bitSampler" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_4bit_sampler_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "Userport4bitSampler", "Enable 4 bit sampler");
}


/** \brief  Create widget for the "Userport8BSS" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_8bit_stereo_sampler_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "Userport8BSS", "Enable 8 bit stereo sampler");
}


/** \brief  Create widget for the "UserportDAC" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_dac_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportDAC", "Enable 8 bit DAC");
}


/** \brief  Create widget for the "UserportDIGIMAX" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_digimax_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportDIGIMAX", "Enable DIGIMAX");
}


/** \brief  Create widget for the "UserportRTC58321a" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rtc_58321a_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportRTC58321a", "Enable RTC (58321a)");
}


/** \brief  Create widget for the "UserportRTC58321aSave" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rtc_58321a_save_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportRTC58321aSave", "Enable RTC (58321a) saving");
}


/** \brief  Create widget for the "UserportRTCDS1307" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rtc_ds1307_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportRTCDS1307", "Enable RTC (DS1307)");
}


/** \brief  Create widget for the "UserportRTCDS1307Save" resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rtc_ds1307_save_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "UserportRTCDS1307Save", "Enable RTC (DS1307) saving");
}



/** \brief  Create layout for x64/x64sc/xscpu64/x128/xcbm2
 *
 * \param[in,out]   grid    grid to use for the widgets
 */
static void create_c64_cbm2_layout(GtkWidget *grid)
{
    gtk_grid_attach(GTK_GRID(grid), create_4bit_sampler_widget(), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_8bit_stereo_sampler_widget(),
            0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_dac_widget(), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_digimax_widget(), 0, 3, 1, 1);

    rtc_58321a = create_rtc_58321a_widget();
    rtc_58321a_save = create_rtc_58321a_save_widget();
    g_signal_connect(rtc_58321a, "toggled", G_CALLBACK(on_58321a_toggled),
            NULL);
    on_58321a_toggled(rtc_58321a, NULL);

    gtk_grid_attach(GTK_GRID(grid), rtc_58321a, 0, 4, 1, 1);
    g_object_set(rtc_58321a_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rtc_58321a_save, 0, 5, 1, 1);

    rtc_ds1307 = create_rtc_ds1307_widget();
    rtc_ds1307_save = create_rtc_ds1307_save_widget();
    g_signal_connect(rtc_ds1307, "toggled", G_CALLBACK(on_ds1307_toggled),
            NULL);
    on_ds1307_toggled(rtc_ds1307, NULL);

    gtk_grid_attach(GTK_GRID(grid), rtc_ds1307, 0, 6, 1, 1);
    g_object_set(rtc_ds1307_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rtc_ds1307_save, 0, 7, 1, 1);
}


/** \brief  Create layout for xvic/xpet
 *
 * \param[in,out]   grid    grid to use for the widgets
 */
static void create_vic20_pet_layout(GtkWidget *grid)
{
    gtk_grid_attach(GTK_GRID(grid), create_dac_widget(), 0, 0, 1, 1);

    rtc_58321a = create_rtc_58321a_widget();
    rtc_58321a_save = create_rtc_58321a_save_widget();
    g_signal_connect(rtc_58321a, "toggled", G_CALLBACK(on_58321a_toggled),
            NULL);
    on_58321a_toggled(rtc_58321a, NULL);

    gtk_grid_attach(GTK_GRID(grid), rtc_58321a, 0, 1, 1, 1);
    g_object_set(rtc_58321a_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rtc_58321a_save, 0, 2, 1, 1);

    rtc_ds1307 = create_rtc_ds1307_widget();
    rtc_ds1307_save = create_rtc_ds1307_save_widget();
    g_signal_connect(rtc_ds1307, "toggled", G_CALLBACK(on_ds1307_toggled),
            NULL);
    on_ds1307_toggled(rtc_ds1307, NULL);

    gtk_grid_attach(GTK_GRID(grid), rtc_ds1307, 0, 3, 1, 1);
    g_object_set(rtc_ds1307_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rtc_ds1307_save, 0, 4, 1, 1);
}


/** \brief  Create layout for plus4
 *
 * \param[in,out]   grid    grid to use for the widgets
 */
static void create_plus4_layout(GtkWidget *grid)
{
    gtk_grid_attach(GTK_GRID(grid), create_dac_widget(), 0, 0, 1, 1);
}


/** \brief  Create widget to select userport devices
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *userport_devices_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_CBM6x0:
            create_c64_cbm2_layout(grid);
            break;

        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PET:
            create_vic20_pet_layout(grid);
            break;

        case VICE_MACHINE_PLUS4:
            create_plus4_layout(grid);
            break;

        default:
            /* should never get here */
            break;
    }

    gtk_widget_show_all(grid);
    return grid;
}
