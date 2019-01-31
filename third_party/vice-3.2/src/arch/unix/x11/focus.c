/*
 * focus.c - Focusing terminal window
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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
#include "focus.h"

#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include "lib.h"
#include "x11ui.h"

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/*
 * Wait for the window to de-iconify, and optionally handle events in the mean
 * time, so that it is safe to call XSetInputFocus() on it.
 *
 * Use another approach than the Gnome UI: looking at a window property
 * depends on the window manager maintaining it, so just ask X.
 * When the window is in another workspace, a window is typically not mapped
 * (due to the way window managers typically implement workspaces), and it will
 * never get mapped.
 *
 * Therefore, if the window is not our own, loop a limited number of times with
 * a small delay. As long as the delay isn't ridiculously long, it doesn't
 * matter, since the user doesn't see the target window anyway.
 *
 * If the window is our own, we can wait indefinitely, since the user
 * interacting with the window will keep generating new events, one of which
 * must be the result of the de-iconification.
 * This needs to have MapNotify (StructureNotifyMask) events selected
 * or there is no sensible event to wait for.
 */

static int wait_for_deiconify(Display *display, Window w)
{
    int loop = 0;

    for (;;) {
        XWindowAttributes wa;

        XGetWindowAttributes(display, w, &wa);
        if (wa.map_state == IsUnviewable) {
            DBG(("wait_for_deiconify: IsUnviewable, %d loops", loop));
            return 0;
        }
        if (wa.map_state == IsViewable) {
            DBG(("wait_for_deiconify: IsViewable, %d loops", loop));
            return 1;
        }
        if (loop > 10) {
            DBG(("wait_for_deiconify: IsUnmapped, %d loops", loop));
            return 0;
        }
        usleep(30 * 1000);
        loop++;
    }
}

/* ------------------------------------------------------------------------- */

/*
    transfer focus to the monitor ui window

    note: the respective code in gnome/x11ui.c can probably be shared between
          xaw and gtk (it is plain X11 code anyway).
    This version has been generalised somewhat compared to the gtk version
    though.
*/

#if defined(sun) || defined(__sun)
#  if defined(__SVR4) || defined(__svr4__)
#    define IS_SOL
#  endif
#endif

/* TODO: put this properly in configure */
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFlyBSD__) || defined(IS_SOL)
# define PROCFS_STATUS          "status"
# define PROCFS_STATUS_PPID     3               /* 3rd field */
#else
# define PROCFS_STATUS          "stat"          /* 4th field */
# define PROCFS_STATUS_PPID     4
#endif

#define PROCSTATLEN     0x200
static pid_t get_ppid_from_pid(pid_t pid) 
{
    pid_t ppid = 0;
    FILE *f;
    char *p;
    char pscmd[0x40];
    char status[PROCSTATLEN + 1];
    int ret;
    char *saveptr;
    int i;

    sprintf(pscmd, "/proc/%d/"PROCFS_STATUS, (int)pid);

    f = fopen(pscmd, "r");
    if (f == NULL) {
        return 0;
    }
    memset(status, 0, PROCSTATLEN + 1);
    ret = fread(status, 1, PROCSTATLEN, f);
    fclose(f);
    if (ret < 1) {
        return 0;
    }

    /* Get the PROCFS_STATUS_PPID'th field */
    p = strtok_r(status, " ", &saveptr);
    for (i = 1; i < PROCFS_STATUS_PPID; i++) {
        p = strtok_r(NULL, " ", &saveptr);
    }

    if (p) {
        ppid = strtoul(p, NULL, 10);
        return ppid;
    }
    return 0;
}

/* check if winpid is an ancestor of pid, returns distance if found or 0 if not */
#define NUM_PARENT_PIDS         20
static int num_parent_pids;
static pid_t parent_pids[NUM_PARENT_PIDS];

static void initialize_parent_pids(pid_t pid)
{
    pid_t ppid;
    int i;

    if (num_parent_pids > 0) {
        return;
    }

    for (i = 0; i < NUM_PARENT_PIDS && pid > 1; i++) {
        ppid = get_ppid_from_pid(pid);
        DBG(("initialize_parent_pids: [%d] = %ld", i, (long)ppid));
        parent_pids[i] = ppid;
        pid = ppid;
    }
    num_parent_pids = i;
}

static int check_ancestor(pid_t winpid)
{
    int i;
    pid_t pid = winpid;

    for (i = 0; i < num_parent_pids && pid > 1; i++) {
        pid = parent_pids[i];
        if (pid == winpid) {
            return i;
        }
    }

    return 0;
}

#include <X11/Xatom.h>
/* get list of client windows for given display */
static Window *getwinlist(Display *disp, log_t ui_log, unsigned long *len) 
{
    Atom type;
    Atom net_client_list = XInternAtom(disp, "_NET_CLIENT_LIST", False);
    int form;
    unsigned long remain;
    unsigned char *list;

    if (XGetWindowProperty(disp, XDefaultRootWindow(disp), net_client_list,
                0, 1024, False, AnyPropertyType, &type, &form,
                len, &remain, &list) != Success) {
        log_error(ui_log, "getwinlist: XGetWindowProperty");
        return 0;
    }
    if (*len == 0) {
        Atom win_client_list = XInternAtom(disp, "_WIN_CLIENT_LIST", False);

        if (XGetWindowProperty(disp, XDefaultRootWindow(disp), win_client_list,
                    0, 1024, False, AnyPropertyType, &type, &form,
                    len, &remain, &list) != Success) {
            log_error(ui_log, "getwinlist: XGetWindowProperty");
            return NULL;
        }
    }

    if (type == XA_WINDOW || type == XA_CARDINAL) {
        return (Window *)list;
    }

    XFree(list);

    return NULL;
}

