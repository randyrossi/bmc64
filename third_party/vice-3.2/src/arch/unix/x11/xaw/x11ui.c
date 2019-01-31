/** \file   x11ui.c
 * \brief   Simple Xaw(3d)-based graphical user interface
 *
 * It uses widgets from the Free Widget Foundation and Robert W. McMullen.
 *
 * \author  Olaf Seibert <rhialto@falu.nl>
 * \author  Ettore Perazzoli <ettore@comm2000.it>
 * \author  Andre Fachat <fachat@physik.tu-chemnitz.de>
 * \author  Andreas Boose <viceteam@t-online.de>
 */

/*
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

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <locale.h>

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Shell.h>

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeBSB.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/Command.h>
#include <X11/Xaw3d/Form.h>
#include <X11/Xaw3d/Paned.h>
#include <X11/Xaw3d/Box.h>
#include <X11/Xaw3d/AsciiText.h>
#include <X11/Xaw3d/Tip.h>
#else
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Paned.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Tip.h>
#endif

#include <X11/keysym.h>

#ifdef HAVE_X11_SUNKEYSYM_H
#include <X11/Sunkeysym.h>
#endif

#ifdef HAVE_LIBXPM
#ifdef HAVE_XPM_H
#include <xpm.h>
#endif
#ifdef HAVE_X11_XPM_H
#include <X11/xpm.h>
#endif
#endif

#include "fullscreenarch.h"
#include "ioutil.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "mouse.h"
#include "mousedrv.h"
#include "psid.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
#include "uicolor.h"
#include "uicommands.h"
#include "uidrivestatus.h"
#include "uihotkey.h"
#include "uilib.h"
#include "uimenu.h"
#include "uisettings.h"
#include "util.h"
#include "vsync.h"
#include "widgets/Canvas.h"
#include "widgets/FileSel.h"
#include "widgets/TextField.h"
#include "viewport.h"
#include "video.h"
#include "videoarch.h"
#include "vsiduiunix.h"
#include "vice-event.h"
#include "x11ui.h"
#include "lightpen.h"
#include "lightpendrv.h"
#include "uipalcontrol.h"
#include "uistatusbar.h"
#include "focus.h"
#include "kbd.h"

/* #define DEBUG_X11UI */
/* #define DEBUGMOUSECURSOR */

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/* FIXME: We want these to be static.  */
Visual *visual;
XFontStruct *cbm_font_struct;
static int have_truecolor;

static Display *display;
int screen;
static int depth;

/* UI logging goes here.  */
log_t ui_log = LOG_ERR;
extern log_t vsid_log;

Cursor blankCursor;
static video_canvas_t *ui_cached_video_canvas;
static Widget last_visited_canvas;

static Widget get_last_visited_app_shell(void);

/* ------------------------------------------------------------------------- */

static int popped_up_count = 0;

/* Left-button and right-button menu.  */
static Widget left_menu, right_menu;

/* Translations for the left and right menus.  */
static XtTranslations left_menu_translations, right_menu_translations;
static XtTranslations left_menu_disabled_translations, right_menu_disabled_translations;

/* Application context. */
static XtAppContext app_context;

/* This is needed to catch the `Close' command from the Window Manager. */
Atom wm_delete_window;
Atom wm_protocols;

/* Toplevel widget. */
Widget _ui_top_level = NULL;
/* Our colormap. */
Colormap colormap;

/* Application icon.  */
static Pixmap icon_pixmap;

/* This allows us to pop up the transient shells centered to the last visited
   shell. */
static Widget last_visited_app_shell = NULL;

app_shell_type app_shells[MAX_APP_SHELLS];
static int num_app_shells = 0;

/* Pixels for updating the drive LED's state.  */
Pixel drive_led_on_red_pixel, drive_led_on_green_pixel, drive_led_off_pixel;

/* If != 0, we should save the settings. */
/* static int resources_have_changed = 0; */

static char *filesel_dir = NULL;

static void mouse_cursor_grab(int grab, Cursor cursor);

/* ------------------------------------------------------------------------- */
/*
 * TODO: drag and drop support
 *
 * setup a handler for "drop" event, and then pass the filename associated with
 * the dropped object to drop_cb
 */
static int (*drop_cb)(char*) = NULL; 

void ui_set_drop_callback(void *cb)
{
    drop_cb = cb;
}

int ui_focus_monitor(void) 
{
    return ui_focus_terminal(display, ui_log);
}

/*
    restore the main emulator window and transfer focus to it. in detail this
    function should:

    - move the active toplevel shell window to top of the window stack
    - make sure the window is visible (move if offscreen, de-iconify, etc)
    - transfer the window managers keyboard focus to the window

    note: the "focus stealing prevention" feature of eg KDE must be disabled or
          all this will not work at all.

    this function is called by uimon_window_close and -_suspend, ie when exiting
    the ml monitor.
*/
void ui_restore_focus(void)
{
    Widget s;

    ui_dispatch_events();

    s = get_last_visited_app_shell();
    if (s) {
        Window w = XtWindow(s);
        ui_focus_window(display, w);                 /* raise and de-iconify */
    }
    x11kbd_focus_change();
}

/* ------------------------------------------------------------------------- */

void ui_restore_mouse(void)
{
    mouse_cursor_grab(0, None);
}

static void initBlankCursor(Widget canvas)
{
#ifdef DEBUGMOUSECURSOR1
    static char no_data[] = { 255, 129, 129, 129, 129, 129, 129, 255 };
#else
    static char no_data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
#endif
    static Pixmap blank;
    XColor trash, dummy;

    if (blankCursor) {
        return;
    }

    XAllocNamedColor(display, DefaultColormapOfScreen(DefaultScreenOfDisplay(display)), "black", &trash, &dummy);

    /*
     * "The pixmaps can be freed immediately if no further explicit
     * references to them are to be made." XCreateFontCursor(3)
     */
    blank = XCreateBitmapFromData(display, XtWindow(canvas), no_data, 8, 8);

    /* warning: must call XFreeCursor() when finished! */
    blankCursor = XCreatePixmapCursor(display, blank, blank, &trash, &trash, 0, 0);

    XFreePixmap(display, blank);
}

/* seemingly dead code */
#if 0
static void freeBlankCursor(void)
{
    if (blankCursor) {
        XFreeCursor(display, blankCursor);
        blankCursor = 0;
    }
}
#endif

static int mouse_lasteventx, mouse_lasteventy;
static int mouse_warped = 0;
#define MOUSE_WRAP_MARGIN  40

static void mouse_handler_canvas(Widget w, XtPointer client_data, XEvent *report, Boolean *ctd)
{
    video_canvas_t *canvas = (video_canvas_t *)client_data;
    float mouse_dx, mouse_dy;

    switch(report->type) {
        case MotionNotify:
            /* handle pointer motion events for mouse emulation */
#ifdef HAVE_FULLSCREEN
            if (canvas->fullscreenconfig && canvas->fullscreenconfig->enable) {
                fullscreen_mouse_moved(canvas,
                        (int)report->xmotion.x, (int)report->xmotion.y, 0);
            }
#endif
            if (_mouse_enabled) {
                if (mouse_warped) {
                    /* ignore this event, its the result of us having moved the pointer */
                    mouse_warped = 0;
                } else {
                    int x=0, y=0, w=0, h=0, warp=0;
                    int ptrx, ptry;
                    /* float taspect; */

                    /* get cursor position */
                    ptrx = x = (int)report->xmotion.x;
                    ptry = y = (int)report->xmotion.y;

#ifdef HAVE_XVIDEO
                    if (canvas->videoconfig->hwscale && canvas->xv_image) {
                        w = canvas->xv_geometry.w + canvas->xv_geometry.x * 2;
                        h = canvas->xv_geometry.h + canvas->xv_geometry.y * 2;
                    } else
#endif
                    {
                        w = canvas->draw_buffer->canvas_physical_width;
                        h = canvas->draw_buffer->canvas_physical_height;
                    }

                    if (ptrx < MOUSE_WRAP_MARGIN) {
                        x = mouse_lasteventx = w - (MOUSE_WRAP_MARGIN + 10);
                        warp = 1;
                    }
                    else if (ptrx > (w - MOUSE_WRAP_MARGIN)) {
                        x = mouse_lasteventx = (MOUSE_WRAP_MARGIN + 10);
                        warp = 1;
                    }

                    if (ptry < (MOUSE_WRAP_MARGIN)) {
#ifdef HAVE_FULLSCREEN
                        if (canvas->fullscreenconfig->enable == 0) {
                            mouse_lasteventy = h - (MOUSE_WRAP_MARGIN + 10);
                        } else {
                            mouse_lasteventy = h - ((MOUSE_WRAP_MARGIN + 10) *2);
                        }
#else
                        mouse_lasteventy = h - (MOUSE_WRAP_MARGIN + 10);
#endif
                        y = mouse_lasteventy;
                        warp = 1;
                    } else if (ptry > (h - MOUSE_WRAP_MARGIN)) {
                        mouse_lasteventy = (MOUSE_WRAP_MARGIN + 10);
                        y = mouse_lasteventy;
                        warp = 1;
                    }

                    if (warp) {
                        /* set new cusor position */
                        mouse_warped = 1;
                        XWarpPointer(display, None, XtWindow(last_visited_canvas), 0, 0, 0, 0, x, y);
                    } else {
                        mouse_dx = (float)((ptrx - mouse_lasteventx) << 1);
                        mouse_dy = (float)((ptry - mouse_lasteventy) << 1);
#ifdef HAVE_XVIDEO
                        if (canvas->videoconfig->hwscale && canvas->xv_image) {
                            mouse_dx = mouse_dx * canvas->draw_buffer->canvas_width / canvas->xv_geometry.w;
                            mouse_dy = mouse_dy * canvas->draw_buffer->canvas_height / canvas->xv_geometry.h;
                        } else
#endif
                        {
                            mouse_dx /= (float)canvas->videoconfig->scalex;
                            mouse_dy /= (float)canvas->videoconfig->scaley;
                        }
                        mouse_move(mouse_dx, mouse_dy);
                        mouse_lasteventx = ptrx;
                        mouse_lasteventy = ptry;
                    }
                }
            }
            if (lightpen_enabled) {
                xaw_lightpen_update_xy(report->xmotion.x, report->xmotion.y);
            }
            break;
        case ButtonPress:
        case ButtonRelease:
            if (_mouse_enabled) {
                mouse_button(report->xbutton.button - 1, (report->type == ButtonPress));
            }
            if (lightpen_enabled) {
                xaw_lightpen_setbutton(report->xbutton.button, (report->type == ButtonPress));
            }
            break;
    }
}

