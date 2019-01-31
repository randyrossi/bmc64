/*
 * uiscreenshot.c - Screenshot UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Daniel Sladic <dsladic@cs.cmu.edu>
 *  Andreas Boose <viceteam@t-online.de>
 *  Olaf "Rhialto" Seibert <rhialto@falu.nl>
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

#include "gfxoutput.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "screenshot.h"
#include "uilib.h"
#include "uimenu.h"
#include "uiscreenshot.h"
#include "util.h"
#include "uiapi.h"

#ifdef HAVE_FFMPEG
#include "gfxoutputdrv/ffmpegdrv.h"
#endif

/* #define DEBUG_UI 1 */

#if DEBUG_UI
#define DBG(x)  printf x
#else
#define DBG(x)
#endif

static Widget screenshot_dialog;
static Widget screenshot_dialog_pane;
static Widget file_name_form;
static Widget file_name_label;
static Widget file_name_field;
static Widget browse_button;
static Widget options_form;

static Widget *driver_buttons;
static Widget driver_label;

static Widget optional_box;
static Widget button_box;
static Widget save_button;
static Widget cancel_button;

#ifdef HAVE_FFMPEG
static Widget ffmpeg_options;
static Widget format_button;
static Widget audio_button;
static Widget audio_rate_field;
static Widget video_button;
static Widget video_rate_field;
static Widget half_frame_rate_toggle;
static Widget ffmpeg_format_menu, ffmpeg_audio_menu, ffmpeg_video_menu;
static int ffmpeg_format_selected, selected_ac, selected_vc;
#endif /* HAVE_FFMPEG */

static Widget native_options;
static Widget oversize_button, oversize_menu_widget;
static Widget undersize_button, undersize_menu_widget;
static Widget multicolor_button, multicolor_menu_widget;
static Widget ted_luma_button, ted_luma_menu_widget;
static Widget crtc_textcolor_button, crtc_textcolor_menu_widget;

static char *screenshot_file_name;
static gfxoutputdrv_t *gfxdriver;

#define OPTION_LABELS_WIDTH     (XtArgVal)160
#define OPTION_LABELS_JUSTIFY   XtJustifyLeft
#define OPTION_BUTTONS_WIDTH    (XtArgVal)260

extern Widget rec_button;

#ifndef ENABLE_TEXTFIELD
    static char *text_box_translations = "#override\n<Key>Return: no-op()";
#else
    static char *text_box_translations = "<Btn1Down>: select-start() focus-in()";
#endif

static UI_CALLBACK(oversize_menu_cb);
static UI_CALLBACK(undersize_menu_cb);
static UI_CALLBACK(multicolor_menu_cb);
static UI_CALLBACK(ted_luma_menu_cb);
static UI_CALLBACK(crtc_textcolor_menu_cb);

#ifdef HAVE_FFMPEG

static UI_CALLBACK(ffmpeg_format_cb);
static UI_CALLBACK(ffmpeg_audio_cb);
static UI_CALLBACK(ffmpeg_video_cb);

static int is_ffmpeg(void)
{
    return gfxdriver && strcmp(gfxdriver->name, "FFMPEG") == 0;
}

#endif /* HAVE_FFMPEG */

static int is_koala(void)
{
    return gfxdriver && strncmp(gfxdriver->name, "KOALA", 5) == 0;
}

static int is_doodle(void)
{
    return gfxdriver && strncmp(gfxdriver->name, "DOODLE", 6) == 0;
}

static int is_native(void)
{
    return is_koala() || is_doodle();
}

/*
 * Called when the user clicks the "Browse" button.
 */
static UI_CALLBACK(browse_callback)
{
    ui_button_t button;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;
    char *filename;

    filename = ui_select_file(_("Save screenshot file"), NULL, False, NULL, &filter, 1, &button, 0, NULL, UI_FC_LOAD);

    if (button == UI_BUTTON_OK) {
        Arg args[1];

        XtSetArg(args[0], XtNstring, filename);
        XtSetValues(file_name_field, args, 1);
    }

    lib_free(filename);
}

#ifdef HAVE_FFMPEG

static int select_codec(gfxoutputdrv_codec_t *codecs, int id, XtCallbackProc callback)
{
    int i = 0;

    for (i = 0; codecs[i].name; i++) {
        if (codecs[i].id == id) {
            DBG(("found codec id %d: %s\n", id, codecs[i].name));
            callback(NULL, int_to_void_ptr(i), 0);
            return i;
        }
    }
    return 0;
}

#endif /* HAVE_FFMPEG */

/*
 * Called when one of the gfx driver buttons is clicked.
 */
