#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "TabString.h"

/*
 *	Like DrawString, except it takes an additional  "tabs"
 *	argument, used to specify what horizontal pixel position to
 *	move to when tab characters are present in the string.  If
 *	the "tabs" argument is NULL, works exactly like its
 *	counterpart.
 */
void XmbfwfDrawString(Display *display, Drawable drawable, XFontSet fs, GC gc, int x, int y, String string, int length, int *tabs)
{
    register char *p, *ep;
    register int tx, tab;

    tab = tx = 0;
    for (p = string; length;) {
        ep = strnchr(p, '\t', length);
        if (ep && tabs) {
            XmbDrawString(display, drawable, fs, gc, x + tx, y, p, ep - p);
            tx = tabs[tab++];
            length -= ep - p + 1;
            p = ep + 1;
        } else {
            XmbDrawString(display, drawable, fs, gc, x + tx, y, p, length);
            break;
        }
    }
}

void XfwfDrawString(Display *display, Drawable drawable, GC gc, int x, int y, String string, int length, int *tabs)
{
    register char *p, *ep;
    register int tx, tab;

    tab = tx = 0;
    for (p = string; length;) {
        ep = strnchr(p, '\t', length);
        if (ep && tabs) {
            XDrawString(display, drawable, gc, x + tx, y, p, ep - p);
            tx = tabs[tab++];
            length -= ep - p + 1;
            p = ep + 1;
        } else {
            XDrawString(display, drawable, gc, x + tx, y, p, length);
            break;
        }
    }
}