/* ------------------------------------------------------------------------- */

static void enable_mouse_menus(void)
{
    int i;

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].canvas, left_menu_translations);
        XtOverrideTranslations(app_shells[i].canvas, right_menu_translations);
    }
}

static void disable_mouse_menus(void)
{
    int i;

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].canvas, left_menu_disabled_translations);
        XtOverrideTranslations(app_shells[i].canvas, right_menu_disabled_translations);
    }
}

static void enable_mouse_callback(void)
{
    int i;

    for (i = 0; i < num_app_shells; i++) {
        XtAddEventHandler(app_shells[i].canvas,
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                False,
                (XtEventHandler)mouse_handler_canvas,
                (XtPointer)app_shells[i].video_canvas);
    }
}

static void disable_mouse_callback(void)
{
    int i;

    for (i = 0; i < num_app_shells; i++) {
        XtRemoveEventHandler(app_shells[i].canvas,
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                False,
                (XtEventHandler)mouse_handler_canvas,
                (XtPointer)app_shells[i].video_canvas);
    }
}


/*
    grab pointer and keyboard, set mouse pointer shape

    TODO: also route lightpen stuff through this function
*/
static int mouse_grabbed = 0;
static void mouse_cursor_grab(int grab, Cursor cursor)
{
    if (mouse_grabbed) {
        XUndefineCursor(display, XtWindow(last_visited_canvas));
        XUngrabPointer(display, CurrentTime);
        XUngrabKeyboard(display, CurrentTime);
        mouse_grabbed = 0;
    }
    if (grab) {
        XGrabKeyboard(display, XtWindow(last_visited_canvas), 1,
                GrabModeAsync, GrabModeAsync,  CurrentTime);

        XGrabPointer(display, XtWindow(last_visited_canvas), 0,
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync,
                GrabModeAsync,
                XtWindow(last_visited_canvas),
                cursor,
                CurrentTime);
        mouse_grabbed = 1;
    }
}

void ui_check_mouse_cursor(void)
{
    int callbacks_wanted = 0;
    if (!ui_cached_video_canvas) {
        return;
    }

#ifdef HAVE_FULLSCREEN
    if (ui_cached_video_canvas->fullscreenconfig->enable) {
        if (_mouse_enabled) {
            mouse_cursor_grab(1, blankCursor);
        } else {
            mouse_cursor_grab(1, None);
        }
        callbacks_wanted = 1;
    } else
#endif

    if (_mouse_enabled) {
        mouse_cursor_grab(1, blankCursor);
    } else {
        mouse_cursor_grab(0, None);
    }

    if (_mouse_enabled || lightpen_enabled) {
        disable_mouse_menus();
        callbacks_wanted = 1;
    } else {
        enable_mouse_menus();
    }

    if (callbacks_wanted) {
        enable_mouse_callback();
    } else {
        disable_mouse_callback();
    }
}

/* ------------------------------------------------------------------------- */

static int alloc_colormap(void);
static Widget build_file_selector(Widget parent, ui_button_t *button_return);
static Widget build_error_dialog(Widget parent, ui_button_t *button_return, const String message);
static Widget build_input_dialog(Widget parent, ui_button_t *button_return, Widget *InputDialogLabel, Widget *InputDialogField);
static Widget build_show_text(Widget parent, ui_button_t *button_return, const String text, int width, int height);
static Widget build_confirm_dialog(Widget parent, ui_button_t *button_return, Widget *ConfirmDialogMessage);
static void close_action(Widget w, XEvent *event, String *params, Cardinal *num_params);

static UI_CALLBACK(enter_window_callback_shell);
static UI_CALLBACK(structure_callback_shell);
static UI_CALLBACK(exposure_callback_canvas);
static UI_CALLBACK(structure_callback_canvas);

/* ------------------------------------------------------------------------- */

/*
 * Original unscaled:
 * "-freetype-vice cbm-medium-r-normal-medium-12-120-100-72-m-104-symbol-0"
 *  -foundry -font family      -set width     -pixels-xdpi-spacing
 *                    -weight-slant   -???      -10*points   -average width (10*pixels)
 *                                                      -ydpi    -characterset
 *
 * First try the scaled version, if unavailable fall back to the original size,
 * which is a bit small for most current screen resolutions.
 */
static String cbm_fontspec1 = "-freetype-vice cbm-medium-r-normal-medium-24-*-*-*-m-*-symbol-0";
static String cbm_fontspec2 = "-freetype-vice cbm-medium-r-normal-medium-12-*-*-*-m-*-symbol-0";

/*
 * Reminder to the user: you can specify additional resource strings
 * in your $HOME/.Xresources file and on the command line with
 * the -xrm option.
 */
