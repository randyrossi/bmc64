#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "TabString.h"

/*
 *	Like DrawImageString, except it takes an additional  "tabs"
 *	argument, used to specify what horizontal pixel position to
 *	move to when tab characters are present in the string.  If
 *	the "tabs" argument is NULL, works exactly like its
 *	counterpart.
 */
void XfwfDrawImageString(Display *display, Drawable drawable, GC gc, int x, int y, String string, int length, int *tabs)
{
    register char *p, *ep;
    register int tx, tab;

    tab = tx = 0;
    for (p = string; length;) {
        ep = strnchr(p, '\t', length);
        if (ep && tabs) {
            XDrawImageString(display, drawable, gc, x + tx, y, p, ep - p);
            tx = tabs[tab++];
            length -= ep - p + 1;
            p = ep + 1;
        } else {
            XDrawImageString(display, drawable, gc, x + tx, y, p, length);
            break;
        }
    }
}
