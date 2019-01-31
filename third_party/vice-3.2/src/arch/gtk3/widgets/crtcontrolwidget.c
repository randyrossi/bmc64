/** \file   crtcontrolwidget.c
 * \brief   GTK3 CRT settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Provides a list of GtkScale widgets to alter CRT settings.
 *
 * Supported settings per chip/video mode:
 *
 * |Setting             |VICIIP|VICIIN|VDC|VICP|VICN|TEDP|TEDN|CRTC|
 * |--------------------|------|------|---|----|----|----|----|----|
 * |Brightness          | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Contrast            | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Saturation          | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Tint                | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Gamma               | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Blur                | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Scanline shade      | yes  | yes  |yes|yes |yes |yes |yes |yes |
 * |Odd lines phase     | yes  |  no  | no|yes | no |yes | no | no |
 * |Odd lines offset    | yes  |  no  | no|yes | no |yes | no | no |
 *
 * TODO:    Fix display of sliders when switching between PAL and NTSC
 */

/*
 * $VICERES CrtcColorBrightness     xpet xcbm2
 * $VICERES CrtcColorContrast       xpet xcbm2
 * $VICERES CrtcColorGamma          xpet xcbm2
 * $VICERES CrtcColorSaturation     xpet xcbm2
 * $VICERES CrtcColorTint           xpet xcbm2
 * $VICERES CrtcPALBlur             xpet xcbm2
 * $VICERES CrtcPALScanLineShade    xpet xcbm2
 *
 * $VICERES TEDColorBrightness      xplus4
 * $VICERES TEDColorContrast        xplus4
 * $VICERES TEDColorGamma           xplus4
 * $VICERES TEDColorSaturation      xplus4
 * $VICERES TEDColorTint            xplus4
 * $VICERES TEDPALBlur              xplus4
 * $VICERES TEDPALOddLineOffset     xplus4
 * $VICERES TEDPALOddLinePhase      xplus4
 * $VICERES TEDPALScanLineShade     xplus4
 *
 * $VICERES VDCColorBrightness      x128
 * $VICERES VDCColorContrast        x128
 * $VICERES VDCColorGamma           x128
 * $VICERES VDCColorSaturation      x128
 * $VICERES VDCColorTint            x128
 * $VICERES VDCPALBlur              x128
 * $VICERES VDCPALScanLineShade     x128
 *
 * $VICERES VICColorBrightness      xvic
 * $VICERES VICColorContrast        xvic
 * $VICERES VICColorGamma           xvic
 * $VICERES VICColorSaturation      xvic
 * $VICERES VICColorTint            xvic
 * $VICERES VICPALBlur              xvic
 * $VICERES VICPALOddLineOffset     xvic
 * $VICERES VICPALOddLinePhase      xvic
 * $VICERES VICPALScanLineShade     xvic
 *
 * $VICERES VICIIColorBrightness    x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIColorContrast      x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIColorGamma         x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIColorSaturation    x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIColorTint          x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIPALBlur            x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIPALOddLineOffset   x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIPALOddLinePhase    x64 x64sc x64dtv xscpu64 x128 xcbm5x0
 * $VICERES VICIIPALScanLineShade   x64 x64sc x64dtv xscpu64 x128 xcbm5x0
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

#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "resources.h"
#include "lib.h"
#include "machine.h"


#include "crtcontrolwidget.h"

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


/** \brief  Video chip identifiers
 *
 * Allows for easier/faster switching than using strcmp()
 */
enum {
    CHIP_CRTC,
    CHIP_TED,
    CHIP_VDC,
    CHIP_VIC,
    CHIP_VICII,

    CHIP_ID_COUNT
};


/** \brief  Struct mapping chip names to chip IDs
 */
typedef struct chip_id_s {
    const char *name;   /**< chip name (ie VDC, VICII, etc) */
    int id;             /**< chip ID */
} chip_id_t;


static const chip_id_t chips[CHIP_ID_COUNT] = {
    { "CRTC",   CHIP_CRTC },
    { "TED",    CHIP_TED },
    { "VDC",    CHIP_VDC },
    { "VIC",    CHIP_VIC },
    { "VICII",  CHIP_VICII }
};


/** \brief  Find chip ID by \a name
 *
 * \param[in]   name    chip name
 *
 * \return  chip ID or -1 on error
 */
static int get_chip_id(const char *name)
{
    int i;

    for (i = 0; i < CHIP_ID_COUNT; i++) {
        if (strcmp(name, chips[i].name) == 0) {
            return chips[i].id;
        }
    }
    return -1;
}


/** \brief  Object holding internal state of a CRT control widget
 *
 * Since we can have two video chips (C128's VICII+VDC), we cannot use static
 * references to widgets and need to allocate memory for the references and
 * clean that memory up once the widget is destroyed.
 */
typedef struct crt_control_data_s {
    char *chip;
    GtkWidget *color_brightness;
    GtkWidget *color_contrast;
    GtkWidget *color_gamma;
    GtkWidget *color_saturation;
    GtkWidget *color_tint;
    GtkWidget *pal_blur;
    GtkWidget *pal_scanline_shade;
    GtkWidget *pal_oddline_offset;
    GtkWidget *pal_oddline_phase;
} crt_control_data_t;


