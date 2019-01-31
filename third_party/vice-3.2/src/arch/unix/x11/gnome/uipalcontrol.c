/*
 * uipalcontrol.c - GTK only, UI controls for CRT emu
 *
 * Written by
 *  Ettore Perazzoli
 *  Oliver Schaertel
 *  pottendo <pottendo@gmx.net>
 *  groepaz <groepaz@gmx.net>
 *
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

#include "lib.h"
#include "machine.h"
#include "ui.h"
#include "uiarch.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "videoarch.h"

#include "uipalcontrol.h"


typedef struct pal_templ_s {
    char *label;        /* Label of Adjustmentbar */
    char *res;          /* Associated resource */
    int scale;          /* Scale to adjust to value range 0..40000 */
    int offset;         /* offset to add to scaled value */
    int perchip;        /* resource per chip (=1) or global (=0) */
    int vsid;           /* 0: omit if vsid 1: show if vsid */
} pal_templ_t;

typedef struct pal_res_s {
    char *label;        /* Label of Adjustmentbar */
    char *res;          /* Associated resource */
    int scale;          /* Scale to adjust to value range 0..40000 */
    int offset;         /* offset to add to scaled value */
    GtkAdjustment *adj; /* pointer to widget */
    GtkWidget *w;       /* widget holding the scrollbar+label */
    video_canvas_t *canvas;
    struct pal_res_s *first;
} pal_res_t;

static pal_templ_t ctrls[] = {
    { N_("Brightness"), "ColorBrightness", 20, 0, 1, 0 },
    { N_("Contrast"), "ColorContrast", 20, 0, 1, 0 },
    { N_("Saturation"), "ColorSaturation", 20, 0, 1, 0 },
    { N_("Tint"), "ColorTint", 20, 0, 1, 0 },
    { N_("Gamma"), "ColorGamma", 10, 0, 1, 0 },
    /* PAL/CRT related settings */
    { N_("Blur"), "PALBlur", 40, 0, 1, 0 },
    { N_("Scanline shade"), "PALScanLineShade", 40, 0, 1, 0 },
    { N_("Odd lines phase"), "PALOddLinePhase", 20, 0, 1, 0 },
    { N_("Odd lines offset"), "PALOddLineOffset", 20, 0, 1, 0 },
    /* volume settings */
    { N_("Volume"), "SoundVolume", 400, 0, 0, 1 },
    { N_("Drives Volume"), "DriveSoundEmulationVolume", 10, 0, 0, 0 },
#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
    { N_("ReSID Passband"), "SidResidPassband", 444, 0, 0, 1 }, /* 0..90 */
    { N_("ReSID Gain"), "SidResidGain", 4000, 90, 0, 1 }, /* 90..100 */
    { N_("ReSID Filter Bias"), "SidResidFilterBias", 4, -5000, 0, 1 }, /* -5000...5000 */
#endif
#ifdef USE_UI_THREADS
    { N_("Display Thread Rate"), "DThreadRate", 800, 0, 0, 0 },
    { N_("Display Thread Ghosting"), "DThreadGhosting", 5000, 0, 0, 0 },
#endif
};

#define NUMSLIDERS (sizeof(ctrls) / sizeof(pal_templ_t))

