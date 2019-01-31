/*
 * about.c - Simple Xaw(3d)-based graphical user interface.  It uses widgets
 * from the Free Widget Foundation and Robert W. McMullen.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *
 * Support for multiple visuals and depths by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Intrinsic.h>

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Paned.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/Label.h>
#include <X11/Xaw3d/Command.h>
#else
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#endif

#include "info.h"
#include "lib.h"
#include "uiapi.h"
#include "util.h"
#include "version.h"
#include "vicefeatures.h"
#include "videoarch.h"
#include "vsync.h"

#include "uimenu.h"

#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif

static char *get_compiletime_features(void)
{
    feature_list_t *list;
    char *str, *lstr;
    unsigned int len = 0;

    list = vice_get_feature_list();
    while (list->symbol) {
        len += strlen(list->descr) + strlen(list->symbol) + (15);
        ++list;
    }
    str = lib_malloc(len);
    lstr = str;
    list = vice_get_feature_list();
    while (list->symbol) {
        sprintf(lstr, "%4s\t%s (%s)\n", list->isdefined ? "yes " : "no  ", list->descr, list->symbol);
        lstr += strlen(lstr);
        ++list;
    }
    return str;
}


static UI_CALLBACK(info_dialog_close_callback)
{
    *((ui_button_t *)UI_MENU_CB_PARAM) = 1;
}

/* ------------------------------------------------------------------------- */

static UI_CALLBACK(info_dialog_license_callback)
{
    ui_show_text(_("VICE is FREE software!"), info_license_text, -1, -1);
}

static UI_CALLBACK(info_dialog_no_warranty_callback)
{
    ui_show_text(_("No warranty!"), info_warranty_text, -1, -1);
}

static UI_CALLBACK(info_dialog_contrib_callback)
{
    ui_show_text(_("Contributors to the VICE project"), info_contrib_text, -1, -1);
}

static UI_CALLBACK(info_dialog_features_callback)
{
    char *features = NULL;

    features = get_compiletime_features();
    ui_show_text(_("Compile time features"), features, -1, -1);
    lib_free(features);
}

static char *textlist[100] = { NULL };

static Widget build_info_dialog(Widget parent, int *return_flag)
{
    Widget shell, pane, info_form, button_form, tmp, prevlabel = NULL;
    int i = 0;

    shell = ui_create_transient_shell(parent, "infoDialogShell");
    pane = XtVaCreateManagedWidget("infoDialog",
                                   panedWidgetClass, shell,
                                   NULL);
    info_form = XtVaCreateManagedWidget("textForm",
                                        formWidgetClass, pane,
                                        NULL);
    button_form = XtVaCreateManagedWidget("buttonBox",
                                          boxWidgetClass, pane,
                                          XtNshowGrip, False,
                                          XtNskipAdjust, True,
                                          XtNorientation, XtorientHorizontal,
                                          NULL);
    while (textlist[i]) {
        tmp = XtVaCreateManagedWidget("infoString",
                                      labelWidgetClass, info_form,
                                      XtNlabel, textlist[i],
                                      XtNjustify, XtJustifyCenter,
                                      XtNresize, False,
                                      XtNwidth, 220,
                                      NULL);
        if (prevlabel) {
            XtVaSetValues(tmp, XtNfromVert, prevlabel, NULL);
        }
        prevlabel = tmp;
        i++;
    }
    tmp = XtVaCreateManagedWidget("closeButton",
                                  commandWidgetClass, button_form,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, info_dialog_close_callback, (XtPointer)return_flag);
    tmp = XtVaCreateManagedWidget("licenseButton",
                                  commandWidgetClass, button_form,
                                  XtNfromHoriz, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, info_dialog_license_callback, NULL);
    tmp = XtVaCreateManagedWidget("noWarrantyButton",
                                  commandWidgetClass, button_form,
                                  XtNfromHoriz, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, info_dialog_no_warranty_callback, NULL);
    tmp = XtVaCreateManagedWidget("contribButton",
                                  commandWidgetClass, button_form,
                                  XtNfromHoriz, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, info_dialog_contrib_callback, NULL);
    tmp = XtVaCreateManagedWidget("featuresButton",
                                  commandWidgetClass, button_form,
                                  XtNfromHoriz, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, info_dialog_features_callback, NULL);
    return pane;
}

UI_CALLBACK(ui_about)
{
    static Widget info_dialog;
    static int is_closed;
    int i = 0;
    int j;

    if (!info_dialog) {
        textlist[i++] = lib_stralloc("");
        textlist[i++] = lib_stralloc("V I C E");
        textlist[i++] = lib_stralloc("");
#ifdef USE_SVN_REVISION
#ifdef USE_XAW3D
        textlist[i++] = lib_stralloc("Version " VERSION " rev " VICE_SVN_REV_STRING " (XAW3D)");
#else
        textlist[i++] = lib_stralloc("Version " VERSION " rev " VICE_SVN_REV_STRING " (XAW)");
#endif
#else
#ifdef USE_XAW3D
        textlist[i++] = lib_stralloc("Version " VERSION " (XAW3D)");
#else
        textlist[i++] = lib_stralloc("Version " VERSION " (XAW)");
#endif
#endif
#ifdef UNSTABLE
        textlist[i++] = lib_stralloc("(unstable)");
#endif
        textlist[i++] = lib_stralloc("");
        for (j = 0; core_team[j].name; j++) {
            textlist[i++] = util_concat("Copyright C ", core_team[j].years, " ", core_team[j].name, NULL);
        }
        textlist[i++] = lib_stralloc("");
        textlist[i++] = lib_stralloc("Official VICE homepage:");
        textlist[i++] = lib_stralloc("http://vice-emu.sourceforge.net/");
        textlist[i++] = lib_stralloc("");
        textlist[i] = NULL;

        info_dialog = build_info_dialog(_ui_top_level, &is_closed);
    }
    vsync_suspend_speed_eval();
    ui_popup(XtParent(info_dialog), _("VICE Information"), False);

    is_closed = 0;
    while (!is_closed) {
        ui_dispatch_next_event();
    }
    ui_popdown(XtParent(info_dialog));
}

void ui_about_shutdown(void)
{
    int i;

    if (textlist[0]) {
        for (i = 0; textlist[i]; i++) {
            lib_free(textlist[i]);
        }
        textlist[0] = NULL;
    }
}
