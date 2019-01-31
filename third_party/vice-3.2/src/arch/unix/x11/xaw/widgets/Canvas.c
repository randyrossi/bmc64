/*

    Canvas.c - a widget that allows programmer-specified refresh procedures.
    Copyright (C) 1990,93,94 Robert H. Forsman Jr.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#include "vice.h"

#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>

#include <stdio.h>

#include "CanvasP.h"

#define offset(field) XtOffset(CanvasWidget, canvas.field)

#ifdef MINIX_SUPPORT
static XtResource resources[] = {
    { XtNexposeProc, XtCExposeProc, XtRFunction, sizeof(XfwfCanvasExposeProc), 116, XtRFunction, NULL },
    { XtNexposeProcData, XtCExposeProcData, XtRPointer, sizeof(XtPointer), 120, XtRFunction, NULL },
    { XtNresizeProc, XtCResizeProc, XtRFunction, sizeof(XfwfCanvasResizeProc), 124, XtRFunction, NULL },
    { XtNresizeProcData, XtCResizeProcData, XtRPointer, sizeof(XtPointer), 128, XtRFunction, NULL },
    { XtNvisual, XtCVisual, XtRVisual, sizeof(Visual*), 132, XtRImmediate, CopyFromParent }
};
#else
static XtResource resources[] = {
    { XtNexposeProc, XtCExposeProc, XtRFunction, sizeof(XfwfCanvasExposeProc), offset(redraw), XtRFunction, NULL },
    { XtNexposeProcData, XtCExposeProcData, XtRPointer, sizeof(XtPointer), offset(redraw_data), XtRFunction, NULL },
    { XtNresizeProc, XtCResizeProc, XtRFunction, sizeof(XfwfCanvasResizeProc), offset(resize), XtRFunction, NULL },
    { XtNresizeProcData, XtCResizeProcData, XtRPointer, sizeof(XtPointer), offset(resize_data), XtRFunction, NULL },
    { XtNvisual, XtCVisual, XtRVisual, sizeof(Visual*), offset(visual), XtRImmediate, CopyFromParent }
};
#endif

static void CanvasRealize(Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attributes);
static void Redisplay(Widget w, XExposeEvent *event, Region region);
static void Resize(CanvasWidget cw);
static Boolean SetValues(CanvasWidget current, CanvasWidget request, CanvasWidget new, ArgList args, Cardinal *nargs);

CanvasClassRec canvasClassRec = {
    {
    /* core_class fields     */
    /* superclass            */ (WidgetClass) &widgetClassRec,
    /* class_name            */ "Canvas",
    /* widget_size           */ sizeof(CanvasRec),
    /* class_initialize      */ NULL,
    /* class_part_initialize */ NULL,
    /* class_inited          */ False,
    /* initialize            */ NULL,
    /* initialize_hook       */ NULL,
    /* realize               */ CanvasRealize,
    /* actions               */ NULL,
    /* num_actions           */ 0,
    /* resources             */ resources,
    /* num_resources         */ XtNumber(resources),
    /* xrm_class             */ NULLQUARK,
    /* compress_motion       */ True,
    /* compress_exposure     */ XtExposeCompressMultiple,
    /* compress_enterleave   */ True,
    /* visible_interest      */ True,
    /* destroy               */ NULL,
    /* resize                */ (XtWidgetProc)Resize,
    /* expose                */ (XtExposeProc)Redisplay,
    /* set_values            */ (XtSetValuesFunc)SetValues,
    /* set_values_hook       */ NULL,
    /* set_values_almost     */ XtInheritSetValuesAlmost,
    /* get_values_hook       */ NULL,
    /* accept_focus          */ NULL,
    /* version               */ XtVersion,
    /* callback_private      */ NULL,
    /* tm_table              */ NULL,
    /* query_geometry        */ NULL,
    /* display_accelerator   */ XtInheritDisplayAccelerator,
    /* extension             */ NULL
    },
    {
      0 /* some stupid compilers barf on empty structures */
    },
};

WidgetClass xfwfcanvasWidgetClass = (WidgetClass)&canvasClassRec;

static void CanvasRealize(Widget widget, XtValueMask *value_mask, XSetWindowAttributes *attributes)
{
    CanvasWidget cw = (CanvasWidget)widget;
    XtCreateWindow(widget, (unsigned int)InputOutput, (Visual *)cw->canvas.visual, *value_mask, attributes);
} /* CoreRealize */

static void Redisplay(Widget w, XExposeEvent *event, Region region)
{
    CanvasWidget cw = (CanvasWidget)w;
    if (!XtIsRealized(w)) {
        return;
    }

    if (cw->canvas.redraw) {
        (cw->canvas.redraw)((Widget)cw, event, region, cw->canvas.redraw_data);
    }
}

static Boolean SetValues(CanvasWidget current, CanvasWidget request, CanvasWidget new, ArgList args, Cardinal *nargs)
{
    int i;

    for (i = 0; i < *nargs; i++) {
        if (strcmp(XtNexposeProc,args[i].name) == 0 || strcmp(XtNexposeProcData,args[i].name) == 0) {
            return True;
        }
    }
    return False;
}

static void Resize(CanvasWidget cw)
{
    if (cw->canvas.resize) {
        (cw->canvas.resize)((Widget)cw, cw->canvas.resize_data);
    }
}