static int getmaxlen(void)
{
    int i, n, len = 0;
    for (i = 0; i < NUMSLIDERS; i++) {
        n = strlen(ctrls[i].label);
        if (n > len) {
            len = n;
        }
    }
    return len;
}
static void pal_ctrl_update_internal(pal_res_t *ctrldata)
{
    int i, rmode, filter, ispal, drvsnd, hassid = 1;
    unsigned int enabled;
    video_canvas_t *canvas;

    ctrldata = ctrldata->first;
    canvas = ctrldata->canvas;

    if (machine_class == VICE_MACHINE_VSID) {
        enabled = 0xffff;
    } else {
        enabled = 0xffff;
        resources_get_int("DriveSoundEmulation", &drvsnd);
        if ((machine_class == VICE_MACHINE_VIC20) ||
            (machine_class == VICE_MACHINE_PLUS4) ||
            (machine_class == VICE_MACHINE_PET)) {
            resources_get_int("SidCart", &hassid);
        }
        if (!drvsnd) {
            enabled &= ~(1 << 10);
        }
#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
        if (!hassid) {
            enabled &= ~(7 << 11);
        }
#endif
        if (canvas) {
            rmode = canvas->videoconfig->rendermode;
            filter = canvas->videoconfig->filter;
            ispal = (rmode == VIDEO_RENDER_PAL_1X1) || (rmode == VIDEO_RENDER_PAL_2X2);
            if (!ispal) {
                enabled &= ~(1 << 3);
            }
            if (filter != VIDEO_FILTER_CRT) {
                enabled &= ~((1 << 5) | (1 << 6));
            }
            if (!((ispal) && (filter == VIDEO_FILTER_CRT))) {
                enabled &= ~((1 << 7) | (1 << 8));
            }
        }
    }

    for (i = 0; i < NUMSLIDERS; i++) {
        if (machine_class == VICE_MACHINE_VSID) {
            enabled &= (0xfffe + ctrls[i].vsid);
        }
        gtk_widget_set_sensitive(ctrldata[i].w, (enabled & 1) ? TRUE : FALSE);
        if (enabled & 1) {
            gtk_widget_show(ctrldata[i].w);
        } else {
            gtk_widget_hide(ctrldata[i].w);
        }
        enabled >>= 1;
    }
}

static gboolean value_changed_cb(GtkAdjustment *adj, gpointer data)
{
    int v = (int) gtk_adjustment_get_value(adj);
    pal_res_t *p = (pal_res_t *)data;

    v = (int)v / p->scale;
    v += p->offset;

    resources_set_int(p->res, v);

    pal_ctrl_update_internal(p);
    video_canvas_refresh_all(p->canvas);
    return 0;
}

/* reset all sliders to default values */
static gboolean pal_ctrl_reset(GtkWidget *w, gpointer data)
{
    pal_res_t *p = (pal_res_t *)data;
    unsigned int i;
    int tmp;

    for (i = 0; i < NUMSLIDERS; i++) {
        if ((ctrls[i].vsid == 0) && (machine_class == VICE_MACHINE_VSID)) {
            tmp = 0;
        } else {
            resources_get_default_value(p[i].res, (void *)&tmp);
            resources_set_int(p[i].res, tmp);
        }
        tmp = (tmp - p[i].offset) * p[i].scale;
        if (tmp < 0) {
            tmp = 0;
        } else if (tmp > 40100) {
            tmp = 40100;
        }

        if (p[i].adj) {
            gtk_adjustment_set_value(GTK_ADJUSTMENT(p[i].adj), (gfloat)tmp);
        }
    }

    pal_ctrl_update_internal(p);
    video_canvas_refresh_all(p->canvas);
    return 0;
}

/* set all sliders to value of associated resource */
static gboolean pal_ctrl_refresh(pal_res_t *p)
{
    unsigned int i;
    int tmp;

    for (i = 0; i < NUMSLIDERS; i++) {
        if ((ctrls[i].vsid == 0) && (machine_class == VICE_MACHINE_VSID)) {
            tmp = 0;
        } else {
            resources_get_int(p[i].res, (void *)&tmp);
        }
        tmp = (tmp - p[i].offset) * p[i].scale;
        if (tmp < 0) {
            tmp = 0;
        } else if (tmp > 40100) {
            tmp = 40100;
        }

        if (p[i].adj) {
            gtk_adjustment_set_value(GTK_ADJUSTMENT(p[i].adj), (gfloat)tmp);
        }
    }
    return 0;
}

void ui_update_palctrl(void)
{
    int i;
    int num_app_shells = get_num_shells();

    for (i = 0; i < num_app_shells; i++) {
        pal_res_t *p = (pal_res_t *)app_shells[i].pal_ctrl_data;
        pal_ctrl_refresh(p);
        pal_ctrl_update_internal(p);
        ui_trigger_window_resize(p->canvas);
    }
}