static UI_CALLBACK(gfxdriver_callback)
{
    if (call_data) {
        /* Usually it means the button is selected... */
        gfxdriver = (gfxoutputdrv_t *)client_data;
    } else {
        /* but it may be deselected as well. */
        gfxdriver = NULL;
    }

    XtSetSensitive(save_button, gfxdriver != NULL);

#ifdef HAVE_FFMPEG
    if (ffmpeg_options) {
        if (is_ffmpeg()) {
            const char *format;
            gfxoutputdrv_format_t *gfmt;
            int i, current_ac_id, current_vc_id, intval;
            Arg args[1];
            char strbuf[16];

            resources_get_string("FFMPEGFormat", &format);
            resources_get_int("FFMPEGAudioCodec", &current_ac_id);
            resources_get_int("FFMPEGVideoCodec", &current_vc_id);

            for (i = 0; gfmt = &ffmpegdrv_formatlist[i], gfmt->name; i++) {
                if (strcmp(gfmt->name, format) == 0) {
                    DBG(("found FFMPEG %s\n", gfmt->name));
                    ffmpeg_format_cb(NULL, int_to_void_ptr(i), 0);

                    /* Override the selecion of item #0 made in ffmpeg_format_cb */
                    select_codec(gfmt->audio_codecs, current_ac_id, ffmpeg_audio_cb);
                    select_codec(gfmt->video_codecs, current_vc_id, ffmpeg_video_cb);

                    break;
                }
            }

            resources_get_int("FFMPEGAudioBitrate", &intval);
            sprintf(strbuf, "%d", intval);
            XtSetArg(args[0], XtNstring, strbuf);
            XtSetValues(audio_rate_field, args, 1);

            resources_get_int("FFMPEGVideoBitrate", &intval);
            sprintf(strbuf, "%d", intval);
            XtSetValues(video_rate_field, args, 1);

            resources_get_int("FFMPEGVideoHalveFramerate", &intval);
            XtSetArg(args[0], XtNstate, intval);
            XtSetValues(half_frame_rate_toggle, args, 1);

            /* Show it */
            XtManageChild(ffmpeg_options);
        } else {
            /* Hide it */
            XtUnmanageChild(ffmpeg_options);
        }
        /* At this point the window resizes because of XtNallowShellResize */
    }
#endif /* HAVE_FFMPEG */

    if (is_native()) {
        int n0 = 0, n1 = 0, n2 = 0, n3 = 0, n4 = 0;

        if (is_doodle()) {
            resources_get_int("DoodleOversizeHandling", &n0);
            resources_get_int("DoodleUndersizeHandling", &n1);
            resources_get_int("DoodleMultiColorHandling", &n2);

            if (machine_class == VICE_MACHINE_PLUS4) {
                resources_get_int("DoodleTEDLumHandling", &n3);
            }

            if ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)) {
                resources_get_int("DoodleCRTCTextColor", &n4);
            }

            XtSetSensitive(multicolor_button, True);
        } else {
            resources_get_int("KoalaOversizeHandling", &n0);
            resources_get_int("KoalaUndersizeHandling", &n1);
            /* resources_get_int("KoalaMultiColorHandling", &n2); */

            if (machine_class == VICE_MACHINE_PLUS4) {
                resources_get_int("KoalaTEDLumHandling", &n3);
            }

            if ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)) {
                resources_get_int("KoalaCRTCTextColor", &n4);
            }

            XtSetSensitive(multicolor_button, False);
        }

        oversize_menu_cb(NULL, int_to_void_ptr(n0), 0);
        undersize_menu_cb(NULL, int_to_void_ptr(n1), 0);
        multicolor_menu_cb(NULL, int_to_void_ptr(n2), 0);

        if (machine_class == VICE_MACHINE_PLUS4) {
            ted_luma_menu_cb(NULL, int_to_void_ptr(n3), 0);
        }

        if ((machine_class == VICE_MACHINE_PET) ||(machine_class == VICE_MACHINE_CBM6x0)) {
            crtc_textcolor_menu_cb(NULL, int_to_void_ptr(n4), 0);
        }

        XtSetSensitive(ted_luma_button,
                (machine_class == VICE_MACHINE_PLUS4));
        XtSetSensitive(crtc_textcolor_button,
                ((machine_class == VICE_MACHINE_PET) || (machine_class == VICE_MACHINE_CBM6x0)));

        XtManageChild(native_options);
    } else {
        XtUnmanageChild(native_options);
    }
}

/*
 * Called when the user clicks the "Cancel" button.
 */
static UI_CALLBACK(cancel_callback)
{
    ui_popdown(screenshot_dialog);
}

/*
 * Called when the user clicks the "Save" button.
 */
