/** \file   vsidmixerwidget.c
 * \brief   GTK3 mixer widget for VSID
 *
 * Needs some way of switching between SID model to display the proper values.
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SoundVolume             vsid
 * $VICERES SidResidPassband        vsid
 * $VICERES SidResidGain            vsid
 * $VICERES SidResidFilterBias      vsid
 * $VICERES SidResid8580Passband    vsid
 * $VICERES SidResid8580Gain        vsid
 * $VICERES SidResid8580FilterBias  vsid

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


/** \brief  CSS for the scales
 *
 * This makes the sliders take up less vertical space. The margin can be set
 * to a negative value (in px) to allow the slider to be larger than the scale
 * itself.
 *
 * Probably will require some testing/tweaking to get this to look acceptable
 * with various themes (and OSes).
 */
#define SLIDER_CSS "scale slider { min-width: 10px; min-height: 10px; margin: -3px; } scale { margin-top: -4px; margin-bottom: -4px; }"


/** \brief  CSS for the labels
 *
 * Make font smaller and reduce the vertical size the labels use
 *
 * Here Be Dragons!
 */
#define LABEL_CSS "label { font-size: 80%; margin-top: -2px; margin-bottom: -2px; }"


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
#ifdef HAVE_RESID
    int model;
#endif


    resources_get_default_value("SoundVolume", &value);
    gtk_range_set_value(GTK_RANGE(volume), (gdouble)value);
#ifdef HAVE_RESID

    if (resources_get_int("SidModel", &model) < 0) {
        /* assume 6581 */
        model = 0;
    }

    if (model == 0) {
        resources_get_default_value("SidResidPassband", &value);
    } else {
        resources_get_default_value("SidResid8580Passband", &value);
    }
    gtk_range_set_value(GTK_RANGE(passband), (gdouble)value);

    if (model == 0) {
        resources_get_default_value("SidResidGain", &value);
    } else {
        resources_get_default_value("SidResid8580Gain", &value);
    }
    gtk_range_set_value(GTK_RANGE(gain), (gdouble)value);

    if (model == 0) {
        resources_get_default_value("SidResidFilterBias", &value);
    } else {
        resources_get_default_value("SidResid8580FilterBias", &value);
    }
    gtk_range_set_value(GTK_RANGE(bias), (gdouble)value);
#endif
}


/** \brief  Create a right-align label
 *
 * \param[in]   text    text for the label
 * \param[in]   minimal use CSS to reduce size of use as statusbar widget
 *
 * \return  GtkLabel
 */
static GtkWidget *create_label(const char *text)
{
    GtkWidget *label;

    label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Create a customized GtkScale for \a resource
 *
 * \param[in]   resource    resource name without the video \a chip name prefix
 * \param[in]   low         lower bound
 * \param[in]   high        upper bound
 * \param[in]   step        step used to increase/decrease slider value
 * \param[in]   minimal     reduce slider size to be used in the statusbar
 *
 * \return  GtkScale
 */
static GtkWidget *create_slider(
        const char *resource,
        int low, int high, int step)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new(resource,
            GTK_ORIENTATION_HORIZONTAL, low, high, step);
    gtk_widget_set_hexpand(scale, TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_TOP);

    gtk_scale_set_draw_value(GTK_SCALE(scale), TRUE);
    return scale;
}



/** \brief  Create slider for main volume
 *
 * \param[in]   minimal resize slider to minimal size
 *
 * \return  GtkScale
 */
static GtkWidget *create_volume_widget(void)
{
    return create_slider("SoundVolume", 0, 100, 5);
}


#ifdef HAVE_RESID

/** \brief  Create slider for ReSID passband
 *
 * \return  GtkScale
 */
static GtkWidget *create_passband_widget(int model)
{
    if (model == 0) {
        return create_slider("SidResidPassband", 0, 90, 5);
    } else {
        return create_slider("SidResid8580Passband", 0, 90, 5);
    }
}


/** \brief  Create slider for ReSID gain
 *
 * \return  GtkScale
 */
static GtkWidget *create_gain_widget(int model)
{
    if (model == 0) {
        return create_slider("SidResidGain", 90, 100, 1);
    } else {
        return create_slider("SidResid8580Gain", 90, 100, 1);
    }
}


/** \brief  Create slider for ReSID filter bias
 *
 * \return  GtkScale
 */
static GtkWidget *create_bias_widget(int model)
{
    if (model == 0) {
        return create_slider("SidResidFilterBias", -5000, 5000, 1000);
    } else {
        return create_slider("SidResid8580FilterBias", -5000, 5000, 1000);
    }
}
#endif  /* ifdef HAVE_RESID */


/** \brief  Create VSID mixer widget
 *
 * \return  GtkGrid
 *
 */
GtkWidget *vsid_mixer_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;

#ifdef HAVE_RESID
    int model;

    if (resources_get_int("SidModel", &model) < 0) {
        /* assume 6581 */
        model = 0;
    }
#endif

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, 0);
    g_object_set(G_OBJECT(grid), "margin-right", 16, NULL);
    gtk_widget_set_hexpand(grid, TRUE);

    label = create_label("Volume");
    volume = create_volume_widget();
    gtk_widget_set_hexpand(volume, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), volume, 1, 0, 1, 1);

#ifdef HAVE_RESID

    label = gtk_label_new(NULL);
    if (model == 0) {
        gtk_label_set_markup(GTK_LABEL(label), "<b>ReSID 6581 settings</b>");
    } else {
        gtk_label_set_markup(GTK_LABEL(label), "<b>ReSID 8580 settings</b>");
    }
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 2, 1);


    label = create_label("Passband");
    passband = create_passband_widget(model);
    gtk_widget_set_hexpand(passband, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), passband, 1, 2, 1, 1);

    label = create_label("Gain");
    gain = create_gain_widget(model);
    gtk_widget_set_hexpand(gain, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gain, 1, 3, 1, 1);

    label = create_label("Bias");
    bias = create_bias_widget(model);
    gtk_widget_set_hexpand(bias, TRUE);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), bias, 1, 4, 1, 1);
#endif

    button = gtk_button_new_with_label("Reset to defaults");
    gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 2, 1);
    g_signal_connect(button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
