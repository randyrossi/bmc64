/** \file   sidsoundwidget.c
 * \brief   Settings for SID emulation
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/* Note: These only make sense with a SidCart attached for certain machines
 *
 * $VICERES SidEngine                   all
 * $VICERES SidStereo                   all
 * $VICERES SidResidSampling            all
 * $VICERES SidResidPassband            all
 * $VICERES SidResidGain                all
 * $VICERES SidResidFilterBias          all
 * $VICERES SidFilters                  all
 * $VICERES SidStereoAddressStart       all
 * $VICERES SidTripleAddressStart       all
 * $VICERES SidQuadAddressStart         -vsid
 *  (Until PSID files support a fourth SID, this will be -vsid)
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
 */


#include "vice.h"

#include <gtk/gtk.h>

#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "basewidgets.h"
#include "debug_gtk3.h"

#ifdef HAVE_CATWEASELMKIII
#include "catweaselmkiii.h"
#endif

#ifdef HAVE_HARDSID
#include "hardsid.h"
#endif

#ifdef HAVE_PARSID
#include "parsid.h"
#endif

#include "sidsoundwidget.h"


/** \brief  Values for the "SidEngine" resource
 */
static vice_gtk3_radiogroup_entry_t sid_engines[] = {
    { "FastSID", 0 },
    { "ReSID", 1 },
    { "Catweasel MKIII", 2 },
    { "HardSID", 3 },
    { "ParSID port 1", 4 },
    { "ParSID port 2", 5 },
    { "ParSID port 3", 6 },
    { NULL, -1 }
};


#ifdef HAVE_RESID
/** \brief  Values for the "SidResidSampling" resource
 */
static const vice_gtk3_radiogroup_entry_t resid_sampling_modes[] = {
    { "Fast", 0 },
    { "Interpolation", 1 },
    { "Resampling", 2 },
    { "Fast resampling", 3 },
    { NULL, -1 }
};
#endif


/** \brief  Values for the "number of sids" widget
 */
static const vice_gtk3_radiogroup_entry_t num_sids[] = {
    { "One", 0 },
    { "Two", 1 },
    { "Three", 2 },
    { "Four" , 3 }, /* selecting this with FastSID causes a memory corruption
                       warning in sid.c:364 */
    { NULL, -1 }
};


/** \brief  Values for the "number of sids" widget in vsid
 */
static const vice_gtk3_radiogroup_entry_t num_sids_vsid[] = {
    { "One", 0 },
    { "Two", 1 },
    { "Three", 2 },
    { NULL, -1 }
};



/** \brief  I/O addresses for extra SID's for the C64
 *
 * \note    Yes, I know I can generate this table
 */
static const vice_gtk3_combo_entry_int_t sid_address_c64[] = {
    { "$d420", 0xd420 }, { "$d440", 0xd440 }, { "$d460", 0xd460 },
    { "$d480", 0xd480 }, { "$d4a0", 0xd4a0 }, { "$d4c0", 0xd4c0 },
    { "$d4e0", 0xd4e0 },

    { "$d500", 0xd500 }, { "$d520", 0xd520 }, { "$d540", 0xd540 },
    { "$d560", 0xd560 }, { "$d580", 0xd580 }, { "$d5a0", 0xd5a0 },
    { "$d5c0", 0xd5c0 }, { "$d5e0", 0xd5e0 },

    { "$d600", 0xd600 }, { "$d620", 0xd620 }, { "$d640", 0xd640 },
    { "$d660", 0xd660 }, { "$d680", 0xd680 }, { "$d6a0", 0xd6a0 },
    { "$d6c0", 0xd6c0 }, { "$d6e0", 0xd6e0 },

    { "$d700", 0xd700 }, { "$d720", 0xd720 }, { "$d740", 0xd740 },
    { "$d760", 0xd760 }, { "$d780", 0xd780 }, { "$d7a0", 0xd7a0 },
    { "$d7c0", 0xd7c0 }, { "$d7e0", 0xd7e0 },

    { "$de00", 0xde00 }, { "$de20", 0xde20 }, { "$de40", 0xde40 },
    { "$de60", 0xde60 }, { "$de80", 0xde80 }, { "$dea0", 0xdea0 },
    { "$dec0", 0xdec0 }, { "$dee0", 0xdee0 },

    { "$df00", 0xdf00 }, { "$df20", 0xdf20 }, { "$df40", 0xdf40 },
    { "$df60", 0xdf60 }, { "$df80", 0xdf80 }, { "$dfa0", 0xdfa0 },
    { "$dfc0", 0xdfc0 }, { "$dfe0", 0xdfe0 },
    VICE_GTK3_COMBO_ENTRY_INT_LIST_END
};


