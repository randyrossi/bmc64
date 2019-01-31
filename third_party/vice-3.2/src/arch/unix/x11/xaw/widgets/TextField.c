/*-----------------------------------------------------------------------------
 * TextField	A single line text entry widget
 *
 * Copyright (c) 1995 Robert W. McMullen
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 * Author: Rob McMullen <rwmcm@orion.ae.utexas.edu>
 *         http://www.ae.utexas.edu/~rwmcm
 *
 * Small patch for X11R4 compatibility by Ettore Perazzoli
 * <ettore@comm2000.it>
 *
 * International support by Olaf Seibert <rhialto@falu.nl>
 */

#include "vice.h"

#define _TextField_

#include <X11/Xlib.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmu/Xmu.h>

#include <stdio.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#  ifndef _SYS_INT_TYPES_H
     typedef long int intptr_t;
#  endif
#endif

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#  ifdef XLC_FUNCS_PRESENT
#    define INTERNATIONAL_SUPPORT
#  endif
#else
#  if !defined(_WCHAR_T_DEFINED_) && !defined(_GCC_WCHAR_T) && !defined(_WCHAR_T) && !defined(_BSD_WCHAR_T_DEFINED_)
     typedef char wchar_t;
#  endif
#endif

#include "util.h"

#include "TextFieldP.h"

#define offset(field) XtOffsetOf(TextFieldRec, text.field)

static XtResource resources[] = {
    { XtNallowSelection, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(AllowSelection), XtRString, "True" },
    { XtNdisplayCaret, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(DisplayCursor), XtRString, "True" },
    { XtNecho, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(Echo), XtRString, "True" },
    { XtNeditable, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(Editable), XtRString, "True" },
    { XtNinternational, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(international), XtRString, "False" },
    { XtNfont, XtCFont, XtRFontStruct, sizeof(XFontStruct *), offset(font), XtRString, XtDefaultFont },
    { XtNfontSet, XtCFontSet, XtRFontSet, sizeof(XFontSet), offset(fontSet), XtRString, XtDefaultFontSet },
    { XtNforeground, XtCForeground, XtRPixel, sizeof(Pixel), offset(foreground_pixel), XtRString, XtDefaultForeground },
    { XtNinsertPosition, XtCInsertPosition, XtRInt, sizeof(int), offset(CursorPos), XtRString, "0" },
    { XtNlength, XtCLength, XtRInt, sizeof(int), offset(TextMaxLen), XtRString, "0" },
    { XtNmargin, XtCMargin, XtRDimension, sizeof(Dimension), offset(Margin), XtRString, "3" },
    { XtNpendingDelete, XtCBoolean, XtRBoolean, sizeof(Boolean), offset(PendingDelete), XtRString, "True" },
    { XtNstring, XtCString, XtRString, sizeof(char *), offset(DefaultString), XtRString, NULL },
    { XtNactivateCallback, XtCCallback, XtRCallback, sizeof(XtCallbackList), offset(ActivateCallback), XtRCallback, NULL },
    { XtNcallback, XtCCallback, XtRCallback, sizeof(XtCallbackList), offset(TextChangeCallback), XtRCallback, NULL },
};

#undef offset

static Atom xa_compound_text, xa_text, xa_utf8_string;
XlcConv xlcconv_wide_to_multibyte;     /* cache converters between */
XlcConv xlcconv_multibyte_to_wide;     /* character representations */

