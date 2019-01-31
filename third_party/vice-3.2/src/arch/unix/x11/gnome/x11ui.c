/*
 * ui.c - Gnome/Gtk+ based UI
 *
 * Written by
 *  Ettore Perazzoli
 *  Oliver Schaertel
 *  pottendo <pottendo@gmx.net>
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

/* #define DEBUG_X11UI */

#define _UI_C /* WTH is this? */

#include "vice.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pango/pango.h>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

#include "ui.h"
#include "uiapi.h"
#include "uiarch.h"

#ifdef HAVE_HWSCALE
#if !GTK_CHECK_VERSION(3, 0, 0)
/* in gtk3 we should not include these directly */
#include <gtk/gtkgl.h>
#include <gdk/gdkgl.h>
#endif
#include <GL/gl.h>
#endif

#include "charset.h"
#include "dragdrop.h"
#include "fullscreenarch.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "keyboard.h"
#include "resources.h"
#include "types.h"
#include "uicolor.h"
#include "uimenu.h"
#include "uipalcontrol.h"
#include "uisettings.h"
#include "uicommands.h"
#include "uicontents.h"
#include "uifilechooser.h"
#include "uifileentry.h"
#include "uilib.h"
#include "uistatusbar.h"
#include "uitapestatus.h"
#include "uidrivestatus.h"
#include "uijoystatus.h"
#include "util.h"
#include "version.h"
#include "vsync.h"
#include "video.h"
#include "videoarch.h"
#include "vsidui.h"
#include "vsiduiunix.h"
#include "lightpendrv.h"
#include "x11mouse.h"
#include "focus.h"
#include "gnomekbd.h"
#ifdef USE_UI_THREADS
#include "ui-threads.h"
#endif

#if defined(USE_XF86_EXTENSIONS) || !defined(HAVE_VTE)
#include <gdk/gdkx.h>
#endif

#include "x11ui.h"

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

#ifdef USE_XF86_EXTENSIONS
static Display *display;
int screen;
static int depth;
#endif

/******************************************************************************/

/* UI logging goes here.  */
log_t ui_log = LOG_ERR;

/* the minimum (and initial) size of the VSID window */
#define VSID_WINDOW_MINW     (400)
#define VSID_WINDOW_MINH     (300)
/* minimum size of regular emulator window */
#define WINDOW_MINW     (320 / 2 + 1)
#define WINDOW_MINH     (200 / 2 + 1)

/* FIXME: perhaps also move these into app_shell_type */
int have_cbm_font = FALSE;
char *fixedfontname="CBM 10";
#ifdef HAVE_PANGO
PangoFontDescription *fixed_font_desc;
#endif

static int popped_up_count = 0;

#if !defined(HAVE_CAIRO)
/* Toplevel widget. */
static GdkGC *app_gc = NULL;
#endif

/* GdkColormap *colormap; */

app_shell_type app_shells[MAX_APP_SHELLS]; /* FIXME: we want this to be the exclusive global info */
static unsigned int num_app_shells = 0;
static unsigned int active_shell = 0;

/******************************************************************************/

static gboolean enter_window_callback(GtkWidget *w, GdkEvent *e, gpointer p);
static gboolean leave_window_callback(GtkWidget *w, GdkEvent *e, gpointer p);
static gboolean configure_callback_app(GtkWidget *w, GdkEvent *e, gpointer p);
static gboolean configure_callback_canvas(GtkWidget *w, GdkEvent *e, gpointer p);
static gboolean exposure_callback_canvas(GtkWidget *w, GdkEventExpose *e, gpointer p);
static gboolean map_callback(GtkWidget *widget, GdkEvent  *event, gpointer user_data);
static gboolean update_menu_callback(GtkWidget *w, GdkEvent *event,gpointer data);

static void toggle_aspect(video_canvas_t *canvas);
static void setup_aspect(video_canvas_t *canvas);
static void setup_aspect_geo(video_canvas_t *canvas, int winw, int winh);
static gfloat get_aspect(video_canvas_t *canvas);

void ui_trigger_resize(void);

/******************************************************************************/
#if !defined(HAVE_CAIRO)
GdkGC *get_toplevel(void)
{
    return app_gc;
}
#endif

int get_num_shells(void)
{
    return num_app_shells;
}

static void set_active_shell(unsigned int shell)
{
    DBG(("set_active_shell (%d)", shell));
    if (shell >= num_app_shells) {
        log_error(ui_log, "set_active_shell: bad params (%d)", shell);
        return;
    }
    active_shell = shell;
}
int get_active_shell(void)
{
    return active_shell;
}
GtkWidget *get_active_toplevel(void)
{
    unsigned int key = get_active_shell();
    if (app_shells[key].shell) {
        return gtk_widget_get_toplevel(app_shells[key].shell);
    }
    return NULL;
}

video_canvas_t *get_active_canvas(void)
{
    unsigned int key = get_active_shell();
    return app_shells[key].canvas;
}

/******************************************************************************/

static char windowres[4][14] = { "Window0Xpos", "Window0Ypos", "Window0Width", "Window0height" };

static void set_window_resources(video_canvas_t *canvas, int x, int y, int w, int h)
{
    int i;

    if ((canvas == NULL) || (x < 0) || (y < 0) || (w < WINDOW_MINW) || (h < WINDOW_MINH)) {
        return;
    }

    DBG(("set_window_resources (%d) x:%d y:%d w:%d h:%d", canvas->app_shell, x, y, w, h));
    if ((canvas->app_shell >= num_app_shells) || (canvas != app_shells[canvas->app_shell].canvas)) {
        log_error(ui_log, "set_window_resources: bad params");
        return;
    }

    for (i = 0; i < 4; i++) {
        windowres[i][6] = '0' + canvas->app_shell;
    }
    resources_set_int(windowres[0], x);
    resources_set_int(windowres[1], y);
    resources_set_int(windowres[2], w);
    resources_set_int(windowres[3], h);
}

static void get_window_resources(video_canvas_t *canvas, int *x, int *y, int *w, int *h)
{
    int i;

    if ((canvas == NULL) || (x == NULL) || (y == NULL) || (w == NULL) || (h == NULL)) {
        return;
    }

    if ((canvas->app_shell >= num_app_shells) || (canvas != app_shells[canvas->app_shell].canvas)) {
        log_error(ui_log, "get_window_resources: bad params");
        return;
    }

    for (i = 0; i < 4; i++) {
        windowres[i][6] = '0' + canvas->app_shell;
    }
    resources_get_int(windowres[0], x);
    resources_get_int(windowres[1], y);
    resources_get_int(windowres[2], w);
    resources_get_int(windowres[3], h);
    DBG(("get_window_resources x:%d y:%d w:%d h:%d", *x, *y, *w, *h));
}

/******************************************************************************/
/*
    transfer focus to the monitor ui window

    note: the "focus stealing prevention" feature of eg KDE must be disabled or
          all this will not work at all.

    for the time being, this is a kindof nasty hack which is needed because the
    monitor is not actually running in a "real" gui window (but in a terminal
    window instead). once a real monitor gui is implemented it can be replaced
    by the proper gtk function(s) :)
*/

#if !defined(HAVE_VTE)
int ui_focus_monitor(void)
{
    int i;
    Display *disp;

    DBG(("uimon_grab_focus"));

    if (!(disp = XOpenDisplay(NULL))) {
        log_error(ui_log, "uimon_grab_focus: no display");
        return -1;
    }

    i = ui_focus_terminal(disp, ui_log);

    XCloseDisplay(disp);

    return i;
}

/*
    Same as the GTK variant below, but without using the GTK focusing stuff.

    That's because the GTK one seemd to give back focus to the main window
    (according to window decorations), but still all keyboard input went to
    the terminal instead.

    So I use the X version instead. If it worked for the terminal focusing
    it works for the main window focusing. And sure it does ;)
*/

/* Might be needed for gtk3. */
#if GTK_CHECK_VERSION(3, 0, 0)
#define GDK_DRAWABLE_XID GDK_WINDOW_XID
#endif

