/** \file   vsidmixerwidget.c
 * \brief   GTK3 mixer widget for VSID
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SoundVolume         vsid
 * $VICERES SidResidPassband    vsid
 * $VICERES SidResidGain        vsid
 * $VICERES SidResidFilterBias  vsid
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

#include <stdlib.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "debug.h"
#include "machine.h"
#include "lib.h"
#include "log.h"
#include "resources.h"

#include "vsidmixerwidget.h"

/** \brief  Main volume slider */
static GtkWidget *volume;

#ifdef HAVE_RESID

/** \brief  ReSID passband slider */
static GtkWidget *passband;

/** \brief  ReSID gain slider */
static GtkWidget *gain;

/** \brief  ReSID filter bias slider */
static GtkWidget *bias;

#endif


/** \brief  Handler for the 'clicked' event of the reset button
 *
 * Resets the slider to when the widget was created.
 *
 * \param[in]   widget  button (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_reset_clicked(GtkWidget *widget, gpointer data)
{
    int value;

    resources_get_default_value("SoundVolume", &value);
    gtk_range_set_value(GTK_RANGE(volume), (gdouble)value);
#ifdef HAVE_RESID
    resources_get_default_value("SidResidPassband", &value);
    gtk_range_set_value(GTK_RANGE(passband), (gdouble)value);
    resources_get_default_value("SidResidGain", &value);
    gtk_range_set_value(GTK_RANGE(gain), (gdouble)value);
    resources_get_default_value("SidResidFilterBias", &value);
    gtk_range_set_value(GTK_RANGE(bias), (gdouble)value);
#endif
}


/** \brief  Create slider for main volumne
 *
 * \return  GtkScale
 */
static GtkWidget *create_volume_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SoundVolume",
            GTK_ORIENTATION_HORIZONTAL, 0, 100, 5);
    return scale;
}


#ifdef HAVE_RESID

/** \brief  Create slider for ReSID passband
 *
 * \return  GtkScale
 */
static GtkWidget *create_passband_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidPassband",
            GTK_ORIENTATION_HORIZONTAL, 0, 90, 5);
    return scale;
}


/** \brief  Create slider for ReSID gain
 *
 * \return  GtkScale
 */
static GtkWidget *create_gain_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidGain",
            GTK_ORIENTATION_HORIZONTAL, 90, 100, 1);
    /* gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT); */
    return scale;
}


/** \brief  Create slider for ReSID filter bias
 *
 * \return  GtkScale
 */
static GtkWidget *create_bias_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidFilterBias",
            GTK_ORIENTATION_HORIZONTAL, -5000, 5000, 100);
    return scale;
}
#endif  /* ifdef HAVE_RESID */


/** \brief  Create VSID mixer widget
 *
 * XXX: takes up a little too much vertical space
 *
 * \return  GtkGrid
 */
GtkWidget *vsid_mixer_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, 0);
    g_object_set(G_OBJECT(grid), "margin-right", 16, NULL);
    gtk_widget_set_hexpand(grid, TRUE);

    label = gtk_label_new("Volume");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    volume = create_volume_widget();
    gtk_widget_set_hexpand(volume, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), volume, 1, 0, 1, 1);

#ifdef HAVE_RESID
    label = gtk_label_new("ReSID Passband");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    passband = create_passband_widget();
    gtk_widget_set_hexpand(passband, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), passband, 1, 1, 1, 1);

    label = gtk_label_new("ReSID Gain");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gain = create_gain_widget();
    gtk_widget_set_hexpand(gain, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gain, 1, 2, 1, 1);

    label = gtk_label_new("ReSID Filter Bias");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    bias = create_bias_widget();
    gtk_widget_set_hexpand(bias, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bias, 1, 3, 1, 1);
#endif

    button = gtk_button_new_with_label("Reset to defaults");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);
    g_signal_connect(button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
