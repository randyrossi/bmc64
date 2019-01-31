/*
 * uipalcontrol.c - Xaw(3d) only, UI controls for CRT emu
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Scrollbar.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/Toggle.h>
#else
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Toggle.h>
#endif

#include "lib.h"
#include "util.h"
#include "resources.h"
#include "videoarch.h"
#include "uiarch.h"

#include "uipalcontrol.h"


void destroy_pal_ctrl_widget(Widget w, XtPointer client_data, XtPointer call_data);

typedef struct pal_res_s {
    char *label;        /* Label of Adjustmentbar */
    char *res;          /* Associated resource */
    int base;           /* Value ranges [ base ... base+range > */
    int range;
    Widget labelwidget; /* widget holding the label */
    Widget scrollbar;   /* pointer to scrollbar */
} pal_res_t;

static pal_res_t ctrls[] = {
    { N_("Blur"),             "+PALBlur",              0,  1000, },
    { N_("Scanline shade"),   "+PALScanLineShade",     0,  1000, },
    { N_("Saturation"),       "+ColorSaturation",      0,  2000, },
    { N_("Contrast"),         "+ColorContrast",        0,  2000, },
    { N_("Brightness"),       "+ColorBrightness",      0,  2000, },
    { N_("Gamma"),            "+ColorGamma",           0,  4000, },
    { N_("Tint"),             "+ColorTint",            0,  2000, },
    { N_("Odd lines phase"),  "+PALOddLinePhase",      0,  2000, },
    { N_("Odd lines offset"), "+PALOddLineOffset",     0,  2000, },
    /* volume settings */
    { N_("Volume"),           "SoundVolume",           0,   100, },
    { N_("Drives Volume"), "DriveSoundEmulationVolume",0,  4000, },
#if defined(HAVE_RESID) || defined(HAVE_RESID_DTV)
    /* SID settings */
    { N_("ReSID Passband"),   "SidResidPassband",      0,    90, },
    { N_("ReSID Gain"),       "SidResidGain",         90,    10, },
    { N_("ReSID Filter Bias"),"SidResidFilterBias",-5000, 10000, },
#endif
};

typedef struct {
    Widget shell;
    video_canvas_t *cached_canvas;
    pal_res_t ctrls[0];
} cleanup_data_t;

#define THUMB_SIZE      ((float) 0.05)

static float ScrollbarSetThumb(Widget scrollbar, float position)
{
    if (position > 1.0 - THUMB_SIZE) {
        position = 1.0 - THUMB_SIZE;
    } else if (position < 0.0) {
        position = 0.0;
    }
    XawScrollbarSetThumb(scrollbar, position, THUMB_SIZE);

    return position;
}

static void JumpProc(Widget scrollbar, XtPointer client_data, XtPointer percent_ptr)
{
    float fraction;
    int value;
    pal_res_t *p = (pal_res_t *)client_data;

    fraction = *(float *)percent_ptr;
    if (fraction > 1.0 - THUMB_SIZE) {
        fraction = 1.0 - THUMB_SIZE;
    }
    value = p->base + (fraction * p->range) + 0.5;

    resources_set_int(p->res, value);
}

static void ScrollProc(Widget scrollbar, XtPointer client_data, XtPointer positionptr /* int */)
{
    float delta;
    float oldposition;
    long position = (long)positionptr;
    pal_res_t *p = (pal_res_t *)client_data;

    XtVaGetValues(scrollbar,
                  XtNtopOfThumb, &oldposition,
                  NULL);
    if (position < 0) { /* right button, strangely enough */
        delta = +THUMB_SIZE;
    } else {
        delta = -THUMB_SIZE;
    }

    oldposition += delta;
    oldposition = ScrollbarSetThumb(scrollbar, oldposition);
    resources_set_int(p->res, (int)(p->base + (oldposition * p->range) + 0.5));
}

static void GetWH(Widget widget, int *w, int *h)
{
    XtVaGetValues(widget,
                  XtNwidth, w,
                  XtNheight, h,
                  NULL);
}