void ui_restore_focus(void)
{
    GtkWidget *widget = get_active_toplevel();
    GdkWindow *window;
    Display *disp;

    window = widget ? gtk_widget_get_window(widget) : NULL;
    DBG(("ui_restore_focus %p:%p", window, widget));

    if (!(disp = XOpenDisplay(NULL))) {
        log_error(ui_log, "uimon_grab_focus: no display");
        return;
    }

    if (window) {
        ui_focus_window(disp, GDK_DRAWABLE_XID(window));
    }

    XCloseDisplay(disp);
}
#else
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
    GtkWidget *widget = get_active_toplevel();
    GdkWindow *window;

    window = widget ? gtk_widget_get_window(widget) : NULL;
    DBG(("ui_restore_focus %p:%p", window, widget));
    if (window) {
        ui_dispatch_events();
        gdk_flush();
        ui_unblock_shells();
        /* yes it looks weird, and it is. GTK sucks */
        gdk_window_raise(window);
        gdk_window_show(window);
        gtk_window_present(GTK_WINDOW(widget));
        gdk_window_focus(window, GDK_CURRENT_TIME);
    }
}
#endif

/******************************************************************************/

static void atexit_handler(void)
{
    /* ui_autorepeat_on(); */
}

#ifdef HAVE_HWSCALE
/* GL_TEXTURE_RECTANGLE is standardised as _EXT in OpenGL 1.4. Here's some
 * aliases in the meantime. */
#ifndef GL_TEXTURE_RECTANGLE_EXT
#if defined(GL_TEXTURE_RECTANGLE_NV)
#define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_RECTANGLE_NV
#elif defined(GL_TEXTURE_RECTANGLE_ARB)
#define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_RECTANGLE_ARB
#else
#error "Your headers do not supply GL_TEXTURE_RECTANGLE. Disable HWSCALE and try again."
#endif
#endif
#endif /* HAVE_HWSCALE */

#ifdef USE_UI_THREADS
int ui_init(int *argc, char **argv)
{
    dthread_init();

    return dthread_ui_init(argc, argv);
}

/* Initialize the GUI and parse the command line. */
int ui_init2(int *argc, char **argv)
#else
int ui_init(int *argc, char **argv)
#endif        /* USE_UI_THREADS */
{
    if (console_mode) {
        return 0;
    }

#ifdef USE_UI_THREADS
    XInitThreads();
    /* init threads */
    /* gdk_threads_init(); */ /* commented out, as it causes troubles on NetBSD */
#endif

    gtk_init(argc, &argv);

#ifdef DEBUG_X11UI
    {
        int i;
        for (i = 1; i < *argc; i++) {
            DBG(("arg %d:%s", i, argv[i]));
        }
    }
#endif

#ifdef HAVE_HWSCALE
    if (gtk_gl_init_check(argc, &argv) == TRUE) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable (GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glTexEnvi(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glEnable(GL_BLEND);
    }

#endif

#ifdef DEBUG_X11UI
    {
        int i;
        for (i = 1; i < *argc; i++) {
            DBG(("arg %d:%s\n", i, argv[i]));
        }
    }
#endif

#ifdef USE_XF86_EXTENSIONS
    display = gdk_x11_get_default_xdisplay();
    depth = gdk_visual_get_depth(gdk_visual_get_system());
    screen = gdk_screen_get_number(gdk_screen_get_default());
#endif

    atexit(atexit_handler);

    ui_common_init();
    return 0;
}

#ifdef USE_UI_THREADS
int ui_init_finish()
{
    return dthread_ui_init_finish();
}

/* Continue GUI initialization after resources are set. */
int ui_init_finish2(void)
#else
int ui_init_finish()
#endif /* USE_UI_THREADS */
{
#ifdef HAVE_CAIRO
    char *usecairo = "yes";
#else
    char *usecairo = "no";
#endif
#ifdef HAVE_PANGO
    char *usepango = "yes";
#else
    char *usepango = "no";
#endif
#ifdef HAVE_VTE
    char *usevte = "yes";
#else
    char *usevte = "no";
#endif
#ifdef HAVE_HWSCALE
    char *usegl = "yes";
#else
    char *usegl = "no";
#endif
#ifdef HAVE_FULLSCREEN
    char *usefs = "yes";
#else
    char *usefs = "no";
#endif
#ifdef USE_XF86_EXTENSIONS
    char *usexf86ext = "yes";
#else
    char *usexf86ext = "no";
#endif
#ifdef USE_UI_THREADS
    char *useuithreads = "yes";
#else
    char *useuithreads = "no";
#endif
    ui_log = log_open("X11");

    log_message(ui_log, "GTK version compiled with: %d.%d (xf86 ext:%s cairo:%s pango:%s VTE:%s hwscale:%s fullscreen:%s ui-threads:%s)", GTK_MAJOR_VERSION, GTK_MINOR_VERSION, usexf86ext, usecairo, usepango, usevte, usegl, usefs, useuithreads);

#ifdef HAVE_PANGO
    have_cbm_font = TRUE;
    fixed_font_desc = pango_font_description_from_string(fixedfontname);
    if (!fixed_font_desc) {
        log_warning(ui_log, "Cannot load CBM font %s.", fixedfontname);
        have_cbm_font = FALSE;
    }
#endif

#ifdef HAVE_FULLSCREEN
    if (fullscreen_init() != 0) {
        log_warning(ui_log, "Some fullscreen devices aren't initialized properly.");
    }
#endif
    return ui_menu_init();
}

int ui_init_finalize(void)
{
    ui_check_mouse_cursor();
    return 0;
}

/******************************************************************************/

/*
 * FIXME: although the OS does ofcourse take care of it, we should still cleanly
 *        free all memory allocated by lib_.. functions, since that will make
 *        tracking down actual memory leaks using --enable-debug much easier.
 *
 * TODO:  menu structures (uimenu.c:222,224,256)
 */
void ui_shutdown(void)
{
    unsigned int i;
    ui_common_shutdown();
    ui_menu_shutdown();
    for (i = 0; i < num_app_shells; i++) {
        lib_free(app_shells[i].title);
        if (machine_class == VICE_MACHINE_VSID) {
            shutdown_vsid_ctrl_widget();
        }
        shutdown_pal_ctrl_widget(app_shells[i].pal_ctrl, app_shells[i].pal_ctrl_data);
    }
    shutdown_file_selector();
}

/* exit the application */
static gboolean delete_event_callback(GtkWidget *w, GdkEvent *e, gpointer data)
{
    vsync_suspend_speed_eval();
    ui_exit();
    /* ui_exit() will exit the application if user allows it. So if
       we return here then we should keep going => return TRUE */
    return TRUE;
}

/******************************************************************************/

#ifdef USE_XF86_EXTENSIONS
int x11ui_get_display_depth(void)
{
    return depth;
}

Display *x11ui_get_display_ptr(void)
{
    return display;
}

Window x11ui_get_X11_window(void)
{
    GdkWindow *window = gtk_widget_get_window(get_active_toplevel());
    if (window) {
        return GDK_WINDOW_XID(window);
    } else {
        return 0;
    }
}

int x11ui_get_screen(void)
{
    return screen;
}
#endif

/******************************************************************************/

#ifdef USE_UI_THREADS
static void build_screen_canvas_widget(video_canvas_t *c)
{

    dthread_build_screen_canvas(c);
    return;
}

void build_screen_canvas_widget2(video_canvas_t *c)
#else
static void build_screen_canvas_widget(video_canvas_t *c)
#endif /* USE_UI_THREADS */
{
    GtkWidget *new_canvas = gtk_drawing_area_new();

    DBG(("build_screen_canvas_widget %p", c));

    /* if the eventbox already has a child, get rid of it, we are resizing */
    GtkWidget *kid = gtk_bin_get_child(GTK_BIN(c->pane));
    if (kid != NULL) {
        gtk_container_remove(GTK_CONTAINER(c->pane), kid);
    }

#ifdef HAVE_HWSCALE
    if (c->videoconfig->hwscale) {
        GdkGLConfig *gl_config = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DOUBLE);

        if (gl_config == NULL) {
            log_warning(ui_log, "HW scaling will not be available");
            c->videoconfig->hwscale = 0;
            resources_set_int("HwScalePossible", 0);
        } else {
            if (!gtk_widget_set_gl_capability(GTK_WIDGET(new_canvas), gl_config, NULL, TRUE, GDK_GL_RGBA_TYPE)) {
                g_critical("Failed to add gl capability");
            }
        }
    }
#endif
    /* supress events, add mask later */
    gtk_widget_set_events(new_canvas, 0);

    /* XVideo must be refreshed when the application window is moved. */
    if (c->draw_buffer->canvas_physical_width) { /* HACK: do not connect events to initial dummy canvas */
        DBG(("build_screen_canvas_widget resizing to %d,%d", c->draw_buffer->canvas_physical_width, c->draw_buffer->canvas_physical_height));
        g_signal_connect(G_OBJECT(new_canvas), "configure-event", G_CALLBACK(configure_callback_canvas), (void*)c);
#if GTK_CHECK_VERSION(3, 0, 0)
        g_signal_connect(G_OBJECT(new_canvas), "draw", G_CALLBACK(exposure_callback_canvas), (void*)c);
#else
        g_signal_connect(G_OBJECT(new_canvas), "expose-event", G_CALLBACK(exposure_callback_canvas), (void*)c);
#endif
        g_signal_connect(G_OBJECT(new_canvas), "enter-notify-event", G_CALLBACK(enter_window_callback), (void *)c);
        g_signal_connect(G_OBJECT(new_canvas), "focus-in-event", G_CALLBACK(enter_window_callback), (void *) c);
        g_signal_connect(G_OBJECT(new_canvas), "visibility-notify-event", G_CALLBACK(enter_window_callback), (void *) c);
        g_signal_connect(G_OBJECT(new_canvas), "leave-notify-event", G_CALLBACK(leave_window_callback), (void *)c);
        g_signal_connect(G_OBJECT(new_canvas), "focus-out-event", G_CALLBACK(leave_window_callback), (void *)c);
        g_signal_connect(G_OBJECT(new_canvas), "map-event", G_CALLBACK(map_callback), NULL);
        mouse_connect_handler(new_canvas, (void*)c);
        kbd_connect_handler(new_canvas, (void*)c);
    }

    if (c->videoconfig->hwscale) {
        /* For hwscale, it's a feature that new_canvas must bloat to 100% size
         * of the containing GtkEventWindow. Unfortunately, for the other
         * path, it's a PITA. */
        gtk_container_add(GTK_CONTAINER(c->pane), new_canvas);
    } else {
        /* Believe it or not, but to get a gtkdrawingarea of fixed dimensions
         * with a black background within our layout vbox requires this:
         *
         * toplvl < ui   < black bg < centering < SCREEN HERE
         * window < vbox < eventbox < hbox<vbox < drawingarea.
         *
         * We do this to make fullscreen work. More gory details in the
         * x11ui_fullscreen about how "nice" that is to get to work. */
        GtkWidget *canvascontainer1 = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_widget_show(canvascontainer1);
        gtk_container_add(GTK_CONTAINER(c->pane), canvascontainer1);
        GtkWidget *canvascontainer2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_widget_show(canvascontainer2);
        gtk_box_pack_start(GTK_BOX(canvascontainer1), canvascontainer2, TRUE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(canvascontainer2), new_canvas, TRUE, FALSE, 0);
    }

    gtk_widget_show(new_canvas);
    gtk_widget_set_can_focus(new_canvas, TRUE);
    gtk_widget_grab_focus(new_canvas);
    c->emuwindow = new_canvas;
/*
explicitly setup the events we want to handle. the following are the remaining
events that are NOT handled:

GDK_BUTTON_MOTION_MASK         GDK_MOTION_NOTIFY (while a button is pressed)
GDK_BUTTON1_MOTION_MASK        GDK_MOTION_NOTIFY (while button 1 is pressed)
GDK_BUTTON2_MOTION_MASK        GDK_MOTION_NOTIFY (while button 2 is pressed)
GDK_BUTTON3_MOTION_MASK        GDK_MOTION_NOTIFY (while button 3 is pressed)
GDK_PROPERTY_CHANGE_MASK       GDK_PROPERTY_NOTIFY
GDK_PROXIMITY_IN_MASK          GDK_PROXIMITY_IN
GDK_PROXIMITY_OUT_MASK         GDK_PROXIMITY_OUT
GDK_SUBSTRUCTURE_MASK          Receive  GDK_STRUCTURE_MASK events for child windows
*/
    if (c->draw_buffer->canvas_physical_width) { /* HACK: do not connect events to initial dummy canvas */
        gtk_widget_add_events(new_canvas,
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_KEY_PRESS_MASK |
                            GDK_KEY_RELEASE_MASK |
                            GDK_FOCUS_CHANGE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_STRUCTURE_MASK |
                            GDK_VISIBILITY_NOTIFY_MASK |
                            GDK_EXPOSURE_MASK);
    }
}

