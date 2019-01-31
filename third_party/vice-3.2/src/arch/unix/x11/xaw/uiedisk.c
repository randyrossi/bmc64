/*
 * uiedisk.c - emptydisk dialog for the Xaw(3d) widget set.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * focus fix by
 *  Ingo D. Rullhusen <d01c@uni-bremen.de>
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

#include "diskimage.h"
#include "lib.h"
#include "machine.h"
#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uiedisk.h"
#include "uilib.h"
#include "util.h"
#include "vdrive-internal.h"

static Widget emptydisk_dialog;
static Widget emptydisk_dialog_pane;
static Widget file_name_form;
static Widget file_name_label;
static Widget file_name_field;
static Widget image_name_form;
static Widget image_name_label;
static Widget image_name_field;
static Widget browse_button;
static Widget options_form;

static Widget disk_type_d64_button, disk_type_d67_button;
static Widget disk_type_d71_button;
static Widget disk_type_d81_button, disk_type_d80_button, disk_type_d82_button;
static Widget disk_type_d1m_button, disk_type_d2m_button, disk_type_d4m_button;
static Widget disk_type_g64_button;
static Widget disk_type_g71_button;
static Widget disk_type_p64_button;
static Widget disk_type_x64_button;
static Widget disk_type_label;

static Widget button_box;
static Widget save_button;
static Widget cancel_button;

static char *edisk_file_name;

#define FILL_BOX_WIDTH        10
#define OPTION_LABELS_WIDTH   100
#define OPTION_LABELS_JUSTIFY XtJustifyLeft

static UI_CALLBACK(browse_callback)
{
    ui_button_t button;
    char *filename;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    filename = ui_select_file(_("Save empty disk file"), NULL, False, NULL, &filter, 1, &button, 0, NULL, UI_FC_SAVE);

    if (button == UI_BUTTON_OK) {
        XtVaSetValues(file_name_field, XtNstring, filename, NULL);
    }

    lib_free(filename);
}

static UI_CALLBACK(cancel_callback)
{
    ui_popdown(emptydisk_dialog);
}

#define NR_FORMATS 13

static char *extensions[NR_FORMATS] = { "d64", "d67", "d71", "d81", "d80", "d82", "d1m", "d2m", "d4m", "g64", "g71", "p64", "x64" };

static UI_CALLBACK(save_callback)
{
    int dtypes[] = {
         DISK_IMAGE_TYPE_D64,
         DISK_IMAGE_TYPE_D67,
         DISK_IMAGE_TYPE_D71,
         DISK_IMAGE_TYPE_D81,
         DISK_IMAGE_TYPE_D80,
         DISK_IMAGE_TYPE_D82,
         DISK_IMAGE_TYPE_D1M,
         DISK_IMAGE_TYPE_D2M,
         DISK_IMAGE_TYPE_D4M,
         DISK_IMAGE_TYPE_G64,
         DISK_IMAGE_TYPE_G71,
         DISK_IMAGE_TYPE_P64,
         DISK_IMAGE_TYPE_X64,
    };

    char *filename;
    String name;
    String iname;
    int type_cnt;
    Boolean disk_type_flag;

    ui_popdown(emptydisk_dialog);

    /* XXX: WTF is this? (BW) */
    type_cnt = 0;
    XtVaGetValues(disk_type_d64_button, XtNstate, &disk_type_flag, NULL);
    if (disk_type_flag == False) {
        type_cnt ++;
        XtVaGetValues(disk_type_d67_button, XtNstate, &disk_type_flag, NULL);
        if (disk_type_flag == False) {
            type_cnt ++;
            XtVaGetValues(disk_type_d71_button, XtNstate, &disk_type_flag, NULL);
            if (disk_type_flag == False) {
                type_cnt ++;
                XtVaGetValues(disk_type_d81_button, XtNstate, &disk_type_flag, NULL);
                if (disk_type_flag == False) {
                    type_cnt ++;
                    XtVaGetValues(disk_type_d80_button, XtNstate, &disk_type_flag, NULL);
                    if (disk_type_flag == False) {
                        type_cnt ++;
                        XtVaGetValues(disk_type_d82_button, XtNstate, &disk_type_flag, NULL);
                        if (disk_type_flag == False) {
                            type_cnt ++;
                            XtVaGetValues(disk_type_d1m_button, XtNstate, &disk_type_flag, NULL);
                            if (disk_type_flag == False) {
                                type_cnt ++;
                                XtVaGetValues(disk_type_d2m_button, XtNstate, &disk_type_flag, NULL);
                                if (disk_type_flag == False) {
                                    type_cnt ++;
                                    XtVaGetValues(disk_type_d4m_button, XtNstate, &disk_type_flag, NULL);
                                    if (disk_type_flag == False) {
                                        type_cnt++;
                                        XtVaGetValues(disk_type_g64_button, XtNstate, &disk_type_flag, NULL);
                                        if (disk_type_flag == False) {
                                            type_cnt++;
                                            XtVaGetValues(disk_type_g71_button, XtNstate, &disk_type_flag, NULL);

                                            if (disk_type_flag == False) {
                                                type_cnt ++;
                                                XtVaGetValues(disk_type_p64_button, XtNstate, &disk_type_flag, NULL);
                                                if (disk_type_flag == False) {
                                                    type_cnt ++;
                                                    XtVaGetValues(disk_type_x64_button, XtNstate, &disk_type_flag, NULL);
                                                    if (disk_type_flag == False) {
                                                         type_cnt ++;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (type_cnt < 0 || type_cnt >= NR_FORMATS) {
        return;
    }

    XtVaGetValues(file_name_field, XtNstring, &name, NULL);
    XtVaGetValues(image_name_field, XtNstring, &iname, NULL);

    filename = lib_stralloc(name);
    util_add_extension(&filename, extensions[type_cnt]);

    if (vdrive_internal_create_format_disk_image(filename, iname, dtypes[type_cnt]) < 0) {
        ui_error(_("Couldn't create disk image"));
    } else {
        strcpy(edisk_file_name, filename);
    }

    lib_free(filename);
}

static void ui_focus_to(XtPointer XtP)
{
    XtSetKeyboardFocus(emptydisk_dialog_pane, *(Widget*)XtP);
}

static void build_emptydisk_dialog(void)
{
#ifndef ENABLE_TEXTFIELD
    static char *text_box_translations = "#override\n<Key>Return: no-op()";
#else
    static char *text_box_translations = "<Btn1Down>: select-start() focus-in()";
#endif

    char *button_title;
    char *filename = util_concat(_("File name"), ":", NULL);

    if (emptydisk_dialog != NULL) {
        return;
    }

    emptydisk_dialog = ui_create_transient_shell(_ui_top_level, "emptydiskDialog");

    emptydisk_dialog_pane = XtVaCreateManagedWidget("emptydiskDialogPane",
                                                    panedWidgetClass, emptydisk_dialog,
                                                    NULL);

    file_name_form = XtVaCreateManagedWidget("fileNameForm",
                                             formWidgetClass, emptydisk_dialog_pane,
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
#else
                                              textfieldWidgetClass, file_name_form,
                                              XtNstring, "",         /* Otherwise, it does not work correctly.  */
#endif
                                              XtNwidth, 240,
                                              XtNfromHoriz, file_name_label,
                                              NULL);
    XtOverrideTranslations(file_name_field, XtParseTranslationTable(text_box_translations));

    button_title = util_concat(_("Browse"), "...", NULL);
    browse_button = XtVaCreateManagedWidget("browseButton",
                                            commandWidgetClass, file_name_form,
                                            XtNfromHoriz, file_name_field,
                                            XtNlabel, button_title,
                                            NULL);
    lib_free(button_title);
    XtAddCallback(browse_button, XtNcallback, browse_callback, NULL);

    image_name_form = XtVaCreateManagedWidget("imageNameForm",
                                              formWidgetClass, emptydisk_dialog_pane,
                                              XtNfromVert, file_name_form,
                                              XtNshowGrip, False, NULL);

    image_name_label = XtVaCreateManagedWidget("imageNameLabel",
                                               labelWidgetClass, image_name_form,
                                               XtNjustify, XtJustifyLeft,
                                               XtNlabel, _("Disk name:"),
                                               XtNborderWidth, 0,
                                               NULL);

    image_name_field = XtVaCreateManagedWidget("imageNameField",
#ifndef ENABLE_TEXTFIELD
                                               asciiTextWidgetClass, image_name_form,
                                               XtNtype, XawAsciiString,
                                               XtNeditType, XawtextEdit,
#else
                                               textfieldWidgetClass, image_name_form,
                                               XtNstring, "",         /* Otherwise, it does not work correctly.  */
#endif
                                               XtNfromHoriz, image_name_label,
                                               XtNwidth, 240,
                                               NULL);
    XtOverrideTranslations(image_name_field, XtParseTranslationTable(text_box_translations));

    options_form = XtVaCreateManagedWidget("optionsForm",
                                           formWidgetClass, emptydisk_dialog_pane,
                                           XtNskipAdjust, True,
                                           NULL);

    disk_type_label = XtVaCreateManagedWidget("ImageTypeLabel",
                                              labelWidgetClass, options_form,
                                              XtNborderWidth, 0,
                                              XtNjustify, OPTION_LABELS_JUSTIFY,
                                              XtNwidth, OPTION_LABELS_WIDTH,
                                              XtNleft, XawChainLeft,
                                              XtNright, XawChainLeft,
                                              XtNheight, 20,
                                              XtNlabel, _("Disk format:"),
                                              NULL);

    disk_type_d64_button = XtVaCreateManagedWidget("ImageTypeD64Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D64",
                                                   NULL);

    disk_type_d67_button = XtVaCreateManagedWidget("ImageTypeD67Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d64_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D67",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d71_button = XtVaCreateManagedWidget("ImageTypeD71Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d67_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D71",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d81_button = XtVaCreateManagedWidget("ImageTypeD81Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d71_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D81",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d80_button = XtVaCreateManagedWidget("ImageTypeD80Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d81_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D80",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d82_button = XtVaCreateManagedWidget("ImageTypeD82Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d80_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D82",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d1m_button = XtVaCreateManagedWidget("ImageTypeD1MButton",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d82_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D1M",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d2m_button = XtVaCreateManagedWidget("ImageTypeD2MButton",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d1m_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D2M",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_d4m_button = XtVaCreateManagedWidget("ImageTypeD4MButton",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d2m_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "D4M",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_g64_button = XtVaCreateManagedWidget("ImageTypeG64Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_d4m_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "G64",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);
    disk_type_g71_button = XtVaCreateManagedWidget("ImageTypeG71Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_g64_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "G71",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);


    disk_type_p64_button = XtVaCreateManagedWidget("ImageTypeP64Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_g71_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "P64",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    disk_type_x64_button = XtVaCreateManagedWidget("ImageTypeX64Button",
                                                   toggleWidgetClass, options_form,
                                                   XtNfromHoriz, disk_type_p64_button,
                                                   XtNfromVert, disk_type_label,
                                                   XtNwidth, 40,
                                                   XtNheight, 20,
                                                   XtNright, XtChainRight,
                                                   XtNleft, XtChainRight,
                                                   XtNlabel, "X64",
                                                   XtNradioGroup, disk_type_d64_button,
                                                   NULL);

    button_box = XtVaCreateManagedWidget("buttonBox",
                                         boxWidgetClass, emptydisk_dialog_pane,
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

    XtVaSetValues(disk_type_d64_button, XtNstate, True, NULL);

    XtVaSetValues(image_name_field, XtNstring, "VICE,01", NULL);

    XtAddEventHandler(file_name_field, ButtonPressMask, False, (XtEventHandler)ui_focus_to, (XtPointer)&file_name_field);
    XtAddEventHandler(image_name_field, ButtonPressMask, False, (XtEventHandler)ui_focus_to, (XtPointer)&image_name_field);

    ui_focus_to((XtPointer)&file_name_field);
}

int ui_empty_disk_dialog(char *name)
{
    edisk_file_name = name;
    *edisk_file_name= 0;

    build_emptydisk_dialog();
    ui_popup(emptydisk_dialog, _("Create empty disk"), True);
    return *name ? 0 : -1;
}