static String fallback_resources[] = {
    "*international:                                 True",
    /* If international, the fonts are taller (presumably allowing for more
     * extensive ascenders/descenders) which makes the menus too big.
     * So, "fix" the height here. It is a percentage to add to the height.
     * If "international" is disabled "vertSpace" should be too.
     */
    "*SmeBSB.vertSpace:                              -15",
#define INTERNATIONAL_RESOURCES         2
    "*font:                                        -*-lucida-bold-r-*-*-12-*",
    "*fontSet:                                     -*-lucida-bold-r-*-*-12-*-*-*-*-*-iso8859-1,-*-lucida-bold-r-*-*-12-*-*-*-*-*-iso10646-*",
    "*Command.font:                                -*-lucida-bold-r-*-*-12-*",
    "*Command.fontSet:                             -*-lucida-bold-r-*-*-12-*-*-*-*-*-iso8859-1,-*-lucida-bold-r-*-*-12-*-*-*-*-*-iso10646-*",
    "*fileSelector.width:			     380",
    "*fileSelector.height:			     300",
    "*inputDialog.inputForm.borderWidth:	     0",
    "*inputDialog.inputForm.field.width:	     300",
    "*inputDialog.inputForm.field.scrollHorizontal:  True",
    "*inputDialog.inputForm.label.width:	     250",
    "*inputDialog.inputForm.label.borderWidth:	     0",
    "*inputDialog.inputForm.label.internalWidth:     0",
    "*inputDialog.buttonBox.borderWidth:	     0",
    "*errorDialog.messageForm.borderWidth:	     0",
    "*errorDialog.buttonBox.borderWidth:	     0",
    "*errorDialog.messageForm.label.borderWidth:     0",
    "*jamDialog.messageForm.borderWidth:	     0",
    "*jamDialog.buttonBox.borderWidth:		     0",
    "*jamDialog.messageForm.label.borderWidth:       0",
    "*infoDialogShell.width:			     380",
    "*infoDialogShell.height:			     290",
    "*infoDialog.textForm.infoString.borderWidth:    0",
    "*infoDialog.textForm.borderWidth:		     0",
    "*infoDialog.textForm.defaultDistance:	     0",
    "*infoDialog.buttonBox.borderWidth:		     0",
    "*infoDialog.buttonBox.internalWidth:	     5",
    "*infoDialog.textForm.infoString.internalHeight: 0",
    "*confirmDialogShell.width:			     300",
    "*confirmDialog.messageForm.message.borderWidth: 0",
    "*confirmDialog.messageForm.message.height:      20",
    "*showText.textBox.text.width:		     480",
    "*showText.textBox.text.height:		     305",
    /* when the next one is True, credits disappear,
     * probably exceeding the max size of an X widget */
    "*showText.textBox.text*international:           False",
    "*showText.textBox.text*font:       -*-lucidatypewriter-medium-r-*-*-12-*",
    "*showText.textBox.text*fontSet:    -*-lucidatypewriter-medium-r-*-*-12-*",
    "*okButton.label:				     Confirm",
    "*cancelButton.label:			     Cancel",
    "*closeButton.label:			     Dismiss",
    "*yesButton.label:				     Yes",
    "*resetButton.label:			     Reset",
    "*hardResetButton.label:                         Hard Reset",
    "*monButton.label:				     Monitor",
    "*noneButton.label:				     Continue",
    "*debugButton.label:			     XDebugger",
    "*noButton.label:				     No",
    "*licenseButton.label:			     License...",
    "*noWarrantyButton.label:			     No warranty!",
    "*contribButton.label:			     Contributors...",
    "*Text.translations:			     #override \\n"
                                                    "<Key>Return: no-op()\\n"
                                                    "<Key>Linefeed: no-op()\\n"
                                                    "Ctrl<Key>J: no-op() \\n",

    /* These fonts will be overridden later if the VICE-CBM font is available */
    "*rightDrive8Menu*SmeBSB.international:          False",
    "*rightDrive8Menu*SmeBSB.vertSpace:              25",
    "*rightDrive8Menu*SmeBSB.font:       -*-lucidatypewriter-medium-r-*-*-12-*",
    "*rightDrive9Menu*SmeBSB.international:          False",
    "*rightDrive9Menu*SmeBSB.vertSpace:              25",
    "*rightDrive9Menu*SmeBSB.font:       -*-lucidatypewriter-medium-r-*-*-12-*",
    "*rightDrive10Menu*SmeBSB.international:         False",
    "*rightDrive10Menu*SmeBSB.font:      -*-lucidatypewriter-medium-r-*-*-12-*",
    "*rightDrive10Menu*SmeBSB.vertSpace:              25",
    "*rightDrive11Menu*SmeBSB.international:         False",
    "*rightDrive11Menu*SmeBSB.font:      -*-lucidatypewriter-medium-r-*-*-12-*",
    "*rightDrive11Menu*SmeBSB.vertSpace:              25",
    "*rightTapeMenu*SmeBSB.international:         False",
    "*rightTapeMenu*SmeBSB.font:      -*-lucidatypewriter-medium-r-*-*-12-*",
    "*rightTapeMenu*SmeBSB.vertSpace:              25",

    /* Default color settings (suggestions are welcome...) */
    "*foreground:				     black",
    "*background:				     gray80",
    "*borderColor:				     black",
    "*internalBorderColor:			     black",
    "*TransientShell*Dialog.background:		     gray80",
    "*TransientShell*Label.background:		     gray80",
    "*TransientShell*Box.background:		     gray80",
    "*fileSelector.background:			     gray80",
    "*Command.background:			     gray90",
    "*Toggle.background:                             gray90",
    "*Menubutton.background:		             gray80",
    "*Scrollbar.background:		             gray80",
    "*Form.background:				     gray80",
    "*Label.background:				     gray80",
    "*Canvas.background:                             black",
    "*Tip.background:                                rgb:f/f/8",
/*
    "*Tip.borderWidth:                               0",
    "*Tip.displayList: foreground rgb:8/8/4; lines 1,-1,-1,-1,-1,1; foreground rgb:f/f/c; lines -1,0,0,0,0,-1",
 */
    "*driveTrack1.cursorName:                  hand1",
    "*driveTrack1.font:                        -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack1.fontSet:                     -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack2.cursorName:                  hand1",
    "*driveTrack2.font:                        -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack2.fontSet:                     -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack3.cursorName:                  hand1",
    "*driveTrack3.font:                        -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack3.fontSet:                     -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack4.cursorName:                  hand1",
    "*driveTrack4.font:                        -*-helvetica-medium-r-*-*-12-*",
    "*driveTrack4.fontSet:                     -*-helvetica-medium-r-*-*-12-*",
    "*speedStatus.cursorName:                  hand1",
    "*speedStatus.font:                        -*-helvetica-medium-r-*-*-12-*",
    "*speedStatus.fontSet:                     -*-helvetica-medium-r-*-*-12-*",
    "*statustext.font:                         -*-helvetica-medium-r-*-*-12-*",
    "*statustext.fontSet:                      -*-helvetica-medium-r-*-*-12-*",
    "*tapeCounter1.cursorName:                 hand1",
    "*tapeCounter1.font:                       -*-helvetica-medium-r-*-*-12-*",
    "*tapeCounter1.fontSet:                    -*-helvetica-medium-r-*-*-12-*",
    "*tapeButtons1.translations:               #override\\n~Shift ~Ctrl ~Meta<BtnDown>: TapePlayStop()\\n"
                                              "Ctrl<BtnDown>: TapePlayStop(rew)\\n"
                                              "Meta<BtnDown>: TapePlayStop(ffwd)\\n"
                                              "Shift<BtnDown>(2): TapePlayStop(rec)\\n",
    /* ~Shift -> Shift not pressed */
    NULL
};

/* ------------------------------------------------------------------------- */

static unsigned int wm_command_size;
static unsigned char *wm_command_data = NULL;
static Atom wm_command_atom;
static Atom wm_command_type_atom;

static void prepare_wm_command_data(int argc, char **argv)
{
    unsigned int offset, i;

    wm_command_size = 0;
    for (i = 0; i < (unsigned int) argc; i++) {
        wm_command_size += strlen(argv[i]) + 1;
    }

    wm_command_data = lib_malloc(wm_command_size);

    offset = 0;
    for (i = 0; i < (unsigned int) argc; i++) {
        unsigned int len;

        len = strlen(argv[i]);
        memcpy(wm_command_data + offset, argv[i], len);
        wm_command_data[offset + len] = 0;
        offset += len + 1;
    }
}

static void finish_prepare_wm_command(void)
{
    wm_command_atom = XInternAtom(display, "WM_COMMAND", False);
    wm_command_type_atom = XInternAtom(display, "STRING", False);
}

/* Initialize the GUI and parse the command line. */
int ui_init(int *argc, char **argv)
{
    static XtActionsRec actions[] = {
        { "Close", close_action },
        { "RebuildTapeMenu", rebuild_tape_menu_action },
        { "TapePlayStop", tape_play_stop_action },
        { "RebuildDiskMenu", rebuild_disk_menu_action },
    };
    int skip_resources = 0;

    /*
     * If there is no locale, it is better to turn "international support"
     * off, since many Xmb* and Xwc* functions then assume 7-bit ASCII...
     */
    char *lc = setlocale(LC_CTYPE, NULL);
    if (lc[0] == 'C' && lc[1] == '\0') {
        skip_resources = INTERNATIONAL_RESOURCES;
    }
    XtSetLanguageProc(NULL, NULL, NULL);
    prepare_wm_command_data(*argc, argv);

    /* Create the toplevel. */
    /* Using `sessionShellWidgetClass` in this call works with some WM's, but
     * causes the UI init to hang later on other WM's
     *
     * Xfce4, Mate and Cinnamon on Debian 8.6 failed, OpenBox worked. -- BW
     */
    _ui_top_level = XtOpenApplication(&app_context, "VICE", NULL, 0,
            argc, argv, fallback_resources + skip_resources,
            applicationShellWidgetClass, NULL, 0);
    if (!_ui_top_level) {
        return -1;
    }

    display = XtDisplay(_ui_top_level);
    screen = XDefaultScreen(display);
    atexit(ui_autorepeat_on);

    /*
     * Check if we can use the VICE-CBM font.
     */
    cbm_font_struct = XLoadQueryFont(display, cbm_fontspec1);
    if (cbm_font_struct == NULL) {
        cbm_font_struct = XLoadQueryFont(display, cbm_fontspec2);
    }
    if (cbm_font_struct == NULL) {
        log_warning(ui_log, "The VICE-CBM font is not available.");
    }

    wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
    wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);

    XtAppAddActions(app_context, actions, XtNumber(actions));

    ui_common_init();

    finish_prepare_wm_command();

    return 0;
}

void ui_shutdown(void)
{
    int i;

    /* Shuts down various ui subparts, including menus */
    ui_common_shutdown();

    for (i = 0; i < num_app_shells; i++) {
        lib_free(app_shells[i].title);
    }
    /*
     * This recursively deletes all its (remaining) children, including
     * all app_shells[i].shell, but also all menus that have not been
     * cleaned up yet. If they are separately destroyed after this
     * point, that leads to accessing freed memory.
     *
     * At least that would happen if app_context->dispatch_level == 0,
     * which is not true when quitting from a keyboard or menu action,
     * but can be true when exiting because of control-C.
     */
    XtDestroyWidget(_ui_top_level);

    lib_free(wm_command_data);
    lib_free(filesel_dir);

    /*
     * Frees extra menu resources, such as still needed when destroying
     * tickmark menu items.
     */
    uimenu_shutdown();

    if (cbm_font_struct) {
        XFreeFont(display, cbm_font_struct);
    }

    XtCloseDisplay(display);
    XtDestroyApplicationContext(app_context);
    /* Definitely no X access after this point! */
}

typedef struct {
    char *name;
    int class;
} namedvisual_t;