static void get_initial_window_geo(video_canvas_t *canvas, int *x, int *y, int *w, int *h)
{
    gint window_width, window_height, window_xpos, window_ypos;

    get_window_resources(canvas, &window_xpos, &window_ypos, &window_width, &window_height);
    window_xpos = (window_xpos < 0) ? 0 : window_xpos;
    window_ypos = (window_ypos < 0) ? 0 : window_ypos;
    if (machine_class == VICE_MACHINE_VSID) {
        window_width = VSID_WINDOW_MINW;
        window_height = VSID_WINDOW_MINH;
    } else {
        if ((window_width < WINDOW_MINW) || (window_height < WINDOW_MINH)) {
            /* FIXME: use proper defaults here */
            window_width = WINDOW_MINW-1;
            window_height = WINDOW_MINH-1;
            /* not initialized yet?
            window_width = canvas->draw_buffer->canvas_physical_width;
            window_height = canvas->draw_buffer->canvas_physical_height;
            */
        }
        /* FIXME: adjust size according to actually used settings */
    }

    *x = window_xpos;
    *y = window_ypos;
    *w = window_width;
    *h = window_height;
    DBG(("get_initial_window_geo (winx: %d winy: %d winw: %d winh: %d)", *x, *y, *w, *h));
}


/* Create a shell with a canvas widget in it.
   called from arch/unix/gui/vsidui.c:vsid_ui_init (vsid) or
               arch/unix/x11/gnome/gnomevideo.c:video_canvas_create (other)
 */

#ifdef USE_UI_THREADS
int ui_open_canvas_window(video_canvas_t *c, const char *title, int w, int h, int no_autorepeat)
{
    return dthread_ui_open_canvas_window(c, title, w, h, no_autorepeat);
}

