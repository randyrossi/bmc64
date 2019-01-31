/*
 * uiedit.c - "Edit" submenu (copy / paste) for Xaw
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
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

#include <stdio.h>
#include <string.h>

#include "charset.h"
#include "clipboard.h"
#include "ui.h"
#include "uiarch.h"
#include "uiapi.h"
#include "uilib.h"
#include "uimenu.h"
#include "lib.h"
#include "kbdbuf.h"

/* Generic X11 code */

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>

#include <X11/Xmu/Atoms.h>
#include <X11/Xmu/StdSel.h>

String selection_type = "CLIPBOARD";
Atom selection;
extern Widget _ui_top_level;
char *selection_text;
static void paste_callback(const char *text, int size);

static void SelectionCallback(
    Widget w,
    XtPointer client_data,
    Atom *selection,
    Atom *type,
    XtPointer value,
    unsigned long *length,
    int *format)
{
    paste_callback(value, *length);

    XtFree(value);
}


static Boolean ConvertSelection(
    Widget w,
    Atom *selection,
    Atom *target,
    Atom *type,
    XtPointer *value,
    unsigned long *length,
    int *format)
{
    Display* d = XtDisplay(w);
    XSelectionRequestEvent* req =
        XtGetSelectionRequest(w, *selection, (XtRequestId)NULL);

    if (*target == XA_STRING || *target == XA_TEXT(d)) {
        int buflen = strlen(selection_text);

        *type = XA_STRING;
        *value = XtMalloc((Cardinal) buflen + 1);
        memcpy((char *) *value, selection_text, buflen);
        *length = buflen;
        *format = 8;
        return True;
    }

    if (XmuConvertStandardSelection(w, req->time, selection, target, type,
                                    (XPointer *)value, length, format))
        return True;

    return False;
}

static void LoseSelection(Widget w, Atom *selection)
{
    lib_free(selection_text);
    selection_text = NULL;
}


/* ARGSUSED */
static void GetSelection(
    Widget w,
    XtPointer closure,          /* unused */
    XtPointer callData)         /* unused */
{
    XmuInternStrings(XtDisplay(_ui_top_level), &selection_type, 1, &selection);
    XtGetSelectionValue(w, selection, XA_STRING,
                        SelectionCallback, NULL,
                        XtLastTimestampProcessed(XtDisplay(w)));
}


/* ARGSUSED */
static void SetSelection(
    Widget w,
    XtPointer closure,
    XtPointer callData)         /* unused */
{
    XmuInternStrings(XtDisplay(_ui_top_level), &selection_type, 1, &selection);
    XtOwnSelection(w, selection,
                   XtLastTimestampProcessed(XtDisplay(w)),
                   ConvertSelection, LoseSelection, NULL);
}

/* VICE code */

static UI_CALLBACK(ui_edit_command_copy)
{
    if (!CHECK_MENUS) {
        lib_free(selection_text);
        selection_text = clipboard_read_screen_output("\n");
        if (selection_text != NULL) {
            /* gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), text, strlen(text)); */
            SetSelection(_ui_top_level, NULL, NULL);
        }
    }
}

static void paste_callback(const char *text, int size)
{
    char *text_in_petscii;
    if (text == NULL) {
        return;
    }
    text_in_petscii = lib_malloc(size+1);
    memcpy(text_in_petscii, text, size);
    text_in_petscii[size] = '\0';

    if (text_in_petscii) {
        charset_petconvstring((unsigned char*)text_in_petscii, 0);
        kbdbuf_feed(text_in_petscii);
        lib_free(text_in_petscii);
    }
}

static UI_CALLBACK(ui_edit_command_paste)
{
    if (!CHECK_MENUS) {
        GetSelection(_ui_top_level, NULL, NULL);
    }
}

ui_menu_entry_t ui_edit_commands_submenu[] = {
    { N_("Copy"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_edit_command_copy, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Paste"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_edit_command_paste, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_edit_commands_menu[] = {
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
