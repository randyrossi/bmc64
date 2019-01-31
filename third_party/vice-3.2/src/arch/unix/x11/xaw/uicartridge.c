/*
 * uicartridge.c - Cartridge save image dialog for the Xaw(3d) widget set.
 *
 * Written by
 *  Nathan Huizinga <nathan.huizinga@chess.nl>
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
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/MenuButton.h>
#include <X11/Xaw3d/Paned.h>
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/Toggle.h>
#else
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Toggle.h>
#endif

#ifndef ENABLE_TEXTFIELD
#ifdef USE_XAW3D
#include <X11/Xaw3d/AsciiText.h>
#else
#include <X11/Xaw/AsciiText.h>
#endif
#else
#include "widgets/TextField.h"
#endif

#include "uicartridge.h"

#include "cartridge.h"
#include "lib.h"
#include "machine.h"
#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uilib.h"
#include "util.h"

static Widget cartridge_dialog;
static Widget cartridge_dialog_pane;
static Widget file_name_form;
static Widget file_name_label;
static Widget file_name_field;
static Widget browse_button;
static Widget button_box;
static Widget save_button;
static Widget cancel_button;

static int save_type = 0;

#define FILL_BOX_WIDTH        10
#define OPTION_LABELS_WIDTH   50
#define OPTION_LABELS_JUSTIFY XtJustifyLeft

static UI_CALLBACK(browse_callback)
{
    ui_button_t button;
    uilib_file_filter_enum_t filter = UILIB_FILTER_CRT_CARTRIDGE;
    char *filename;

    filename = ui_select_file(_("Save cartridge image"), NULL, False, NULL, &filter, 1, &button, 0, NULL, UI_FC_SAVE);

    if (button == UI_BUTTON_OK) {
        XtVaSetValues(file_name_field, XtNstring, filename, NULL);
    }

    lib_free(filename);
}

static UI_CALLBACK(cancel_callback)
{
    ui_popdown(cartridge_dialog);
}

static UI_CALLBACK(save_callback)
{
    String name;
    
    ui_popdown(cartridge_dialog);

    XtVaGetValues(file_name_field, XtNstring, &name, NULL);
    
    if (cartridge_save_image (save_type, name) < 0) {
        ui_error(_("Cannot write cartridge image file\n`%s'\n"), name);
    }
}

static void build_cartridge_dialog(void)
{
    char *button_title;
    char *filename = util_concat(_("File name"), ":", NULL);

    if (cartridge_dialog != NULL) {
        return;
    }

    cartridge_dialog = ui_create_transient_shell(_ui_top_level, "cartridgeDialog");

    cartridge_dialog_pane = XtVaCreateManagedWidget("cartridgeDialogPane",
                                                    panedWidgetClass, cartridge_dialog,
                                                    NULL);

    file_name_form = XtVaCreateManagedWidget("fileNameForm",
                                             formWidgetClass, cartridge_dialog_pane,
                                             XtNshowGrip, False,
                                             NULL);

    file_name_label = XtVaCreateManagedWidget("fileNameLabel",
                                              labelWidgetClass, file_name_form,
                                              XtNjustify, XtJustifyLeft,
                                              XtNlabel, filename,
                                              XtNborderWidth, 0,
                                              NULL);
    lib_free(filename);

    file_name_field = XtVaCreateManagedWidget("fileNameField",
#ifndef ENABLE_TEXTFIELD
                                              asciiTextWidgetClass, file_name_form,
                                              XtNtype, XawAsciiString,
                                              XtNeditType, XawtextEdit,
#else
                                              textfieldWidgetClass, file_name_form,
                                              XtNstring, "",         /* Otherwise, it does not work correctly.  */
#endif
                                              XtNfromHoriz, file_name_label,
                                              XtNwidth, 200,
                                              NULL);

    button_title = util_concat(_("Browse"), "...", NULL);
    browse_button = XtVaCreateManagedWidget("browseButton",
                                            commandWidgetClass, file_name_form,
                                            XtNfromHoriz, file_name_field,
                                            XtNlabel, button_title,
                                            NULL);
    lib_free(button_title);
    XtAddCallback(browse_button, XtNcallback, browse_callback, NULL);

    button_box = XtVaCreateManagedWidget("buttonBox",
                                         boxWidgetClass, cartridge_dialog_pane,
                                         XtNshowGrip, False,
                                         NULL);

    save_button = XtVaCreateManagedWidget("saveButton",
                                          commandWidgetClass, button_box,
                                          XtNlabel, _("Save"),
                                          NULL);
    XtAddCallback(save_button, XtNcallback, save_callback, NULL);
    
    cancel_button = XtVaCreateManagedWidget("cancelButton",
                                            commandWidgetClass, button_box,
                                            XtNlabel, _("Cancel"),
                                            NULL);
    XtAddCallback(cancel_button, XtNcallback, cancel_callback, NULL);

    XtSetKeyboardFocus(cartridge_dialog_pane, file_name_field);
}

void ui_cartridge_save_dialog(int type)
{
    build_cartridge_dialog();
    save_type = type;
    ui_popup(cartridge_dialog, _("Save cartridge image"), True);
}