/* Continue GUI initialization after resources are set. */
int ui_init_finish(void)
{
    static namedvisual_t classes[] = {
        { "PseudoColor", PseudoColor },
        { "TrueColor", TrueColor },
        { "StaticGray", StaticGray },
        { NULL }
    };

    XVisualInfo visualinfo;

    ui_log = log_open("X11");

    resources_get_int("DisplayDepth", &depth);

    if (depth != 0) {
        int i;

        for (i = 0; classes[i].name != NULL; i++) {
            if (XMatchVisualInfo(display, screen, depth, classes[i].class, &visualinfo)) {
                break;
            }
        }
        if (!classes[i].name) {
            log_error(ui_log, "This display does not support suitable %dbit visuals.", depth);
            log_error(ui_log, "Please select a bit depth supported by your display.");
            return -1;
        } else {
            log_message(ui_log, "Found %dbit/%s visual.", depth, classes[i].name);
            have_truecolor = (classes[i].class == TrueColor);
        }
    } else {
        /* Autodetect. */
        int i, j, done;
        int depths[8];

        depths[0] = DefaultDepth(display, screen);
        depths[1] = 0;

        for (i = done = 0; depths[i] != 0 && !done; i++) {
            for (j = 0; classes[j].name != NULL; j++) {
                if (XMatchVisualInfo(display, screen, depths[i], classes[j].class, &visualinfo)) {
                    depth = depths[i];
                    log_message(ui_log, "Found %dbit/%s visual.", depth, classes[j].name);
                    have_truecolor = (classes[j].class == TrueColor);
                    done = 1;
                    break;
                }
            }
        }
        if (!done) {
            log_error(ui_log, "Cannot autodetect a proper visual.");
            return -1;
        }
    }

    visual = visualinfo.visual;

    /* Allocate the colormap. */
    alloc_colormap();

    /* Recreate _ui_top_level to support non-default display depths.  */

    /* Goodbye...  */
    XtDestroyWidget(_ui_top_level);

    /* Create the new `_ui_top_level'.  */
    _ui_top_level = XtVaAppCreateShell(machine_name, "VICE",
                                       sessionShellWidgetClass, display,
                                       XtNvisual, visual,
                                       XtNdepth, depth,
                                       XtNcolormap, colormap,
                                       XtNmappedWhenManaged, False,
                                       XtNwidth, 1,
                                       XtNheight, 1,
                                       NULL);
    XtRealizeWidget(_ui_top_level);

    /* Set the `WM_COMMAND' property in the new _ui_top_level. */
    XChangeProperty(display, XtWindow(_ui_top_level), wm_command_atom, wm_command_type_atom, 8, PropModeReplace, wm_command_data, wm_command_size);

#ifdef HAVE_FULLSCREEN
    if (fullscreen_init() != 0) {
        log_warning(ui_log, "Some fullscreen devices aren't initialized properly.");
    }
#endif 

    return ui_menu_init(app_context, display, screen);
}

int ui_init_finalize(void)
{
    return 0;
}

int x11ui_get_display_depth(void)
{
    return depth;
}

Display *x11ui_get_display_ptr(void)
{
    return display;
}

Window x11ui_get_X11_window()
{
    return XtWindow(_ui_top_level);
}

#if DEBUG_X11UI
void printxywh(char *name, Widget widget)
{
    Dimension x = -1, y = -1, w = -1, h = -1;

    XtVaGetValues(widget,
                    XtNx, &x,
                    XtNy, &y,
                    XtNwidth, &w,
                    XtNheight, &h,
                    NULL);
    printf("%s: x=%d y=%d w=%d h=%d\n", name, x, y, w, h);
}
#endif /* DEBUG_X11UI */

/*
 * Create a shell with a canvas widget in it.
 * +-----------------------------------------------------------------------+
 * |+---------------------------------------------------------------------+|
 * ||                                                                     ||
 * ...                           video canvas                            ...
 * ||                                                                     ||
 * |+---------------------------------------------------------------------+|
 * |100%, 50fps            |    9: Track 18,0   XX |    8: Track 18,0   XX |
 * |    CRT Controls       |   11: Track 18,0   XX |   10: Track 18,0   XX |
 * |(statustext)           |recording..|event reco |   Tape #1: 000     [] |
 * +-----------------------------------------------------------------------+
 */
int ui_open_canvas_window(video_canvas_t *c, const char *title, int width, int height, int no_autorepeat)
{
    /* Note: this is correct because we never destroy CanvasWindows.  */
    Widget shell;
    Widget pane;
    Widget canvas;
    XSetWindowAttributes attr;

    if (machine_class != VICE_MACHINE_VSID) {
        if (uicolor_alloc_colors(c) < 0) {
            return -1;
        }
    }

    /* colormap might have changed after ui_alloc_colors, so we set it again */
    XtVaSetValues(_ui_top_level, XtNcolormap, colormap, NULL);

    if (++num_app_shells > MAX_APP_SHELLS) {
        log_error(ui_log, "Maximum number of toplevel windows reached.");
        return -1;
    }

    /* This creates a new window; in principle the initial _ui_top_level
     * is already a SessionShell or a "main top-level window". But for
     * two-screen use (such as with the 128) having one screen be a
     * child Widget of the other is somewhat asymmetric.
     */
    shell = XtVaCreatePopupShell(title, topLevelShellWidgetClass, _ui_top_level, XtNinput, True, XtNtitle, title, XtNiconName, title, NULL);

    /* Xt only allows you to change the visual of a shell widget, so the
       visual and colormap must be created before the shell widget is
       created. When creating another shell widget, the new widget inherits
       the colormap and depth from the parent widget, but it inherits the
       visual from the parent window (the root window). Thus on every shell
       you create you must specify visual, colormap, and depth. Note that
       popup dialogs and menus are also shells. */
    XtVaSetValues(shell, XtNvisual, visual, XtNdepth, depth, XtNcolormap, colormap, NULL);

#define DD      2               /* default distance */

    pane = XtVaCreateManagedWidget("Form",
                                   formWidgetClass, shell,
                                   XtNdefaultDistance, DD,
                                   NULL);

    if (machine_class != VICE_MACHINE_VSID) {
        canvas = XtVaCreateManagedWidget("Canvas",
                                        xfwfcanvasWidgetClass, pane,
                                        XtNwidth, width,
                                        XtNheight, height,
                                        XtNborderWidth, 0,
                                        XtNbackground, BlackPixel(display, screen),
                                        /* Constraints: */
                                        XtNresizable, True,
                                        XtNbottom, XawChainBottom,
                                        XtNtop, XawChainTop,
                                        XtNleft, XawChainLeft,
                                        XtNright, XawChainRight,
                                        NULL);
    } else {
        vsid_ctrl_widget_set_parent(pane);
        canvas = build_vsid_ctrl_widget();
    }

    last_visited_canvas = canvas;

    /* XVideo must be refreshed when the shell window is moved. */
    if (machine_class != VICE_MACHINE_VSID) {
        XtAddEventHandler(shell, StructureNotifyMask, False, (XtEventHandler)structure_callback_shell, (XtPointer)c);
        XtAddEventHandler(shell, EnterWindowMask, False, (XtEventHandler)enter_window_callback_shell, (XtPointer)c);

        XtAddEventHandler(canvas, ExposureMask, False, (XtEventHandler)exposure_callback_canvas, (XtPointer)c);
        XtAddEventHandler(canvas, StructureNotifyMask, False, (XtEventHandler)structure_callback_canvas, (XtPointer)c);
    }

    /* Create the status bar on the bottom.  */
    ui_create_status_bar(pane, width, canvas, c, num_app_shells - 1);

    /* Assign proper translations to open the menus, if already
       defined.  */
    if (left_menu_translations != NULL) {
        XtOverrideTranslations(canvas, left_menu_translations);
    }
    if (right_menu_translations != NULL) {
        XtOverrideTranslations(canvas, right_menu_translations);
    }

    /* Attach the icon pixmap, if already defined.  */
    if (icon_pixmap) {
        XtVaSetValues(shell, XtNiconPixmap, icon_pixmap, NULL);
    }

    XtAddEventHandler(canvas, (EnterWindowMask | LeaveWindowMask | KeyReleaseMask | KeyPressMask), True, (XtEventHandler)kbd_event_handler, NULL);

    /* FIXME: ...REALLY ugly... */
    XtAddEventHandler(XtParent(canvas), (EnterWindowMask | LeaveWindowMask | KeyReleaseMask | KeyPressMask), True, (XtEventHandler)kbd_event_handler, NULL);

    if (no_autorepeat) {
        XtAddEventHandler(canvas, EnterWindowMask, False, (XtEventHandler)ui_autorepeat_off, NULL);
        XtAddEventHandler(canvas, LeaveWindowMask, False, (XtEventHandler)ui_autorepeat_on, NULL);
    }

    XtRealizeWidget(shell);
    XtPopup(shell, XtGrabNone);

    attr.backing_store = Always;
    XChangeWindowAttributes(display, XtWindow(canvas), CWBackingStore, &attr);

    app_shells[num_app_shells - 1].shell = shell;
    app_shells[num_app_shells - 1].canvas = canvas;
    app_shells[num_app_shells - 1].title = lib_stralloc(title);
    app_shells[num_app_shells - 1].video_canvas = c;

    XSetWMProtocols(display, XtWindow(shell), &wm_delete_window, 1);
    XtOverrideTranslations(shell, XtParseTranslationTable("<Message>WM_PROTOCOLS: Close()"));

    if (machine_class != VICE_MACHINE_VSID) {
        ui_init_drive_status_widget();
    }

    initBlankCursor(canvas);
    c->app_shell = num_app_shells - 1;
    c->emuwindow = canvas;

    ui_cached_video_canvas = c;

    if (machine_class != VICE_MACHINE_VSID) {
        xaw_init_lightpen(display);
        xaw_lightpen_update_canvas(ui_cached_video_canvas, TRUE);
    }

    return 0;
}

/* Attach `w' as the left menu of all the current open windows.  */
void ui_set_left_menu(ui_menu_entry_t *menu)
{
    Widget w = ui_menu_create("leftMenu", menu, NULL);
    char *translation_table;
    char *name = XtName(w);
    int i;

    /*
     * Make sure to realise the widget, since the XawPositionSimpleMenu()
     * action may want to look at its Window.
     */
    XtRealizeWidget(w);
    translation_table = util_concat("<Btn1Down>: XawPositionSimpleMenu(", name, ") XtMenuPopup(", name, ")\n"
                                    "Meta Shift <KeyDown>z: FakeButton(1) XawPositionSimpleMenu(", name, ") XtMenuPopup(", name, ")\n",
                                    NULL);

    left_menu_translations = XtParseTranslationTable(translation_table);
    lib_free(translation_table);

    translation_table = util_concat("<Btn1Down>: \n",
                                    NULL);

    left_menu_disabled_translations = XtParseTranslationTable(translation_table);
    lib_free(translation_table);

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].canvas, left_menu_translations);
    }

    if (left_menu != NULL) {
        ui_menu_delete(left_menu);
    }
    left_menu = w;
}