static UI_CALLBACK(save_callback)
{
    struct video_canvas_s *canvas;
    String name;
    char *tmp;
    Arg args[1];
    
    ui_popdown(screenshot_dialog);

    if (!gfxdriver) {
        return;
    }

    canvas = (struct video_canvas_s *)client_data;

#ifdef HAVE_FFMPEG
    if (is_ffmpeg()) {
        /*
         * To be consistent with the "native" settings, the resources
         * should be set when the user clicks them, not only when they
         * click Save. But the GTK version does it like this too.
         */
        int audio_bitrate, video_bitrate;
        Boolean half_frame_rate;
        char *value = "";

        XtSetArg(args[0], XtNstring, &value);

        XtGetValues(audio_rate_field, args, 1);
        audio_bitrate = atoi(value);

        XtGetValues(video_rate_field, args, 1);
        video_bitrate = atoi(value);

        XtSetArg(args[0], XtNstate, &half_frame_rate);
        XtGetValues(half_frame_rate_toggle, args, 1);

        resources_set_int("FFMPEGAudioBitrate", audio_bitrate);
        resources_set_int("FFMPEGVideoBitrate", video_bitrate);

        resources_set_string("FFMPEGFormat", ffmpegdrv_formatlist[ffmpeg_format_selected].name);
        resources_set_int("FFMPEGAudioCodec", selected_ac);
        resources_set_int("FFMPEGVideoCodec", selected_vc);
        resources_set_int("FFMPEGVideoHalveFramerate", half_frame_rate);

        DBG(("FFMPEG: Format: %s, ac: %d, vc: %d\n", ffmpegdrv_formatlist[ffmpeg_format_selected].name, selected_ac, selected_vc));
    }
#endif /* HAVE_FFMPEG */

    XtSetArg(args[0], XtNstring, &name);
    XtGetValues(file_name_field, args, 1);

    tmp = lib_stralloc(name);   /* Make a copy of widget-managed string */

    if (!util_get_extension(tmp)) {
        util_add_extension(&tmp, gfxdriver->default_extension);
    }

    if (screenshot_save(gfxdriver->name, tmp, canvas) < 0) {
        ui_error(_("Couldn't write screenshot to `%s' with gfxdriver `%s'."), tmp, gfxdriver->name);
        lib_free(tmp);
        return;
    } else {
        if (screenshot_is_recording()) {
            XtManageChild(rec_button);
        }
        /* ui_message(_("Successfully wrote `%s'"), tmp); */
        lib_free(tmp);
    }
}

/*
 * In the popup menus we have set all items to UI_MENU_TYPE_NORMAL.
 * That does leave space for the checkmarks, but it doesn't set or clear
 * them.  Do that here.
 * When using UI_MENU_TYPE_TICK, there can be unexpeced callbacks with
 * weird values for the client_data.
 */
static void menu_adjust_tick(Widget menu, int nr)
{
    Arg args[2];
    int num_children;
    WidgetList children;
    int i;

    if (!menu) {
        return;
    }

    XtSetArg(args[1], XtNchildren, &children);
    XtSetArg(args[0], XtNnumChildren, &num_children);
    XtGetValues(menu, args, 2);

    for (i = 0; i < num_children; i++) {
        Widget menu_item = children[i];
        ui_menu_set_tick(menu_item, i == nr);
    }
}

#ifdef HAVE_FFMPEG

/*
 * Callback when an audio codec menu item is chosen by the user.
 */
static UI_CALLBACK(ffmpeg_audio_cb)
{
    int selected = vice_ptr_to_int(UI_MENU_CB_PARAM);
    gfxoutputdrv_codec_t *codecs = ffmpegdrv_formatlist[ffmpeg_format_selected].audio_codecs;
    Arg args[2];

    selected_ac = codecs[selected].id;

    XtSetArg(args[0], XtNlabel, (char *)codecs[selected].name);
    XtSetArg(args[1], XtNwidth, OPTION_BUTTONS_WIDTH);
    XtSetValues(audio_button, args, 2);

    menu_adjust_tick(ffmpeg_audio_menu, selected);
}

/*
 * Callback when a video codec menu item is chosen by the user.
 */
static UI_CALLBACK(ffmpeg_video_cb)
{
    int selected = vice_ptr_to_int(UI_MENU_CB_PARAM);
    gfxoutputdrv_codec_t *codecs = ffmpegdrv_formatlist[ffmpeg_format_selected].video_codecs;
    Arg args[2];

    selected_vc = codecs[selected].id;

    XtSetArg(args[0], XtNlabel, (char *)codecs[selected].name);
    XtSetArg(args[1], XtNwidth, OPTION_BUTTONS_WIDTH);
    XtSetValues(video_button, args, 2);

    menu_adjust_tick(ffmpeg_video_menu, selected);
}

/*
 * Dynamically create a menu for choosing an audio/video codec.
 * What the contents are depends on the FFMPEG format.
 */
static void create_ffmpeg_codec_menu(gfxoutputdrv_codec_t *c, char *name, Widget *w, ui_callback_t cb, Widget caller)
{
    int i, ncodecs;
    ui_menu_entry_t *menu;
    Boolean enabled;
    Arg args[2];

    enabled = c != NULL;

    XtSetSensitive(caller, enabled);

    if (!enabled) {
        XtSetArg(args[0], XtNlabel, "-----");
        XtSetArg(args[1], XtNwidth, OPTION_BUTTONS_WIDTH);
        XtSetValues(caller, args, 2);

        return;
    }

    /* Count the number of ffmpeg audio/video codecs */
    for (ncodecs = 0; c[ncodecs].name; ncodecs++) {
        DBG(("%s: %s id=%d\n", name, c[ncodecs].name, c[ncodecs].id));
    }

    menu = lib_calloc(sizeof(ui_menu_entry_t), ncodecs + 1);

    for (i = 0; c[i].name; i++) {
        menu[i].string = (char *)c[i].name;
        menu[i].type = UI_MENU_TYPE_NORMAL;
        menu[i].callback = cb;
        menu[i].callback_data = int_to_void_ptr(i);
    }

    if (*w) {
        ui_menu_delete(*w);
    }
    *w = ui_menu_create(name, menu, NULL);

    lib_free(menu);
    
    /* Arbitrarily select the first item from the menu */
    cb(0, 0, 0);
}