static void ResetProc(Widget w, XtPointer client_data, XtPointer dummy)
{
    cleanup_data_t *p = (cleanup_data_t *)client_data;
    unsigned int i;
    int tmp;
    float fraction;

    for (i = 0; i < util_arraysize(ctrls); i++) {
        pal_res_t *ctrl = &p->ctrls[i];

        resources_get_default_value(ctrl->res, (void *)&tmp);
        resources_set_int(ctrl->res, tmp);
        fraction = (float)(tmp - ctrl->base) / ctrl->range;
        if (ctrl->scrollbar) {
            ScrollbarSetThumb(ctrl->scrollbar, fraction);
        }
    }

    /* unneeded: video_canvas_refresh_all(p->cached_canvas); */
}

static Widget build_pal_ctrl_widget_sliders(video_canvas_t *canvas, Widget parent, cleanup_data_t **cleanup_p)
{
    unsigned int i;
    int v;
    char *chip;
    pal_res_t *ctrldata;
    Widget widestlabel = 0;
    int widestsize = -1, labelheight;
    Widget form, toplabel, reset;
    Widget fromVert;
    cleanup_data_t *cleanupdata;

    cleanupdata = lib_malloc(sizeof(cleanup_data_t) + sizeof(ctrls));
    cleanupdata->cached_canvas = canvas;
    ctrldata = (pal_res_t *)(cleanupdata + 1);
    memcpy(ctrldata, ctrls, sizeof(ctrls));
    *cleanup_p = cleanupdata;

    form = XtVaCreateManagedWidget("palControlsForm",
                                    formWidgetClass, parent,
                                    NULL);

    toplabel = XtVaCreateManagedWidget("topLabel",
                                       labelWidgetClass, form,
                                       XtNjustify, XtJustifyLeft,
                                       XtNlabel, _("CRT emulation settings"),
                                       XtNborderWidth, 0,
                                       XtNtop, XawChainTop,
                                       XtNbottom, XawChainTop,
                                       NULL);

    reset = XtVaCreateManagedWidget("reset",
                                    commandWidgetClass, form,
                                    XtNlabel, _("Reset"),
                                    XtNfromHoriz, toplabel,
                                    XtNtop, XawChainTop,
                                    XtNbottom, XawChainTop,
                                    NULL);
    XtAddCallback(reset, XtNcallback, ResetProc, (XtPointer)cleanupdata);

    chip = canvas->videoconfig->chip_name;
    fromVert = toplabel;

    /* First place the labels, from top to bottom */
    for (i = 0; i < util_arraysize(ctrls); i++) {
        char *resname;
        char *labelname;
        Widget label;
        int width;

        /* A + before the resource name indicates the video chip name */
        if (ctrls[i].res[0] == '+') {
            resname = util_concat(chip, ctrls[i].res + 1, NULL);
        } else {
            resname = lib_stralloc(ctrls[i].res);
        }

        ctrldata[i].res = resname;

        labelname = ctrldata[i].label;
        label = XtVaCreateManagedWidget(labelname,
                                        labelWidgetClass, form,
                                        XtNjustify, XtJustifyLeft,
                                        XtNlabel, _(labelname),
                                        XtNborderWidth, 0,
                                        XtNfromVert, fromVert,
                                        XtNleft, XawChainLeft,
                                        XtNright, XawChainLeft,
                                        NULL);

        fromVert = label;
        ctrldata[i].labelwidget = label;

        /* Determine which of the labels is the widest */
        width = 0;
        GetWH(label, &width, &labelheight);
        if (width > widestsize) {
            widestlabel = label;
            widestsize = width;
        }
    }

    /* Then place the scollbars, to the right of the widest label */
    fromVert = toplabel;
    for (i = 0; i < util_arraysize(ctrls); i++) {
        char *resname = ctrldata[i].res;
        char *scrollbarname;
        Widget scroll;
        int borderwidth = 1;

        scrollbarname = resname;
        scroll = XtVaCreateManagedWidget(scrollbarname,
                                        scrollbarWidgetClass, form,
                                        XtNorientation, XtorientHorizontal,
                                        XtNfromHoriz, widestlabel,
                                        XtNfromVert, fromVert,
                                        XtNheight, labelheight - 2 * borderwidth,
                                        XtNwidth, 200,
                                        XtNborderWidth, borderwidth,
                                        XtNleft, XawChainLeft,
                                        XtNright, XawChainRight,
                                        NULL);
        XtAddCallback(scroll, XtNscrollProc, ScrollProc, (XtPointer)&ctrldata[i]);
        XtAddCallback(scroll, XtNjumpProc, JumpProc, (XtPointer)&ctrldata[i]);

        ctrldata[i].scrollbar = scroll;
        fromVert = scroll;

        ctrldata[i].range /= (1.0 - THUMB_SIZE);
        resources_get_int(resname, &v);
        ScrollbarSetThumb(scroll,
                (float)((float)(v - ctrldata[i].base) / ctrldata[i].range));
    }

    return form;
}