/** \brief  Reset all sliders to the resource value when creating the widget
 *
 * \param[in]   widget      reset button
 * \param[in]   user_data   extra event data (unused)
 */
static void on_reset_clicked(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *parent;
    crt_control_data_t *data;

    /* parent widget (grid), contains the InternalState object */
    parent = gtk_widget_get_parent(widget);
    data = g_object_get_data(G_OBJECT(parent), "InternalState");

    if (data->color_brightness != NULL) {
        vice_gtk3_resource_scale_int_reset(data->color_brightness);
    }
    if (data->color_contrast != NULL) {
        vice_gtk3_resource_scale_int_reset(data->color_contrast);
    }
    if (data->color_gamma != NULL) {
        vice_gtk3_resource_scale_int_reset(data->color_gamma);
    }
    if (data->color_saturation != NULL) {
        vice_gtk3_resource_scale_int_reset(data->color_saturation);
    }
    if (data->color_tint != NULL) {
        vice_gtk3_resource_scale_int_reset(data->color_tint);
    }
    if (data->pal_blur != NULL) {
        vice_gtk3_resource_scale_int_reset(data->pal_blur);
    }
    if (data->pal_scanline_shade != NULL) {
        vice_gtk3_resource_scale_int_reset(data->pal_scanline_shade);
    }
    if (data->pal_oddline_offset != NULL) {
        vice_gtk3_resource_scale_int_reset(data->pal_oddline_offset);
    }
    if (data->pal_oddline_phase != NULL) {
        vice_gtk3_resource_scale_int_reset(data->pal_oddline_phase);
    }
}


/** \brief  Clean up memory used by the internal state of \a widget
 *
 * \param[in]   widget      widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_widget_destroy(GtkWidget *widget, gpointer user_data)
{
    crt_control_data_t *data;

    data = (crt_control_data_t *)(g_object_get_data(
                G_OBJECT(widget), "InternalState"));
    lib_free(data->chip);
    lib_free(data);
}




/** \brief  Create right-aligned label with a smaller font
 *
 * \param[in]   text    label text
 *
 * \return  GtkLabel
 */
static GtkWidget *create_label(const char *text)
{
    GtkWidget *label;
    GtkCssProvider *provider;
    GtkStyleContext *context;
    GError *err = NULL;

    label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_END);

    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, LABEL_CSS, -1, &err);
    if (err != NULL) {
        fprintf(stderr, "CSS error: %s\n", err->message);
        g_error_free(err);
    }

    context = gtk_widget_get_style_context(label);
    if (context != NULL) {
        gtk_style_context_add_provider(context,
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    return label;
}


/** \brief  Create a customized GtkScale for \a resource
 *
 * \param[in]   resource    resource name without the video \a chip name prefix
 * \param[in]   chip        video chip name
 * \param[in]   low         lower bound
 * \param[in]   high        upper bound
 * \param[in]   step        step used to increase/decrease slider value
 *
 * \return  GtkScale
 */
static GtkWidget *create_slider(const char *resource, const char *chip,
        int low, int high, int step)
{
    GtkWidget *scale;
    GtkCssProvider *css_provider;
    GtkStyleContext *style_context;
    GError *err = NULL;

    scale = vice_gtk3_resource_scale_int_new_sprintf("%s%s",
            GTK_ORIENTATION_HORIZONTAL, low, high, step,
            chip, resource);
    gtk_widget_set_hexpand(scale, TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(scale), GTK_POS_RIGHT);

    /* set up custom CSS to make the scale take up less space */
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, SLIDER_CSS, -1, &err);
    if (err != NULL) {
        fprintf(stderr, "CSS error: %s\n", err->message);
        g_error_free(err);
    }

    style_context = gtk_widget_get_style_context(scale);
    if (style_context != NULL) {
        gtk_style_context_add_provider(style_context,
                GTK_STYLE_PROVIDER(css_provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER);
    }

    /* don't draw the value next to the scale */
    gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);

    return scale;
}



/** \brief  Add GtkScale sliders to \a grid
 *
 * \param[in,out]   grid    grid to add widgets to
 * \param[in,out]   data    internal data of the main widget
 *
 * \return  row number of last widget added
 */