static void ClassInitialize(void);
static void Initialize(Widget treq, Widget tnew, ArgList args, Cardinal *num);
static void Destroy(TextFieldWidget w);
static void Redisplay(Widget aw, XExposeEvent *event, Region region);
static void Resize(Widget aw);
static Boolean SetValues(Widget current, Widget request, Widget reply, ArgList args, Cardinal *nargs);
/* static void GetValuesHook(Widget aw, ArgList arglist, Cardinal *argc); */
static void Draw(TextFieldWidget w);
static void DrawInsert(TextFieldWidget w);
static void MassiveChangeDraw(TextFieldWidget w);
static void DrawTextReposition(TextFieldWidget w);
static void ClearHighlight(TextFieldWidget w);
static void DrawHighlight(TextFieldWidget w);
static void DrawCursor(TextFieldWidget w);
static void EraseCursor(TextFieldWidget w);
static Boolean PositionCursor(TextFieldWidget w);
static Boolean MassiveCursorAdjust(TextFieldWidget w);
static void Nothing(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void Activate(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void InsertChar(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ForwardChar(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void BackwardChar(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void DeleteNext(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void DeletePrev(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void SelectStart(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ExtendStart(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ExtendAdjust(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ExtendEnd(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void InsertSelection(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ActionFocusIn(Widget aw, XEvent *event, String *params, Cardinal *num_params);
static void ActionFocusOut(Widget aw, XEvent *event, String *params, Cardinal *num_params);

static char defaultTranslations[] = 
"<Key>Right:	forward-char()\n\
 <Key>Left:	backward-char()\n\
 <Key>Delete:	delete-next-char()\n\
 <Key>BackSpace:	delete-previous-char()\n\
 <Key>Return:	activate()\n\
 <Key>:		insert-char()\n\
 Shift<Btn1Down>:	extend-start()\n\
 <Btn1Down>:	select-start()\n\
 <Btn1Motion>:	extend-adjust()\n\
 <Btn1Up>:	extend-end()\n\
 <Btn2Down>:	insert-selection()\n\
 <Btn3Down>:	extend-start()\n\
 <Btn3Motion>:	extend-adjust()\n\
 <Btn3Up>:	extend-end()\n\
 <EnterWindow>:	enter-window()\n\
 <LeaveWindow>:	leave-window()\n\
 <FocusIn>:	focus-in()\n\
 <FocusOut>:	focus-out()";

static XtActionsRec actions[] = {
    { "insert-char", InsertChar },
    { "forward-char", ForwardChar },
    { "backward-char", BackwardChar },
    { "delete-next-char", DeleteNext },
    { "delete-previous-char", DeletePrev },
    { "activate", Activate },
    { "select-start", SelectStart },
    { "extend-start", ExtendStart },
    { "extend-adjust", ExtendAdjust },
    { "extend-end", ExtendEnd },
    { "insert-selection", InsertSelection },
    { "enter-window", Nothing },
    { "leave-window", Nothing },
    { "focus-in", ActionFocusIn },
    { "focus-out", ActionFocusOut },
};

TextFieldClassRec textfieldClassRec = {
    {
    /* core_class fields      */
    /* superclass             */ (WidgetClass) & widgetClassRec,
    /* class_name             */ "TextField",
    /* widget_size            */ sizeof(TextFieldRec),
    /* class_initialize       */ ClassInitialize,
    /* class_part_initialize  */ NULL,
    /* class_inited           */ False,
    /* initialize             */ Initialize,
    /* initialize_hook        */ NULL,
    /* realize                */ XtInheritRealize,
    /* actions                */ actions,
    /* num_actions            */ XtNumber(actions),
    /* resources              */ resources,
    /* num_resources          */ XtNumber(resources),
    /* xrm_class              */ NULLQUARK,
    /* compress_motion        */ True,
    /* compress_exposure      */ XtExposeCompressMultiple,
    /* compress_enterleave    */ True,
    /* visible_interest       */ True,
    /* destroy                */ (XtWidgetProc)Destroy,
    /* resize                 */ (XtWidgetProc)Resize,
    /* expose                 */ (XtExposeProc)Redisplay,
    /* set_values             */ SetValues,
    /* set_values_hook        */ NULL,
    /* set_values_almost      */ XtInheritSetValuesAlmost,
    /* get_values_hook        */ NULL,
    /* accept_focus           */ NULL,
    /* version                */ XtVersion,
    /* callback_private       */ NULL,
    /* tm_table               */ defaultTranslations,
    /* query_geometry         */ XtInheritQueryGeometry,
    /* display_accelerator    */ XtInheritDisplayAccelerator,
    /* extension              */ NULL
    },
    {
        0    /* some stupid compilers barf on empty structures */
    }
};

WidgetClass textfieldWidgetClass = (WidgetClass)&textfieldClassRec;

/* Convenience macros */
#define TopMargin(w)    (int)(w->text.Margin - 1)
#define BottomMargin(w) (int)(w->text.Margin)
#define abs(a)          ((a) >= 0? (a) : -(a))
#define min(a, b)       ((a) < (b)? (a) : (b))

/*
 * Theory of operation:
 *
 * Communication with the user program is always via the contents
 * of text.Text.
 *
 * if text.international == False:
 *      text.Text contains ISO 8859-1 (aka Latin1) text.
 *      text.SelectionText points to chars.
 *
 * if text.international == True:
 *      text.Text contains text in the user's locale.
 *          This may be a single byte (locale C), multibyte or variable
 *          length (locale xx_XX.utf-8) encoding.
 *          Xmb* functions work on this.  Since this is inconvenient to
 *          manipulate, for most operations we use
 *
 *      text.IntlText contains text in wide characters (in theory, also
 *          consistent with the user's locale, but in practice I expect
 *          always Unicode).
 *          Xwc* functions work on this.
 *          Every time IntlText changes, Text is updated by making a
 *          full copy with conversion.
 *
 *      text.SelectionText points to wchar_ts.
 */
/* Font functions */

static int FontAscent(TextFieldWidget tfw)
{
    if (tfw->text.international) {
        XFontSetExtents *ext = XExtentsOfFontSet(tfw->text.fontSet);
        return abs(ext->max_ink_extent.y);
    } else {
        XFontStruct *f = tfw->text.font;
        return f->max_bounds.ascent;
    }
}

static int FontDescent(TextFieldWidget tfw)
{
    if (tfw->text.international) {
        XFontSetExtents *ext = XExtentsOfFontSet(tfw->text.fontSet);
        int ascent = abs(ext->max_ink_extent.y);
        return ext->max_ink_extent.height - ascent;
    } else {
        XFontStruct *f = tfw->text.font;
        return f->max_bounds.descent;
    }
}

static int FontHeight(TextFieldWidget tfw)
{
    if (tfw->text.international) {
        XFontSetExtents *ext = XExtentsOfFontSet(tfw->text.fontSet);
        return ext->max_ink_extent.height;
    } else {
        XFontStruct *f = tfw->text.font;
        return f->max_bounds.ascent + f->max_bounds.descent + 2;
    }
}

static int TextWidthFromPos(TextFieldWidget tfw, int pos, int len)
{
    if (tfw->text.international) {
        return XwcTextEscapement(tfw->text.fontSet, tfw->text.IntlText + pos, len);
    } else {
        XFontStruct *f = tfw->text.font;
        return XTextWidth(f, tfw->text.Text + pos, len);
    }
}

static int TextWidthFromLeft(TextFieldWidget tfw, int len)
{
    if (tfw->text.international) {
        return XwcTextEscapement(tfw->text.fontSet, tfw->text.IntlText, len);
    } else {
        XFontStruct *f = tfw->text.font;
        return XTextWidth(f, tfw->text.Text, len);
    }
}
static int TextWidth(TextFieldWidget tfw)
{
    if (tfw->text.international) {
        return XwcTextEscapement(tfw->text.fontSet, tfw->text.IntlText, tfw->text.IntlTextLen);
    } else {
        XFontStruct *f = tfw->text.font;
        return XTextWidth(f, tfw->text.Text, tfw->text.TextLen);
    }
}

static void InitializeGC(TextFieldWidget w)
{
    XGCValues values;
    XtGCMask mask;
    XtGCMask dynamic_mask;
    Display *dpy = XtDisplay((Widget)w);
    Window wnd = XtWindow((Widget)w);

    if (!wnd) {
        Screen *screen = XtScreen((Widget)w);
        wnd = RootWindowOfScreen(screen);
    }

    values.line_style = LineSolid;
    values.line_width = 0;
    values.fill_style = FillSolid;
    values.background = w->core.background_pixel;
    values.foreground = w->text.foreground_pixel;
    mask = GCLineStyle | GCLineWidth | GCFillStyle | GCForeground | GCBackground;
    dynamic_mask = GCClipXOrigin | GCClipYOrigin | GCClipMask;
    values.clip_x_origin = 0;
    values.clip_y_origin = 0;
    values.clip_mask = None;
    if (!w->text.international) {
        values.font = w->text.font->fid;
        mask |= GCFont;
    }
    w->text.drawGC = XCreateGC(dpy, wnd, mask|dynamic_mask, &values);

    values.foreground = w->core.background_pixel;
    values.background = w->text.foreground_pixel;
    w->text.highlightGC = XCreateGC(dpy, wnd, mask|dynamic_mask, &values);

    values.line_style = LineSolid;
    values.line_width = 0;
    values.background = w->core.background_pixel;
    values.foreground = w->text.foreground_pixel;
    mask = GCLineStyle | GCLineWidth | GCForeground | GCBackground;
    w->text.cursorGC = XtGetGC((Widget)w, mask, &values);

    values.foreground = w->core.background_pixel;
    values.background = w->text.foreground_pixel;
    w->text.eraseGC = XtGetGC((Widget)w, mask, &values);

    values.line_style = LineOnOffDash;
    values.background = w->core.background_pixel;
    values.foreground = w->text.foreground_pixel;
    values.dash_offset = 0;
    values.dashes = 1;  /* same as a 2-element list [1,1] to XSetDashes() */
    mask |= GCDashOffset | GCDashList;
    w->text.dashGC = XtGetGC((Widget)w, mask, &values);

    w->text.FontAscent = FontAscent(w);
    w->text.FontDescent = FontDescent(w);
    w->text.FontHeight = FontHeight(w);
    w->text.YOffset = TopMargin(w) + w->text.FontAscent;
}

static void ClipGC(TextFieldWidget w)
{
    XRectangle clip;

    clip.x = 0;
    clip.y = w->text.YOffset - w->text.FontAscent + 1;
    clip.width = w->text.ViewWidth + 1;
    clip.height = w->text.FontHeight;
    XSetClipRectangles(XtDisplay((Widget)w), w->text.drawGC, w->text.Margin, 0, &clip, 1, Unsorted);
    XSetClipRectangles(XtDisplay((Widget)w), w->text.highlightGC, w->text.Margin, 0, &clip, 1, Unsorted);
}

#ifdef INTERNATIONAL_SUPPORT
/* Should be in <X11/Xlcint.h> */
# if 0  /* slightly more documented but slower */
#  define multibyte_to_wide(dest, src, nchars, nbytes)    _Xlcmbstowcs(NULL, dest, src, nchars)
#  define wide_to_multibyte(dest, src, nbytes, nchars)    _Xlcwcstombs(NULL, dest, src, nbytes)
# else  /* expanded from source */
#  define multibyte_to_wide(wdest, msrc, wchars, mbytes)    do_convert(xlcconv_multibyte_to_wide, wdest, msrc, wchars, /*strlen(msrc)*/mbytes)
# define wide_to_multibyte(mdest, wsrc, mbytes, wchars)    do_convert(xlcconv_wide_to_multibyte, mdest, wsrc, mbytes, /*wcslen(wsrc)*/wchars)
# endif
#else /* no INTERNATIONAL_SUPPORT */
# define multibyte_to_wide(wdest, msrc, wchars, mbytes) (-1)
# define wide_to_multibyte(mdest, wsrc, mbytes, wchars) (-1)
#endif /* INTERNATIONAL_SUPPORT */

/*
 * Return the number of characters in a nonterminated multibyte string.
 */
static int mb_strlen(char *s, int bytes)
{
    int len = 0;

    while (bytes > 0) {
        int onechar;

        /* Heuristic: 1 byte can't be more than 1 char */
        if (bytes == 1) {
            return len + 1;
#ifdef INTERNATIONAL_SUPPORT
        } else if ((onechar = _Xmblen(s, bytes)) <= 0) {
            break;
#endif
        }
        s += onechar;
        bytes -= onechar;
        len++;
    }

    return len;
}

/*
 * Similar to _Xlcmbstowcs() in xlibi18n/lcStd.c but without looking
 * up the converters every time.
 * Differences: Does not zero-terminate, but also does not require
 * source to be zero-terminated.
 */
#ifdef INTERNATIONAL_SUPPORT
static int do_convert(XlcConv conv, void *dest, void *src, int destsize, int srcsize)
{
    XPointer from, to;
    int from_left, to_left;

    from = (XPointer)src;
    from_left = srcsize;
    to = (XPointer)dest;
    to_left = destsize;

    if (_XlcConvert(conv, &from, &from_left, &to, &to_left, NULL, 0) < 0) {
        return -1;
    }

    return destsize - to_left;
}
#endif

static void SetString(TextFieldWidget w, char *s)
{
    int len;

    if (s) {
        len = strlen(s);
        if (len >= w->text.TextAlloc) {
            w->text.TextAlloc += len;
            w->text.Text = XtRealloc(w->text.Text, w->text.TextAlloc);
        }
        strcpy(w->text.Text, s);
        w->text.TextLen = len;
        if ((w->text.TextMaxLen > 0) && (w->text.TextLen > w->text.TextMaxLen)) {
            w->text.TextMaxLen = w->text.TextLen;
        }
#ifdef INTERNATIONAL_SUPPORT
        if (w->text.international) {
             w->text.IntlText = (wchar_t *)XtRealloc((char *)w->text.IntlText,
                                        sizeof(wchar_t) * w->text.TextAlloc);
             /*
              * "Len" 1 is the max number of characters in the destination.
              * "Len" 2 is the number of bytes in the source.
              */
             w->text.IntlTextLen = multibyte_to_wide(w->text.IntlText, s, len, len);
             w->text.IntlText[w->text.IntlTextLen] = L'\0';
        }
#endif
        w->text.TextWidth = w->text.OldTextWidth = TextWidth(w);
    }
    w->text.DefaultString = w->text.Text;
}

static void ClassInitialize(void)
{
#ifdef INTERNATIONAL_SUPPORT
    XLCd lcd = _XlcCurrentLC();
    if (lcd) {
        xlcconv_wide_to_multibyte = _XlcOpenConverter(lcd, XlcNWideChar, lcd, XlcNMultiByte);
        xlcconv_multibyte_to_wide = _XlcOpenConverter(lcd, XlcNMultiByte, lcd, XlcNWideChar);
    }
#endif
}

/* Unused... */
#if 0
static void ClassDestroy(void)
{
#ifdef INTERNATIONAL_SUPPORT
    if (xlcconv_multibyte_to_wide) {
        _XlcCloseConverter(xlcconv_multibyte_to_wide);
    }
    if (xlcconv_wide_to_multibyte) {
        _XlcCloseConverter(xlcconv_wide_to_multibyte);
    }
#endif
}
#endif

static void Initialize(Widget treq, Widget tnew, ArgList args, Cardinal *num)
{
    TextFieldWidget new;
    int height;

    new = (TextFieldWidget)tnew;

    new->text.timer_id = (XtIntervalId)0;
    new->text.multi_click_time = XtGetMultiClickTime(XtDisplay((Widget)new));
    new->text.highlight_time = new->text.multi_click_time / 2;

    if (new->text.TextMaxLen > 0) {
        new->text.TextAlloc = new->text.TextMaxLen + 1;
    } else {
        new->text.TextAlloc = TEXTFIELD_ALLOC_SIZE;
    }
    new->text.Text = (char *)XtMalloc(new->text.TextAlloc);
    if (new->text.international) {
        /*
         * Invariant assumption: the number of bytes in Text (TextAlloc)
         * will be sufficient as the number of wide characters in IntlText,
         * so they can share the value.
         */
        new->text.IntlText = (wchar_t *)XtMalloc(sizeof(wchar_t) * new->text.TextAlloc);
    } else {
        new->text.IntlText = NULL;
    }
    new->text.TextLen = 0;
    new->text.SelectionText.t_char = NULL;
    new->text.TextWidth = new->text.OldTextWidth = 0;
    if (new->text.DefaultString) {
        SetString(new, new->text.DefaultString);
    }

    if (new->text.CursorPos > 0) {
        if (new->text.CursorPos > new->text.TextLen) {
            new->text.CursorPos = new->text.TextLen;
        }
    } else {
        new->text.CursorPos = 0;
    }
    new->text.OldCursorX = -1;
    new->text.HighlightStart = new->text.HighlightEnd = -1;
    new->text.OldHighlightStart = new->text.OldHighlightEnd = -1;

    height = FontHeight(new);
    if (new->core.height == 0) {
        new->core.height = (Dimension) height + TopMargin(new) + BottomMargin(new);
    }

    if (new->core.width == 0) {
        new->text.ViewWidth = 200;
        new->core.width = new->text.ViewWidth + 2 * new->text.Margin;
    } else {
        int width;

        width = (int)new->core.width - 2 * new->text.Margin;
        if (width < 0) {
            new->text.ViewWidth = new->core.width;
        } else {
            new->text.ViewWidth = width;
        }
    }

    new->text.XOffset = new->text.OldXOffset = 0;

    InitializeGC(new);

    ClipGC(new);

    new->text.xim = 0;
    new->text.xic = 0;
    new->text.selection_type = 0;

    /*
     * Assume a single program doesn't use multiple displays.
     * Apart from that, these values are constant.
     * And otherwise it could be put in ClassInitialize().
     */
    if (xa_compound_text == 0) {
        xa_compound_text = XA_COMPOUND_TEXT(XtDisplay(new));
    }
    if (xa_text == 0) {
        xa_text = XA_TEXT(XtDisplay(new));
    }
    if (xa_utf8_string == 0) {
#ifdef X_HAVE_UTF8_STRING
        xa_utf8_string = XA_UTF8_STRING(XtDisplay(new));
#else
        xa_utf8_string = -1;
#endif
    }
}

static void Destroy(TextFieldWidget w)
{
    Display *dpy = XtDisplay((Widget)w);
    XFreeGC(dpy, w->text.drawGC);
    XFreeGC(dpy, w->text.highlightGC);
    XtReleaseGC((Widget) w, w->text.cursorGC);
    XtReleaseGC((Widget) w, w->text.eraseGC);
    XtReleaseGC((Widget) w, w->text.dashGC);
    if (w->text.SelectionText.t_char) {
        XtFree(w->text.SelectionText.t_char);
    }
    XtFree(w->text.Text);
    if (w->text.IntlText) {
        XtFree((char *)w->text.IntlText);
    }
    if (w->text.xic) {
        XDestroyIC(w->text.xic);
    }
    if (w->text.xim) {
        XCloseIM(w->text.xim);
    }
}

static void Redisplay(Widget aw, XExposeEvent *event, Region region)
{
    TextFieldWidget w = (TextFieldWidget) aw;

    if (!XtIsRealized(aw)) {
        return;
    }

    Draw(w);
}

static Boolean SetValues(Widget current, Widget request, Widget reply, ArgList args, Cardinal *nargs)
{
    TextFieldWidget w = (TextFieldWidget)current;
    TextFieldWidget new = (TextFieldWidget)reply;
    Boolean redraw = False;

#ifndef INTERNATIONAL_SUPPORT
    new->text.international = False;
#endif

    if ((w->text.foreground_pixel != new->text.foreground_pixel) ||
        (w->core.background_pixel != new->core.background_pixel) ||
        (w->text.international != new->text.international) ||
        ((w->text.fontSet != new->text.fontSet) && w->text.international) ||
        (w->text.font != new->text.font)) {
        XtReleaseGC((Widget)w, w->text.drawGC);
        XtReleaseGC((Widget)w, w->text.highlightGC);
        XtReleaseGC((Widget)w, w->text.cursorGC);
        XtReleaseGC((Widget)w, w->text.eraseGC);
        XtReleaseGC((Widget)w, w->text.dashGC);
        InitializeGC(new);
        redraw = True;
    }

    if ((w->text.CursorPos != new->text.CursorPos) || (w->text.DisplayCursor != new->text.DisplayCursor)) {
        redraw = True;
    }
    if (w->text.DefaultString != new->text.DefaultString) {
        redraw = True;
        SetString(new, new->text.DefaultString);
        new->text.HighlightStart = new->text.HighlightEnd = -1;
        if (new->text.international) {
            new->text.CursorPos = new->text.IntlTextLen;
        } else {
            new->text.CursorPos = new->text.TextLen;
        }
#ifdef DEBUG_TF
        printf("SetValues: %s\n", new->text.DefaultString);
#endif
    }

    return redraw;
}

#if 0
static void GetValuesHook(Widget aw, ArgList arglist, Cardinal *argc)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int i;

    for (i = 0; i < *argc; i++) {
        /*
         * All values for XtGetValues() are pointers, so it is
         * guaranteed we can dereference them.
         */
        //if (strcmp(XtNstring, arglist[i].name) == 0)
        if (*(char **)arglist[i].value == w->text.DefaultString) {
            /* FixupText2(aw); */
        }
    }
}
#endif

static void Resize(Widget aw)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int width, height;

    width = w->core.width - 2 * w->text.Margin;
    if (width < 0) {
        w->text.ViewWidth = w->core.width;
    } else {
        w->text.ViewWidth = width;
    }

    height = (((int) w->core.height - w->text.FontHeight) / 2) + w->text.FontAscent;
    w->text.YOffset = height;

    ClipGC(w);

    if (XtIsRealized(aw)) {
        MassiveChangeDraw(w);
    }
}

static int FixupText(TextFieldWidget w)
{
    char *text = w->text.Text;
    wchar_t *intltext = w->text.IntlText;
    int textlen = w->text.TextAlloc - 1;
    int intltextlen = w->text.IntlTextLen;
    textlen = wide_to_multibyte(text, intltext, textlen, intltextlen);
    text[textlen] = '\0';
    return textlen;
}

#if 0
static void FixupText2(TextFieldWidget w)
{
    if (w->text.Text[0] == 0 && w->text.TextLen > 0) {
        if (w->text.international) {
            /* _X LoCale Wide Character String TO Multi-Byte String */
            wide_to_multibyte(w->text.Text, w->text.IntlText, w->text.TextAlloc);
        } else {
            int len = w->text.TextAlloc - 1;
            char *buf = w->text.Text;
            wchar_t *wbuf = w->text.IntlText;
            /* Quick And Dirty Unicode -> Latin1 */
            while (len > 0 && *wbuf) {
                if (*wbuf < 0x0100) {
                    *buf++ = *wbuf;
                } else {
                    *buf++ = '?';
                }
                wbuf++;
                len--;
            }
            *buf++ = 0;
            //return buf - w->w->text.Text;
        }
    }
}
#endif

static void TextDelete(TextFieldWidget w, int start, int len)
{
    int i;

    if (len > 0) {
        if (w->text.international) {
            for (i = start + len; i < w->text.IntlTextLen; i++) {
                w->text.IntlText[i - len] = w->text.IntlText[i];
            }
            w->text.IntlTextLen -= len;
            w->text.IntlText[w->text.IntlTextLen] = 0;
            w->text.TextLen = FixupText(w);
        } else {
            for (i = start + len; i < w->text.TextLen; i++) {
                w->text.Text[i - len] = w->text.Text[i];
            }
            w->text.TextLen -= len;
            w->text.Text[w->text.TextLen] = 0;
        }
        w->text.TextWidth = TextWidth(w);
    }
}

static void TextDeleteHighlighted(TextFieldWidget w)
{
    if (w->text.HighlightStart >= 0) {
        TextDelete(w, w->text.HighlightStart, w->text.HighlightEnd - w->text.HighlightStart);
        w->text.CursorPos = w->text.HighlightStart;
        w->text.HighlightStart = w->text.HighlightEnd = -1;
    }
}

/* returns value indicating if the text can be redrawn using the fast
 * method */
static Boolean TextInsert(TextFieldWidget w, char *buf, int len)
{
    int i;
    Boolean regular_copy, fast_insert;

    fast_insert = True;
    if (len > 0) {
        if (w->text.HighlightStart >= 0) {
            fast_insert = False;
            if (w->text.PendingDelete) {
                TextDeleteHighlighted(w);
            } else {
                ClearHighlight(w);
            }
        }

        regular_copy = True;
        if (w->text.TextMaxLen > 0) {
            if (w->text.TextLen + len > w->text.TextMaxLen) {
                regular_copy = False;
            }
        } else if (w->text.TextLen + len >= w->text.TextAlloc) {
            i = TEXTFIELD_ALLOC_SIZE;
            if (i < len) {
                i = len;
            }
            w->text.TextAlloc += i + 1;
            w->text.Text = XtRealloc(w->text.Text, w->text.TextAlloc);
            if (w->text.international) {
                w->text.IntlText = (wchar_t *)XtRealloc((char *)w->text.IntlText, sizeof(wchar_t) * w->text.TextAlloc);
            }
#ifdef DEBUG_TF
            printf("TextInsert: Alloced new space: %d bytes\n", w->text.TextAlloc);
#endif
            w->text.DefaultString = w->text.Text;
        }
        if (regular_copy) {
            w->text.FastInsertCursorStart = w->text.CursorPos;

            if (w->text.international) {
                int intllen = mb_strlen(buf, len);

                for (i = w->text.IntlTextLen - 1; i >= w->text.CursorPos; i--) {
                    w->text.IntlText[i + intllen] = w->text.IntlText[i];
                }
                multibyte_to_wide(&w->text.IntlText[w->text.CursorPos], buf, intllen, len);

                w->text.CursorPos += intllen;
                w->text.IntlTextLen += intllen;
                w->text.IntlText[w->text.IntlTextLen] = 0;
                FixupText(w);
                w->text.FastInsertTextLen = intllen;
            } else {
                for (i = w->text.TextLen - 1; i >= w->text.CursorPos; i--) {
                    w->text.Text[i + len] = w->text.Text[i];
                }
                strncpy(&w->text.Text[w->text.CursorPos], buf, len);
                w->text.CursorPos += len;
                w->text.FastInsertTextLen = len;
            }
            w->text.TextLen += len;
        } else {
            int i1;

            if (w->text.international) {
                int intllen = mb_strlen(buf, len);
                int tocopy;

                for (i = w->text.IntlTextLen - 1; i >= w->text.CursorPos; i--) {
                    if (i + len < w->text.TextMaxLen) {
                        w->text.IntlText[i + intllen] = w->text.IntlText[i];
                    }
                }
                w->text.IntlTextLen += intllen;
                if (w->text.IntlTextLen > w->text.TextMaxLen) {
                    w->text.IntlTextLen = w->text.TextMaxLen;
                }
                i1 = w->text.CursorPos;
                tocopy = w->text.TextMaxLen - i1;
                tocopy = min(tocopy, intllen);
                multibyte_to_wide(&w->text.IntlText[i1], buf, tocopy, len);
                i1 += tocopy;

                w->text.IntlText[w->text.IntlTextLen] = 0;
                w->text.TextLen = FixupText(w);
            } else {
                for (i = w->text.TextLen - 1; i >= w->text.CursorPos; i--) {
                    if (i + len < w->text.TextMaxLen) {
                        w->text.Text[i + len] = w->text.Text[i];
                    }
                }
                w->text.TextLen += len;
                if (w->text.TextLen > w->text.TextMaxLen) {
                    w->text.TextLen = w->text.TextMaxLen;
                }
                i1 = w->text.CursorPos;
                for (i = 0; i < len; i++) {
                    if (i1 < w->text.TextMaxLen) {
                        w->text.Text[i1] = *buf++;
                    } else {
                        break;
                    }
                    i1++;
                }
            }

            w->text.FastInsertCursorStart = w->text.CursorPos;
            w->text.FastInsertTextLen = i1 - w->text.CursorPos;
            w->text.CursorPos = i1;
        }
        w->text.TextWidth = TextWidth(w);
        w->text.Text[w->text.TextLen] = 0;
    }
    return fast_insert;
}

static int TextPixelToPos(TextFieldWidget w, int x)
{
    int i, tot, cur, pos;
    int textlen = w->text.international ? w->text.IntlTextLen
                                        : w->text.TextLen;

    pos = 0;

    x -= (int) w->text.Margin + w->text.XOffset;

    /* check if the cursor is before the 1st character */
    if (x <= 0) {
        pos = 0;
    } else if (x > w->text.TextWidth) {
        pos = textlen;
    } else {
        tot = 0;
        pos = -1;
        for (i = 0; i < textlen; i++) {
            cur = TextWidthFromPos(w, i, 1);
            if (x < tot + (cur / 2)) {
                pos = i;
                break;
            }
            tot += cur;
        }
        if (pos < 0) {
            pos = textlen;
        }
    }
    return pos;
}

/*
 * Do callback
 */
static void TextChanged(TextFieldWidget w)
{
    if (XtNcallback) {
        char *text = w->text.Text;
        XtCallCallbacks((Widget)w, XtNcallback, text);
    }
}

/*
 * TextField Widget Action procedures
 */

/* ARGSUSED */
static void Nothing(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
}

/* ARGSUSED */
static void Activate(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    TextFieldReturnStruct ret;

    ret.reason = 0;
    ret.event = event;
    ret.string = w->text.Text;

    if (XtNactivateCallback) {
        XtCallCallbacks(aw, XtNactivateCallback, &ret);
    }
}

/* ARGSUSED */
static void ForwardChar(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int textlen;

    if (!w->text.Editable) {
        return;
    }

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;
    ClearHighlight(w);
    if (w->text.CursorPos < textlen) {
        w->text.CursorPos++;
        EraseCursor(w);
        if (PositionCursor(w)) {
            DrawTextReposition(w);
        }
        DrawCursor(w);
    }
}

/* ARGSUSED */
static void BackwardChar(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!w->text.Editable) {
        return;
    }

    ClearHighlight(w);
    if (w->text.CursorPos > 0) {
        w->text.CursorPos--;
        EraseCursor(w);
        if (PositionCursor(w)) {
            DrawTextReposition(w);
        }
        DrawCursor(w);
    }
}

/* ARGSUSED */
static void InsertChar(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int len;
    static XComposeStatus compose;
    Status status;

#define INSERTCHARBUFSIZ 32
    char buf[INSERTCHARBUFSIZ];

    if (!w->text.Editable) {
        return;
    }
    if (w->text.xic) {
        /*
         * One would think that XmbLookupString and XwcLookupString
         * would return the same characters (modulo representation)...
         * but no. At least in the "C" locale, dead keys and Compose
         * only work with XmbLookupString().
         */
        len = XmbLookupString(w->text.xic, (XKeyEvent *)event, buf, BUFSIZ, NULL, &status);
    } else {
        len = XLookupString((XKeyEvent *)event, buf, BUFSIZ, NULL, &compose);
    }

    if (len > 0) {
        EraseCursor(w);
        if (TextInsert(w, buf, len)) {
            DrawInsert(w);
        } else {
            Draw(w);
        }
        TextChanged(w);
    }
}

/* ARGSUSED */
static void DeleteNext(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int textlen;

    if (!w->text.Editable) {
        return;
    }

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;

    if (w->text.HighlightStart >= 0 && w->text.PendingDelete) {
        TextDeleteHighlighted(w);
        MassiveChangeDraw(w);
    } else if (w->text.CursorPos < textlen) {
        ClearHighlight(w);
        TextDelete(w, w->text.CursorPos, 1);
        Draw(w);
    }
    TextChanged(w);
}

/* ARGSUSED */
static void DeletePrev(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!w->text.Editable) {
        return;
    }

    if (w->text.HighlightStart >= 0 && w->text.PendingDelete) {
        TextDeleteHighlighted(w);
        MassiveChangeDraw(w);
    } else if (w->text.CursorPos > 0) {
        ClearHighlight(w);
        TextDelete(w, w->text.CursorPos - 1, 1);
        w->text.CursorPos--;
        Draw(w);
    }
    TextChanged(w);
}

/* ARGSUSED */
static void SelectStart(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget) aw;

    if (!w->text.AllowSelection) {
        return;
    }

    w->text.CursorPos = TextPixelToPos(w, event->xbutton.x);
    w->text.HighlightPivotStart = w->text.HighlightPivotEnd = w->text.CursorPos;

    if (w->text.HighlightStart >= 0) {
        ClearHighlight(w);
    } else {
        EraseCursor(w);
        DrawCursor(w);
    }
}

/* ARGSUSED */
static void ExtendStart(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget) aw;
    int pos;

    if (!w->text.AllowSelection) {
        return;
    }

    pos = TextPixelToPos(w, event->xbutton.x);

    EraseCursor(w);
    if (w->text.HighlightStart < 0) {
        w->text.HighlightStart = w->text.HighlightEnd = w->text.HighlightPivotStart = w->text.HighlightPivotEnd = w->text.CursorPos;
    } else {
        w->text.HighlightPivotStart = w->text.HighlightStart;
        w->text.HighlightPivotEnd = w->text.HighlightEnd;
    }
    if (pos < w->text.HighlightStart) {
        w->text.HighlightStart = pos;
    } else {
        w->text.HighlightEnd = pos;
    }
    w->text.CursorPos = pos;
#ifdef DEBUG_TF
    printf("ExtendStart: %d - %d\n", w->text.HighlightStart, w->text.HighlightEnd);
#endif
    DrawHighlight(w);
    DrawCursor(w);
}

static void ExtendHighlight(TextFieldWidget w)
{
    int x, pos;

    if (!w->text.AllowSelection) {
        return;
    }

    x = w->text.timer_x;
    pos = TextPixelToPos(w, x);

    if (x < (int) w->text.Margin) {
        pos = TextPixelToPos(w, (int)0);
        if (pos > 0) {
            pos--;
        } else if (pos == w->text.CursorPos) {
            return;
        }
    } else if (x > (int)(w->text.Margin + w->text.ViewWidth)) {
        pos = TextPixelToPos(w, (int) (w->text.Margin + w->text.ViewWidth));
        if (pos < w->text.TextLen) {
            pos++;
        } else if (pos == w->text.CursorPos) {
            return;
        }
    }
    if (pos == w->text.CursorPos) {
        return;
    }

    EraseCursor(w);
    if (pos <= w->text.HighlightPivotStart) {
        w->text.HighlightStart = pos;
        w->text.HighlightEnd = w->text.HighlightPivotEnd;
    } else {
        w->text.HighlightStart = w->text.HighlightPivotStart;
        w->text.HighlightEnd = pos;
    }
    w->text.CursorPos = pos;
#ifdef DEBUG_TF
    printf("Highlighting: x=%d pos=%d  %d - %d\n", x, pos, w->text.HighlightStart, w->text.HighlightEnd);
#endif
    if (PositionCursor(w)) {
        DrawTextReposition(w);
    }
    DrawHighlight(w);
    DrawCursor(w);
}

static void ExtendTimer(XtPointer client_data, XtIntervalId *idp)
{
    TextFieldWidget w = (TextFieldWidget)client_data;

    ExtendHighlight(w);
    w->text.timer_id = XtAppAddTimeOut(XtWidgetToApplicationContext((Widget)w), (unsigned long)w->text.highlight_time, ExtendTimer, (XtPointer)w);
}

/* ARGSUSED */
static void ExtendAdjust(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!w->text.AllowSelection) {
        return;
    }

    w->text.timer_x = event->xbutton.x;

    if (event->xbutton.x < w->text.Margin || event->xbutton.x > w->text.Margin + w->text.ViewWidth) {
        if (w->text.timer_id) {
            ExtendHighlight(w);
        } else {
            ExtendTimer((XtPointer)w, (XtIntervalId)0);
        }
    } else {
        if (w->text.timer_id) {
            XtRemoveTimeOut(w->text.timer_id);
            w->text.timer_id = (XtIntervalId)0;
        }
        ExtendHighlight(w);
    }
}

/*
 * Convert selection for use by other programs, if asked.
 */
/* ARGSUSED */
static Boolean ConvertSelection(Widget aw, Atom *selection, Atom *target, Atom *type, XtPointer *value, unsigned long *length, int *format)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    XSelectionRequestEvent *req = XtGetSelectionRequest(aw, *selection, NULL);

    if (*target == XA_TARGETS(XtDisplay(aw))) {
        /* A request to know which formats we support */
        Atom *targetP;
        XPointer std_targets;
        unsigned long std_length;
        int num_atoms;

        XmuConvertStandardSelection(aw, req->time, selection, target, type, &std_targets, &std_length, format);

        num_atoms = std_length + 1;
        if (w->text.international) {
#ifdef X_HAVE_UTF8_STRING
            num_atoms += 3;
#else
            num_atoms += 2;
#endif
        }

        *value = XtMalloc((unsigned)sizeof(Atom) * num_atoms);
        targetP = *(Atom **)value;
        *length = num_atoms;
        if (w->text.international) {
#ifdef X_HAVE_UTF8_STRING
            *targetP++ = xa_utf8_string;
#endif
            *targetP++ = xa_text;
            *targetP++ = xa_compound_text;
        }
        *targetP++ = XA_STRING;
        memmove((char *)targetP, (char *)std_targets, sizeof(Atom) * std_length);
        XtFree((char *)std_targets);
        *type = XA_ATOM;
        *format = sizeof(Atom) * 8;
        return True;
    } else if (w->text.international &&
                (*target == xa_utf8_string ||   /* UTF-8 */
                 *target == xa_text ||          /* User's locale */
                 *target == xa_compound_text || /* you don't want to know */
                 *target == XA_STRING)) {       /* Latin-1 */
        XTextProperty text_prop;
        Atom tgt = *target;
        XICCEncodingStyle style = XStdICCTextStyle; /* STRING, else COMPOUND_TEXT */
        int success;

        if (tgt == XA_STRING) {
            style = XStringStyle;
#ifdef X_HAVE_UTF8_STRING
        } else if (tgt == xa_utf8_string) {
            style = XUTF8StringStyle;
#endif
        } else if (tgt == xa_text) {
            style = XTextStyle;
        }

        success = XwcTextListToTextProperty(XtDisplay(w),
                (wchar_t **)&w->text.SelectionText, 1, style, &text_prop);
        if (success == Success) {
            *length = text_prop.nitems;
            *value = (XtPointer)(text_prop.value);
            *type = text_prop.encoding;
            *format = text_prop.format;
            return True;
        }
        /* Intrinsics XFree()s this memory since there is no DoneProc. */
    } else if (w->text.international == False && *target == XA_STRING) { /* Latin-1 */
        *length = (long)w->text.SelectionLen;
        *value = XtMalloc(*length);
        strncpy(*value, w->text.SelectionText.t_char, w->text.SelectionLen);
        *type = XA_STRING;
        *format = 8;
        return True;
        /* Intrinsics XFree()s this memory since there is no DoneProc. */
    } else if (XmuConvertStandardSelection(aw, req->time, selection, target,
                type, (XPointer *)value, length, format)) {
        return True;
    }
    return False;
}

/* ARGSUSED */
static void LoseSelection(Widget aw, Atom *selection)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    ClearHighlight(w);
}

/* ARGSUSED */
static void ExtendEnd(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int len;

    if (!w->text.AllowSelection) {
        return;
    }

    if (w->text.timer_id) {
        XtRemoveTimeOut(w->text.timer_id);
        w->text.timer_id = (XtIntervalId)0;
    }
    len = w->text.HighlightEnd - w->text.HighlightStart;
    if (len > 0) {
        w->text.SelectionLen = len;
        if (w->text.SelectionText.t_char) {
            XtFree(w->text.SelectionText.t_char);
        }
#ifdef INTERNATIONAL_SUPPORT
        if (w->text.international) {
            wchar_t *ptr;
            ptr = (wchar_t *)XtMalloc((len+1) * sizeof(wchar_t));
            wcsncpy(ptr, &w->text.IntlText[w->text.HighlightStart], len);
            ptr[len] = 0;
            w->text.SelectionText.t_char = (char *)ptr;
        } else
#endif
        {
            w->text.SelectionText.t_char = XtMalloc(len+1);
            strncpy(w->text.SelectionText.t_char, &w->text.Text[w->text.HighlightStart], len);
            w->text.SelectionText.t_char[len] = 0;
        }

        XtOwnSelection(aw, XA_PRIMARY, event->xbutton.time, ConvertSelection, LoseSelection, NULL);
        if (w->text.international == False) {
            XChangeProperty(XtDisplay(aw), DefaultRootWindow(XtDisplay(aw)), XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, (unsigned char *)w->text.SelectionText.t_char, len);
        }
    }
}

/* ARGSUSED */
static void ReceiveSelection(Widget aw, XtPointer client, Atom * selection, Atom * type, XtPointer value, unsigned long *length, int *format)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (*type == XT_CONVERT_FAIL || *length == 0) {
        /* Try another type of conversion */
        Atom next_type;
#ifdef X_HAVE_UTF8_STRING
        if (w->text.selection_type == xa_utf8_string) {
            next_type = xa_text;
        } else
#endif
        if (w->text.selection_type == xa_text) {
            next_type = xa_compound_text;
        } else if (w->text.selection_type == xa_compound_text) {
            next_type = XA_STRING;
        } else {    /* give up */
            w->text.selection_type = 0;
            return;
        }
        /* Try again with next type */
        w->text.selection_type = next_type;
        XtGetSelectionValue(aw, XA_PRIMARY, next_type, ReceiveSelection,
                            value, w->text.selection_time);
        return;
    } else
    if ((value == NULL) || (*length == 0)) {
#ifdef DEBUG_TF
        printf("ReceiveSelection: no selection\n");
#endif
    } else {
        int savex;

        ClearHighlight(w);
        savex = w->text.OldCursorX;
        w->text.CursorPos = (int)(intptr_t)client;
#ifdef DEBUG_TF
        printf("ReceiveSelection: format %d type %d inserting '%s' length=%d at pos: %d\n", *format, (int)*type, (char *)value, (int)(*length), w->text.CursorPos);
#endif
        if (w->text.international) {
            XTextProperty text_prop;
            char        **text_list = NULL;
            int           status;
            int           n_text = 0;

            text_prop.value = value;
            text_prop.encoding = *type;
            text_prop.format = *format;
            text_prop.nitems = *length;

            status = XmbTextPropertyToTextList(XtDisplay(aw), &text_prop,
                                                         &text_list, &n_text);
            if (status == Success && n_text >= 1) {
                TextInsert(w, text_list[0], strlen(text_list[0]));
            }
            if (text_list != NULL) {
                XFreeStringList(text_list);
            }
        } else {
            /* We only requested XA_STRING */
            TextInsert(w, (char *)value, (int)(*length));
        }
        w->text.OldCursorX = savex;
        Draw(w);
        TextChanged(w);
    }
    w->text.selection_type = 0;
}

/* ARGSUSED */
static void InsertSelection(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int pos;
    Atom type;

    /* Assure that no multiple paste operations will be in progress at once */
    if (!w->text.AllowSelection || w->text.selection_type != 0) {
        return;
    }

    pos = TextPixelToPos(w, event->xbutton.x);
#ifdef DEBUG_TF
    printf("InsertSelection: event at pos: %d\n", pos);
#endif
    /* xterm(1) section SELECTION TARGETS explains. */
#ifdef X_HAVE_UTF8_STRING
    type = w->text.international ? xa_utf8_string : XA_STRING;
#else
    type = w->text.international ? xa_text : XA_STRING;
#endif
    w->text.selection_type = type;
    w->text.selection_time = event->xbutton.time;
    XtGetSelectionValue(aw, XA_PRIMARY, type, ReceiveSelection,
            (XtPointer)(intptr_t)pos, event->xbutton.time);
}

static void ActionFocusIn(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (w->text.xim == NULL) {
        /*
         * Do this before the first keys are hit.
         */
        w->text.xim = XOpenIM(XtDisplay(aw), NULL, NULL, NULL);
        if (w->text.xim) {
            w->text.xic = XCreateIC(w->text.xim,
                            XNInputStyle, XIMPreeditNothing|XIMStatusNothing,
                            XNClientWindow, XtWindow(aw),
                            XNFocusWindow, XtWindow(aw),
                            NULL);
        }
    }
    if (w->text.xic) {
        XSetICFocus(w->text.xic);
    }
}

static void ActionFocusOut(Widget aw, XEvent *event, String *params, Cardinal *num_params)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (w->text.xic) {
        XUnsetICFocus(w->text.xic);
    }
}

/*
 * TextField private drawing functions
 */

static Boolean PositionCursor(TextFieldWidget w)
{
    int x, start, end;
    Boolean moved;
    int textlen;

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;

    moved = False;
    if (w->text.CursorPos < 0) {
        w->text.CursorPos = 0;
    } else if (w->text.CursorPos > textlen) {
        w->text.CursorPos = textlen;
    }
    x = TextWidthFromLeft(w, w->text.CursorPos);
    start = -w->text.XOffset;
    end = start + w->text.ViewWidth;
    if (x < start) {
        w->text.XOffset = -x;
        moved = True;
    } else if (x > end) {
        w->text.XOffset = w->text.ViewWidth - x;
        moved = True;
    }
    return moved;
}

static Boolean MassiveCursorAdjust(TextFieldWidget w)
{
    int start, end, last;
    Boolean moved;

    moved = False;
    end = TextWidthFromLeft(w, w->text.CursorPos);
    if (w->text.HighlightStart >= 0) {
        start = TextWidthFromLeft(w, w->text.HighlightStart);
    } else {
      start = end;
    }

    if (end < w->text.ViewWidth) {
        if (w->text.XOffset < 0) {
            w->text.XOffset = 0;
            moved = True;
        }
    } else if (start >= w->text.XOffset && end < w->text.XOffset + w->text.ViewWidth) {
        return moved;
    } else {
        last = w->text.TextWidth;
        if (start - end > w->text.ViewWidth) {
            if (last - end > w->text.ViewWidth) {
                w->text.XOffset = w->text.ViewWidth - last;
            } else {
                w->text.XOffset = w->text.ViewWidth - end;
            }
        } else if (end > w->text.ViewWidth) {
            w->text.XOffset = w->text.ViewWidth - end;
        } else {
            w->text.XOffset = 0;
        }
        moved = True;
    }
    return moved;
}

/*
 * Actually draw a range of text onto the widget
 */
static void DrawText(TextFieldWidget w, int start, int end, Boolean highlight)
{
    int x;
    GC gc;
    int textlen;

    if (!w->text.Echo) {
        return;
    }

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;

    if (textlen > 0) {
        if (start < 0) {
            return;
        } else if (end < start) {
            int temp;

            temp = start;
            start = end;
            end = temp;
        }
        if (end <= textlen) {
            if (highlight) {
                gc = w->text.highlightGC;
            } else {
                gc = w->text.drawGC;
            }
            x = w->text.Margin + w->text.XOffset + TextWidthFromLeft(w, start);
            if (w->text.international) {
                XwcDrawImageString(XtDisplay(w), XtWindow(w), w->text.fontSet,
                    gc, x, w->text.YOffset, &w->text.IntlText[start], end - start);
            } else {
                XDrawImageString(XtDisplay(w), XtWindow(w),
                    gc, x, w->text.YOffset, &w->text.Text[start], end - start);
            }
        }
    }
}

static void DrawTextRange(TextFieldWidget w, int start, int end)
{
    if (!w->text.Echo) {
        return;
    }

    if (w->text.TextLen > 0) {
        if (start < 0) {
            return;
        } else if (end < start) {
            int temp;

            temp = start;
            start = end;
            end = temp;
        }

        if (w->text.HighlightStart < 0 || start >= w->text.HighlightEnd || end <= w->text.HighlightStart) {
            DrawText(w, start, end, False);
        } else {
            int clip;

            while (start < end) {
                if (start < w->text.HighlightStart) {
                    if (end <= w->text.HighlightStart) {
                        clip = end;
                    } else {
                        clip = w->text.HighlightStart;
                    }
                    DrawText(w, start, clip, False);
                    start = clip;
                } else if (start < w->text.HighlightEnd) {
                    if (end <= w->text.HighlightEnd) {
                        clip = end;
                    } else {
                        clip = w->text.HighlightEnd;
                    }
                    DrawText(w, start, clip, True);
                    start = clip;
                } else {
                    DrawText(w, start, end, False);
                    start = end;
                }
            }
        }
    }
}

static void DrawTextReposition(TextFieldWidget w)
{
    int xsrc, xdest, width, start, end;

    if (!w->text.Echo) {
        return;
    }

    if (w->text.XOffset < w->text.OldXOffset) {
        xsrc = w->text.OldXOffset - w->text.XOffset;
        xdest = 0;
        width = w->text.ViewWidth - xsrc + 1;

        /* Need to redraw some characters at the end. */

        end = TextPixelToPos(w, w->text.Margin + w->text.ViewWidth);
        start = TextPixelToPos(w, w->text.Margin + w->text.ViewWidth - xsrc);
    } else if (w->text.XOffset > w->text.OldXOffset) {
        xsrc = 0;
        xdest = w->text.XOffset - w->text.OldXOffset;
        width = w->text.ViewWidth - xdest + 1;

        /* Need to redraw some characters at the beginning. */

        start = TextPixelToPos(w, w->text.Margin);
        end = TextPixelToPos(w, w->text.Margin + xdest);
    } else {
        return;
    }

    if (width > 0) {
#ifdef DEBUG_TF
        printf("Reposition: xoff=%d old=%d src=%d dest=%d width=%d refresh %d-%d\n", w->text.XOffset, w->text.OldXOffset, xsrc, xdest, width, start, end);
#endif
        XCopyArea(XtDisplay(w), XtWindow(w), XtWindow(w), w->text.drawGC, w->text.Margin + xsrc, 0, (unsigned int)width, (unsigned int)w->core.height, w->text.Margin + xdest, 0);
        DrawTextRange(w, start, end);
    }
    w->text.OldXOffset = w->text.XOffset;
}

static void DrawTextWithCopyArea(TextFieldWidget w)
{
    int x, insert_width;
    int xsrc, xdest, width;

    if (!w->text.Echo) {
        return;
    }

    x = w->text.XOffset;
    insert_width = TextWidthFromPos(w, w->text.FastInsertCursorStart, w->text.FastInsertTextLen);
    if (PositionCursor(w)) {
        /*
         *  if the text is scrolled, then:
         * 1.  the cursor is at the end
         * 2.  the copy will move to the left.
         */
        xsrc = 0;
        width = w->text.OldCursorX + x;
        xdest = w->text.ViewWidth - (x + w->text.OldCursorX) - insert_width;
        XCopyArea(XtDisplay(w), XtWindow(w), XtWindow(w), w->text.drawGC, w->text.Margin + xsrc, 0, (unsigned int)width, (unsigned int)w->core.height, w->text.Margin + xdest, 0);
#ifdef DEBUG_TF
        printf("DrawInsert: x=%d xsrc=%d xdest=%d width=%d\n", x, xsrc, xdest, width);
#endif
    } else {

        /*
         * the text hasn't been scrolled, so:
         * 1.  the text left of the cursor won't change
         * 2.  the stuff after the cursor will be moved right.
         */
        xsrc = TextWidthFromLeft(w, w->text.FastInsertCursorStart) + x;
        width = w->text.ViewWidth - xsrc;
        xdest = xsrc + insert_width;
        XCopyArea(XtDisplay(w), XtWindow(w), XtWindow(w), w->text.drawGC, w->text.Margin + xsrc, 0, (unsigned int)width, (unsigned int)w->core.height, w->text.Margin + xdest, 0);
#ifdef DEBUG_TF
        printf("DrawInsert: x=%d xsrc=%d xdest=%d width=%d\n", x, xsrc, xdest, width);
#endif
    }
    DrawTextRange(w, w->text.FastInsertCursorStart, w->text.FastInsertCursorStart + w->text.FastInsertTextLen);
    if (w->text.TextMaxLen > 0) {

        /*
         * This is pretty much a hack:
         * clear everything to end of window if this is a
         * fixed length TextField
         */
        xsrc = w->text.XOffset + w->text.TextWidth;
        width = w->text.ViewWidth - xsrc;
        XClearArea(XtDisplay(w), XtWindow(w), w->text.Margin + xsrc, 0, (unsigned int)width, w->core.height, False);
    } else if (w->text.TextWidth < w->text.OldTextWidth) {
        XClearArea(XtDisplay(w), XtWindow(w), w->text.Margin + w->text.XOffset + w->text.TextWidth, 0, w->text.OldTextWidth - w->text.TextWidth + 1, w->core.height, False);
    }
    w->text.OldTextWidth = w->text.TextWidth;
    w->text.OldXOffset = w->text.XOffset;
}

static void DrawAllText(TextFieldWidget w)
{
    int textlen;

    if (!w->text.Echo) {
        return;
    }

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;
    DrawTextRange(w, 0, textlen);
    if (w->text.TextWidth < w->text.OldTextWidth) {
        XClearArea(XtDisplay(w), XtWindow(w), w->text.Margin + w->text.XOffset + w->text.TextWidth, 0, w->text.OldTextWidth - w->text.TextWidth + 1, w->core.height, False);
    }
    w->text.OldTextWidth = w->text.TextWidth;
    w->text.OldXOffset = w->text.XOffset;
    w->text.OldHighlightStart = w->text.HighlightStart;
    w->text.OldHighlightEnd = w->text.HighlightEnd;
}

/* Draw an I-beam cursor */
static void DrawIBeamCursor(TextFieldWidget w, int x, GC gc)
{
    int ascent = w->text.FontAscent;
    int descent = w->text.FontDescent;

    XDrawLine(XtDisplay(w), XtWindow(w), gc, x, w->text.YOffset - ascent - 1, x, w->text.YOffset + descent + 1);
    XDrawLine(XtDisplay(w), XtWindow(w), gc, x - 2, w->text.YOffset - ascent - 1, x + 2, w->text.YOffset - ascent - 1);
    XDrawLine(XtDisplay(w), XtWindow(w), gc, x - 2, w->text.YOffset + descent + 1, x + 2, w->text.YOffset + descent + 1);
}

static void DrawCursor(TextFieldWidget w)
{
    int x;
    GC gc;

    if (w->text.DisplayCursor) {
        x = TextWidthFromLeft(w, w->text.CursorPos);
        w->text.OldCursorPos = w->text.CursorPos;
        w->text.OldCursorX = x;
        x += w->text.Margin + w->text.XOffset;

        gc = w->text.cursorGC;
        DrawIBeamCursor(w, x, gc);
    }
}

static void EraseCursor(TextFieldWidget w)
{
    int x;
    int textlen;

    if (w->text.DisplayCursor && w->text.OldCursorX >= 0) {
        x = w->text.OldCursorX + w->text.Margin + w->text.XOffset;
        DrawIBeamCursor(w, x, w->text.eraseGC);

        /* Little hack to fix up the character that might have been affected by
         * erasing the old cursor.
         */
        textlen = w->text.international ? w->text.IntlTextLen
                                        : w->text.TextLen;
        if (w->text.OldCursorPos < textlen) {
            DrawTextRange(w, w->text.OldCursorPos, w->text.OldCursorPos + 1);
        }
    }
}

static void ClearHighlight(TextFieldWidget w)
{
    if (!w->text.Echo) {
        return;
    }

    if (w->text.HighlightStart >= 0) {
        EraseCursor(w);
        DrawText(w, w->text.HighlightStart, w->text.HighlightEnd, False);
        DrawCursor(w);
        w->text.HighlightStart = w->text.HighlightEnd = -1;
    }
    w->text.OldHighlightStart = w->text.OldHighlightEnd = -1;
}

static void DrawHighlight(TextFieldWidget w)
{
    if (!w->text.Echo) {
        return;
    }

    if (w->text.OldHighlightStart < 0) {
        DrawText(w, w->text.HighlightStart, w->text.HighlightEnd, True);
    } else {
        DrawText(w, w->text.HighlightStart, w->text.OldHighlightStart, (w->text.HighlightStart < w->text.OldHighlightStart));
        DrawText(w, w->text.HighlightEnd, w->text.OldHighlightEnd, (w->text.HighlightEnd > w->text.OldHighlightEnd));
    }
    w->text.OldHighlightStart = w->text.HighlightStart;
    w->text.OldHighlightEnd = w->text.HighlightEnd;
}

/*
 * Special redraw function after a text insertion
 */
static void DrawInsert(TextFieldWidget w)
{
    /*  EraseCursor must be called before this */
    DrawTextWithCopyArea(w);
    DrawCursor(w);
}

/*
 * Redraw the entire widget, but don't scroll the window much
 */
static void Draw(TextFieldWidget w)
{
    EraseCursor(w);
    PositionCursor(w);
    DrawAllText(w);
    DrawCursor(w);
}

/*
 * Like Draw(), but has different rules about scrolling the window to
 * place the cursor in a good place
 */
static void MassiveChangeDraw(TextFieldWidget w)
{
    EraseCursor(w);
    MassiveCursorAdjust(w);
    DrawAllText(w);
    DrawCursor(w);
}

/*
 * Motif-like TextField public functions
 *
 * Note that this set of functions is only a subset of the functions available
 * in the real Motif XmTextField.
 *
 * When international == True:
 * positions are in terms of characters, not bytes.
 * strings are in the user's locale, which may mean UTF-8, or
 * another variable length encoding.
 */
Boolean TextFieldGetEditable(Widget aw)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!XtIsTextField(aw)) {
        return 0;
    }

    return w->text.Editable;
}