/** \brief  I/O addresses for extra SID's for the C128
 */
static const vice_gtk3_combo_entry_int_t sid_address_c128[] = {
    { "$d420", 0xd420 }, { "$d440", 0xd440 }, { "$d460", 0xd460 },
    { "$d480", 0xd480 }, { "$d4a0", 0xd4a0 }, { "$d4c0", 0xd4c0 },
    { "$d4e0", 0xd4e0 },

    { "$d700", 0xd700 }, { "$d720", 0xd720 }, { "$d740", 0xd740 },
    { "$d760", 0xd760 }, { "$d780", 0xd780 }, { "$d7a0", 0xd7a0 },
    { "$d7c0", 0xd7c0 }, { "$d7e0", 0xd7e0 },

    { "$de00", 0xde00 }, { "$de20", 0xde20 }, { "$de40", 0xde40 },
    { "$de60", 0xde60 }, { "$de80", 0xde80 }, { "$dea0", 0xdea0 },
    { "$dec0", 0xdec0 }, { "$dee0", 0xdee0 },

    { "$df00", 0xdf00 }, { "$df20", 0xdf20 }, { "$df40", 0xdf40 },
    { "$df60", 0xdf60 }, { "$df80", 0xdf80 }, { "$dfa0", 0xdfa0 },
    { "$dfc0", 0xdfc0 }, { "$dfe0", 0xdfe0 },
    VICE_GTK3_COMBO_ENTRY_INT_LIST_END
};


#ifdef HAVE_RESID
/** \brief  Reference to resid sampling widget
 *
 * Used to enable/disable when the SID engine changes
 */
static GtkWidget *resid_sampling;

/** \brief  Reference to the SidResidPassband widget
 *
 * Used to enable/disable the widget based on the SidFilters setting
 */
static GtkWidget *resid_passband;

/** \brief  Reference to the SidResidGain widget
 *
 * Used to enable/disable the widget based on the SidFilters setting
 */
static GtkWidget *resid_gain;

/** \brief  Reference to the SidResidFilterBias widget
 *
 * Used to enable/disable the widget based on the SidFilters setting
 */
static GtkWidget *resid_bias;
#endif

/** \brief  Reference to the extra SID address widgets
 *
 * Used to enable/disable depending on the number of SIDs active
 */
static GtkWidget *address_widgets[3];



/** \brief  Extra callback registered to the SidEngine radiogroup
 *
 * \param[in]   widget  widget triggering the event
 * \param[in]   engine  engine ID
 */
static void on_sid_engine_changed(GtkWidget *widget, int engine)
{
    debug_gtk3("SID engine changed to %d\n", engine);
#ifdef HAVE_RESID
    gtk_widget_set_sensitive(resid_sampling, engine == 1);
#endif
}


/** \brief  Extra callback registered to the 'number of SIDs' radiogroup
 *
 * \param[in]   widget  widget triggering the event
 * \param[in]   count   number of extra SIDs (0-3)
 */