/* Attach `w' as the right menu of all the current open windows.  */
void ui_set_right_menu(ui_menu_entry_t *menu)
{
    Widget w = ui_menu_create("rightMenu", menu, NULL);
    char *translation_table;
    char *name = XtName(w);
    int i;

    XtRealizeWidget(w);
    translation_table = util_concat("<Btn3Down>: XawPositionSimpleMenu(", name, ") XtMenuPopup(", name, ")\n"
                                    "Meta Shift <KeyDown>x: FakeButton(3) XawPositionSimpleMenu(", name, ") XtMenuPopup(", name, ")\n",
                                    NULL);
    right_menu_translations = XtParseTranslationTable(translation_table);
    lib_free(translation_table);

    translation_table = util_concat("<Btn3Down>: \n",
                                    NULL);

    right_menu_disabled_translations = XtParseTranslationTable(translation_table);
    lib_free(translation_table);

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].canvas, right_menu_translations);
    }

    if (right_menu != NULL) {
        ui_menu_delete(right_menu);
    }
    right_menu = w;
}

void ui_set_topmenu(ui_menu_entry_t *menu)
{
}

void ui_set_speedmenu(ui_menu_entry_t *menu)
{
    Widget w = ui_menu_create("SpeedMenu", menu, NULL);
    int i;
    static XtTranslations speed_menu_translations;
    static Widget speed_menu;

    XtRealizeWidget(w);
    if (speed_menu_translations == 0) {
        char *translation_table;
        translation_table = "<BtnDown>: XawPositionSimpleMenu(SpeedMenu) XtMenuPopup(SpeedMenu)\n"
                           "Meta Shift <KeyDown>z: FakeButton(1) XawPositionSimpleMenu(SpeedMenu) XtMenuPopup(SpeedMenu)\n";
        speed_menu_translations = XtParseTranslationTable(translation_table);
    }

    for (i = 0; i < num_app_shells; i++) {
        XtOverrideTranslations(app_shells[i].speed_label, speed_menu_translations);
    }

    if (speed_menu != NULL) {
        ui_menu_delete(speed_menu);
    }
    speed_menu = w;
}

void ui_set_application_icon(const char *icon_data[])
{
#ifdef HAVE_LIBXPM
    int i;
    Pixmap icon_pixmap;

    /* Create the icon pixmap. */
    XpmCreatePixmapFromData(display, DefaultRootWindow(display), (char **)icon_data, &icon_pixmap, NULL, NULL);

    for (i = 0; i < num_app_shells; i++) {
        XtVaSetValues(app_shells[i].shell, XtNiconPixmap, icon_pixmap, NULL);
    }
#endif
}

/* ------------------------------------------------------------------------- */

int get_num_shells(void)
{
    return num_app_shells;
}

/* ------------------------------------------------------------------------- */

void ui_exit(void)
{
    ui_button_t b;
    int value;
    char *s = util_concat("Exit ", machine_name, _(" emulator"), NULL);

    resources_get_int("ConfirmOnExit", &value);
    if (value) {
        b = ui_ask_confirmation(s, _("Do you really want to exit?"));
    } else {
        b = UI_BUTTON_YES;
    }

    if (b == UI_BUTTON_YES) {
        resources_get_int("SaveResourcesOnExit", &value);
        if (value) {
            b = ui_ask_confirmation(s, _("Save the current settings?"));
            if (b == UI_BUTTON_YES) {
                if (resources_save(NULL) < 0) {
                    ui_error(_("Cannot save settings."));
                }
            } else if (b == UI_BUTTON_CANCEL) {
                lib_free(s);
                return;
            }
        }
        ui_autorepeat_on();
        ui_restore_mouse();

#ifdef HAVE_FULLSCREEN
        fullscreen_suspend(0);
#endif
        lib_free(s);
        exit(0);
    }

    lib_free(s);
    vsync_suspend_speed_eval();
}

/* ------------------------------------------------------------------------- */

/* Set the colormap variable.  The user must tell us whether he wants the
   default one or not using the `privateColormap' resource.  */
static int alloc_colormap(void)
{
    int use_private_colormap;

    if (colormap) {
        return 0;
    }

    resources_get_int("PrivateColormap", &use_private_colormap);

    if (!use_private_colormap && depth == DefaultDepth(display, screen) && !have_truecolor) {
        colormap = DefaultColormap(display, screen);
    } else {
        log_message(ui_log, "Using private colormap.");
        colormap = XCreateColormap(display, RootWindow(display, screen), visual, AllocNone);
    }

    XtVaSetValues(_ui_top_level, XtNcolormap, colormap, NULL);
    return 0;
}

/* ------------------------------------------------------------------------- */

/* Dispatch the next Xt event.  If not pending, wait for it. */
void ui_dispatch_next_event(void)
{
    XEvent report;

    if (app_context) {
        XtAppNextEvent(app_context, &report);
        XtDispatchEvent(&report);
    }
}

/* Dispatch all the pending Xt events. */
void ui_dispatch_events(void)
{
    if (console_mode) {
        return;
    }

    if (app_context) {
        while (XtAppPending(app_context)) {
            ui_dispatch_next_event();
        }
    }
}

int x11ui_fullscreen(int enable)
{
    static Atom _net_wm_state = None;
    static Atom _net_wm_state_fullscreen = None;
    XEvent xev;
    int mode;

    if (strcmp(machine_name, "C128") == 0) {
        /* mode == 1 -> VICII, mode == 0 VDC */
        resources_get_int("C128ColumnKey", &mode); 
    } else {
        mode = 0;
    }

    if (_net_wm_state == None) {
        _net_wm_state = XInternAtom(display, "_NET_WM_STATE", False);
        _net_wm_state_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    }
    memset(&xev, 0, sizeof(xev));
    xev.xclient.type = ClientMessage;
    xev.xclient.window = XtWindow(app_shells[mode].shell); /* hardwired use of resource `40/80ColumnKey' */
    xev.xclient.message_type = _net_wm_state;
    xev.xclient.format = 32;
    xev.xclient.data.l[0] = enable;
    xev.xclient.data.l[1] = _net_wm_state_fullscreen;

    XSendEvent(display, DefaultRootWindow(display), False, SubstructureRedirectMask, &xev);

    ui_dispatch_events();
    /*
     * calling ui_check_mouse_cursor() would be useful here, except that
     * the field it checks for full-screen isn't updated yet.
     */
    mouse_cursor_grab(enable, None);

    return 0;
}

/* Resize one window. */
void x11ui_resize_canvas_window(ui_window_t w, int width, int height)
{
    Dimension canvas_width, canvas_height;
    Dimension form_width, form_height;

    /* Ok, form widgets are stupid animals; in a perfect world, I should be
       allowed to resize the canvas and let the Form do the rest.  Unluckily,
       this does not happen, so let's do things the dirty way then.  This
       sucks badly.
       Even setting the "resizable" constraint property to True does not help.
     */

    XtVaGetValues((Widget)w,
                  XtNwidth, &canvas_width,
                  XtNheight, &canvas_height,
                  NULL);
    XtVaGetValues(XtParent(XtParent((Widget)w)),
                  XtNwidth, &form_width,
                  XtNheight, &form_height,
                  NULL);

    XtVaSetValues(XtParent(XtParent((Widget)w)),
                  XtNwidth, form_width + width - canvas_width,
                  XtNheight, form_height + height - canvas_height,
                  NULL);

    return;
}

void x11ui_move_canvas_window(ui_window_t w, int x, int y)
{
    XtVaSetValues(XtParent(XtParent((Widget)w)),
                  XtNwidthInc, 1,
                  XtNheightInc, 1,
                  XtNx, x,
                  XtNy, y,
                  NULL);
}

void x11ui_canvas_position(ui_window_t w, int *x, int *y)
{
    Dimension pos_x, pos_y;
    Dimension tl_x, tl_y;
    Dimension shell_x, shell_y;

    XtVaGetValues(XtParent(XtParent((Widget)w)),
                  XtNx, &tl_x,
                  XtNy, &tl_y,
                  NULL);

    XtVaGetValues(XtParent((Widget)w),
                  XtNx, &shell_x,
                  XtNy, &shell_y,
                  NULL);

    XtVaGetValues((Widget)w,
                  XtNx, &pos_x,
                  XtNy, &pos_y,
                  NULL);
    *x = (int) (pos_x + tl_x + shell_x);
    *y = (int) (pos_y + tl_y + shell_y);
    XRaiseWindow(display, XtWindow(_ui_top_level));
}

void x11ui_get_widget_size(ui_window_t win, int *w, int *h)
{
    Dimension x, y;
    XtVaGetValues((Widget)win,
                  XtNwidth, &x,
                  XtNheight, &y,
                  NULL);
    *w = (int) x;
    *h = (int) y;
}

void x11ui_destroy_widget(ui_window_t w)
{
    XtDestroyWidget(w);
}

/* Enable autorepeat. */
void ui_autorepeat_on(void)
{
    XAutoRepeatOn(display);
    XFlush(display);
}

