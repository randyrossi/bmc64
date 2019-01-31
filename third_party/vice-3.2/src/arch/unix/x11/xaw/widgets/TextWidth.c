#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include "TabString.h"

/*
 *	Like TextWidth, except it takes an additional  "tabs"
 *	argument, used to specify what horizontal pixel position to
 *	move to when tab characters are present in the string.  If
 *	the "tabs" argument is NULL, works exactly like its
 *	counterpart.
 */
int XfwfTextWidth(XFontStruct *font, String str, int length, int *tabs)
{
    register char *p, *ep;
    register int tx, tab, rc;

    tab = tx = 0;
    if (length == 0) {
        return 0;
    }
    for (p = str; length;) {
        ep = strnchr(p, '\t', length);
        if (ep && tabs) {
            tx = tabs[tab++];
            length -= ep - p + 1;
            p = ep + 1;
        } else {
            rc = XTextWidth(font, p, length);
            if (rc < 0) {
                return rc;
            } else {
                return rc + tx;
            }
        }
    }
    return 0;
}
