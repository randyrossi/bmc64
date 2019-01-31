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
 */

#ifndef VICE_TextFieldP_H
#define VICE_TextFieldP_H

#include <X11/Core.h>

#include "TextField.h"

#define TEXTFIELD_ALLOC_SIZE	256

/*
 * This should be from <XlcPublic.h> or <X11/Xlcint.h> or related headers,
 * but strangely enough those header files are not included in typical
 * installations.
 * This stuff is "documented" in i18n/Framework.PS. or
 * http://www.x.org/releases/X11R7.6/doc/libX11/specs/i18n/framework/framework.html
 */
typedef struct _XLCd *XLCd;
typedef struct _XlcConvRec *XlcConv;
extern XLCd _XlcCurrentLC (void);
#define XlcNWideChar            "wideChar"
#define XlcNMultiByte           "multiByte"
extern XlcConv _XlcOpenConverter(
    XLCd                from_lcd,
    const char*         from_type,
    XLCd                to_lcd,
    const char*         to_type
);

extern void _XlcCloseConverter(
    XlcConv             conv
);

extern int _XlcConvert(
    XlcConv             conv,
    XPointer*           from,
    int*                from_left,
    XPointer*           to,
    int*                to_left,
    XPointer*           args,
    int                 num_args
);

/* From lib/X11/lcStd.c aka libX11/dist/src/xlibi18n/lcStd.c */
extern int _Xmblen(char *str, int len);
#define _Xmblen(str, bytes)        _Xmbtowc((wchar_t *) NULL, str, bytes)
#define _Xmbtowc(wstr, str, bytes) _Xlcmbtowc((XLCd) NULL, wstr, str, bytes)
extern int _Xlcmbtowc  (XLCd lcd, wchar_t *wstr, char *str, int bytes);
extern int _Xlcmbstowcs(XLCd lcd, wchar_t *wstr, char *str, int bytes);
extern int _Xlcwcstombs(XLCd lcd, char *str, wchar_t *wstr, int len);

/* end of stuff that should have been in a distributed header file */

typedef struct {
    int dummy;			/* keep compiler happy with dummy field */
} TextFieldClassPart;

typedef struct _TextFieldClassRec {
    CoreClassPart core_class;
    TextFieldClassPart TextField_class;
} TextFieldClassRec;

extern TextFieldClassRec textfieldClassRec;

typedef struct {
    /* Public stuff ... */
    long foreground_pixel;	/* data storage for resources ... */
    long cursor_pixel;
    XFontStruct *font;
    XFontSet fontSet;
    Dimension Margin;
    int TextMaxLen;
    Boolean Echo;
    Boolean Editable;
    Boolean DisplayCursor;
    Boolean AllowSelection;
    Boolean PendingDelete;
    Boolean international;
    char *DefaultString;
    XtCallbackList ActivateCallback;
    XtCallbackList TextChangeCallback;

    /* Private stuff ... */
    GC drawGC;			/* GC for drawing and copying */
    GC highlightGC;		/* GC for highlighting text */
    GC cursorGC;		/* GC for cursor (not clipped like drawGC) */
    GC dashGC;			/* GC for cursor when we don't have focus */
    GC eraseGC;			/* GC for erasing (not clipped) */

    int CursorPos;		/* text position of cursor */
    int OldCursorPos;		/* previous position */
    int OldCursorX;		/* previous pixel pos of cursor */
    int HighlightStart;		/* text pos of leftmost highlight pos */
    int HighlightEnd;		/* text pos of rightmost highlight pos */
    int HighlightPivotStart;	/* left pivot pos for ExtendHighlight */
    int HighlightPivotEnd;	/* right ... */
    int OldHighlightStart;	/* save data */
    int OldHighlightEnd;

    char *Text;                 /* pointer to the text */
    int TextAlloc;              /* number of bytes allocated for the text */
    int TextLen;                /* current length of text (in characters) */
    wchar_t *IntlText;          /* wc version of text */
    int IntlTextLen;            /* length in wide characters */

    /* type pun fix */
    union {
        char *t_char;           /* pointer to text selection, when needed */
        wchar_t *t_wchar_t;
    } SelectionText;

    int SelectionLen;           /* length */

    int FastInsertCursorStart;  /* data storage for some text optimization */
    int FastInsertTextLen;

    Dimension ViewWidth;        /* visible width of widget */
    int XOffset;                /* offset from x=0 to start of text string */
    int OldXOffset;
    int FontAscent;
    int FontDescent;
    int FontHeight;
    int YOffset;                /* y pixel offset to baseline of font */
    int TextWidth;              /* char width of text */
    int OldTextWidth;

    XtIntervalId timer_id;	/* timer for double click test */
    int timer_x;			/* save event x pos */
    int highlight_time;		/* time delay for scrolling */
    int multi_click_time;       /* local storage for XtGetMultiClickTime */
    XIM xim;                    /* X Input Method */
    XIC xic;                    /* X Input Context */
    Atom selection_type;        /* Requested type for paste into text */
    long selection_time;
} TextFieldPart;

typedef struct _TextFieldRec {
    CorePart core;
    TextFieldPart text;
} TextFieldRec;


#endif /* _TextFieldP_H */