GtkWidget *build_pal_ctrl_widget(video_canvas_t *canvas, void *data)
{
    GtkWidget *b, *hb;
    GtkAdjustment *adj;
    GtkWidget *sb;
    GtkWidget *f;
    GtkWidget *l, *c;
    GtkWidget *box;
    GtkWidget *rb;
    unsigned int i;
    int v;
    char *name;
    char *resname;
    char *chip;
    pal_res_t *ctrldata;

    chip = canvas->videoconfig->chip_name;
    ctrldata = lib_malloc(sizeof(pal_res_t) * NUMSLIDERS);

    /* we cant properly add a margin around the frame label, so we add a
       leading space instead */
    name = (machine_class != VICE_MACHINE_VSID) ? _("CRT emulation settings") : _("Mixer");
    f = gtk_frame_new(name);
    gtk_frame_set_label_align(GTK_FRAME(f), 0.025f, 0);
    gtk_frame_set_shadow_type(GTK_FRAME(f), GTK_SHADOW_IN);

    b = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    for (i = 0; i < NUMSLIDERS; ++i) {

        resname = util_concat(ctrls[i].perchip ? chip : "", ctrls[i].res, NULL);
        hb = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        c = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_set_size_request(GTK_WIDGET(c), 10 * getmaxlen(), 10);

        ctrldata[i].label = ctrls[i].label;
        l = gtk_label_new(_(ctrldata[i].label));
        gtk_container_add(GTK_CONTAINER(c), l);
        gtk_widget_show(l);

        gtk_box_pack_start(GTK_BOX(hb), c, FALSE, FALSE, 5);
        gtk_widget_show(c);

        ctrldata[i].scale = ctrls[i].scale;
        ctrldata[i].offset = ctrls[i].offset;
        ctrldata[i].adj = adj = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 40100, ctrls[i].scale, ctrls[i].scale * 10, 100));

        if ((ctrls[i].vsid == 0) && (machine_class == VICE_MACHINE_VSID)) {
            v = 0;
        } else {
            resources_get_int(resname, &v);
        }
        ctrldata[i].res = resname;

        gtk_adjustment_set_value(GTK_ADJUSTMENT(adj), (gfloat)((v - ctrldata[i].offset) * ctrldata[i].scale));
        sb = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj));
#if !GTK_CHECK_VERSION(2, 24, 0)
        /* deprecated since 2.24, removed in 3.0 ("continuous" is default however) */
        gtk_range_set_update_policy(GTK_RANGE(sb), GTK_UPDATE_CONTINUOUS);
#endif
        gtk_box_pack_start(GTK_BOX(hb), sb, TRUE, TRUE, 0);

        g_signal_connect(G_OBJECT(adj), "value_changed", G_CALLBACK (value_changed_cb), &ctrldata[i]);

        gtk_widget_show(sb);
        gtk_box_pack_start(GTK_BOX(b), hb, TRUE, TRUE, 0);
        gtk_widget_show(hb);
        ctrldata[i].w = hb;

        ctrldata[i].first = ctrldata;
        ctrldata[i].canvas = canvas;
    }

    /* "Reset" button */
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    rb = gtk_button_new_with_label(_("Reset"));
    gtk_box_pack_start(GTK_BOX(box), rb, FALSE, FALSE, 5);
    g_signal_connect(G_OBJECT(rb), "clicked", G_CALLBACK(pal_ctrl_reset), &ctrldata[0]);
    gtk_widget_set_can_focus(rb, 0);
    gtk_widget_show(rb);

    gtk_widget_show(box);
    gtk_box_pack_start(GTK_BOX(b), box, FALSE, FALSE, 5);

    gtk_widget_show(b);
    gtk_container_add(GTK_CONTAINER(f), b);
    gtk_widget_show(f);

    *(pal_res_t **)data = ctrldata;
    pal_ctrl_update_internal(ctrldata);
    return f;
}

void shutdown_pal_ctrl_widget(GtkWidget *f, void *ctrldata)
{
    unsigned int i;
    pal_res_t *ctrl_data = (pal_res_t*)ctrldata;
    for (i = 0; i < NUMSLIDERS; ++i) {
        lib_free(ctrl_data[i].res);
    }
    lib_free(ctrl_data);
}

int palctrl_get_height(video_canvas_t *canvas)
{
    app_shell_type *appshell;
    GtkWidget *palctrl;
    if (canvas) {
        appshell = &app_shells[canvas->app_shell];
        if (appshell) {
            palctrl = appshell->pal_ctrl;
            if (palctrl) {
                /* return height if visible */
                if (gtk_widget_get_visible(palctrl)) {
                    return gtk_widget_get_allocated_height(palctrl);
                }
            }
        }
    }
    return 0;
}