int ui_open_canvas_window2(video_canvas_t *c, const char *title, int w, int h, int no_autorepeat)
#else
int ui_open_canvas_window(video_canvas_t *c, const char *title, int w, int h, int no_autorepeat)
#endif
{
    GtkWidget *new_window, *topmenu, *panelcontainer, *pal_ctrl_widget = NULL;
    GtkAccelGroup* accel;
    GdkColor black = { 0, 0, 0, 255 };
    gint window_width, window_height, window_xpos, window_ypos;

    DBG(("ui_open_canvas_window %p (w: %d h: %d)", c, w, h));

    if (++num_app_shells > MAX_APP_SHELLS) {
        log_error(ui_log, "Maximum number of toplevel windows reached.");
        return -1;
    }
    memset(&app_shells[num_app_shells - 1], 0, sizeof(app_shell_type));

    sleep(1);
    new_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* supress events, add mask later */
    gtk_widget_set_events(new_window, 0);

    set_active_shell(num_app_shells - 1);

    app_shells[num_app_shells - 1].shell = new_window;
    app_shells[num_app_shells - 1].canvas = c;
    c->app_shell = num_app_shells - 1;

    g_signal_connect(G_OBJECT(new_window), "configure-event", G_CALLBACK(configure_callback_app), (void*)c);
    g_signal_connect(G_OBJECT(new_window), "delete_event", G_CALLBACK(delete_event_callback), NULL);
    g_signal_connect(G_OBJECT(new_window), "destroy_event", G_CALLBACK(delete_event_callback), NULL);
    kbd_connect_enterleave_handler(new_window, NULL);

    set_drop_target_widget(new_window);

    accel = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(new_window), accel);
    app_shells[num_app_shells - 1].accel = accel;

    app_shells[num_app_shells - 1].title = lib_stralloc(title);
    gtk_window_set_title(GTK_WINDOW(new_window), title);

    /* create all widgets for this window */
    panelcontainer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(new_window), panelcontainer);
    gtk_widget_show(panelcontainer);

    /* create the menu bar */
    topmenu = gtk_menu_bar_new();
    gtk_widget_show(topmenu);
    g_signal_connect(G_OBJECT(topmenu), "button-press-event", G_CALLBACK(update_menu_callback), NULL);
    gtk_box_pack_start(GTK_BOX(panelcontainer), topmenu, FALSE, TRUE, 0);

    app_shells[num_app_shells - 1].topmenu = topmenu;

    c->pane = gtk_event_box_new();
    if (machine_class != VICE_MACHINE_VSID) {
        gtk_widget_modify_bg(c->pane, GTK_STATE_NORMAL, &black);
    }
    gtk_box_pack_start(GTK_BOX(panelcontainer), c->pane, TRUE, TRUE, 0);
    gtk_widget_show(c->pane);

    mouse_connect_wrap_handler(c->pane, (void*)c);

    /* create the window canvas widget */
    if (machine_class == VICE_MACHINE_VSID) {
        GtkWidget *new_canvas = build_vsid_ctrl_widget();
        gtk_container_add(GTK_CONTAINER(c->pane), new_canvas);
        gtk_widget_show(new_canvas);
        c->emuwindow = NULL;
    } else {
        build_screen_canvas_widget(c);
    }

    /* FIXME: ideally we want to do this last */
    /* FIXME: we want to show the window as late as possible, when the final
              size is known. */
    /* FIXME: this doesnt work because c->draw_buffer->canvas_physical_width etc is
              not initialized yet :/ */
    /* FIXME: this also does not work, because we have to take the size of the window
              decorations into account, which is not known here yet */
    get_initial_window_geo(c, &window_xpos, &window_ypos, &window_width, &window_height);

    if (machine_class != VICE_MACHINE_VSID) {
        /* maintain aspect ratio */
        setup_aspect_geo(c, window_width, window_height);
        toggle_aspect(c);
#if 1
        /* set initial (properly scaled) window size */
        window_width = app_shells[num_app_shells - 1].geo.min_width;
        window_height = app_shells[num_app_shells - 1].geo.min_height;
#endif
    }

    DBG(("ui_open_canvas_window window size (w: %d h: %d)", window_width, window_height));
    /* gtk_window_resize(GTK_WINDOW(new_window), window_width, window_height); */
    gtk_window_set_default_size(GTK_WINDOW(new_window), window_width, window_height);
    gtk_window_set_position(GTK_WINDOW(new_window), GTK_WIN_POS_NONE); /* prevent the WM from auto-placing */
    gtk_window_move(GTK_WINDOW(new_window), window_xpos, window_ypos);
    gtk_widget_show(new_window);

    /* FIXME: the pixmap stuff in tape- and drive- widgets currently prevents
              us from doing this earlier */
    /* create the status bar area */
    app_shells[num_app_shells - 1].status_bar = ui_create_status_bar(panelcontainer);
    /* crt emu control / mixer */
    pal_ctrl_widget = build_pal_ctrl_widget(c, &app_shells[num_app_shells - 1].pal_ctrl_data);
    gtk_box_pack_end(GTK_BOX(panelcontainer), pal_ctrl_widget, FALSE, FALSE, 0);
    gtk_widget_hide(pal_ctrl_widget);
    app_shells[num_app_shells - 1].pal_ctrl = pal_ctrl_widget;


    /* window managers may ignore the move/position requests completely, some may
       only ignore them unless the window is visible, so we explicitly move/resize
       again */
    gtk_widget_show(new_window);
    gtk_window_resize(GTK_WINDOW(new_window), window_width, window_height);
    gtk_window_move(GTK_WINDOW(new_window), window_xpos, window_ypos);

#if 0
    if (no_autorepeat) {
        g_signal_connect(G_OBJECT(new_window), "enter-notify-event", G_CALLBACK(ui_autorepeat_off), NULL);
        g_signal_connect(G_OBJECT(new_window), "leave-notify-event", G_CALLBACK(ui_autorepeat_on), NULL);
    }
#endif

    if (machine_class != VICE_MACHINE_VSID) {

#if !defined(HAVE_CAIRO)
        if (!app_gc) {
            app_gc = gdk_gc_new(new_window->window);
        }
#endif
        if (uicolor_alloc_colors(c) < 0) {
            return -1;
        }

        ui_init_drive_status_widget();
        ui_init_joystick_status_widget();
        mouse_init_cursor();
        gtk_init_lightpen();

    }

    ui_init_checkbox_style();
/*
explicitly setup the events we want to handle. the following are the remaining
events that are NOT handled:

GDK_BUTTON_MOTION_MASK         GDK_MOTION_NOTIFY (while a button is pressed)
GDK_BUTTON1_MOTION_MASK        GDK_MOTION_NOTIFY (while button 1 is pressed)
GDK_BUTTON2_MOTION_MASK        GDK_MOTION_NOTIFY (while button 2 is pressed)
GDK_BUTTON3_MOTION_MASK        GDK_MOTION_NOTIFY (while button 3 is pressed)
GDK_PROPERTY_CHANGE_MASK       GDK_PROPERTY_NOTIFY
GDK_PROXIMITY_IN_MASK          GDK_PROXIMITY_IN
GDK_PROXIMITY_OUT_MASK         GDK_PROXIMITY_OUT
GDK_SUBSTRUCTURE_MASK          Receive  GDK_STRUCTURE_MASK events for child windows
*/
    gtk_widget_add_events(new_window,
                            GDK_LEAVE_NOTIFY_MASK |
                            GDK_ENTER_NOTIFY_MASK |
                            GDK_BUTTON_PRESS_MASK |
                            GDK_BUTTON_RELEASE_MASK |
                            GDK_KEY_PRESS_MASK |
                            GDK_KEY_RELEASE_MASK |
                            GDK_FOCUS_CHANGE_MASK |
                            GDK_POINTER_MOTION_MASK |
                            GDK_VISIBILITY_NOTIFY_MASK |
                            GDK_STRUCTURE_MASK |
                            GDK_EXPOSURE_MASK);

    ui_dispatch_events();
    return 0;
}

void ui_set_topmenu(ui_menu_entry_t *menu)
{
    int i;

    DBG(("ui_set_topmenu (%d)", num_app_shells));

    ui_block_shells();

    for (i = 0; i < num_app_shells; i++) {
        DBG(("ui_set_topmenu %d: %p", i, app_shells[i].topmenu));
        if (app_shells[i].topmenu) {
            gtk_container_foreach(GTK_CONTAINER(app_shells[i].topmenu), (GtkCallback)gtk_widget_destroy, NULL);
            /* gtk_window_add_accel_group (GTK_WINDOW (app_shells[i].shell), app_shells[i].accel); */
            ui_menu_create(app_shells[i].topmenu, app_shells[i].accel, "TopLevelMenu", menu);
        }
    }

    ui_unblock_shells();
}

/* FIXME: we need to know the height of the menubar when calculating the
          initial window size and geometry hints. since this currently happens
          before we want to show the window, the size is still unknown. this is
          worked around by using a resource to save the last known size (which
          probably wont change often :))
*/
static int topmenu_get_height(video_canvas_t *canvas)
{
    app_shell_type *appshell;
    GtkWidget *topmenu;
    int size;

    if (canvas) {
#ifdef HAVE_FULLSCREEN
        if (canvas->fullscreenconfig->enable) {
            return canvas->fullscreenconfig->ui_border_top;
        } else {
#endif
            appshell = &app_shells[canvas->app_shell];
            if (appshell) {
                topmenu = appshell->topmenu;
                if (topmenu) {
                    size = gtk_widget_get_allocated_height(topmenu);
                    if (size > 5) {
                        resources_set_int("WindowTopHint", size);
                    }
                }
            }
#ifdef HAVE_FULLSCREEN
        }
#endif
    }
    resources_get_int("WindowTopHint", &size);
    return size;
}

/* gdk_pixmap_create_from_xpm_d has been deprecated since version 2.22 and has
   been removed in 3.0
 */
#if !GTK_CHECK_VERSION(3, 0, 0)
void ui_set_application_icon(const char *icon_data[])
{
    int i;
    GdkWindow *window = gtk_widget_get_window(get_active_toplevel());
    GdkPixmap *icon;

    icon = gdk_pixmap_create_from_xpm_d(window, NULL, NULL, (char **)icon_data);

    for (i = 0; i < num_app_shells; i++) {
        gdk_window_set_icon(gtk_widget_get_window(app_shells[i].shell), NULL, icon, NULL);
    }
}
#else
void ui_set_application_icon(const char *icon_data[])
{
    int i;
    GdkPixbuf *icon;
    GList *iconlist=NULL;

    icon = gdk_pixbuf_new_from_xpm_data ((const char **) icon_data);
    iconlist = g_list_append (iconlist, icon);

    for (i = 0; i < num_app_shells; i++) {
        gdk_window_set_icon_list(gtk_widget_get_window(app_shells[i].shell), iconlist);
    }
}
#endif

/******************************************************************************/


/* Dispatch the next Xt event.  If not pending, wait for it. */
void ui_dispatch_next_event(void)
{
    gtk_main_iteration();
}

#ifdef USE_UI_THREADS
void ui_dispatch_events(void)
{
    dthread_ui_dispatch_events();
}