/* Disable autorepeat. */
void ui_autorepeat_off(void)
{
    XAutoRepeatOff(display);
    XFlush(display);
}

/* ------------------------------------------------------------------------- */

/* Button callbacks.  */

#define DEFINE_BUTTON_CALLBACK(button)          \
    static UI_CALLBACK(button##_callback)       \
    {                                           \
        *((ui_button_t *)client_data) = button; \
    }

DEFINE_BUTTON_CALLBACK(UI_BUTTON_OK)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_CANCEL)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_YES)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_NO)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_CLOSE)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_MON)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_NONE)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_RESET)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_HARDRESET)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_CONTENTS)
DEFINE_BUTTON_CALLBACK(UI_BUTTON_AUTOSTART)

/* ------------------------------------------------------------------------- */

/* Report an error to the user.  */
void ui_error(const char *format,...)
{
    char *str;
    va_list ap;
    static Widget error_dialog;
    static ui_button_t button;

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    error_dialog = build_error_dialog(_ui_top_level, &button, str);
    ui_popup(XtParent(error_dialog), _("VICE Error!"), False);
    button = UI_BUTTON_NONE;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_NONE);
    ui_popdown(XtParent(error_dialog));
    XtDestroyWidget(XtParent(error_dialog));
    ui_dispatch_events();
    vsync_suspend_speed_eval();
    lib_free(str);
}

/* Report a message to the user.  */
void ui_message(const char *format,...)
{
    char *str;
    va_list ap;
    static Widget error_dialog;
    static ui_button_t button;

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    error_dialog = build_error_dialog(_ui_top_level, &button, str);
    ui_popup(XtParent(error_dialog), "VICE", False);
    button = UI_BUTTON_NONE;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_NONE);
    ui_popdown(XtParent(error_dialog));
    ui_check_mouse_cursor();
    XtDestroyWidget(XtParent(error_dialog));
    ui_dispatch_events();
    vsync_suspend_speed_eval();
    lib_free(str);
}

/* Report a message to the user, allow different buttons. */
ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    char *str;
    va_list ap;
    static Widget jam_dialog, shell, tmp, mform, bbox;
    static ui_button_t button;

    if (console_mode) {
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        va_end(ap);
        exit(0);
    }

    shell = ui_create_transient_shell(_ui_top_level, "jamDialogShell");
    jam_dialog = XtVaCreateManagedWidget("jamDialog",
                                         panedWidgetClass, shell,
                                         NULL);
    mform = XtVaCreateManagedWidget("messageForm",
                                    formWidgetClass, jam_dialog,
                                    NULL);

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    tmp = XtVaCreateManagedWidget("label",
                                  labelWidgetClass, mform,
                                  XtNresize, False,
                                  XtNjustify, XtJustifyCenter,
                                  XtNlabel, str,
                                  NULL);

    bbox = XtVaCreateManagedWidget("buttonBox",
                                   boxWidgetClass, jam_dialog,
                                   XtNshowGrip, False,
                                   XtNskipAdjust, True,
                                   XtNorientation, XtorientHorizontal,
                                   NULL);

    tmp = XtVaCreateManagedWidget("resetButton",
                                  commandWidgetClass, bbox,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_RESET_callback, (XtPointer)&button);

    tmp = XtVaCreateManagedWidget("hardResetButton",
                                  commandWidgetClass, bbox,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_HARDRESET_callback, (XtPointer)&button);

    tmp = XtVaCreateManagedWidget("monButton",
                                  commandWidgetClass, bbox,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_MON_callback, (XtPointer)&button);

    tmp = XtVaCreateManagedWidget("noneButton",
                                  commandWidgetClass, bbox,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_NONE_callback, (XtPointer)&button);

    ui_popup(XtParent(jam_dialog), "VICE", False);
    button = UI_BUTTON_CANCEL;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_CANCEL);
    ui_popdown(XtParent(jam_dialog));
    XtDestroyWidget(XtParent(jam_dialog));

    vsync_suspend_speed_eval();
    ui_dispatch_events();
    lib_free(str);

    switch (button) {
        case UI_BUTTON_MON:
            ui_restore_mouse();
#ifdef HAVE_FULLSCREEN
            fullscreen_suspend(0);
#endif
            return UI_JAM_MONITOR;
        case UI_BUTTON_HARDRESET:
            return UI_JAM_HARD_RESET;
        case UI_BUTTON_NONE:
            return UI_JAM_NONE;
        case UI_BUTTON_RESET:
        default:
            return UI_JAM_RESET;
    }
}

int ui_extend_image_dialog(void)
{
    ui_button_t b;

    vsync_suspend_speed_eval();
    b = ui_ask_confirmation(_("Extend disk image"), ("Do you want to extend the disk image?"));
    return (b == UI_BUTTON_YES) ? 1 : 0;
}

/* this must be in sync with uilib_file_filter_enum_t */
static const char* file_filters[] = {
/* all */ "*",
/* palette */ "*.vpl",
/* snapshot */ "*.vsf",
/* disk */ "*.{[gpx]64,d{6[47],71,8[012],[124]m},g41,g71}",
/* tape */ "*.{t64,tap}",
/* cartridge */ "*.{crt,bin}",
/* crt filter */ "*.crt",
/* flip_list */ "*.vfl",
/* romset */ "*.vrs",
/* romset archive */ "*.vra",
/* keymap */ "*.vkm",
/* emulator_filter unused in X11 */ "",
/* wav */ "*.wav",
/* voc */ "*.voc",
/* iff */ "*.iff",
/* aiff */ "*.aiff",
/* mp3 */ "*.mp3",
/* flac */ "*.flac",
/* ogg/vorbis */ "*.ogg",
/* serial */ "ttyS*",
/* vic20cart */ "*.{prg,bin}",
/* sid */ "*.{psid,sid}",
/* dtvrom */ "*.bin",
/* compressed */ "*.*z*",
/* eth */ "eth*",
/* midi */ "mi*",
/* hd_image */ "*.{[hf]dd,iso,cfa}",
};

/* File browser. */
char *ui_select_file(const char *title, read_contents_func_type read_contents_func, unsigned int allow_autostart, const char *default_dir,
                     enum uilib_file_filter_enum_s* patterns, int num_patterns, ui_button_t *button_return, unsigned int show_preview,
                     int *attach_wp, ui_filechooser_t action)
{
    static ui_button_t button;
    char *ret;
    Widget file_selector = NULL;
    XfwfFileSelectorStatusStruct fs_status;
    char *current_dir;
    const char *pattern;
    int is_ok = 0;

    /* we preserve the current directory over the invocations */
    current_dir = ioutil_current_dir();    /* might be changed elsewhere */
    if (filesel_dir != NULL) {
        ioutil_chdir(filesel_dir);
    }

    /* We always rebuild the file selector from scratch (which is slow),
       because there seems to be a bug in the XfwfScrolledList that causes
       the file and directory listing to disappear randomly.  I hope this
       fixes the problem...  */
    file_selector = build_file_selector(_ui_top_level, &button);

    /* Strangely enough, even on UI_FD_LOAD, new filenames should be allowed */
    XtVaSetValues(file_selector, XtNcheckExistence, False, NULL);
    if (action == UI_FC_DIRECTORY) {
        XtVaSetValues(file_selector, XtNSelectDirectory, True, NULL);
    }
    if (patterns[0] < util_arraysize(file_filters)) {
        pattern = file_filters[patterns[0]];
    } else {
        pattern = file_filters[UILIB_FILTER_ALL];
    }
    XtVaSetValues(file_selector, XtNshowAutostartButton, allow_autostart,
                                 XtNshowContentsButton, read_contents_func ? 1 : 0,
                                 XtNpattern, pattern,
                                 NULL);
    if (attach_wp) {
        XtVaSetValues(file_selector, XtNshowReadOnlyToggle, True, NULL);
    }

    if (default_dir != NULL) {
        XfwfFileSelectorChangeDirectory((XfwfFileSelectorWidget)file_selector, default_dir);
    } else {
        char *newdir = ioutil_current_dir();

        XfwfFileSelectorChangeDirectory((XfwfFileSelectorWidget)file_selector, newdir);
        lib_free(newdir);
    }

    ui_popup(XtParent(file_selector), title, False);
    for (;;) {
        button = UI_BUTTON_NONE;
        while (button == UI_BUTTON_NONE) {
            ui_dispatch_next_event();
        }
        XfwfFileSelectorGetStatus((XfwfFileSelectorWidget)file_selector, &fs_status);
        if (fs_status.file_selected && button == UI_BUTTON_CONTENTS && read_contents_func != NULL) {
            image_contents_t *contents;
            char *f = util_concat(fs_status.path, fs_status.file, NULL);

            contents = read_contents_func(f);
            lib_free(f);
            if (contents != NULL) {
                char *buf, *tmp;
                image_contents_file_list_t *p;
                int buf_size;
                size_t max_buf_size;

                max_buf_size = 4096;
                buf = lib_malloc(max_buf_size);
                buf_size = 0;

                tmp = image_contents_to_string(contents, 1);

#define BUFCAT(s) util_bufcat((uint8_t *)buf, &buf_size, &max_buf_size, (uint8_t *)(s), strlen(s))

                BUFCAT(tmp);
                lib_free(tmp);

                if (contents->file_list == NULL) {
                    BUFCAT("\n(Empty image.)");
                }
                for (p = contents->file_list; p != NULL; p = p->next) {
                    BUFCAT("\n");
                    tmp = image_contents_file_to_string(p, 1);
                    BUFCAT(tmp);
                    lib_free(tmp);
                }
                if (contents->blocks_free >= 0) {
                    tmp = lib_msprintf("\n%d BLOCKS FREE.", contents->blocks_free);
                    BUFCAT(tmp);
                    lib_free(tmp);
                }

                ui_show_text(fs_status.file, buf, 250, 240);
                image_contents_destroy(contents);
                lib_free(buf);
            } else {
                ui_error(_("Unknown image type"));
            }
        }
        if (button == UI_BUTTON_CANCEL) {
            break;
        }
        if ((button == UI_BUTTON_OK || button == UI_BUTTON_AUTOSTART) /* &&
                (action != UI_FC_LOAD || fs_status.file_selected) */) {
            is_ok = 1;
            break;
        }
    } 

    /* `ret' gets always malloc'ed.  */
    if (is_ok) {
        ret = util_concat(fs_status.path, fs_status.file, NULL);
        if (attach_wp != NULL) {
            Boolean set;
            XtVaGetValues(file_selector, XtNreadOnly, &set, NULL);

            *attach_wp = set;
        }
    } else {
        ret = lib_stralloc("");
    }

    ui_popdown(XtParent(file_selector));
#ifndef __alpha
    /* On Alpha, XtDestroyWidget segfaults, don't know why...  */
    XtDestroyWidget(XtParent(file_selector));
#endif
    lib_free(filesel_dir);
    filesel_dir = ioutil_current_dir();
    if (current_dir != NULL) {
        ioutil_chdir(current_dir);
        lib_free(current_dir);
    }

    *button_return = button;
    if (is_ok) {
        /* Caller has to free the filename.  */
        return ret;
    } else {
        lib_free(ret);
        return NULL;
    }
}

