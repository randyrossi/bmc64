#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <stdlib.h>

#include "TabString.h"

/*
 *	Converts a string list of tabs to an array of tabs
 */
int *XfwfTablist2Tabs(char *tablist)
{
    register int *tabs = NULL;
    register int ntabs = 0;

    if (!tablist) {
        return NULL;
    }
    for (;;) {
        /* Skip leading blanks */
        while (*tablist && *tablist == ' ') {
            ++tablist;
        }
        if (!*tablist) {
            break;
        }

        /* Allocate space for the new tab */
        if (ntabs) {
            tabs = (int *)XtRealloc((char *)tabs, (ntabs + 1) * sizeof(int));
        } else {
            tabs = (int *)XtMalloc((ntabs + 1) * sizeof(int));
        }
        /* Add it to the list */
        tabs[ntabs++] = atoi(tablist);

        /* Skip to the next blank */
        while (*tablist && *tablist != ' ') {
            ++tablist;
        }
    }
    return tabs;
}