/* Dispatch all the pending UI events. */
void ui_dispatch_events2(void)
#else
void ui_dispatch_events(void)
#endif        /* USE_UI_THREADS */
{
    while (gtk_events_pending()) {
        ui_dispatch_next_event();
    }
}

/*******************************************************************************
    enable / disable fullscreen mode

    NOTE: we must make sure that no events resulting from operations done in
          this function fire (asynchronous) with a wrong value in
          ->fullscreenconfig->enable for the active canvas.

    FIXME: this is still buggy when changing mode rapidly, eg by holding ALT-D
*******************************************************************************/
static volatile int fslock = 0;

#ifdef HAVE_FULLSCREEN
static int fsoldx = 0, fsoldy = 0, fsoldw = WINDOW_MINW, fsoldh = WINDOW_MINH;
#endif

int x11ui_fullscreen(int enable)
{
#ifdef HAVE_FULLSCREEN
    video_canvas_t *canvas;
    GtkWidget *s;

    ui_dispatch_events();
    gdk_flush();

    if (fslock) {
        log_debug("x11ui_fullscreen (%d) ignored (locked).", enable);
        return -1;
    }
    fslock = 1;

    if ((canvas = get_active_canvas()) == NULL) {
        log_debug("x11ui_fullscreen (%d) ignored (canvas == NULL).", enable);
        fslock = 0;
        return -1;
    }

    if (enable == canvas->fullscreenconfig->enable) {
        log_debug("x11ui_fullscreen (%d) ignored (was %d).", enable, enable);
        fslock = 0;
        return 0;
    }

    set_active_shell(canvas->app_shell);
    s = get_active_toplevel();

    DBG(("x11ui_fullscreen (shell: %d fullscreen: %d->%d)", canvas->app_shell, canvas->fullscreenconfig->enable, enable));

    if (enable) {
        /* save window dimensions before going to fullscreen */
        get_window_resources(canvas, &fsoldx, &fsoldy, &fsoldw, &fsoldh);
        DBG(("x11ui_fullscreen (fs:%d saved winx: %d winy: %d winw: %d winh: %d)", enable, fsoldx, fsoldy, fsoldw, fsoldh));
        /* when switching to fullscreen, set the flag first */
        canvas->fullscreenconfig->enable = 1;
        /* window managers (bug detected on compiz 0.7.4) may ignore
         * fullscreen requests for windows not visible inside the screen.
         * This can happen especially when using XRandR to resize the desktop.
         * This tries to workaround that problem by ensuring hinting that the
         * window should be placed to the top-left corner. GTK/X sucks. */
        gtk_window_move(GTK_WINDOW(s), 0, 0);
        gtk_window_fullscreen(GTK_WINDOW(s));
        gtk_window_present(GTK_WINDOW(s));
        ui_dispatch_events();
        gdk_flush();
    } else {
        canvas->fullscreenconfig->enable = 0;

        gtk_window_unfullscreen(GTK_WINDOW(s));
        gtk_window_present(GTK_WINDOW(s));
        ui_dispatch_events();
        gdk_flush();

        /* restore previously saved window dimensions */
        DBG(("x11ui_fullscreen (fs:%d restore winx: %d winy: %d winw: %d winh: %d)", enable, fsoldx, fsoldy, fsoldw, fsoldh));
        gtk_window_resize(GTK_WINDOW(s), fsoldw, fsoldh);
        gtk_window_move(GTK_WINDOW(s), fsoldx, fsoldy);
    }
    ui_check_mouse_cursor();
    ui_dispatch_events();
    gdk_flush();

    fslock = 0;
    DBG(("x11ui_fullscreen done"));
#endif
    return 0;
}

int ui_fullscreen_statusbar(struct video_canvas_s *canvas, int enable)
{
#ifdef HAVE_FULLSCREEN
    app_shell_type *appshell = &app_shells[canvas->app_shell];
#endif
    int j;

    DBG(("ui_fullscreen_statusbar (enable:%d)", enable));

    if (!enable
#ifdef HAVE_FULLSCREEN
        && canvas->fullscreenconfig->enable
#endif
       ) {
        for (j = 0; j < num_app_shells; j++) {
            gtk_widget_hide(app_shells[j].status_bar);
            gtk_widget_hide(app_shells[j].topmenu);
        }
    } else {
        for (j = 0; j < num_app_shells; j++) {
            gtk_widget_show(app_shells[j].status_bar);
            gtk_widget_show(app_shells[j].topmenu);
        }
    }

#ifdef HAVE_FULLSCREEN
    if (enable) {
        canvas->fullscreenconfig->ui_border_top = gtk_widget_get_allocated_height(appshell->topmenu);
        canvas->fullscreenconfig->ui_border_bottom = gtk_widget_get_allocated_height(appshell->status_bar);
    } else {
        canvas->fullscreenconfig->ui_border_top = 0;
        canvas->fullscreenconfig->ui_border_bottom = 0;
    }
#endif

    ui_trigger_resize();
    return 0;
}

/* setup geometry hints for given window depending on KeepAspectRatio */
static void toggle_aspect(video_canvas_t *canvas)
{
    int keep_aspect_ratio, flags = 0;
    app_shell_type *appshell = &app_shells[canvas->app_shell];

    DBG(("toggle_aspect"));
    if ((appshell != NULL) && (appshell->shell != NULL)) {
#ifdef HAVE_FULLSCREEN
        DBG(("toggle_aspect fs:%d", canvas->fullscreenconfig->enable));
        if (!canvas->fullscreenconfig->enable) {
#endif
            resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
            if (keep_aspect_ratio) {
                flags |= GDK_HINT_ASPECT;
                if (appshell->geo.max_width) {
                    flags |= GDK_HINT_MAX_SIZE;
                }
            }
            gtk_window_set_geometry_hints (GTK_WINDOW(appshell->shell), NULL, &appshell->geo, GDK_HINT_MIN_SIZE | flags);
#ifdef HAVE_FULLSCREEN
        }
#endif
    }
}

/******************************************************************************/

/* return pixel aspect ratio depending on KeepAspectRatio and TrueAspectRatio */
static gfloat get_aspect(video_canvas_t *canvas)
{
    int keep_aspect_ratio, true_aspect_ratio;
    resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
    if (keep_aspect_ratio) {
        resources_get_int("TrueAspectRatio", &true_aspect_ratio);
        if (true_aspect_ratio) {
#ifdef HAVE_HWSCALE
            if (canvas->videoconfig->hwscale) {
                return canvas->geometry->pixel_aspect_ratio;
            }
#endif
        }
        return 1.0f;
    }
    return 0.0f;
}

static void setup_aspect_geo(video_canvas_t *canvas, int winw, int winh)
{
    gfloat aspect, taspect;
    app_shell_type *appshell = &app_shells[canvas->app_shell];

#ifdef HAVE_FULLSCREEN
    DBG(("setup_aspect_geo in  fullscreen:%d w:%d h:%d", canvas->fullscreenconfig->enable, winw, winh));
#else
    DBG(("setup_aspect_geo in  w:%d h:%d", winw, winh));
#endif

    /* default geometry hints, no scaling, 1:1 aspect */
    appshell->geo.min_width = winw;
    appshell->geo.min_height = winh;
    appshell->geo.max_width = winw;
    appshell->geo.max_height = winh;
    appshell->geo.min_aspect = 1.0f;
    appshell->geo.max_aspect = 1.0f;

    /* FIXME: doing the same also in fullscreen mode seems to work just fine -
              needs some more testing */
/*
#ifdef HAVE_FULLSCREEN
    if (canvas->fullscreenconfig->enable) {
    } else {
#endif
*/
        taspect = get_aspect(canvas);
        if (taspect > 0.0f) {
            aspect = ((float)winw * taspect) / ((float)winh);
            appshell->geo.min_aspect = aspect;
            appshell->geo.max_aspect = aspect;
            appshell->geo.min_width = (int)((float)winw * taspect);
            appshell->geo.max_width = 0;
            appshell->geo.max_height = 0;
        }
/*
#ifdef HAVE_FULLSCREEN
    }
#endif
*/
    DBG(("setup_aspect_geo out min w:%d h:%d max w:%d h:%d asp %f %f", appshell->geo.min_width, appshell->geo.min_height,
         appshell->geo.max_width, appshell->geo.max_height, appshell->geo.min_aspect, appshell->geo.max_aspect));
}