static void create_ffmpeg_audio_menu(int index)
{
    gfxoutputdrv_codec_t *c = ffmpegdrv_formatlist[index].audio_codecs;

    create_ffmpeg_codec_menu(c, "ffmpegAudioMenu", &ffmpeg_audio_menu,
            ffmpeg_audio_cb, audio_button);
}

static void create_ffmpeg_video_menu(int index)
{
    gfxoutputdrv_codec_t *c = ffmpegdrv_formatlist[index].video_codecs;

    create_ffmpeg_codec_menu(c, "ffmpegVideoMenu", &ffmpeg_video_menu,
            ffmpeg_video_cb, video_button);
}

/*
 * Callback when an ffmpeg format is chosen.
 * Adapt the audio and video codec menus to the available codecs.
 */
static UI_CALLBACK(ffmpeg_format_cb)
{
    int selected = vice_ptr_to_int(UI_MENU_CB_PARAM);
    Arg args[2];

    ffmpeg_format_selected = selected;

    XtSetArg(args[0], XtNlabel, ffmpegdrv_formatlist[ffmpeg_format_selected].name);
    XtSetArg(args[1], XtNwidth, OPTION_BUTTONS_WIDTH);
    XtSetValues(format_button, args, 2);

    menu_adjust_tick(ffmpeg_format_menu, selected);

    create_ffmpeg_audio_menu(selected);
    create_ffmpeg_video_menu(selected);
}

static void create_ffmpeg_format_menu(void)
{
    int i, nformats;
    ui_menu_entry_t *menu;

    /* Count the number of ffmpeg formats */
    for (nformats = 0; ffmpegdrv_formatlist[nformats].name; nformats++) {
        DBG(("FFMPEG format: %s\n", ffmpegdrv_formatlist[nformats].name));
    }

    menu = lib_calloc(sizeof(ui_menu_entry_t), nformats + 1);

    for (i = 0; ffmpegdrv_formatlist[i].name; i++) {
        menu[i].string = ffmpegdrv_formatlist[i].name;
        menu[i].type = UI_MENU_TYPE_NORMAL;
        menu[i].callback = (ui_callback_t)ffmpeg_format_cb;
        menu[i].callback_data = int_to_void_ptr(i);
    }

    if (ffmpeg_format_menu) { 
        ui_menu_delete(ffmpeg_format_menu);
    }
    ffmpeg_format_menu = ui_menu_create("ffmpegFormatMenu", menu, NULL);

    lib_free(menu);

    /* Select the first item from the menu */
    ffmpeg_format_cb(NULL, 0, 0);
}

/*
 * For the FFMPEG driver, more options are needed.
 * Build that sub-dialog here.
 */