static void add_sliders(GtkGrid *grid, crt_control_data_t *data)
{
    GtkWidget *label;
    const char *chip;
    int video_standard;
    int row = 1;
    int chip_id;
    bool enabled;

    chip = data->chip;
    chip_id = get_chip_id(chip);
    if (chip_id < 0) {
        debug_gtk3("failed to get chip ID for '%s'\n", chip);
        return;
    }

    /* get PAL/NTSC mode */
    if (resources_get_int("MachineVideoStandard", &video_standard) < 0) {
        debug_gtk3("failed to get MachineVideoStandard resource value\n");
        return;
    }

    /* Standard controls: brightness, gamma etc */

    label = create_label("Brightness:");
    data->color_brightness = create_slider("ColorBrightness", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->color_brightness, 1, row, 1, 1);
    row++;

    label = create_label("Contrast:");
    data->color_contrast = create_slider("ColorContrast", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->color_contrast, 1, row, 1, 1);
    row++;

    label = create_label("Saturation:");
    data->color_saturation = create_slider("ColorSaturation", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->color_saturation, 1, row, 1, 1);
    row++;

    label = create_label("Tint:");
    data->color_tint = create_slider("ColorTint", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->color_tint, 1, row, 1, 1);
    row++;

    label = create_label("Gamma:");
    data->color_gamma = create_slider("ColorGamma", chip,
            0, 4000, 200);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->color_gamma, 1, row, 1, 1);
    row++;

    label = create_label("Blur:");
    data->pal_blur = create_slider("PALBlur", chip,
            0, 1000, 50);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->pal_blur, 1, row, 1, 1);
    row++;

    label = create_label("Scanline shade:");
    data->pal_scanline_shade = create_slider("PALScanLineShade", chip,
            0, 1000, 50);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->pal_scanline_shade, 1, row, 1, 1);
    row++;

    label = create_label("Odd lines phase:");
    data->pal_oddline_phase = create_slider("PALOddLinePhase", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->pal_oddline_phase, 1, row, 1, 1);
    row++;

    label = create_label("Odd lines offset:");
    data->pal_oddline_offset = create_slider("PALOddLineOffset", chip,
            0, 2000, 100);
    gtk_grid_attach(grid, label, 0, row, 1, 1);
    gtk_grid_attach(grid, data->pal_oddline_offset, 1, row, 1, 1);
    row++;

    enabled = ((video_standard == 0 /* PAL */
                || video_standard == 1 /* Old PAL */
                || video_standard == 4 /* PAL-N/Drean */
                ) && chip_id != CHIP_CRTC && chip_id != CHIP_VDC);

    gtk_widget_set_sensitive(data->pal_oddline_phase, enabled);
    gtk_widget_set_sensitive(data->pal_oddline_offset, enabled);

}

#if 0
/** \brief  Callback for the timeout used to check PAL/NTSC
 *
 * \param who cares
 */
static gint timeout_callback(gpointer data)
{
    GtkWidget *widget = (GtkWidget *)data;
    crt_control_data_t *state;
    int video_standard;
    int chip_id;
    bool enabled;

    state = g_object_get_data(G_OBJECT(widget), "InternalState");
    if (state == NULL) {
        debug_gtk3("oeps\n");
        return 0;
    }

    chip_id = get_chip_id(state->chip);

    resources_get_int("MachineVideoStandard", &video_standard);
    enabled = ((video_standard == 0 /* PAL */
                || video_standard == 1 /* Old PAL */
                || video_standard == 4 /* PAL-N/Drean */
                ) && chip_id != CHIP_CRTC && chip_id != CHIP_VDC);

    gtk_widget_set_sensitive(state->pal_oddline_phase, enabled);
    gtk_widget_set_sensitive(state->pal_oddline_offset, enabled);

    return TRUE;    /* keep going */
}
#endif


/** \brief  Create CRT control widget for \a chip
 *
 * \param[in]   parent  parent widget
 * \param[in]   chip    video chip name
 *
 * \return  GtkGrid
 */
GtkWidget *crt_control_widget_create(GtkWidget *parent, const char *chip)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;
    gchar buffer[256];

    crt_control_data_t *data;

    data = lib_malloc(sizeof *data);
    data->chip = lib_stralloc(chip);
    data->color_brightness = NULL;
    data->color_contrast = NULL;
    data->color_gamma = NULL;
    data->color_saturation = NULL;
    data->color_tint = NULL;
    data->pal_blur = NULL;
    data->pal_oddline_offset = NULL;
    data->pal_oddline_phase = NULL;
    data->pal_scanline_shade = NULL;

    grid = vice_gtk3_grid_new_spaced(16, 0);
    /* g_object_set(grid, "font-size", 9, NULL); */
    g_object_set(grid, "margin-left", 8, "margin-right", 8, NULL);

    g_snprintf(buffer, 256, "<small><b>CRT settings (%s)</b></small>", chip);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), buffer);
    gtk_widget_set_halign(label, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    add_sliders(GTK_GRID(grid), data);

    button = gtk_button_new_with_label("Reset");
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
    g_signal_connect(button, "clicked", G_CALLBACK(on_reset_clicked), NULL);

    g_object_set_data(G_OBJECT(grid), "InternalState", (gpointer)data);
    g_signal_connect(grid, "destroy", G_CALLBACK(on_widget_destroy), NULL);

    /*
     * When in doubt, do weird shit:
     *
     * Set up a timeout that checks the MachineVideoStandard resource and
     * enables/disables the PAL related sliders.
     *
     * This happens every second, and every time the resource is read and
     * the widgets' sensitivity set, even when the resource hasn't changed.
     */
#if 0
    g_timeout_add(1000, timeout_callback, (gpointer)grid);
#endif
    gtk_widget_show_all(grid);
    return grid;
}