static void setup_aspect(video_canvas_t *canvas)
{
    int w, h, winw, winh;

    /* get size of drawing buffer */
    w = canvas->draw_buffer->canvas_width;
    h = canvas->draw_buffer->canvas_height;
    w *= canvas->videoconfig->scalex;
    h *= canvas->videoconfig->scaley;
    /* calculate unscaled size of window */
    winw = w;
    winh = h + topmenu_get_height(canvas) + statusbar_get_height(canvas) + palctrl_get_height(canvas);
    DBG(("setup_aspect w:%d h:%d winw:%d winh:%d (%d %d %d)", w, h, winw, winh, topmenu_get_height(canvas), statusbar_get_height(canvas), palctrl_get_height(canvas)));

    setup_aspect_geo(canvas, winw, winh);
}

/* Resize one window. */
void ui_resize_canvas_window(video_canvas_t *canvas)
{
    int window_xpos, window_ypos, window_width, window_height;
    app_shell_type *appshell = &app_shells[canvas->app_shell];
    int def;

    get_window_resources(canvas, &window_xpos, &window_ypos, &window_width, &window_height);

    DBG(("ui_resize_canvas_window (winw: %d winh: %d hwscale:%d)", window_width, window_height,canvas->videoconfig->hwscale));

    def = 0;
    if (!canvas->videoconfig->hwscale || (window_width < WINDOW_MINW) || (window_height < WINDOW_MINH)) {
        def = 1;
        window_width = canvas->draw_buffer->canvas_physical_width;
        window_height = canvas->draw_buffer->canvas_physical_height;
    }

    build_screen_canvas_widget(canvas);
    if (! canvas->videoconfig->hwscale) {
        gtk_widget_set_size_request(canvas->emuwindow, window_width, window_height);
    }

    if (def) {
        /* maintain aspect ratio */
        setup_aspect(canvas);
        toggle_aspect(canvas);
        /* set initial (properly scaled) window size */
        window_width = appshell->geo.min_width;
        window_height = appshell->geo.min_height;
    }

#ifdef HAVE_FULLSCREEN
    if (!canvas->fullscreenconfig->enable)
#endif
    {
        set_window_resources(canvas, window_xpos, window_ypos, window_width, window_height);
    }

    DBG(("ui_resize_canvas_window exit (w:%d h:%d)", window_width, window_height));

    gtk_window_resize(GTK_WINDOW(appshell->shell), window_width, window_height);
    ui_dispatch_events();
}

/*
    trigger recalculation of screen/window dimensions
 */
#ifdef USE_UI_THREADS
void ui_trigger_resize(void)
{
    dthread_ui_trigger_resize();
}

void ui_trigger_resize2(void)
#else
void ui_trigger_resize(void)
#endif        /* USE_UI_THREADS */
{
    GtkWidget *toplevel = get_active_toplevel();
    if ((toplevel) && (gtk_widget_get_window(toplevel))) {
        DBG(("ui_trigger_resize"));
        gdk_flush();
        gdk_window_raise(gtk_widget_get_window(toplevel));
    }
}

#ifdef USE_UI_THREADS
void ui_trigger_window_resize(video_canvas_t *canvas)
{
    dthread_ui_trigger_window_resize(canvas);
}

void ui_trigger_window_resize2(video_canvas_t *canvas)
#else
void ui_trigger_window_resize(video_canvas_t *canvas)
#endif
{
    int window_xpos, window_ypos, window_width, window_height;
    GdkEvent event;
    if (canvas) {
        ui_dispatch_events();
#ifdef HAVE_FULLSCREEN
        if (canvas->fullscreenconfig->enable) {
            DBG(("FIXME: ui_trigger_window_resize (fullscreen)"));
            /* FIXME: instead of getting the window dimensions from the resources,
                      they should be determined from the current fullscreen mode.
            */
            return; /* HACK: exiting here will do less harm than resizing with
                             the wrong dimensions */
        } else {
#endif
            get_window_resources(canvas, &window_xpos, &window_ypos, &window_width, &window_height);
#ifdef HAVE_FULLSCREEN
        }
#endif
        DBG(("ui_trigger_window_resize (w:%d h:%d)", window_width, window_height));
        event.configure.width = window_width;
        event.configure.height = window_height -
            (topmenu_get_height(canvas) +
             statusbar_get_height(canvas) +
             palctrl_get_height(canvas));
        configure_callback_canvas(canvas->emuwindow, &event, canvas);
    }
}

/******************************************************************************/

void x11ui_move_canvas_window(ui_window_t w, int x, int y)
{
    DBG(("x11ui_move_canvas_window x:%d y:%d", x, y));
    gdk_window_move(gdk_window_get_toplevel(gtk_widget_get_window(w)), x, y);
    gdk_flush();
}

void x11ui_canvas_position(ui_window_t w, int *x, int *y)
{
    gint tl_x, tl_y, pos_x, pos_y;

    gdk_flush();
    gdk_window_get_position(gdk_window_get_toplevel(gtk_widget_get_window(w)), &tl_x, &tl_y);
    gdk_window_get_position(gtk_widget_get_window(w), &pos_x, &pos_y);
    *x = (pos_x + tl_x);
    *y = (pos_y + tl_y);
    DBG(("x11ui_canvas_position x:%d y:%d", *x, *y));
    gdk_window_raise(gdk_window_get_toplevel(gtk_widget_get_window(w)));
}

void x11ui_get_widget_size(ui_window_t win, int *w, int *h)
{
    GtkRequisition req;

    gtk_widget_size_request(win, &req);
    *w = (int)req.width;
    *h = (int)req.height;
}

void x11ui_destroy_widget(ui_window_t w)
{
    gtk_widget_destroy(w);
}

/******************************************************************************/
/* these are just dummies (and we really should leave it to the WM) */

/* Enable autorepeat. */
void ui_autorepeat_on(void)
{
}
#if 0
/* Disable autorepeat. */
void ui_autorepeat_off(void)
{
}
#endif

/******************************************************************************/

void ui_make_window_transient(GtkWidget *parent,GtkWidget *window)
{
    gtk_window_set_transient_for(GTK_WINDOW(window), GTK_WINDOW(parent));
#if 0
    gdk_window_set_colormap(window->window, colormap);
#endif
}

unsigned char *convert_utf8(unsigned char *s)
{
    unsigned char *d, *r;

    r = d = lib_malloc((size_t)(strlen((char *)s) * 2 + 1));
    while (*s) {
        if (*s < 0x80) {
            *d = *s;
        } else {
            /* special latin1 character handling */
            if (*s == 0xa0) {
                *d = 0x20;
            } else {
                if (*s == 0xad) {
                    *s = 0xed;
                }
                *d++ = 0xc0 | (*s >> 6);
                *d = (*s & ~0xc0) | 0x80;
            }
        }
        s++;
        d++;
    }
    *d = '\0';
    return r;
}

/* Update the menu items with a checkmark according to the current resource
   values. */
void ui_update_menus(void)
{
    DBG(("ui_update_menus"));
    if (machine_class != VICE_MACHINE_VSID) {
        ui_update_palctrl();
    }
    ui_menu_update_all_GTK();
}

void ui_block_shells(void)
{
    int i;

    DBG(("ui_block_shells (%d)", num_app_shells));

    for (i = 0; i < num_app_shells; i++) {
        gtk_widget_set_sensitive(app_shells[i].shell, FALSE);
    }
}

void ui_unblock_shells(void)
{
    video_canvas_t *canvas = get_active_canvas();
    int i;

    DBG(("ui_unblock_shells (%d)", num_app_shells));

    for (i = 0; i < num_app_shells; i++) {
        gtk_widget_set_sensitive(app_shells[i].shell, TRUE);
    }
    /* this is an ugly workaround to fix the focus issue on pop-down reported by count zero
       - dead keyboard after popdown of some dialogs
       this is neither correct nor elegant, as it messes with the mousecursor,
       which is evil UI design, imho; unfortunately I don't know a "better" way :( - pottendo */
    keyboard_key_clear();
    if (canvas) {
        if (machine_class == VICE_MACHINE_VSID) {
            /* FIXME */
        } else {
            gdk_pointer_grab(gtk_widget_get_window(canvas->emuwindow), 1, 0,
                             gtk_widget_get_window(canvas->emuwindow),
                             NULL, GDK_CURRENT_TIME);
        }
        gdk_pointer_ungrab(GDK_CURRENT_TIME);
        ui_check_mouse_cursor();
    }
}

/* ------------------------------------------------------------------------- */

/* Event handler for menu bar menus */
static gboolean update_menu_callback(GtkWidget *w, GdkEvent *event,gpointer data)
{
    ui_menu_update_all_GTK();
    return 0;
}