static void ToggleProc(Widget w, XtPointer client_data, XtPointer togglevalue)
{
    Widget shell = (Widget)client_data;

    if (togglevalue) {
        Display *display = XtDisplay(shell);

        XtPopup(shell, XtGrabNone);
        XSetWMProtocols(display, XtWindow(shell), &wm_delete_window, 1);
    } else {
        XtPopdown(shell);
    }
}

/*
 * Receive message from window manager about the close box being clicked.
 * All example programs use a translation table and an Action for this,
 * ( "<Message>WM_PROTOCOLS: close()"; { "close", CloseAction } )
 * but you can do it directly!
 */
static
void nonmaskable_callback_shell(Widget w_shell, XtPointer client_data, XtPointer call_data)
{
    XEvent *e = (XEvent *)call_data;
    Widget toggle = (Widget)client_data;

    if (e->type == ClientMessage &&
            e->xclient.message_type == wm_protocols &&
            e->xclient.data.l[0] == (long) wm_delete_window) {
        XtVaSetValues(toggle,
                      XtNstate, False,
                      NULL);
        /*
         * The XtVaSetValues() call does not do the Translations/Actions
         * that go with clicking on the toggle widget. We have to do
         * that ourselves here.
         * We could call XtCallActionProc(toggle, "notify", ...) or
         * XtCallCallbacks(toggle, XtNcallback, False) but this has the
         * same eventual effect:
         */
        ToggleProc(toggle, w_shell, (XtPointer)False);
    }
}

/*
 * Create a toggle button, to be placed on the UI window.
 * If clicked, it toggles a transient shell, which contains
 * the controls to change the CRT settings.
 */
Widget build_pal_ctrl_widget(video_canvas_t *canvas, Widget parent, ArgList args, Cardinal num_args)
{
    Widget sliders;
    Widget toggle;
    Widget shell;
    cleanup_data_t *cleanupdata;

    toggle = XtCreateManagedWidget("toggle",
                                     toggleWidgetClass, parent,
                                     args, num_args);

    /* popup window */
    shell = ui_create_transient_shell(parent, "PAL CRT Controls");
    /* put in the sliders */
    sliders = build_pal_ctrl_widget_sliders(canvas, shell, &cleanupdata);
    (void)sliders;
    cleanupdata->shell = shell;

    XtAddCallback(toggle, XtNcallback, ToggleProc, (XtPointer)shell);
    XtAddCallback(toggle, XtNdestroyCallback, destroy_pal_ctrl_widget, cleanupdata);

    XtDisplay(toggle);

    XtAddEventHandler(shell, 0, True, (XtEventHandler)nonmaskable_callback_shell, (XtPointer)toggle);

    return toggle;
}

void destroy_pal_ctrl_widget(Widget w, XtPointer client_data, XtPointer call_data)
{
    cleanup_data_t *clean = (cleanup_data_t *)client_data;
    int i;

    if (clean->shell) {
        XtDestroyWidget(clean->shell);
    }
    for (i = 0; i < util_arraysize(ctrls); i++) {
        lib_free(clean->ctrls[i].res);
    }
    lib_free(clean);
}