int TextFieldGetInsertionPosition(Widget aw)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!XtIsTextField(aw)) {
        return 0;
    }

    return w->text.CursorPos;
}

char *TextFieldGetString(Widget aw)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    char *ret;

    if (!XtIsTextField(aw)) {
        ret = XtMalloc(1);
        *ret = '\0';
        return ret;
    }

    ret = XtMalloc(w->text.TextLen + 1);
    strncpy(ret, w->text.Text, w->text.TextLen);
    ret[w->text.TextLen] = '\0';
    return ret;
}

void TextFieldInsert(Widget aw, int pos, char *str)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int len, textlen;

    if (!XtIsTextField(aw)) {
        return;
    }
    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;

    if (str && ((len = strlen(str)) > 0) && pos >= 0 && pos <= textlen) {
        w->text.HighlightStart = w->text.HighlightEnd = pos;
        TextInsert(w, str, len);
        MassiveChangeDraw(w);
        TextChanged(w);
    }
}

void TextFieldReplace(Widget aw, int first, int last, char *str)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int len;

    if (!XtIsTextField(aw)) {
        return;
    }

    if (str) {
        int textlen = w->text.international ? w->text.IntlTextLen
                                            : w->text.TextLen;
        len = strlen(str);
        if (last > textlen) {
            last = textlen;
        }
        if (first <= last) {
            w->text.HighlightStart = first;
            w->text.HighlightEnd = last;
            TextDeleteHighlighted(w);
            TextInsert(w, str, len);
            MassiveChangeDraw(w);
            TextChanged(w);
        }
    }
}