/* Pop up a popup shell and center it to the last visited AppShell */
void ui_popup(GtkWidget *w, const char *title, gboolean wait_popdown)
{
#ifdef HAVE_FULLSCREEN
    fullscreen_suspend(1);
#endif

    ui_restore_mouse();

    /* Keep sure that we really know which was the last visited shell. */
    ui_dispatch_events();
    gtk_window_set_title(GTK_WINDOW(w),title);

    gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(get_active_toplevel()));
    gtk_widget_show(w);
    gtk_window_present(GTK_WINDOW(w));

    gdk_window_set_decorations(gtk_widget_get_window(w), GDK_DECOR_ALL | GDK_DECOR_MENU);
    gdk_window_set_functions(gtk_widget_get_window(w), GDK_FUNC_ALL | GDK_FUNC_RESIZE);

    ui_block_shells();
    /* If requested, wait for this widget to be popped down before
       returning. */
    if (wait_popdown) {
        int oldcnt = popped_up_count++;

        while (oldcnt != popped_up_count) {
            ui_dispatch_next_event();
        }
        ui_unblock_shells();
    } else {
        popped_up_count++;
    }
}

/* Pop down a popup shell. */
void ui_popdown(GtkWidget *w)
{
    ui_check_mouse_cursor();
    if (w) {
        gtk_widget_hide(w);
    }
    if (--popped_up_count < 0) {
        popped_up_count = 0;
    }
    ui_unblock_shells();
#ifdef HAVE_FULLSCREEN
    fullscreen_resume();
#endif
}

/* ------------------------------------------------------------------------- */

/* Miscellaneous callbacks.  */

static gboolean enter_window_callback(GtkWidget *w, GdkEvent *e, gpointer p)
{
    DBG(("enter_window_callback %p", p));

    set_active_shell(((video_canvas_t *)p)->app_shell);

    /* cv: ensure focus after dialogs were opened */
    gtk_widget_grab_focus(w);
    gtk_lightpen_update_canvas(p, TRUE);
    keyboard_key_clear();

#ifdef HAVE_FULLSCREEN
    fullscreen_mouse_moved((struct video_canvas_s *)p, 0, 0, 2);
#endif

    return 0;
}

static gboolean leave_window_callback(GtkWidget *w, GdkEvent *e, gpointer p)
{
    DBG(("leave_window_callback %p", p));
#ifdef HAVE_FULLSCREEN
    fullscreen_mouse_moved((struct video_canvas_s *)p, 0, 0, 1);
#endif
    return 0;
}

static gboolean map_callback(GtkWidget *w, GdkEvent *event, gpointer user_data)
{
    DBG(("map_callback %p", user_data));
#ifdef HAVE_HWSCALE
    video_canvas_t *canvas = (video_canvas_t *)user_data;

    if (canvas) {
        GdkGLContext *gl_context = gtk_widget_get_gl_context(w);
        GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable(w);

        gdk_gl_drawable_gl_begin (gl_drawable, gl_context);

        glGenTextures(1, &canvas->screen_texture);

        gdk_gl_drawable_gl_end(gl_drawable);
    }
#endif
    return FALSE;
}

#ifdef USE_UI_THREADS
gboolean configure_callback_canvas(GtkWidget *w, GdkEvent *event, gpointer client_data)
{
    return dthread_configure_callback_canvas(w, event, client_data);
}

gboolean configure_callback_canvas2(GtkWidget *w, GdkEvent *event, gpointer client_data)
#else
gboolean configure_callback_canvas(GtkWidget *w, GdkEvent *event, gpointer client_data)
#endif        /* USE_UI_THREADS */
{
    GdkEventConfigure *e = &event->configure;
    video_canvas_t *canvas = (video_canvas_t *) client_data;
#ifdef HAVE_HWSCALE
    float ow, oh;
#ifdef HAVE_FULLSCREEN
    int keep_aspect_ratio;
#endif
    GdkGLContext *gl_context;
    GdkGLDrawable *gl_drawable;
#endif

    if ((e->width < WINDOW_MINW) || (e->height < WINDOW_MINH)) {
        /* DBG(("configure_callback_canvas skipped")); */
        return 0;
    }

    if ((canvas == NULL) || (canvas->app_shell >= num_app_shells) ||
        (canvas != app_shells[canvas->app_shell].canvas)) {
        log_error(ui_log, "configure_callback_canvas: bad params");
        return 0;
    }


#ifdef DEBUG_X11UI
#ifdef HAVE_FULLSCREEN
    if (canvas->fullscreenconfig) {
        DBG(("configure_callback_canvas (fullscreen: %d e->width %d e->height %d canvas_width %d canvas_height %d)",
            canvas->fullscreenconfig->enable, e->width, e->height, canvas->draw_buffer->canvas_width, canvas->draw_buffer->canvas_height));
    } else {
#endif
        DBG(("configure_callback_canvas (fullscreen: -- e->width %d e->height %d canvas_width %d canvas_height %d)",
            e->width, e->height, canvas->draw_buffer->canvas_width, canvas->draw_buffer->canvas_height));
#ifdef HAVE_FULLSCREEN
    }
#endif
#endif

    /* This should work, but doesn't... Sigh...
    c->draw_buffer->canvas_width = e->width;
    c->draw_buffer->canvas_height = e->height;
    c->draw_buffer->canvas_width /= c->videoconfig->scalex;
    c->draw_buffer->canvas_height /= c->videoconfig->scaley;
    video_viewport_resize(c);
    */

#ifdef HAVE_HWSCALE
    /* get size of drawing buffer */
    ow = canvas->draw_buffer->canvas_physical_width;
    oh = canvas->draw_buffer->canvas_physical_height;
#ifdef HAVE_FULLSCREEN
    /* in fullscreen mode, scale with aspect ratio */
    if (canvas->fullscreenconfig->enable) {
        resources_get_int("KeepAspectRatio", &keep_aspect_ratio);
        if (keep_aspect_ratio) {
            if ((float)e->height >= (oh / get_aspect(canvas)) * ((float)e->width / ow)) {
                /* full width, scale height */
                oh = (float)e->height / ((float)e->width / ow);
                oh *= get_aspect(canvas);
            } else {
                /* full height, scale width */
                ow = (float)e->width / ((float)e->height / oh);
                ow /= get_aspect(canvas);
            }
        }
    }
    DBG(("configure_callback_canvas scaled: (ow: %f oh:%f)", ow, oh));
#endif
    gl_context = gtk_widget_get_gl_context (w);
    gl_drawable = gtk_widget_get_gl_drawable (w);
    if (gl_context != NULL && gl_drawable != NULL) {
        gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
        /* setup viewport */
        glViewport(0, 0, e->width, e->height);
        /* projection and model view matrix */
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-(ow/2),(ow/2),-(oh/2),(oh/2),-100,100);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        gdk_gl_drawable_gl_end(gl_drawable);
    }
#endif
    /* maintain aspect ratio */
    setup_aspect(canvas);
    toggle_aspect(canvas);

    return 0;
}

/*
  connected to "configure-event" of the window, which is emitted to size,
  position and stack order events.
*/
static gboolean configure_callback_app(GtkWidget *w, GdkEvent *event, gpointer client_data)
{
    video_canvas_t *canvas = (video_canvas_t *) client_data;
    app_shell_type *appshell;
    GdkEventConfigure *e = &event->configure;
    GdkRectangle rect;

    if ((canvas == NULL) || (e->width < WINDOW_MINW) || (e->height < WINDOW_MINH)) {
        /* DBG(("configure_callback_app skipped")); */
        return 0;
    }

    appshell = &app_shells[canvas->app_shell];

    if ((canvas->app_shell >= num_app_shells) || (canvas != appshell->canvas)) {
        log_error(ui_log, "configure_callback_app: bad params (%p) %d", client_data, canvas->app_shell);
        return 0;
    }

#ifdef DEBUG_X11UI
#ifdef HAVE_FULLSCREEN
    if (canvas->fullscreenconfig) {
        DBG(("configure_callback_app (fullscreen: %d x %d y %d w %d h %d) (%p)",canvas->fullscreenconfig->enable, e->x, e->y,e->width, e->height, canvas));
    } else {
#endif
        DBG(("configure_callback_app (fullscreen: -- x %d y %d w %d h %d) (%p)", e->x, e->y,e->width, e->height, canvas));
#ifdef HAVE_FULLSCREEN
    }
#endif
#endif

    /* if not fullscreen then save current window position and size into the resources */
#ifdef HAVE_FULLSCREEN
    if ((machine_class == VICE_MACHINE_VSID) || (!canvas->fullscreenconfig->enable)) {
#endif
        /* the coordinates in the configure-event refer to the GtkWidget, ie to the
           window area that we use, without the window frame and -decorations. to
           restore the exact window position and size, we do however need the position
           and size of the window including frame and decorations. */
        gdk_window_get_frame_extents(gtk_widget_get_window(appshell->shell), &rect);
        DBG(("configure_callback_app actual window x %d y %d w %d h %d", rect.x, rect.y, rect.width, rect.height));
        set_window_resources(canvas, rect.x, rect.y, e->width, e->height);
#ifdef HAVE_FULLSCREEN
    }
#endif
    if (machine_class != VICE_MACHINE_VSID) {
        /* HACK: propagate the event to the canvas widget to make ui_trigger_resize
        *       work.
        */
        GdkEvent event2;

        memcpy(&event2, event, sizeof(GdkEvent));
        event2.configure.x = event2.configure.y = 0;
        event2.configure.width = e->width;
        event2.configure.height = e->height - (topmenu_get_height(canvas) + statusbar_get_height(canvas) + palctrl_get_height(canvas));

        DBG(("configure_callback_app to canvas (x %d y %d w %d h %d)", event2.configure.x, event2.configure.y, event2.configure.width, event2.configure.height));
        configure_callback_canvas(canvas->emuwindow, &event2, canvas);
    }
    return 0;
}