int getprop_failed = 0;

static int getprop_handler(Display *display, XErrorEvent *err)
{
    getprop_failed = 1;

    return 0;
}

/*
 * Get the pid associated with a given window.
 * Since it might have gone away by now (race condition!)
 * we trap errors.
 * This is actually not even very unlikely in case the user just clicked
 * "monitor" in the "JAM" pop-up.
 */
static pid_t getwinpid(Display *disp, Window win)
{
    Atom prop = XInternAtom(disp, "_NET_WM_PID", False), type;
    int form;
    unsigned long remain, len;
    unsigned char *pid_p = NULL;
    pid_t pid;
    int (*olderrorhandler)(Display *, XErrorEvent *);

    getprop_failed = 0;
    olderrorhandler = XSetErrorHandler(getprop_handler);
    if (XGetWindowProperty(disp, win, prop, 0, 1024, False, XA_CARDINAL,
        &type, &form, &len, &remain, &pid_p) != Success || len < 1 ||
            getprop_failed) {
        /* log_error(ui_log, "getwinpid: XGetWindowProperty; win=%lx, len=%ld", (long)win, len); */
        XSetErrorHandler(olderrorhandler);
        return 0;
    }
    XSetErrorHandler(olderrorhandler);

    pid = *(pid_t *)pid_p;
    XFree(pid_p);
    return pid;
}

static char *getwinname(Display *disp, Window win)
{
    Atom prop = XInternAtom(disp, "WM_NAME", False), type;
    int form;
    unsigned long remain, len;
    unsigned char *name_p = NULL;
    char *name;
    int (*olderrorhandler)(Display *, XErrorEvent *);

    getprop_failed = 0;
    olderrorhandler = XSetErrorHandler(getprop_handler);
    if (XGetWindowProperty(disp, win, prop, 0, 1024, False, XA_STRING,
        &type, &form, &len, &remain, &name_p) != Success || len < 1 ||
            getprop_failed) {
        /* log_error(ui_log, "getwinpid: XGetWindowProperty; win=%lx, len=%ld", (long)win, len); */
        XSetErrorHandler(olderrorhandler);
        return NULL;
    }
    XSetErrorHandler(olderrorhandler);

    len *= form / 8;
    name = (char *)lib_malloc(len + 1);
    memcpy(name, name_p, len);
    name[len] = '\0';
    XFree(name_p);
    return name;
}

void ui_focus_window(Display *display, Window foundwin)
{
    XEvent xev;

    memset(&xev, 0, sizeof(xev));
    xev.xclient.type = ClientMessage;
    xev.xclient.send_event = True;
    xev.xclient.window = foundwin;
    xev.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
    xev.xclient.format = 32;
    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

    XMapRaised(display, foundwin);
    XSync(display, False);

    /* The window manager may ignore the request to raise the window (for
       example because it is in a different workspace). We have to check if
       the window is actually visible, because a call to XSetInputFocus()
       will crash if it is not.
    */
    if (wait_for_deiconify(display, foundwin)) {
        XSetInputFocus(display, foundwin, RevertToParent, CurrentTime);
        XSync(display, False);
    }
}

int ui_focus_terminal(Display *display, log_t ui_log)
{
    int i;
    unsigned long len;
    Window *list;
    Window foundwin;
    pid_t winpid, mypid;
    int num, maxnum;
    char *title;
    const char *windowenv;

    DBG(("uimon_focus_terminal"));

    windowenv = getenv("WINDOWID");

    if (windowenv) {
        foundwin = atol(windowenv);

        if (foundwin) {
            DBG(("using win: %lx\n", (long)foundwin));
            ui_focus_window(display, foundwin);
            return 0;
        }
    }

    mypid = getpid();
    maxnum = INT_MAX;
    foundwin = 0;

    /* get a list of our parent process ids */
    initialize_parent_pids(mypid);

    /* get list of all client windows on current display */
    list = (Window*)getwinlist(display, ui_log, &len);
    DBG(("getwinlist: %ld windows\n", len));

    /* title to look for */
    title = lib_msprintf("VICE monitor console (%d)", (int)mypid);

    /* for every window, check if it the title is the right one. */
    if (title) {
        for (i = 0; i < (int)len; i++) {
            char *name = getwinname(display, list[i]);
            if (name == NULL) {
                continue;
            }
            if (!strcmp(title, name)) {
                foundwin = list[i];
            }
            lib_free(name);
        }
        lib_free(title);
    }

    /* for every window, check if it is an ancestor of the current process. the
       one which is the closest ancestor will be the one we are interested in */
    if (!foundwin) {
        for (i = 0; i < (int)len; i++) {
            winpid = getwinpid(display, list[i]);
            num = check_ancestor(winpid);
            if (num > 0) {
                DBG(("found: n:%d win:%lx pid:%ld", num, (long)list[i], (long)winpid));
                if ((num < maxnum) ||
                    /*
                     * Skip hidden Gnome client leader windows; they have the
                     * PID set on them anyway.
                     */
                    (num == maxnum && list[i] > foundwin)) {
                    maxnum = num;
                    foundwin = list[i];
                }
            }
        }
    }

    XFree(list);

    /* if a matching window was found, raise it and transfer focus to it */
    if (foundwin) {
        DBG(("using win: %lx\n", (long)foundwin));
        ui_focus_window(display, foundwin);
    }

    return 0;
}