void TextFieldSetEditable(Widget aw, Boolean editable)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!XtIsTextField(aw)) {
        return;
    }

    w->text.Editable = editable;
}

void TextFieldSetInsertionPosition(Widget aw, int pos)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int textlen;

    if (!XtIsTextField(aw)) {
        return;
    }

    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;
    if (pos >= 0 && pos <= textlen) {
        w->text.CursorPos = pos;
        MassiveChangeDraw(w);
    }
}

/* ARGSUSED */
void TextFieldSetSelection(Widget aw, int start, int end, Time time)
{
    TextFieldWidget w = (TextFieldWidget)aw;
    int textlen;

    if (!XtIsTextField(aw)) {
        return;
    }

    if (end < start) {
        int temp;

        temp = start;
        start = end;
        end = temp;
    }
    if (start < 0) {
        start = 0;
    }
    textlen = w->text.international ? w->text.IntlTextLen
                                    : w->text.TextLen;
    if (end > textlen) {
        end = textlen;
    }
    w->text.HighlightStart = start;
    w->text.HighlightEnd = w->text.CursorPos = end;
    MassiveChangeDraw(w);
}

void TextFieldSetString(Widget aw, char *str)
{
    TextFieldWidget w = (TextFieldWidget)aw;

    if (!XtIsTextField(aw)) {
        return;
    }

    if (str) {
        int textlen = w->text.international ? w->text.IntlTextLen
                                            : w->text.TextLen;
        w->text.HighlightStart = 0;
        w->text.HighlightEnd = textlen;
        TextDeleteHighlighted(w);
        textlen = strlen(str);
        TextInsert(w, str, textlen);
        MassiveChangeDraw(w);
        TextChanged(w);
    }
}