static void on_sid_count_changed(GtkWidget *widget, int count)
{
    debug_gtk3("extra SIDs count changed to %d\n", count);

    gtk_widget_set_sensitive(address_widgets[0], count > 0);
    gtk_widget_set_sensitive(address_widgets[1], count > 1);
    if (machine_class != VICE_MACHINE_VSID) {
        gtk_widget_set_sensitive(address_widgets[2], count > 2);
    }
}


/** \brief  Extra handler for the "toggled" event of the SID filters
 *
 * Enables/disables ReSID filter settings
 *
 * \param[in]   widget      ReSID filters check button
 * \param[in]   user_data   extra data for event (unused)
 */
static void on_sid_filters_toggled(GtkWidget *widget, gpointer user_data)
{
#ifdef HAVE_RESID
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    gtk_widget_set_sensitive(resid_passband, state);
    gtk_widget_set_sensitive(resid_gain, state);
    gtk_widget_set_sensitive(resid_bias, state);
#endif
}


#ifdef HAVE_RESID
/** \brief  Handler for "clicked" event of reset-to-default for passband
 *
 * Restores the ReSID passband slider back to default
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra data for event (unused)
 */
static void on_resid_passband_default_clicked(GtkWidget *widget,
                                              gpointer user_data)
{
    vice_gtk3_resource_scale_int_reset(resid_passband);
}


/** \brief  Handler for "clicked" event of reset-to-default for gain
 *
 * Restores the ReSID gain slider back to default
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra data for event (unused)
 */
static void on_resid_gain_default_clicked(GtkWidget *widget,
                                          gpointer user_data)
{
    vice_gtk3_resource_scale_int_reset(resid_gain);
}


/** \brief  Handler for "clicked" event of reset-to-default for filter bias
 *
 * Restores the ReSID filter bias slider back to default
 *
 * \param[in]   widget      button
 * \param[in]   user_data   extra data for event (unused)
 */
static void on_resid_bias_default_clicked(GtkWidget *widget,
                                          gpointer user_data)
{
    vice_gtk3_resource_scale_int_reset(resid_bias);

}
#endif


/** \brief  Create widget to control the SID engine
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sid_engine_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *radio_group;
    int p;

    grid = gtk_grid_new();
    g_object_set(grid, "margin-left", 8, NULL);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>SID Engine</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-bottom", 8, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("SidEngine", sid_engines,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    for (p = 1; p < 7; p++) {
        gtk_widget_set_sensitive(gtk_grid_get_child_at(
                    GTK_GRID(radio_group), 0, p), FALSE);
    }

#ifdef HAVE_RESID
    gtk_widget_set_sensitive(
            gtk_grid_get_child_at(GTK_GRID(radio_group), 0, 1),
            TRUE);
#endif

#ifdef HAVE_CATWEASELMKIII
    if (catweaselmkiii_available()) {
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(radio_group),
                    0, 2), TRUE);
    }
#endif

#ifdef HAVE_HARDSID
    if (hardsid_available()) {
        gtk_widget_set_sensitive(gtk_grid_get_child_at(GTK_GRID(radio_group),
                    0, 3), TRUE);

    }
#endif

#ifdef HAVE_PARSID
    if (parsid_available()) {
        for (p = 4; p < 7; p++) {
            gtk_widget_set_sensitive(
                    gtk_grid_get_child_at(GTK_GRID(radio_group), 0, p), TRUE);
        }
    }
#endif
    vice_gtk3_resource_radiogroup_add_callback(radio_group, on_sid_engine_changed);

    gtk_widget_show_all(grid);
    return grid;
}


#ifdef HAVE_RESID
/** \brief  Create widget to control ReSID sampling method
 *
 * \return  GtkGrid
 */
static GtkWidget *create_resid_sampling_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *radio_group;

    grid = gtk_grid_new();
    g_object_set(grid, "margin-left", 8, NULL);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>ReSID sampling method</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-bottom", 8, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("SidResidSampling",
            resid_sampling_modes, GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
#endif