/*
This is not used, as textures are generated right before blending

void gl_setup_textures(video_canvas_t *c, struct s_mbufs *buffers)
{
    int i, tw, th;
    GdkGLContext *gl_context = gtk_widget_get_gl_context(c->emuwindow);
    GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable(c->emuwindow);
    gdk_gl_drawable_gl_begin(gl_drawable, gl_context);
    tw = buffers[0].w;
    th = buffers[0].h;
    for (i = 0; i < MAX_BUFFERS; i++) {
        glGenTextures(1, &buffers[i].bindId);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, buffers[i].bindId);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, tw, th, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, buffers[i].buffer);
    }

    gdk_gl_drawable_gl_end (gl_drawable);
}
*/

#ifdef HAVE_HWSCALE
static void gl_update_texture(struct s_mbufs *buffer)
{
    int tw, th;
    tw = buffer->w;
    th = buffer->h;
    /* DBG(("update texture: %ld, %f", buffer->stamp, buffer->alpha)); */

    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, buffer->bindId);
#ifdef __BIG_ENDIAN__
#ifndef GL_ABGR_EXT
#error "Your headers do not supply GL_ABGR_EXT. Disable HWSCALE and try again."
#endif
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, tw, th, 0,
                 GL_ABGR_EXT, GL_UNSIGNED_BYTE, buffer->buffer);
#else
    glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_RGBA, tw, th, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer->buffer);
#endif
}


static void gl_draw_quad(float alpha, int tw, int th)
{
    glBegin (GL_QUADS);
    {
        /* glColor4f(1.0f,1.0f,1.0f,alpha);*/
        glColor4f(alpha, alpha, alpha, alpha);
        /* Lower Right Of Texture */
        glTexCoord2f(0.0f, 0.0f); glVertex2f(-(tw/2), (th/2));
        /* Upper Right Of Texture */
        glTexCoord2f(0.0f, th); glVertex2f(-(tw/2), -(th/2));
        /* Upper Left Of Texture */
        glTexCoord2f(tw, th); glVertex2f((tw/2), -(th/2));
        /* Lower Left Of Texture */
        glTexCoord2f(tw, 0.0f); glVertex2f((tw/2), (th/2));
    }
    glEnd ();
}


void gl_render_canvas(GtkWidget *w, video_canvas_t *canvas,
                      struct s_mbufs *buffers, int from, int to, int do_swap)
{
    int tw, th, d, i = 0;
    struct s_mbufs *t;

    if (!GTK_IS_WIDGET(w)) {
        DBG(("widget not initalized %s", __FUNCTION__));
        return;
    }
    if (canvas->videoconfig->hwscale == 0) {
        return;
    }

    GdkGLContext *gl_context = gtk_widget_get_gl_context(w);
    GdkGLDrawable *gl_drawable = gtk_widget_get_gl_drawable(w);
    gdk_gl_drawable_gl_begin(gl_drawable, gl_context);

    tw = buffers[0].w;
    th = buffers[0].h;

    /* DBG(("%s: from: %d, to: %d", __FUNCTION__, from, to)); */

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glEnable(GL_BLEND);

    t = &buffers[from];
    d = ((to - from) + MAX_BUFFERS) % MAX_BUFFERS + 1;
    if (d < 2) {
        goto lframe;
    }

    glBlendFunc( GL_ONE, GL_ONE );
    gl_update_texture(t);
    gl_draw_quad(t->alpha, tw, th);

    d-=2;                 /* first has been drawn, last is outside of loop */
    t = t->next;
    for (i = 0; i < d; i++, t=t->next) {
        glBlendFunc(GL_ONE, GL_ONE);
        gl_update_texture(t);
        gl_draw_quad(t->alpha, tw, th);
    }
  lframe:
    glBlendFunc(GL_ONE, GL_ONE);
    gl_update_texture(t);
    gl_draw_quad(t->alpha, tw, th);

#if 0
    /* draw vertical line as reference for smooth animations */
    {
        static int x = 0;

        glDisable(GL_TEXTURE_RECTANGLE_EXT);
        glDisable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 0, 1.0f);
        glBegin(GL_LINES);
            glVertex2f(x, 0);
            glVertex2f(x, 200);
        glEnd();
        x-=3;
        if (x < -200) {
            x = 200;
        }
    }
#endif

    if (do_swap) {
        gdk_gl_drawable_swap_buffers (gl_drawable);
    }
    gdk_gl_drawable_gl_end (gl_drawable);
}
#endif  /* HAVE_HWSCALE */


static void gtk_render_canvas(GtkWidget *w, GdkEventExpose *e, gpointer client_data,
                       video_canvas_t *canvas)
{
#if !defined(HAVE_CAIRO)
        int x = e->area.x;
        int y = e->area.y;
        int width = e->area.width;
        int height = e->area.height;

        gdk_draw_image(w->window, app_gc, canvas->gdk_image, x, y, x, y,
                       width, height);
#else
        canvas->cairo_ctx =
            gdk_cairo_create(gtk_widget_get_window(canvas->emuwindow));
        if (canvas->cairo_ctx != NULL) {
            /* FIXME: this always redraws the entire canvas area */
            gdk_cairo_set_source_pixbuf(canvas->cairo_ctx,
                                        canvas->gdk_pixbuf, 0, 0);
            cairo_paint (canvas->cairo_ctx);
            cairo_destroy(canvas->cairo_ctx);
        }
#endif        /* !HAVE_CAIRO */
}

/* this callback actually renders the canvas to screen using opengl or gtk */
gboolean exposure_callback_canvas(GtkWidget *w, GdkEventExpose *e, gpointer client_data)
{
    video_canvas_t *canvas = (video_canvas_t *)client_data;

    /* DBG(("exposure callback")); */

    if ((canvas == NULL) ||
        (canvas->app_shell >= num_app_shells) ||
        (canvas != app_shells[canvas->app_shell].canvas)) {
        log_error(ui_log, "exposure_callback_canvas: bad params");
        return 0;
    }
#ifdef USE_UI_THREADS
    if (canvas->videoconfig->hwscale == 0) {
        gtk_render_canvas(w, e, client_data, canvas);
    }
    return 0; /* rendering is handled by the display thread */
#endif

    /* DBG(("exposure_callback_canvas canvas w/h %d/%d",
       canvas->gdk_image->width, canvas->gdk_image->height)); */

#ifdef HAVE_HWSCALE
    if (canvas->videoconfig->hwscale) {
        static struct s_mbufs t;
#if !defined(HAVE_CAIRO)
        t.w = canvas->gdk_image->width;
        t.h = canvas->gdk_image->height;
#else
        /* FIXME! */
        t.w = canvas->draw_buffer->canvas_physical_width;
        t.h = canvas->draw_buffer->canvas_physical_height;
#endif /* !HAVE_CAIRO */
        t.buffer = canvas->hwscale_image;
        t.alpha = 1.0;
        gl_render_canvas(w, canvas, &t, 0, 0, 1);
        return 0;
    }
#endif        /* HAVE_HWSCALE */
    gtk_render_canvas(w, e, client_data, canvas);
    return 0;
}