static Widget build_ffmpeg_options(Widget parent)
{
    Widget form;
    Widget format_label;
    Widget audio_label;
    Widget audio_rate_label;
    Widget video_label;
    Widget video_rate_label;

    /* Check if ffmpeg formats were found at runtime */
    if (!ffmpegdrv_formatlist || !ffmpegdrv_formatlist[0].name) {
        return 0;
    }

    /* Initially unmanaged: default gfxdriver isn't ffmpeg */
    form = XtVaCreateWidget("ffmpegOptionsForm",
                                   formWidgetClass, parent,
                                   XtNborderWidth, 0,
                                   XtNwidth, 1000L, /* aim for max width */
                                   NULL);

    format_label = XtVaCreateManagedWidget("formatTypeLabel",
                                           labelWidgetClass, form,
                                           XtNborderWidth, 0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, _("Format"),
                                           /* Constraints: */
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);

    format_button = XtVaCreateManagedWidget("formatTypeButton",
                                             commandWidgetClass, form,
                                             XtNwidth, OPTION_BUTTONS_WIDTH,
                                             XtNheight, 20,
                                             XtNlabel, "",
                                             /* Constraints: */
                                             XtNfromHoriz, format_label,
                                             XtNleft, XtChainRight,
                                             XtNright, XtChainRight,
                                             NULL);

    XtOverrideTranslations(format_button, XtParseTranslationTable(
                "<Btn1Down>: XawPositionSimpleMenu(ffmpegFormatMenu) XtMenuPopup(ffmpegFormatMenu)"));

    audio_label = XtVaCreateManagedWidget("audioLabel",
                                           labelWidgetClass, form,
                                           XtNborderWidth, 0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, _("Audio codec"),
                                           /* Constraints: */
                                           XtNfromVert, format_button,
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);

    audio_button = XtVaCreateManagedWidget("audioButton",
                                           commandWidgetClass, form,
                                           XtNwidth, OPTION_BUTTONS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, "",
                                           /* Constraints: */
                                           XtNfromHoriz, audio_label,
                                           XtNfromVert, format_button,
                                           XtNleft, XtChainRight,
                                           XtNright, XtChainRight,
                                           NULL);

    XtOverrideTranslations(audio_button, XtParseTranslationTable(
                "<Btn1Down>: XawPositionSimpleMenu(ffmpegAudioMenu) XtMenuPopup(ffmpegAudioMenu)"));

    audio_rate_label = XtVaCreateManagedWidget("audioRateLabel",
                                           labelWidgetClass, form,
                                           XtNborderWidth, 0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, _("Audio bitrate"),
                                           /* Constraints: */
                                           XtNfromVert, audio_button,
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);

    audio_rate_field = XtVaCreateManagedWidget("audioRateField",
#ifndef ENABLE_TEXTFIELD
                                           asciiTextWidgetClass, form,
                                           XtNtype, XawAsciiString,
                                           XtNeditType, XawtextEdit,
#else
                                           textfieldWidgetClass, form,
#endif
                                           XtNstring, "64000",
                                           XtNwidth, OPTION_BUTTONS_WIDTH,
                                           /* Constraints: */
                                           XtNfromHoriz, audio_rate_label,
                                           XtNfromVert, audio_button,
                                           XtNleft, XtChainRight,
                                           XtNright, XtChainRight,
                                           NULL);

    XtOverrideTranslations(audio_rate_field, XtParseTranslationTable(text_box_translations));

    video_label = XtVaCreateManagedWidget("videoLabel",
                                           labelWidgetClass, form,
                                           XtNborderWidth, 0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, _("Video codec"),
                                           /* Constraints: */
                                           XtNfromVert, audio_rate_field,
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);

    video_button = XtVaCreateManagedWidget("videoButton",
                                           commandWidgetClass, form,
                                           XtNfromHoriz, video_label,
                                           XtNwidth, OPTION_BUTTONS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, "",
                                           /* Constraints: */
                                           XtNfromHoriz, video_label,
                                           XtNfromVert, audio_rate_field,
                                           XtNleft, XtChainRight,
                                           XtNright, XtChainRight,
                                           NULL);

    XtOverrideTranslations(video_button, XtParseTranslationTable(
                "<Btn1Down>: XawPositionSimpleMenu(ffmpegVideoMenu) XtMenuPopup(ffmpegVideoMenu)"));

    video_rate_label = XtVaCreateManagedWidget("videoRateLabel",
                                           labelWidgetClass, form,
                                           XtNborderWidth, 0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           XtNheight, 20,
                                           XtNlabel, _("Video bitrate"),
                                           /* Constraints: */
                                           XtNfromVert, video_button,
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);

    video_rate_field = XtVaCreateManagedWidget("videoRateField",
#ifndef ENABLE_TEXTFIELD
                                           asciiTextWidgetClass, form,
                                           XtNtype, XawAsciiString,
                                           XtNeditType, XawtextEdit,
#else
                                           textfieldWidgetClass, form,
#endif
                                           XtNstring, "800000",
                                           XtNwidth, OPTION_BUTTONS_WIDTH,
                                           /* Constraints: */
                                           XtNfromHoriz, video_rate_label,
                                           XtNfromVert, video_button,
                                           XtNleft, XtChainRight,
                                           XtNright, XtChainRight,
                                           NULL);
    XtOverrideTranslations(video_rate_field, XtParseTranslationTable(text_box_translations));

    half_frame_rate_toggle = XtVaCreateManagedWidget("halfFramerRateToggle",
                                           toggleWidgetClass, form,
                                           XtNlabel, _("Half Framerate (25/30 fps)"),
                                           XtNwidth, OPTION_BUTTONS_WIDTH,
                                           /* Constraints: */
                                           XtNfromHoriz, video_rate_label,
                                           XtNfromVert, video_rate_field,
                                           XtNleft, XtChainRight,
                                           XtNright, XtChainRight,
                                           NULL);
    create_ffmpeg_format_menu();

    return form;
}

#endif /* HAVE_FFMPEG */

/* Koala and Doodle oversize/undersize options */

#define ENTRY(label, callback, data)                       \
    { label, UI_MENU_TYPE_NORMAL, (ui_callback_t)callback, \
      (ui_callback_data_t)data, NULL,                      \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 }