/** \brief  Create widget to set the number of emulated SID's
 *
 * \return  GtkGrid
 */
static GtkWidget *create_num_sids_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *radio_group;

    grid = gtk_grid_new();
    g_object_set(grid, "margin-left", 8, NULL);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Number of SIDs</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-bottom", 8, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    if (machine_class != VICE_MACHINE_VSID) {
        radio_group = vice_gtk3_resource_radiogroup_new("SidStereo",
                num_sids, GTK_ORIENTATION_VERTICAL);
    } else {
        radio_group = vice_gtk3_resource_radiogroup_new("SidStereo",
                num_sids_vsid, GTK_ORIENTATION_VERTICAL);
    }
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    vice_gtk3_resource_radiogroup_add_callback(radio_group,
            on_sid_count_changed);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget for extra SID addresses
 *
 * \param[in]   sid     extra SID number (1-3)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_extra_sid_address_widget(int sid)
{
    GtkWidget *widget;
    const vice_gtk3_combo_entry_int_t *entries;
    char label[256];
    const char *resource[3] = {
        "SidStereoAddressStart",
        "SidTripleAddressStart",
        "SidQuadAddressStart"
    };

    g_snprintf(label, 256, "SID #%d address", sid + 1);
    entries = machine_class == VICE_MACHINE_C128
        ? sid_address_c128 : sid_address_c64;

    widget = vice_gtk3_resource_combo_box_int_new_with_label(
            resource[sid - 1], entries, label);
    gtk_widget_show_all(widget);
    return widget;
}


#ifdef HAVE_RESID
/** \brief  Create scale to control the "SidResidPassband" resource
 *
 * \return  GtkScale
 */
static GtkWidget *create_resid_passband_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidPassband",
            GTK_ORIENTATION_HORIZONTAL, 0, 90, 5);
    vice_gtk3_resource_scale_int_set_marks(scale, 10);
    return scale;
}


/** \brief  Create scale to control the "SidResidGain" resource
 *
 * \return  GtkScale
 */
static GtkWidget *create_resid_gain_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidGain",
            GTK_ORIENTATION_HORIZONTAL, 90, 100, 1);
    vice_gtk3_resource_scale_int_set_marks(scale, 1);
    return scale;
}


/** \brief  Create scale to control the "SidResidFilterBias" resource
 *
 * \return  GtkScale
 */
static GtkWidget *create_resid_bias_widget(void)
{
    GtkWidget *scale;

    scale = vice_gtk3_resource_scale_int_new("SidResidFilterBias",
            GTK_ORIENTATION_HORIZONTAL, -5000, 5000, 500);
    vice_gtk3_resource_scale_int_set_marks(scale, 500);
    return scale;
}
#endif


#ifdef HAVE_RESID
/** \brief  Create "Reset to default" button
 *
 * \param[in]   callback    callback for the button
 *
 * \return  GtkButton
 */
static GtkWidget *create_resource_reset_button(
        void (*callback)(GtkWidget *, gpointer))
{
    GtkWidget *button;

    button = gtk_button_new_with_label("Reset to default");
    gtk_widget_set_valign(button, GTK_ALIGN_END);
    gtk_widget_set_hexpand(button, FALSE);
    g_object_set(button, "margin-left", 16, NULL);

    g_signal_connect(button, "clicked", G_CALLBACK(callback), NULL);

    gtk_widget_show(button);
    return button;
}
#endif