/* prompt for a pathname, must allow to enter a non existing path */
ui_button_t ui_change_dir(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    char *filename;
    ui_button_t button;
    static char *last_dir;
    uilib_file_filter_enum_t filter = UILIB_FILTER_ALL;

    vsync_suspend_speed_eval();

    filename = ui_select_file(title, NULL, 0, last_dir, &filter, 1, &button, 0, NULL, UI_FC_DIRECTORY);

    switch (button) {
        case UI_BUTTON_OK:
            strncpy(buf, filename, buflen);
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    ui_update_menus();
    lib_free(filename);
    return button;
}


/* Ask for a string.  The user can confirm or cancel. */
ui_button_t ui_input_string(const char *title, const char *prompt, char *buf, unsigned int buflen)
{
    String str;
    static Widget input_dialog, input_dialog_label, input_dialog_field;
    static ui_button_t button;

    if (!input_dialog) {
        input_dialog = build_input_dialog(_ui_top_level, &button, &input_dialog_label, &input_dialog_field);
    }
    XtVaSetValues(input_dialog_label, XtNlabel, prompt, NULL);
    XtVaSetValues(input_dialog_field, XtNstring, buf, NULL);
    XtSetKeyboardFocus(input_dialog, input_dialog_field);
    ui_popup(XtParent(input_dialog), title, False);
    button = UI_BUTTON_NONE;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_NONE);
    XtVaGetValues(input_dialog_field, XtNstring, &str, NULL);
    strncpy(buf, str, buflen);
    ui_popdown(XtParent(input_dialog));
    return button;
}

/* Display a text to the user. */
void ui_show_text(const char *title, const char *text, int width, int height)
{
    static ui_button_t button;
    Widget show_text;

    show_text = build_show_text(_ui_top_level, &button, (String)text, width, height);
    ui_popup(XtParent(show_text), title, False);
    button = UI_BUTTON_NONE;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_NONE);
    ui_popdown(XtParent(show_text));
    XtDestroyWidget(XtParent(show_text));
}

/* Ask for a confirmation. */
ui_button_t ui_ask_confirmation(const char *title, const char *text)
{
    static Widget confirm_dialog, confirm_dialog_message;
    static ui_button_t button;

    if (!confirm_dialog) {
        confirm_dialog = build_confirm_dialog(_ui_top_level, &button, &confirm_dialog_message);
    }
    XtVaSetValues(confirm_dialog_message, XtNlabel, text, NULL);
    ui_popup(XtParent(confirm_dialog), title, False);
    button = UI_BUTTON_NONE;
    do {
        ui_dispatch_next_event();
    } while (button == UI_BUTTON_NONE);
    ui_popdown(XtParent(confirm_dialog));
    return button;
}

/* FIXME: make dialog with just "yes" and "no" buttons */
ui_button_t ui_ask_yesno(const char *title, const char *text)
{
    return ui_ask_confirmation(title, text);
}

/* Update the menu items with a checkmark according to the current resource
   values.  */
void ui_update_menus(void)
{
    ui_menu_update_all();
}

Widget ui_create_shell(Widget parent, const char *name, WidgetClass class)
{
    Widget w;

    w = XtVaCreatePopupShell(name, class, parent, XtNinput, True, NULL);

    XtVaSetValues(w, XtNvisual, visual, XtNdepth, depth, XtNcolormap, colormap, NULL);

    return w;
}

Widget ui_create_transient_shell(Widget parent, const char *name)
{
    return ui_create_shell(parent, name, transientShellWidgetClass);
}

static Widget get_last_visited_app_shell(void)
{
    Widget s = NULL;

    if (last_visited_app_shell) {
        s = last_visited_app_shell;
    } else {
        /* Choose one realized shell. */
        int i;
        for (i = 0; i < num_app_shells; i++) {
            if (XtIsRealized(app_shells[i].shell)) {
                s = app_shells[i].shell;
                break;
            }
        }
    }

    return s;
}

/* Pop up a popup shell and center it to the last visited AppShell */
void ui_popup(Widget w, const char *title, Boolean wait_popdown)
{
    Widget s = NULL;

#ifdef HAVE_FULLSCREEN
    fullscreen_suspend(1);
#endif

    ui_restore_mouse();
    /* Keep sure that we really know which was the last visited shell. */
    ui_dispatch_events();

    s = get_last_visited_app_shell();

    {
        /* Center the popup. */
        Dimension my_width, my_height, shell_width, shell_height;
        Dimension my_x, my_y;
        Position tlx, tly;
        int foo;
        unsigned int root_width, root_height, ufoo;
        Window foowin;

        XtRealizeWidget(w);
        XtVaGetValues(w, XtNwidth, &my_width, XtNheight, &my_height, NULL);

        /* Now make sure the whole widget is visible.  */
        XGetGeometry(display, RootWindow(display, screen), &foowin, &foo, &foo, &root_width, &root_height, &ufoo, &ufoo);

        if (s != NULL) {
            XtVaGetValues(s, XtNwidth, &shell_width, XtNheight, &shell_height, XtNx, &tlx, XtNy, &tly, NULL);
            XtTranslateCoords(XtParent(s), tlx, tly, &tlx, &tly);
            my_x = tlx + (shell_width - my_width) / 2;
            my_y = tly + (shell_height - my_height) / 2;

            /* FIXME: Is it really OK to cast to `signed short'?  */
            if ((signed short)my_x < 0) {
                my_x = 0;
            } else if ((unsigned int)((signed short)my_x + my_width) > root_width) {
                my_x = root_width - my_width;
            }

            if ((signed short)my_y < 0) {
                my_y = 0;
            } else if ((unsigned int)((signed short)my_y + my_height) > root_height) {
                my_y = root_height - my_height;
            }
        } else {
            /* We don't have an AppWindow to refer to: center to the root
               window.  */
            my_x = (root_width - my_width) / 2;
            my_y = (root_height - my_height) / 2;
        }

        XtVaSetValues(w, XtNx, my_x, XtNy, my_y, NULL);
    }
    XtVaSetValues(w, XtNtitle, title, NULL);
    XtPopup(w, XtGrabExclusive);
    XSetWMProtocols(display, XtWindow(w), &wm_delete_window, 1);

    /* If requested, wait for this widget to be popped down before
       returning. */
    if (wait_popdown) {
        int oldcnt = popped_up_count++;
        while (oldcnt != popped_up_count) {
            ui_dispatch_next_event();
        }
    } else {
        popped_up_count++;
    }
}

/* Pop down a popup shell. */
void ui_popdown(Widget w)
{
    XtPopdown(w);
    ui_check_mouse_cursor();
    if (--popped_up_count < 0) {
        popped_up_count = 0;
    }

#ifdef HAVE_FULLSCREEN
    fullscreen_resume();
#endif
}

/* ------------------------------------------------------------------------- */

/* These functions build all the widgets. */

static Widget build_file_selector(Widget parent, ui_button_t * button_return)
{
    Widget shell = ui_create_transient_shell(parent, "fileSelectorShell");
    Widget file_selector = XtVaCreateManagedWidget("fileSelector",
                                                   xfwfFileSelectorWidgetClass,
                                                   shell,
                                                   XtNflagLinks, True,
                                                   NULL);

    XtAddCallback((Widget)file_selector, XtNokButtonCallback, UI_BUTTON_OK_callback, (XtPointer)button_return);
    XtAddCallback((Widget)file_selector, XtNcancelButtonCallback, UI_BUTTON_CANCEL_callback, (XtPointer)button_return);
    XtAddCallback((Widget)file_selector, XtNcontentsButtonCallback, UI_BUTTON_CONTENTS_callback, (XtPointer)button_return);
    XtAddCallback((Widget)file_selector, XtNautostartButtonCallback, UI_BUTTON_AUTOSTART_callback, (XtPointer)button_return);
    return file_selector;
}

