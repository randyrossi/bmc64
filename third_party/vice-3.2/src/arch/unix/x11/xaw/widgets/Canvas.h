/*

    Canvas.h - public header file for the Canvas Widget
    -  a widget that allows programmer-specified refresh procedures.
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

#ifndef VICE_Canvas_h
#define VICE_Canvas_h

#include <X11/Core.h>

#ifndef XtNexposeProc
#define XtNexposeProc "exposeProc"
#define XtCExposeProc "ExposeProc"
#endif

#ifndef XtNexposeProcData
#define XtNexposeProcData "exposeProcData"
#define XtCExposeProcData "ExposeProcData"
#endif

#ifndef XtNresizeProc
#define XtNresizeProc "resizeProc"
#define XtCResizeProc "ResizeProc"
#endif

#ifndef XtNresizeProcData
#define XtNresizeProcData "resizeProcData"
#define XtCResizeProcData "ResizeProcData"
#endif

#ifndef XtNvisual
#define XtNvisual "visual"
#define XtCVisual "Visual"
#endif

extern WidgetClass xfwfcanvasWidgetClass;

typedef struct _CanvasClassRec *CanvasWidgetClass;
typedef struct _CanvasRec *CanvasWidget;

typedef void(*XfwfCanvasExposeProc) (
    Widget w,
    XExposeEvent *event,
    Region region,
    XtPointer client_data
);

typedef void(*XfwfCanvasResizeProc)(
    Widget w,
    XtPointer client_data
);

#endif /* _Canvas_h */