static ui_menu_entry_t oversize_menu[] = {
    ENTRY(N_("scale down"),         oversize_menu_cb, 0),
    ENTRY(N_("crop left top"),      oversize_menu_cb, 1),
    ENTRY(N_("crop center top"),    oversize_menu_cb, 2),
    ENTRY(N_("crop right top"),     oversize_menu_cb, 3),
    ENTRY(N_("crop left center"),   oversize_menu_cb, 4),
    ENTRY(N_("crop center"),        oversize_menu_cb, 5),
    ENTRY(N_("crop right center"),  oversize_menu_cb, 6),
    ENTRY(N_("crop left bottom"),   oversize_menu_cb, 7),
    ENTRY(N_("crop center bottom"), oversize_menu_cb, 8),
    ENTRY(N_("crop right bottom"),  oversize_menu_cb, 9),
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t undersize_menu[] = {
    ENTRY(N_("scale up"),           undersize_menu_cb, 0),
    ENTRY(N_("add borders"),        undersize_menu_cb, 1),
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t multicolor_menu[] = {
    ENTRY(N_("b&w"),                multicolor_menu_cb, 0),
    ENTRY(N_("2 colors"),           multicolor_menu_cb, 1),
    ENTRY(N_("4 colors"),           multicolor_menu_cb, 2),
    ENTRY(N_("gray scale"),         multicolor_menu_cb, 3),
    ENTRY(N_("best cell colors"),   multicolor_menu_cb, 4),
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t ted_luma_menu[] = {
    ENTRY(N_("ignore"),             ted_luma_menu_cb, 0),
    ENTRY(N_("dither"),             ted_luma_menu_cb, 1),
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t crtc_textcolor_menu[] = {
    ENTRY(N_("white"),              crtc_textcolor_menu_cb, 0),
    ENTRY(N_("amber"),              crtc_textcolor_menu_cb, 1),
    ENTRY(N_("green"),              crtc_textcolor_menu_cb, 2),
    UI_MENU_ENTRY_LIST_END
};

static void set_button(Widget w, ui_menu_entry_t *menu, int choice)
{
    Arg args[2];

    XtSetArg(args[0], XtNlabel, menu[choice].string);
    XtSetArg(args[1], XtNwidth, OPTION_BUTTONS_WIDTH);
    XtSetValues(w, args, 2);
}

static UI_CALLBACK(oversize_menu_cb)
{
    int choice = vice_ptr_to_int(UI_MENU_CB_PARAM);
    set_button(oversize_button, oversize_menu, choice);
    menu_adjust_tick(oversize_menu_widget, choice);

    if (w) {
        if (is_doodle()) {
            resources_set_int("DoodleOversizeHandling", choice);
        } else if (is_koala()) {
            resources_set_int("KoalaOversizeHandling", choice);
        }
    }
}

static UI_CALLBACK(undersize_menu_cb)
{
    int choice = vice_ptr_to_int(UI_MENU_CB_PARAM);
    set_button(undersize_button, undersize_menu, choice);
    menu_adjust_tick(undersize_menu_widget, choice);

    if (w) {
        if (is_doodle()) {
            resources_set_int("DoodleUndersizeHandling", choice);
        } else if (is_koala()) {
            resources_set_int("KoalaUndersizeHandling", choice);
        }
    }
}

static UI_CALLBACK(multicolor_menu_cb)
{
    int choice = vice_ptr_to_int(UI_MENU_CB_PARAM);
    set_button(multicolor_button, multicolor_menu, choice);
    menu_adjust_tick(multicolor_menu_widget, choice);

    if (w) {
        if (is_doodle()) {
            resources_set_int("DoodleMultiColorHandling", choice);
        }
    }
}

static UI_CALLBACK(ted_luma_menu_cb)
{
    int choice = vice_ptr_to_int(UI_MENU_CB_PARAM);
    set_button(ted_luma_button, ted_luma_menu, choice);
    menu_adjust_tick(ted_luma_menu_widget, choice);

    if (w) {
        if (is_doodle()) {
            resources_set_int("DoodleTEDLumHandling", choice);
        } else if (is_koala()) {
            resources_set_int("KoalaTEDLumHandling", choice);
        }
    }
}

static UI_CALLBACK(crtc_textcolor_menu_cb)
{
    int choice = vice_ptr_to_int(UI_MENU_CB_PARAM);
    set_button(crtc_textcolor_button, crtc_textcolor_menu, choice);
    menu_adjust_tick(crtc_textcolor_menu_widget, choice);

    if (w) {
        if (is_doodle()) {
            resources_set_int("DoodleCRTCTextColor", choice);
        } else if (is_koala()) {
            resources_set_int("KoalaCRTCTextColor", choice);
        }
    }
}

static Widget build_label_and_menu(Widget form,
        char *label_name, char *label_text,
        char *button_name, Widget fromVert,
        char *menu_name, Widget *menu_widget, ui_menu_entry_t *menu_entries)
{
    Widget label, button;
    char *trans;

    label = XtVaCreateManagedWidget(label_name,
                                    labelWidgetClass, form,
                                    XtNborderWidth, 0,
                                    XtNjustify, OPTION_LABELS_JUSTIFY,
                                    XtNwidth, OPTION_LABELS_WIDTH,
                                    XtNheight, 20,
                                    XtNlabel, label_text,
                                    /* Constraints: */
                                    XtNfromVert, fromVert,
                                    XtNleft, XawChainLeft,
                                    XtNright, XawChainRight,
                                    NULL);

    button = XtVaCreateManagedWidget(button_name,
                                     commandWidgetClass, form,
                                     XtNwidth, OPTION_BUTTONS_WIDTH,
                                     XtNheight, 20,
                                     XtNlabel, "",
                                     /* Constraints: */
                                     XtNfromHoriz, label,
                                     XtNfromVert, fromVert,
                                     XtNleft, XtChainRight,
                                     XtNright, XtChainRight,
                                     NULL);

    *menu_widget = ui_menu_create(menu_name, menu_entries, NULL);

    trans = lib_msprintf("<Btn1Down>: XawPositionSimpleMenu(%s) XtMenuPopup(%s)", menu_name, menu_name);

    XtOverrideTranslations(button, XtParseTranslationTable(trans));

    lib_free(trans);

    return button;
}

/*
 * For the "native" file formats (Koala and Doodle), more options are
 * needed.  Build that sub-dialog here.
 */
static Widget build_native_options(Widget parent)
{
    Widget form;

    form = XtVaCreateWidget("nativeOptionsForm",
                                   formWidgetClass, parent,
                                   XtNborderWidth, (XtArgVal)0,
                                   XtNwidth, 1000L, /* aim for max width */
                                   NULL);

    oversize_button = build_label_and_menu(form,
            "oversizeLabel", _("Oversize handling"),
            "oversizeButton", NULL,
            "nativeOversizeMenu", &oversize_menu_widget, oversize_menu);

    undersize_button = build_label_and_menu(form,
            "undersizeLabel", _("Undersize handling"),
            "undersizeButton", oversize_button,
            "nativeUndersizeMenu", &undersize_menu_widget, undersize_menu);

    multicolor_button = build_label_and_menu(form,
            "multicolorLabel", _("Multicolor handling"),
            "multicolorButton", undersize_button,
            "nativeMulticolorMenu", &multicolor_menu_widget, multicolor_menu);

    ted_luma_button = build_label_and_menu(form,
            "tedlumaLabel", _("TED luma handling"),
            "tedlumaButton", multicolor_button,
            "tedLumaMenu", &ted_luma_menu_widget, ted_luma_menu);

    crtc_textcolor_button = build_label_and_menu(form,
            "crtcTextLabel", _("CRTC textcolor"),
            "crtcTextButton", ted_luma_button,
            "crtcTextMenu", &crtc_textcolor_menu_widget, crtc_textcolor_menu);

    return form;
}

/*
 * Create the screenshot dialog, which is a singleton.
 */
static void build_screenshot_dialog(struct video_canvas_s *canvas)
{
    int i, num_buttons;
    gfxoutputdrv_t *driver, *driver0;
    Widget prev;

    char *button_title;
    char *filename;
    char *image_format;

    if (screenshot_dialog) {
        return;
    }

    filename = util_concat(_("File name"), ":", NULL);
    image_format = util_concat(_("Image Format"), ":", NULL);

    screenshot_dialog = ui_create_transient_shell(_ui_top_level, "screenshotDialog");
    XtVaSetValues(screenshot_dialog,
            XtNallowShellResize, (XtArgVal)True,
            NULL);

    screenshot_dialog_pane = XtVaCreateManagedWidget("screenshotDialogPane",
                                                     panedWidgetClass, screenshot_dialog,
                                                     NULL);

    file_name_form = XtVaCreateManagedWidget("fileNameForm",
                                             formWidgetClass, screenshot_dialog_pane,
                                             XtNshowGrip, (XtArgVal)False,
                                             NULL);

    file_name_label = XtVaCreateManagedWidget("fileNameLabel",
                                              labelWidgetClass, file_name_form,
                                              XtNjustify, XtJustifyLeft,
                                              XtNlabel, filename,
                                              XtNborderWidth, (XtArgVal)0,
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
                                              XtNwidth, (XtArgVal)240,
                                              /* Constraints: */
                                              XtNfromHoriz, file_name_label,
                                              NULL);

    XtOverrideTranslations(file_name_field, XtParseTranslationTable(text_box_translations));

    button_title = util_concat(_("Browse"), "...", NULL);
    browse_button = XtVaCreateManagedWidget("browseButton",
                                            commandWidgetClass, file_name_form,
                                            XtNlabel, button_title,
                                            /* Constraints: */
                                            XtNfromHoriz, file_name_field,
                                            NULL);
    lib_free(button_title);
    XtAddCallback(browse_button, XtNcallback, browse_callback, NULL);

    options_form = XtVaCreateManagedWidget("optionsForm",
                                           formWidgetClass, screenshot_dialog_pane,
                                           /* Constraints: */
                                           XtNskipAdjust, (XtArgVal)True,
                                           NULL);

    driver_label = XtVaCreateManagedWidget("ImageTypeLabel",
                                           labelWidgetClass, options_form,
                                           XtNborderWidth, (XtArgVal)0,
                                           XtNjustify, OPTION_LABELS_JUSTIFY,
                                           XtNwidth, OPTION_LABELS_WIDTH,
                                           /* Constraints: */
                                           XtNheight, (XtArgVal)20,
                                           XtNlabel, image_format,
                                           XtNleft, XawChainLeft,
                                           XtNright, XawChainRight,
                                           NULL);
    lib_free(image_format);

    num_buttons = gfxoutput_num_drivers();
    driver_buttons = lib_malloc(sizeof(Widget) * num_buttons);
    driver = gfxoutput_drivers_iter_init();
    driver0 = driver;
    prev = NULL;

    for (i = 0; i < num_buttons; i++) {
        driver_buttons[i] = XtVaCreateManagedWidget(driver->displayname,
                                          toggleWidgetClass, options_form,
                                          XtNwidth, OPTION_BUTTONS_WIDTH,
                                          XtNheight, (XtArgVal)20,
                                          XtNlabel, driver->displayname,
                                          XtNradioGroup, prev,
                                          /* Constraints: */
                                          XtNfromHoriz, driver_label,
                                          XtNfromVert, prev,
                                          XtNleft, XtChainRight,
                                          XtNright, XtChainRight,
                                          NULL);
        XtAddCallback(driver_buttons[i], XtNcallback, gfxdriver_callback, driver);
        driver = gfxoutput_drivers_iter_next();
        prev = driver_buttons[i];
    }

    optional_box = XtVaCreateManagedWidget("optionalBox",
                                         boxWidgetClass, screenshot_dialog_pane,
                                         XtNhSpace, (XtArgVal)0,
                                         XtNvSpace, (XtArgVal)0,
                                         /* Constraints: */
                                         XtNresizeToPreferred, (XtArgVal)True,
                                         XtNallowResize, (XtArgVal)True,
                                         XtNshowGrip, (XtArgVal)False,
                                         XtNleft, XawChainLeft,
                                         XtNright, XawChainRight,
                                         NULL);

#ifdef HAVE_FFMPEG
    ffmpeg_options = build_ffmpeg_options(optional_box);
#endif
    native_options = build_native_options(optional_box);

    button_box = XtVaCreateManagedWidget("buttonBox",
                                         boxWidgetClass, screenshot_dialog_pane,
                                         XtNshowGrip, (XtArgVal)False,
                                         NULL);

    save_button = XtVaCreateManagedWidget("saveButton",
                                          commandWidgetClass, button_box,
                                          XtNlabel, _("Save"),
                                          NULL);
    XtAddCallback(save_button, XtNcallback, save_callback, (XtPointer)canvas);

    cancel_button = XtVaCreateManagedWidget("cancelButton",
                                            commandWidgetClass, button_box,
                                            XtNlabel, _("Cancel"),
                                            NULL);
    XtAddCallback(cancel_button, XtNcallback, cancel_callback, NULL);


    /* Do this after save_button has been set */
    XtVaSetValues(driver_buttons[0], XtNstate, (XtArgVal)True, NULL);
    gfxdriver_callback((Widget)driver_buttons[0], /*client_data*/driver0, /*call_data*/int_to_void_ptr(True));

    XtSetKeyboardFocus(file_name_form, file_name_field);
    XtSetKeyboardFocus(options_form, file_name_field);
}

int ui_screenshot_dialog(char *name, struct video_canvas_s *canvas)
{
    screenshot_file_name = name;
    *screenshot_file_name= 0;

    build_screenshot_dialog(canvas);
    ui_popup(screenshot_dialog, _("Screen Snapshot"), True);
    return *name ? 0 : -1;
}

/*
 * Called when exiting and cleaning up GUI stuff.
 */
void uiscreenshot_shutdown(void)
{
    if (screenshot_dialog) {
        XtDestroyWidget(screenshot_dialog);
        screenshot_dialog = 0;
#ifdef HAVE_FFMPEG
        /*
         * XtDestroyWidget(3) doesn't mention anything about unmanaged
         * children, so assume they are destroyed with the rest of them.
         * Then there is no need to treat ffmpeg_options and native_options 
         * specially.
         */
        ffmpeg_options = 0;

        ui_menu_delete(ffmpeg_audio_menu);
        ui_menu_delete(ffmpeg_video_menu);
        ffmpeg_audio_menu = ffmpeg_video_menu = 0;
#endif
        native_options = 0;

        ui_menu_delete(oversize_menu_widget);
        ui_menu_delete(undersize_menu_widget);
        ui_menu_delete(multicolor_menu_widget);
        ui_menu_delete(ted_luma_menu_widget);
        ui_menu_delete(crtc_textcolor_menu_widget);

        oversize_menu_widget = undersize_menu_widget = multicolor_menu_widget =
            ted_luma_menu_widget = crtc_textcolor_menu_widget = 0;

        lib_free(driver_buttons);
    }
}