static Widget build_error_dialog(Widget parent, ui_button_t * button_return, const String message)
{
    Widget shell, ErrorDialog, tmp;

    shell = ui_create_transient_shell(parent, "errorDialogShell");
    ErrorDialog = XtVaCreateManagedWidget("errorDialog",
                                          panedWidgetClass, shell,
                                          NULL);
    tmp = XtVaCreateManagedWidget("messageForm",
                                  formWidgetClass, ErrorDialog,
                                  NULL);
    tmp = XtVaCreateManagedWidget("label",
                                  labelWidgetClass, tmp,
                                  XtNresize, False,
                                  XtNjustify, XtJustifyCenter,
                                  XtNlabel, message,
                                  NULL);
    tmp = XtVaCreateManagedWidget("buttonBox",
                                  boxWidgetClass, ErrorDialog,
                                  XtNshowGrip, False,
                                  XtNskipAdjust, True,
                                  XtNorientation, XtorientHorizontal,
                                  NULL);
    tmp = XtVaCreateManagedWidget("closeButton",
                                  commandWidgetClass, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_CLOSE_callback, (XtPointer)button_return);
    return ErrorDialog;
}

static Widget build_input_dialog(Widget parent, ui_button_t * button_return, Widget *input_dialog_label, Widget *input_dialog_field)
{
    Widget shell, input_dialog, tmp1, tmp2;

    shell = ui_create_transient_shell(parent, "inputDialogShell");
    input_dialog = XtVaCreateManagedWidget("inputDialog",
                                           panedWidgetClass, shell,
                                           NULL);
    tmp1 = XtVaCreateManagedWidget("inputForm",
                                   formWidgetClass, input_dialog,
                                   NULL);
    *input_dialog_label = XtVaCreateManagedWidget("label",
                                                  labelWidgetClass, tmp1,
                                                  XtNresize, False,
                                                  XtNjustify, XtJustifyLeft,
                                                  NULL);
    *input_dialog_field = XtVaCreateManagedWidget("field",
                                                  textfieldWidgetClass, tmp1,
                                                  XtNfromVert, *input_dialog_label,
                                                  NULL);
    XtAddCallback(*input_dialog_field, XtNactivateCallback, UI_BUTTON_OK_callback, (XtPointer)button_return);
    tmp1 = XtVaCreateManagedWidget("buttonBox",
                                   boxWidgetClass, input_dialog,
                                   XtNshowGrip, False,
                                   XtNskipAdjust, True,
                                   XtNorientation, XtorientHorizontal,
                                   NULL);
    tmp2 = XtVaCreateManagedWidget("okButton",
                                   commandWidgetClass, tmp1,
                                   NULL);
    XtAddCallback(tmp2, XtNcallback, UI_BUTTON_OK_callback, (XtPointer) button_return);
    tmp2 = XtVaCreateManagedWidget("cancelButton",
                                   commandWidgetClass, tmp1,
                                   XtNfromHoriz, tmp2,
                                   NULL);
    XtAddCallback(tmp2, XtNcallback, UI_BUTTON_CANCEL_callback, (XtPointer) button_return);
    return input_dialog;
}

static Widget build_show_text(Widget parent, ui_button_t * button_return, const String text, int width, int height)
{
    Widget shell, tmp;
    Widget show_text;

    shell = ui_create_transient_shell(parent, "showTextShell");
    show_text = XtVaCreateManagedWidget("showText",
                                        panedWidgetClass, shell,
                                        NULL);
    tmp = XtVaCreateManagedWidget("textBox",
                                  formWidgetClass, show_text,
                                  NULL);
    tmp = XtVaCreateManagedWidget("text",
                                  asciiTextWidgetClass, tmp,
                                  XtNtype, XawAsciiString,
                                  XtNeditType, XawtextRead,
                                  XtNscrollVertical, XawtextScrollWhenNeeded,
                                  XtNdisplayCaret, False,
                                  XtNstring, text,
                                  XtNwrap, XawtextWrapWord,
                                  NULL);
    if (width > 0) {
        XtVaSetValues(tmp, XtNwidth, (Dimension)width, NULL);
    }
    if (height > 0) {
        XtVaSetValues(tmp, XtNheight, (Dimension)height, NULL);
    }
    tmp = XtVaCreateManagedWidget("buttonBox",
                                  boxWidgetClass, show_text,
                                  XtNshowGrip, False,
                                  XtNskipAdjust, True,
                                  XtNorientation, XtorientHorizontal,
                                  NULL);
    tmp = XtVaCreateManagedWidget("closeButton",
                                  commandWidgetClass, tmp,
                                  NULL);
    XtAddCallback(tmp, XtNcallback, UI_BUTTON_CLOSE_callback, (XtPointer)button_return);
    return show_text;
}

static Widget build_confirm_dialog(Widget parent, ui_button_t *button_return, Widget *confirm_dialog_message)
{
    Widget shell, confirm_dialog, tmp1, tmp2;

    shell = ui_create_transient_shell(parent, "confirmDialogShell");
    confirm_dialog = XtVaCreateManagedWidget("confirmDialog",
                                             panedWidgetClass, shell,
                                             NULL);
    tmp1 = XtVaCreateManagedWidget("messageForm",
                                   formWidgetClass, confirm_dialog,
                                   NULL);
    *confirm_dialog_message = XtVaCreateManagedWidget("message",
                                                      labelWidgetClass, tmp1,
                                                      XtNjustify, XtJustifyCenter,
                                                      NULL);
    tmp1 = XtVaCreateManagedWidget("buttonBox",
                                   boxWidgetClass, confirm_dialog,
                                   XtNshowGrip, False,
                                   XtNskipAdjust, True,
                                   XtNorientation, XtorientHorizontal,
                                   NULL);
    tmp2 = XtVaCreateManagedWidget("yesButton",
                                   commandWidgetClass, tmp1,
                                   NULL);
    XtAddCallback(tmp2, XtNcallback, UI_BUTTON_YES_callback, (XtPointer)button_return);
    tmp2 = XtVaCreateManagedWidget("noButton",
                                   commandWidgetClass, tmp1,
                                   NULL);
    XtAddCallback(tmp2, XtNcallback, UI_BUTTON_NO_callback, (XtPointer)button_return);
    tmp2 = XtVaCreateManagedWidget("cancelButton",
                                   commandWidgetClass, tmp1,
                                   NULL);
    XtAddCallback(tmp2, XtNcallback, UI_BUTTON_CANCEL_callback, (XtPointer)button_return);
    return confirm_dialog;
}

/* ------------------------------------------------------------------------- */

/* Miscellaneous callbacks.  */

static
UI_CALLBACK(enter_window_callback_shell)
{
    video_canvas_t *video_canvas = (video_canvas_t *)client_data;

    last_visited_app_shell = w;
    last_visited_canvas = video_canvas->emuwindow;   /* keep global up to date */
    ui_cached_video_canvas = video_canvas;
    xaw_lightpen_update_canvas(video_canvas, TRUE);
}

/*
 * Structure Notify (mapping, unmapping, configure).
 * This callback appears to do nothing, but the mapping events are wanted
 * in wait_for_deiconify().
 */
static
UI_CALLBACK(structure_callback_shell)
{
#if defined(HAVE_XVIDEO)
    XEvent *event = (XEvent *)call_data;

    if (event->xany.type == ConfigureNotify) {
        video_canvas_t *canvas = (video_canvas_t *)client_data;
        /*
         * XVideo must be refreshed when the shell window is moved.
         * Actually, that doesn't seems to be needed for textured video,
         * only for overlay.
         */
        if (canvas->videoconfig->hwscale && canvas->xv_image && canvas->is_overlay) {
            video_canvas_refresh_all(canvas);
        }
    }
#endif
}

/*
 * Exposure (expose) and Structure Notify (mapping, unmapping, configure),
 * except that the canvas isn't a top-level window and therefore doesn't
 * get the (un)mapping notifications.
 */
static
UI_CALLBACK(exposure_callback_canvas)
{
    XEvent *event = (XEvent *)call_data;
    video_canvas_t *canvas = (video_canvas_t *)client_data;

    if (!canvas) {
        return;
    }

    if (event->xany.type == Expose && event->xexpose.count == 0) {
        video_canvas_refresh_all(canvas);
    }
}

static
UI_CALLBACK(structure_callback_canvas)
{
    XEvent *event = (XEvent *)call_data;
    video_canvas_t *canvas = (video_canvas_t *)client_data;

    if (!canvas) {
        return;
    }

    if (event->xany.type == ConfigureNotify) {
#ifdef HAVE_XVIDEO
        /* No resize for XVideo. */
        if (canvas->videoconfig->hwscale && canvas->xv_image) {
            video_canvas_refresh_all(canvas);
        }
        else
#endif
        {
            Dimension width, height;
            draw_buffer_t *b = canvas->draw_buffer;

            XtVaGetValues(w, XtNwidth, (XtPointer)&width,
                             XtNheight, (XtPointer)&height,
                             NULL);

            if (width != b->canvas_physical_width ||
                height != b->canvas_physical_height) {
                b->canvas_physical_width = width;
                b->canvas_physical_height = height;

                video_viewport_resize(canvas, 0);
            }
        }
    }
}

/* FIXME: this does not handle multiple application shells. */
static void close_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    vsync_suspend_speed_eval();

    ui_exit();
}