/** \brief  Create widget to control SID settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *sid_sound_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *label;
#ifdef HAVE_RESID
    GtkWidget *button;
#endif
    GtkWidget *engine;
    GtkWidget *num_sids = NULL;
    GtkWidget *filters;
    int current_engine;
    int stereo;
    int row;
    int i;

    layout = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(layout), 8);
    gtk_grid_set_row_spacing(GTK_GRID(layout), 8);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>SID settings</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(layout), label, 0, 0, 3, 1);

    engine = create_sid_engine_widget();
    gtk_grid_attach(GTK_GRID(layout), engine, 0, 1, 1,1);

#ifdef HAVE_RESID
    resid_sampling = create_resid_sampling_widget();
    gtk_grid_attach(GTK_GRID(layout), resid_sampling, 1, 1, 1, 1);
#endif
    resources_get_int("SidEngine", &current_engine);
#ifdef HAVE_RESID
    gtk_widget_set_sensitive(resid_sampling, current_engine == 1);
#endif


    num_sids = create_num_sids_widget();
    gtk_grid_attach(GTK_GRID(layout), num_sids, 2, 1, 1, 1);
    /* Plus4, CBM5x0/CBM6x0, PET, DTV only support a single SID */
    if (machine_class == VICE_MACHINE_PLUS4
            || machine_class == VICE_MACHINE_CBM5x0
            || machine_class == VICE_MACHINE_CBM6x0
            || machine_class == VICE_MACHINE_C64DTV
            || machine_class == VICE_MACHINE_PET
            || machine_class == VICE_MACHINE_VIC20)
    {
        gtk_widget_set_sensitive(num_sids, FALSE);
    }

    /* FIXME; doing two machine_class checks is a little silly */
    if (machine_class != VICE_MACHINE_PLUS4
            && machine_class != VICE_MACHINE_CBM5x0
            && machine_class != VICE_MACHINE_CBM6x0
            && machine_class != VICE_MACHINE_C64DTV
            && machine_class != VICE_MACHINE_PET
            && machine_class != VICE_MACHINE_VIC20)
    {
        int max = 4;
        if (machine_class == VICE_MACHINE_VSID) {
            max = 3;
        }
        for (i = 1; i < max; i++) {
            address_widgets[i - 1] = create_extra_sid_address_widget(i);
            gtk_grid_attach(GTK_GRID(layout), address_widgets[i - 1],
                    i - 1, 2, 1, 1);
        }
        row = 3;
    } else {
        row = 2;
    }

    filters = vice_gtk3_resource_check_button_new("SidFilters",
            "Enable SID filter emulation");
    gtk_grid_attach(GTK_GRID(layout), filters, 0, row, 3, 1);

#ifdef HAVE_RESID
    label = gtk_label_new("ReSID passband");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    resid_passband = create_resid_passband_widget();
    button = create_resource_reset_button(on_resid_passband_default_clicked);
    gtk_grid_attach(GTK_GRID(layout), label, 0, row + 1, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), resid_passband, 1, row + 1, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), button, 2, row + 1, 1, 1);

    label = gtk_label_new("ReSID gain");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    resid_gain = create_resid_gain_widget();
    button = create_resource_reset_button(on_resid_gain_default_clicked);
    gtk_grid_attach(GTK_GRID(layout), label, 0, row + 2, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), resid_gain, 1, row + 2, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), button, 2, row + 2, 1, 1);

    label = gtk_label_new("ReSID filter bias");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    resid_bias = create_resid_bias_widget();
    button = create_resource_reset_button(on_resid_bias_default_clicked);
    gtk_grid_attach(GTK_GRID(layout), label, 0, row + 3, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), resid_bias, 1, row + 3, 1, 1);
    gtk_grid_attach(GTK_GRID(layout), button, 2, row + 3, 1, 1);
#endif

    if (machine_class != VICE_MACHINE_PLUS4
            && machine_class != VICE_MACHINE_CBM5x0
            && machine_class != VICE_MACHINE_CBM6x0)
    {
        /* set sensitivity of address widgets */
        resources_get_int("SidStereo", &stereo);
        on_sid_count_changed(NULL, stereo);
    }

    g_signal_connect(filters, "toggled", G_CALLBACK(on_sid_filters_toggled),
            NULL);
    on_sid_filters_toggled(filters, NULL);

    gtk_widget_show_all(layout);
    return layout;
}
