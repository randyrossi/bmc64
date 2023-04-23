/** \file   uistatusbar.c
 *  \brief  Native GTK3 UI statusbar stuff
 *
 *  The status bar widget is part of every machine window. This widget
 *  reacts to UI notifications from the emulation core and otherwise
 *  does not interact with the rest of the main menu.
 *
 *  Functions described as "Statusbar API functions" are called by the
 *  rest of the UI or the emulation system itself to report that the
 *  status displays must be updated to reflect possibly new
 *  information. It is not necessary for the data to be truly new; the
 *  statusbar implementation will treat repeated reports of the same
 *  state as no-ops when necessary for performance.
 *
 *  \author Marco van den Heuvel <blackystardust68@yahoo.com>
 *  \author Michael C. Martin <mcmartin@gmail.com>
 */

/*
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
#include <gtk/gtk.h>

#include "debug_gtk3.h"

#include "vice_gtk3.h"
#include "datasette.h"
#include "drive.h"
#include "joyport.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
#include "uidatasette.h"
#include "uidiskattach.h"
#include "uifliplist.h"
#include "uisettings.h"
#include "userport/userport_joystick.h"

#include "diskcontents.h"
#include "drive.h"
#include "drivetypes.h"
#include "diskimage.h"
#include "diskimage/fsimage.h"
#include "autostart.h"
#include "tapecontents.h"

#include "contentpreviewwidget.h"
#include "dirmenupopup.h"
#include "joystickmenupopup.h"
#include "statusbarspeedwidget.h"

#include "uistatusbar.h"


/** \brief The maximum number of status bars we will permit to exist
 *         at once. */
#define MAX_STATUS_BARS 3


/** \brief  Status bar column indici
 *
 * These values assume a proper emulator statusbar (ie not VSID).
 */
enum {
    SB_COL_SPEED = 0,   /**< message widget */
    SB_COL_SEP_MSG ,    /**< separator between message and ctr/mixer widgets */
    SB_COL_CRT,         /**< crt and mixer widgets */
    SB_COL_SEP_CRT,     /**< separator between crt/mixer and tape widgets */
    SB_COL_TAPE,        /**< tape and joysticks widget */
    SB_COL_SEP_TAPE,    /**< separator between tape and joysticks widgets */
    SB_COL_DRIVE        /**< drives widgets */
};


/** \brief Global data that custom status bar widgets base their rendering
 *         on.
 *
 *  This data is usually set by calls from the emulation core or from
 *  other parts of the UI in response to user commands or I/O events.
 *
 *  \todo The PET can have two tape drives.
 *
 *  \todo Two-unit drive units are not covered by this structure.
 */
typedef struct ui_sb_state_s {
    /** \brief Identifier for the currently displayed status bar
     * message.
     *
     * Used to correlate timeout events so that a new message
     * isn't erased by some older message timing out. */
    intptr_t statustext_msgid;
    /** \brief Current tape state (play, rewind, etc) */
    int tape_control;
    /** \brief Nonzero if the tape motor is powered. */
    int tape_motor_status;
    /** \brief Location on the tape */
    int tape_counter;
    /** \brief Which drives are to be displayed in the status bar.
     *
     *  This is a bitmask, with bits 0-3 representing drives 8-11,
     *  respectively.
     */
    int drives_enabled;
    /** \brief Nonzero if True Drive Emulation is active and drive
     *         LEDs should be drawn. */
    int drives_tde_enabled;
    /** \brief Color descriptors for the drive LED colors.
     *
     *  This value is a bitmask, with bit 0 and 1 set if the
     *  corresponding LED is green. Otherwise it is red. Drives that
     *  only have one LED will have their 'second' LED permanently at
     *  intensity zero so the value is irrelevant in that case. */
    int drive_led_types[DRIVE_NUM];
    /** \brief Current intensity of each drive LED, 0=off,
     *         1000=max. */
    unsigned int current_drive_leds[DRIVE_NUM][2];
    /** \brief Current state for each of the joyports.
     *
     *  This is an 7-bit bitmask, representing, from least to most
     *  significant bits: up, down, left, right, fire button,
     *  secondary fire button, tertiary fire button. */
    int current_joyports[JOYPORT_MAX_PORTS];
    /** \brief Which joystick ports are actually available.
     *
     *  This is a bitmask representing notional ports 0-4, which are
     *  themselves defined in joyport/joyport.h. Cases like a SIDcart
     *  control port on a Plus/4 without other userport control ports
     *  mean that the set of active joyports may be discontinuous. */
    int joyports_enabled;
} ui_sb_state_t;

/** \brief The current state of the status bars across the UI. */
static ui_sb_state_t sb_state;

/** \brief The full structure representing a status bar widget.
 *
 *  This includes the top-level widget and then every subwidget that
 *  needs to be individually addressed or manipulated by the
 *  status-report API. */
typedef struct ui_statusbar_s {
    /** \brief The status bar widget proper. 
     *
     *  This is the widget the rest of the UI code will store and pack
     *  into windows. */
    GtkWidget *bar;
#if 0
    /** \brief The status message widget. */
    GtkLabel *msg;
#endif
    /** \brief  Widget displaying CPU speed and FPS
     *
     * Also used to set refresh rate, CPU speed, pause, warp and adv-frame
     */
    GtkWidget *speed;

    /** \brief CRT control widget checkbox */
    GtkWidget *crt;

    /** \brief  Mixer control widget checkbox */
    GtkWidget *mixer;

    /** \brief The Tape Status widget. */
    GtkWidget *tape;
    /** \brief The Tape Status widget's popup menu. */
    GtkWidget *tape_menu;
    /** \brief The joyport status widget. */
    GtkWidget *joysticks;
    /** \brief The drive status widgets. */
    GtkWidget *drives[DRIVE_NUM];
    /** \brief The popup menus associated with each drive. */
    GtkWidget *drive_popups[DRIVE_NUM];
    /** \brief The hand-shaped cursor to change to when popup menus
     *         are available. */
    GdkCursor *hand_ptr;
} ui_statusbar_t;

/** \brief The collection of status bars currently active. 
 *
 *  Inactive status bars have a NULL pointer for their "bar" field. */
static ui_statusbar_t allocated_bars[MAX_STATUS_BARS];


/* Forward decl. */
static void tape_dir_autostart_callback(const char *image, int index);


/** \brief Initialize the status bar subsystem.
 *
 *  \warning This function _must_ be called before any call to
 *           ui_statusbar_create() and _must not_ be called after any
 *           call to it.
 */
void ui_statusbar_init(void)
{
    int i, j;

    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        allocated_bars[i].bar = NULL;
        allocated_bars[i].speed = NULL;
        allocated_bars[i].crt = NULL;
        allocated_bars[i].mixer = NULL;
        allocated_bars[i].tape = NULL;
        allocated_bars[i].tape_menu = NULL;
        allocated_bars[i].joysticks = NULL;
        for (j = 0; j < DRIVE_NUM; ++j) {
            allocated_bars[i].drives[j] = NULL;
            allocated_bars[i].drive_popups[j] = NULL;
        }
        allocated_bars[i].hand_ptr = NULL;
    }

    sb_state.statustext_msgid = 0;
    sb_state.tape_control = 0;
    sb_state.tape_motor_status = 0;
    sb_state.tape_counter = 0;
    sb_state.drives_enabled = 0;
    sb_state.drives_tde_enabled = 0;
    for (i = 0; i < DRIVE_NUM; ++i) {
        sb_state.drive_led_types[i] = 0;
        sb_state.current_drive_leds[i][0] = 0;
        sb_state.current_drive_leds[i][1] = 0;
    }

    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        sb_state.current_joyports[i] = 0;
    }
    sb_state.joyports_enabled = 0;
}

/** \brief Clean up any resources the statusbar system uses that
 *         weren't cleaned up when the status bars themselves were
 *         destroyed. */
void ui_statusbar_shutdown(void)
{
    /* There are no such resources, so this is a no-op */
}

/** \brief Extracts the list of enabled drives from the DriveType
 *         resources.
 *
 *  \return A bitmask value suitable for ui_sb_state_s::drives_enabled.
 */
static int compute_drives_enabled_mask(void)
{
    int unit, mask;
    int result = 0;
    for (unit = 0, mask = 1; unit < 4; ++unit, mask <<= 1) {
        int status = 0, value = 0;
        status = resources_get_int_sprintf("Drive%dType", &value, unit + 8);
        if (status == 0 && value != 0) {
            result |= mask;
        }
    }
    return result;
}

/** \brief Draws the tape icon based on the current control and motor status. 
 *
 *  \param widget  The tape icon GtkDrawingArea being drawn to.
 *  \param cr      The cairo context that handles the drawing.
 *  \param data    Ignored, but mandated by the function signature
 *
 *  \return FALSE, telling GTK to continue event processing
 *
 *  \todo Once multiple tape drives are supported, the data parameter
 *        will be the integer index of which tape drive this widget
 *        represents.
 */
static gboolean draw_tape_icon_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int width, height;
    double x, y, inset;
    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    if (width > height) {
        x = (width - height) / 2.0;
        y = 0.0;
        inset = height / 10.0;
    } else {
        x = 0.0;
        y = (height - width) / 2.0;
        inset = width / 10.0;
    }

    if (sb_state.tape_motor_status) {
        cairo_set_source_rgb(cr, 0, 0.75, 0);
    } else {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    }
    cairo_rectangle(cr, x + inset, y + inset, inset * 8, inset * 8);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    switch (sb_state.tape_control) {
    case DATASETTE_CONTROL_STOP:
        cairo_rectangle(cr, x + 2.5*inset, y + 2.5*inset, inset * 5, inset * 5);
        cairo_fill(cr);
        break;
    case DATASETTE_CONTROL_START:
        cairo_move_to(cr, x + 3*inset, y + 2.5*inset);
        cairo_line_to(cr, x + 3*inset, y + 7.5*inset);
        cairo_line_to(cr, x + 7*inset, y + 5*inset);
        cairo_close_path(cr);
        cairo_fill(cr);
        break;
    case DATASETTE_CONTROL_FORWARD:
        cairo_move_to(cr, x + 2.5*inset, y + 2.5*inset);
        cairo_line_to(cr, x + 2.5*inset, y + 7.5*inset);
        cairo_line_to(cr, x + 5*inset, y + 5*inset);
        cairo_close_path(cr);
        cairo_fill(cr);
        cairo_move_to(cr, x + 5*inset, y + 2.5*inset);
        cairo_line_to(cr, x + 5*inset, y + 7.5*inset);
        cairo_line_to(cr, x + 7.5*inset, y + 5*inset);
        cairo_close_path(cr);
        cairo_fill(cr);
        break;
    case DATASETTE_CONTROL_REWIND:
        cairo_move_to(cr, x + 5*inset, y + 2.5*inset);
        cairo_line_to(cr, x + 5*inset, y + 7.5*inset);
        cairo_line_to(cr, x + 2.5*inset, y + 5*inset);
        cairo_close_path(cr);
        cairo_fill(cr);
        cairo_move_to(cr, x + 7.5*inset, y + 2.5*inset);
        cairo_line_to(cr, x + 7.5*inset, y + 7.5*inset);
        cairo_line_to(cr, x + 5*inset, y + 5*inset);
        cairo_close_path(cr);
        cairo_fill(cr);
        break;
    case DATASETTE_CONTROL_RECORD:
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 5*inset, y + 5*inset, 2.5*inset, 0, 2 * G_PI);
        cairo_close_path(cr);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 1, 0, 0);
        cairo_new_sub_path(cr);
        cairo_arc(cr, x + 5*inset, y + 5*inset, 2*inset, 0, 2 * G_PI);
        cairo_close_path(cr);
        cairo_fill(cr);
        break;
    case DATASETTE_CONTROL_RESET:
    case DATASETTE_CONTROL_RESET_COUNTER:
    default:
        /* Things that aren't really controls look like we stop it. */
        /* TODO: Should RESET_COUNTER be wiped out by the time it gets here? */
        cairo_rectangle(cr, x + 2.5*inset, y + 2.5*inset, inset * 5, inset * 5);
        cairo_fill(cr);
    }

    return FALSE;
}


/** \brief  Handler for the 'activate' event of the 'Configure drives' item
 *
 * Opens the settings UI at the drive settings "page".
 *
 * \param[in]   widget  menu item triggering the event (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_drive_configure_activate(GtkWidget *widget, gpointer data)
{
    ui_settings_dialog_create_and_activate_node("peripheral/drive");
}


/** \brief  Handler for the 'activate' event of the 'Reset drive #X' item
 *
 * Triggers a reset for drive ((int)data + 8)
 *
 * \param[in]   widget  menu item triggering the event (unused)
 * \param[in]   data    drive number (0-3)
 */
static void on_drive_reset_clicked(GtkWidget *widget, gpointer data)
{
    debug_gtk3("Resetting drive %d", GPOINTER_TO_INT(data) + 8);
    drive_cpu_trigger_reset(GPOINTER_TO_INT(data));
}

/** \brief  Handler for the 'activate' event of the 'Reset drive \#X in ... mode' item
 *
 * Triggers a reset for drive ((int)data + 8).
 *
 * \param[in]   widget  menu item triggering the event (unused)
 * \param[in]   data    drive number (0-3)
 */
static void on_drive_reset_config_clicked(GtkWidget *widget, gpointer data)
{
#if 0
    debug_gtk3("Resetting drive %d (button=%d)", ((GPOINTER_TO_INT(data)>>4)&15) + 8,
       GPOINTER_TO_INT(data) & 15 );
#endif
    drive_cpu_trigger_reset_button(((GPOINTER_TO_INT(data)>>4)&15),
       GPOINTER_TO_INT(data) & 15 );
}


/** \brief Draw the LED associated with some drive's LED state.
 *
 *  \param widget  The drive LED GtkDrawingArea being drawn to.
 *  \param cr      The cairo context that handles the drawing.
 *  \param data    The index (0-3) of which drive this represents.
 *
 *  \return FALSE, telling GTK to continue event processing
 */
static gboolean draw_drive_led_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int width, height, drive, i;
    double red = 0.0, green = 0.0, x, y, w, h;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    drive = GPOINTER_TO_INT(data);
    for (i = 0; i < 2; ++i) {
        int led_color = sb_state.drive_led_types[drive] & (1 << i);
        if (led_color) {
            green += sb_state.current_drive_leds[drive][i] / 1000.0;
        } else {
            red += sb_state.current_drive_leds[drive][i] / 1000.0;
        }
    }
    /* Cairo clamps these for us */
    cairo_set_source_rgb(cr, red, green, 0);
    /* LED is half text height and aims for a 2x1 aspect ratio */
    h = height / 2.0;
    w = 2.0 * h;
    x = (width / 2.0) - h;
    y = height / 4.0;
    cairo_rectangle(cr, x, y, w, h);
    cairo_fill(cr);
    return FALSE;
}

/** \brief Draw the current input status from a joyport.
 *
 *  This produces five squares arranged in a + shape, with directions
 *  represented as green squares when active and black when not. The
 *  fire buttons are represented by the central square, with red,
 *  green, and blue components representing the three possible
 *  buttons.
 *
 *  For traditional Commodore joysticks, there is only one fire button
 *  and this will be diplayed as a red square when the button is
 *  pressed.
 *
 *  \param widget  The joyport GtkDrawingArea being drawn to.
 *  \param cr      The cairo context that handles the drawing.
 *  \param data    The index (0-4) of which joyport this represents.
 *
 *  \return FALSE, telling GTK to continue event processing
 */
static gboolean draw_joyport_cb(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    int width, height, val;
    double e, s, x, y;

    width = gtk_widget_get_allocated_width(widget);
    height = gtk_widget_get_allocated_height(widget);
    val = sb_state.current_joyports[GPOINTER_TO_INT(data)];

    /* This widget "wants" to draw 6x6 squares inside a 20x20
     * space. We compute x and y offsets for a scaled square within
     * the real widget space, and then the actual widths for a square
     * edge (e) and the spaces between them (s). */

    if (width > height) {
        s = height / 20.0;
        x = (width - height) / 2.0;
        y = 0.0;
    } else {
        s = width / 20.0;
        y = (height - width) / 2.0;
        x = 0.0;
    }
    e = s * 5.0;

    /* Then we render the five squares. This seems like it could be
     * done more programatically, but enough changes each iteration
     * that we might as well unroll it. */

    /* Up: Bit 0 */
    cairo_set_source_rgb(cr, 0, (val&0x01) ? 1 : 0, 0);
    cairo_rectangle(cr, x + e + 2*s, y+s, e, e);
    cairo_fill(cr);
    /* Down: Bit 1 */
    cairo_set_source_rgb(cr, 0, (val&0x02) ? 1 : 0, 0);
    cairo_rectangle(cr, x + e + 2*s, y + 2*e + 3*s, e, e);
    cairo_fill(cr);
    /* Left: Bit 2 */
    cairo_set_source_rgb(cr, 0, (val&0x04) ? 1 : 0, 0);
    cairo_rectangle(cr, x + s, y + e + 2*s, e, e);
    cairo_fill(cr);
    /* Right: Bit 3 */
    cairo_set_source_rgb(cr, 0, (val&0x08) ? 1 : 0, 0);
    cairo_rectangle(cr, x + 2*e + 3*s, y + e + 2*s, e, e);
    cairo_fill(cr);
    /* Fire buttons: Bits 4-6. Each of the three notional fire buttons
     * controls the red, green, or blue color of the fire button
     * area. By default, we are using one-button joysticks and so this
     * region will be either black or red. */
    cairo_set_source_rgb(cr, (val&0x10) ? 1 : 0,
                             (val&0x20) ? 1 : 0,
                             (val&0x40) ? 1 : 0);
    cairo_rectangle(cr, x + e + 2*s, y + e + 2*s, e, e);
    cairo_fill(cr);
    return FALSE;
}

/** \brief Create a new drive widget for inclusion in the status bar.
 *
 *  \param unit The drive unit to create (0-3, indicating devices
 *              8-11)
 *
 *  \return The constructed widget. This widget will be a floating
 *          reference.
 */
static GtkWidget *ui_drive_widget_create(int unit)
{
    GtkWidget *grid, *number, *track, *led;
    char drive_id[4];

    grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(grid, FALSE);

    snprintf(drive_id, 4, "%d:", unit + 8);
    drive_id[3] = 0;
    number = gtk_label_new(drive_id);
    gtk_widget_set_halign(number, GTK_ALIGN_START);

    track = gtk_label_new("18.5");
    gtk_widget_set_hexpand(track, TRUE);
    gtk_widget_set_halign(track, GTK_ALIGN_END);

    led = gtk_drawing_area_new();
    gtk_widget_set_size_request(led, 30, 15);
    gtk_widget_set_no_show_all(led, TRUE);
    gtk_container_add(GTK_CONTAINER(grid), number);
    gtk_container_add(GTK_CONTAINER(grid), track);
    gtk_container_add(GTK_CONTAINER(grid), led);
    /* Labels will notice clicks by default, but drawing areas need to
     * be told to. */
    gtk_widget_add_events(led, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(led, "draw", G_CALLBACK(draw_drive_led_cb), GINT_TO_POINTER(unit));
    return grid;
}


/** \brief Respond to mouse clicks on the tape status widget.
 *
 *  This displays the tape control popup menu.
 *
 *  \param widget  The GtkWidget that received the click. Ignored.
 *  \param event   The event representing the bottom operation.
 *  \param data    An integer representing which window's status bar was
 *                 clicked and thus where the popup window should go.
 *
 *  \return TRUE if further event processing should be skipped.
 *
 *  \todo This callback and the way it is configured both will need to
 *        be significantly reworked to manage multiple tape drives.
 */
static gboolean ui_do_datasette_popup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    int i = GPOINTER_TO_INT(data);

    if (((GdkEventButton *)event)->button == GDK_BUTTON_PRIMARY) {
        if (allocated_bars[i].tape && allocated_bars[i].tape_menu
                && event->type == GDK_BUTTON_PRESS) {
            gtk_menu_popup_at_widget(GTK_MENU(allocated_bars[i].tape_menu),
                                     allocated_bars[i].tape,
                                     GDK_GRAVITY_NORTH_EAST,
                                     GDK_GRAVITY_SOUTH_EAST,
                                     event);
        }
        return TRUE;
    } else if (((GdkEventButton *)event)->button == GDK_BUTTON_SECONDARY) {
        GtkWidget *dir_menu;
        debug_gtk3("Got SECONDARY BUTTON");

        dir_menu = dir_menu_popup_create(-1, tapecontents_read,
                tape_dir_autostart_callback);
        gtk_menu_popup_at_widget(GTK_MENU(dir_menu),
                                 widget,
                                 GDK_GRAVITY_NORTH_EAST,
                                 GDK_GRAVITY_SOUTH_EAST,
                                 event);
        return TRUE;
    }
    return FALSE;
}

#if 0
/** \brief  Handler for the "response" event of the directory dialog
 *
 * \param[in,out]   dialog      dialog triggering the event
 * \param[in]       response_id response ID
 * \param[in]       user_data   extra event data (unused)
 */
static void on_response_dir_widget(GtkDialog *dialog,
                                   gint response_id,
                                   gpointer user_data)
{
    debug_gtk3("got response ID = %d.", response_id);
    switch (response_id) {
        case GTK_RESPONSE_CLOSE:
            gtk_widget_destroy(GTK_WIDGET(dialog));
            break;
        default:
            debug_gtk3("unhandled response ID %d,", response_id);
    }
}
#endif

#if 0
/** \brief  Handler for the double-click event of the directory listing
 *
 * \param[in,out]   dialog      dialog triggering the event
 * \param[in]       autostart   autostart flag (FIXME: unused, this is due to
 *                              a hack in the other widgets that use the content
 *                              preview widget)
 * \param[in]       dir_index   directory index of the file to autosart
 */
static void on_dir_widget_selected(GtkWidget *widget,
                                   gint autostart,
                                   gpointer dir_index)
{
    int index = GPOINTER_TO_INT(dir_index);
    debug_gtk3("called with file index %d.", index);
    autostart_disk(autostart_diskimage, NULL, index,
            AUTOSTART_MODE_RUN);
    gtk_widget_destroy(widget);
}
#endif


static void disk_dir_autostart_callback(const char *image, int index)
{
    const char *autostart_image;

    debug_gtk3("Got image '%s', file index %d to autostart",
            image, index);
    /* make a copy of the image name since autostart will reattach the disk
     * image, freeing memory used by the image name passed to us in the process
     */
    autostart_image = lib_stralloc(image);
    autostart_disk(autostart_image, NULL, index + 1, AUTOSTART_MODE_RUN);
    lib_free(autostart_image);
}



static void tape_dir_autostart_callback(const char *image, int index)
{
    const char *autostart_image;

    debug_gtk3("Got image '%s', file index %d to autostart",
            image, index);
    /* make a copy of the image name since autostart will reattach the tape
     * image, freeing memory used by the image name passed to us in the process
     */
    autostart_image = lib_stralloc(image);
    autostart_tape(autostart_image, NULL, index + 1, AUTOSTART_MODE_RUN);
    lib_free(autostart_image);
}


#if 0
/** \brief  Show dialog to run a file of the image attached to \a unit
 *
 * Double clicking on a file will autorun that file. Clicking on Cancel or the
 * 'X' in the Window will cancel the operation.
 *
 * \param[in]   dev     drive index (0-3)
 *
 * \return  GtkDialog
 */
static GtkWidget *ui_statusbar_dir_dialog_create(int dev)
{
    GtkWidget *dialog;
    GtkWidget *content;
    GtkWidget *preview;
    struct drive_context_s *drv_ctx;
    struct drive_s *drv_s;
    struct disk_image_s *image;

    autostart_diskimage = NULL;

    debug_gtk3("Got drive index #%d", dev);
    dialog = gtk_dialog_new_with_buttons(
            "Select file to autostart",
            ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Cancel", GTK_RESPONSE_CLOSE,
            NULL);

    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    preview = content_preview_widget_create(GTK_WIDGET(dialog),
            diskcontents_filesystem_read,
            on_dir_widget_selected);
    gtk_widget_set_size_request(preview, 300, 500);

    /*
     * The following is complete horseshit, this needs to be implemented in a
     * function in drive/vdrive somehow. This much dereferencing in UI code
     * is not normal method.
     */
    drv_ctx = drive_context[dev];
    if (drv_ctx != NULL) {
        drv_s = drv_ctx->drive;
        if (drv_s != NULL) {
            image = drv_s->image;
            if (image != NULL) {
                /* this assumes fsimage, not real-image */
                struct fsimage_s *fsimg = image->media.fsimage;
                if (fsimg != NULL) {
                    autostart_diskimage = fsimg->name;
                }
            }
        }
    }

    debug_gtk3("reading dir of '%s'", autostart_diskimage);
    if (autostart_diskimage != NULL) {
        content_preview_widget_set_image(preview, autostart_diskimage);
    }

    gtk_container_add(GTK_CONTAINER(content), preview);
    return dialog;
}
#endif


/** \brief Respond to mouse clicks on a disk drive status widget.
 *
 *  This displays the drive control popup menu.
 *
 *  \param widget  The GtkWidget that received the click. Ignored.
 *  \param event   The event representing the bottom operation.
 *  \param data    An integer representing which window's status bar was
 *                 clicked and thus where the popup window should go.
 *
 *  \return TRUE if further event processing should be skipped.
 *
 *  \todo This function uses GTK3 version checking to avoid deprecated
 *        functions on version 3.22 and to avoid nonexistent functions
 *        on version 3.16 through 3.20. Once 3.22 becomes a
 *        requirement, this version checking should be eliminated.
 */
static gboolean ui_do_drive_popup(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    int i = GPOINTER_TO_INT(data);
    GtkWidget *drive_menu = allocated_bars[0].drive_popups[i];
    GtkWidget *drive_menu_item;
    gchar buffer[256];

    ui_populate_fliplist_menu(drive_menu, i + 8, 0);

    /* XXX: this code is a duplicate of the drive_menu creation code, so we
     *      should probably refactor this a bit
     */
    gtk_container_add(GTK_CONTAINER(drive_menu),
            gtk_separator_menu_item_new());
    drive_menu_item = gtk_menu_item_new_with_label("Configure drives ...");
    g_signal_connect(drive_menu_item, "activate",
            G_CALLBACK(on_drive_configure_activate), NULL);
    gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);

    /*
     * Add drive reset item
     */
    g_snprintf(buffer, 256, "Reset drive #%d", i + 8);
    drive_menu_item = gtk_menu_item_new_with_label(buffer);
    g_signal_connect(drive_menu_item, "activate",
            G_CALLBACK(on_drive_reset_clicked), GINT_TO_POINTER(i));
    gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);

    /* Add reset to configuration mode for CMD HDs */
    if ((drive_has_buttons(i) & 1) == 1) {
        g_snprintf(buffer, sizeof(buffer),
                "Reset drive #%d to Configuration Mode", i + 8);
        drive_menu_item = gtk_menu_item_new_with_label(buffer);
        g_signal_connect(drive_menu_item, "activate",
               G_CALLBACK(on_drive_reset_config_clicked),
               GINT_TO_POINTER((i << 4) + 1));
        gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);
    }
    /* Add reset to installation mode for CMD HDs */
    if ((drive_has_buttons(i) & 6) == 6) {
        g_snprintf(buffer, sizeof(buffer),
                "Reset drive #%d to Installation Mode", i + 8);
        drive_menu_item = gtk_menu_item_new_with_label(buffer);
        g_signal_connect(drive_menu_item, "activate",
               G_CALLBACK(on_drive_reset_config_clicked),
               GINT_TO_POINTER((i << 4) + 6));
        gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);
    }


    gtk_widget_show_all(drive_menu);
    /* 3.22 isn't available on the latest stable version of all
     * distros yet. This is expected to change when Ubuntu 18.04 is
     * released. Since popup handling is the only thing 3.22 requires
     * be done differently than its predecessors, this is the only
     * place we should rely on version checks. */

    if (((GdkEventButton *)event)->button == GDK_BUTTON_PRIMARY) {
        /* show popup for attaching/detaching disk images */
        gtk_menu_popup_at_widget(GTK_MENU(drive_menu),
                                 widget,
                                 GDK_GRAVITY_NORTH_EAST,
                                 GDK_GRAVITY_SOUTH_EAST,
                                 event);
    } else if (((GdkEventButton *)event)->button == GDK_BUTTON_SECONDARY) {
        /* show popup to run file in currently attached image */
        GtkWidget *dir_menu = dir_menu_popup_create(
                i,
                diskcontents_filesystem_read,
                disk_dir_autostart_callback);
#if 0
        GtkWidget *dialog;

        debug_gtk3("Got secondary button click event");
        dialog = ui_statusbar_dir_dialog_create(i);
        g_signal_connect(dialog, "response",
                G_CALLBACK(on_response_dir_widget), NULL);
        gtk_widget_show_all(dialog);
#endif
        /* show popup for selecting file in currently attached image */
        gtk_menu_popup_at_widget(GTK_MENU(dir_menu),
                                 widget,
                                 GDK_GRAVITY_NORTH_EAST,
                                 GDK_GRAVITY_SOUTH_EAST,
                                 event);
    }

    return TRUE;
}

/** \brief Create a new tape widget for inclusion in the status bar.
 *
 *  \return The constructed widget. This widget will be a floating
 *          reference.
 */
static GtkWidget *ui_tape_widget_create(void)
{
    GtkWidget *grid, *header, *counter, *state;

    grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(grid, FALSE);
    header = gtk_label_new("Tape:");
    gtk_widget_set_hexpand(header, TRUE);
    gtk_widget_set_halign(header, GTK_ALIGN_START);

    counter = gtk_label_new("000");
    state = gtk_drawing_area_new();
    gtk_widget_set_size_request(state, 20, 20);
    /* Labels will notice clicks by default, but drawing areas need to
     * be told to. */
    gtk_widget_add_events(state, GDK_BUTTON_PRESS_MASK);
    gtk_container_add(GTK_CONTAINER(grid), header);
    gtk_container_add(GTK_CONTAINER(grid), counter);
    gtk_container_add(GTK_CONTAINER(grid), state);
    g_signal_connect(state, "draw", G_CALLBACK(draw_tape_icon_cb), GINT_TO_POINTER(0));
    return grid;
}

/** \brief Alter widget visibility within the joyport widget so that
 *         only currently existing joystick ports are displayed. 
 *
 *  It is safe to call this routine regularly, as it will only trigger
 *  UI refresh operations if the configuration has changed to no
 *  longer match the current layout. */
static void vice_gtk3_update_joyport_layout(void)
{
    int i, ok[JOYPORT_MAX_PORTS];
    int userport_joysticks = 0;
    int new_joyport_mask = 0;
    /* Start with all ports enabled */
    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        ok[i] = 1;
    }
    /* Check for userport joystick counts */
    if ((machine_class != VICE_MACHINE_CBM5x0) &&
            (machine_class != VICE_MACHINE_VSID)) {
        int upjoy = 0;
        resources_get_int("UserportJoy", &upjoy);
        if (upjoy) {
            ++userport_joysticks;
        }
        if (machine_class != VICE_MACHINE_C64DTV) {
            int uptype = USERPORT_JOYSTICK_HUMMER;
            resources_get_int("UserportJoyType", &uptype);
            if ((uptype != USERPORT_JOYSTICK_HUMMER) &&
                (uptype != USERPORT_JOYSTICK_OEM) &&
                (upjoy != 0)) {
                ++userport_joysticks;
            }
        }

    }
    /* Port 1 disabled for machines that have no internal joystick
     * ports */
    if ((machine_class == VICE_MACHINE_CBM6x0) ||
        (machine_class == VICE_MACHINE_PET) ||
        (machine_class == VICE_MACHINE_VSID)) {
        ok[0] = 0;
    }
    /* Port 2 disabled for machines that have at most one internal
     * joystick ports */
    if ((machine_class == VICE_MACHINE_VIC20) ||
        (machine_class == VICE_MACHINE_CBM6x0) ||
        (machine_class == VICE_MACHINE_PET) ||
        (machine_class == VICE_MACHINE_VSID)) {
        ok[1] = 0;
    }
    /* Port 3 disabled for machines with no user port and no other
     * joystick adapter type, or where no userport joystick is
     * configured */
    if ((machine_class == VICE_MACHINE_CBM5x0) || (userport_joysticks < 1)) {
        ok[2] = 0;
    }
    /* Port 4 disabled for machines with no user port, or not enough
     * userport lines for 2 port userport adapters, or where at most
     * one userport joystick is configured */
    if ((machine_class == VICE_MACHINE_CBM5x0) ||
        (machine_class == VICE_MACHINE_C64DTV) ||
        (userport_joysticks < 2)) {
        ok[3] = 0;
    }
    /* Port 5 disabled for machines with no 5th control port,  */
    if (machine_class != VICE_MACHINE_PLUS4) {
        ok[4] = 0;
    } else {
        /* Port 5 also disabled if there's no SID joystick configured */
        int sidjoy = 0;
        resources_get_int("SIDCartJoy", &sidjoy);
        if (!sidjoy) {
            ok[4] = 0;
        }
    }
    /* Now that we have a list of disabled/enabled ports, let's check
     * to see if anything has changed */
    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        new_joyport_mask <<= 1;
        if (ok[i]) {
            new_joyport_mask |= 1;
        }
    }
    if (new_joyport_mask != sb_state.joyports_enabled) {
        int j;
        sb_state.joyports_enabled = new_joyport_mask;
        for (j = 0; j < MAX_STATUS_BARS; ++j) {
            GtkWidget *joyports_grid;

            if (allocated_bars[j].joysticks == NULL) {
                continue;
            }
            joyports_grid =  gtk_bin_get_child(
                    GTK_BIN(allocated_bars[j].joysticks));

            /* Hide and show the joystick ports as required */
            for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
                GtkWidget *child = gtk_grid_get_child_at(GTK_GRID(joyports_grid), 1+i, 0);
                if (child) {
                    if (ok[i]) {
                        gtk_widget_set_no_show_all(child, FALSE);
                        gtk_widget_show_all(child);
                    } else {
                        gtk_widget_set_no_show_all(child, TRUE);
                        gtk_widget_hide(child);
                    }
                }
            }
        }
    }
}


/** \brief  Cursor used when hovering over the joysticks widgets
 *
 * TODO:    figure out if I need to clean this up or that Gtk will?
 */
static GdkCursor *joywidget_mouse_ptr = NULL;


/** \brief  Handler for the enter/leave-notify events of the joysticks widget
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   event       event reference
 * \param[in]   user_data   extra event data (unused)
 *
 * \return  bool    (FALSE = keep propagating event, TRUE = stop)
 */
static gboolean on_joystick_widget_hover(GtkWidget *widget, GdkEvent *event,
                                         gpointer user_data)
{
    if (event != NULL) {
        GdkDisplay *display = gtk_widget_get_display(widget);
        GdkWindow *window = gtk_widget_get_window(widget);
        GdkCursor *cursor;

        if (display == NULL) {
            debug_gtk3("failed to retrieve GdkDisplay.");
            return FALSE;
        }
        if (window == NULL) {
            debug_gtk3("failed to retrieve GdkWindow.");
            return FALSE;
        }

        if (event->type == GDK_ENTER_NOTIFY) {
            debug_gtk3("Entered joystick widget area.");
            if (joywidget_mouse_ptr == NULL) {
                joywidget_mouse_ptr = gdk_cursor_new_from_name(display, "pointer");
            }
            cursor = joywidget_mouse_ptr;

        } else {
            debug_gtk3("Left joystick widget area.");
            cursor = NULL;
        }
        gdk_window_set_cursor(window, cursor);
    }
    return FALSE;
}


/** \brief  Handler for button-press events of the joysticks widget
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   event       event reference
 * \param[in]   user_data   extra event data (unused)
 *
 * \return  TRUE to stop other handlers, FALSE to propagate event further
 */
static gboolean on_joystick_widget_button_press(GtkWidget *widget,
                                                GdkEvent *event,
                                                gpointer user_data)
{
    debug_gtk3("Got button click");

    if (((GdkEventButton *)event)->button == GDK_BUTTON_PRIMARY) {
        GtkWidget *menu = joystick_menu_popup_create();
        gtk_menu_popup_at_widget(GTK_MENU(menu), widget,
                GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_SOUTH_WEST,
                event);
        return TRUE;
    }
    return FALSE;
}




/** \brief Create a master joyport widget for inclusion in the status
 *         bar.
 *
 *  Individual joyport representations are part of this widget and
 *  update functions will index the GtkGrid in the master widget to
 *  reach them.
 *
 *  \return The constructed widget. This widget will be a floating
 *          reference.
 */
static GtkWidget *ui_joystick_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *event_box;
    int i;

    grid = gtk_grid_new();
    gtk_orientable_set_orientation(GTK_ORIENTABLE(grid), GTK_ORIENTATION_HORIZONTAL);
    gtk_widget_set_hexpand(grid, FALSE);
    label = gtk_label_new("Joysticks:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_container_add(GTK_CONTAINER(grid), label);
    /* Create all possible joystick displays */
    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        GtkWidget *joyport = gtk_drawing_area_new();
        /* add events it should respond to */
        gtk_widget_add_events(joyport,
                GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_ENTER_NOTIFY_MASK|GDK_LEAVE_NOTIFY_MASK);
        gtk_widget_set_size_request(joyport,20,20);
        gtk_container_add(GTK_CONTAINER(grid), joyport);
        g_signal_connect(joyport, "draw", G_CALLBACK(draw_joyport_cb), GINT_TO_POINTER(i));
        gtk_widget_set_no_show_all(joyport, TRUE);
        gtk_widget_hide(joyport);
    }

    /*
     * Pack the joystick grid into an event box so we can have a popup menu and
     * also change the cursor shape to indicate to the user the joystick widget
     * is clickable.
     */
    event_box = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);
    gtk_container_add(GTK_CONTAINER(event_box), grid);



    g_signal_connect(event_box, "button-press-event",
            G_CALLBACK(on_joystick_widget_button_press), NULL);
    g_signal_connect(event_box, "enter-notify-event",
            G_CALLBACK(on_joystick_widget_hover), NULL);
    g_signal_connect(event_box, "leave-notify-event",
            G_CALLBACK(on_joystick_widget_hover), NULL);

    return event_box;
}

/** Event handler for hovering over a clickable part of the status bar.
 *
 *  This will switch to or from the "hand" cursor as needed, creating
 *  it if necessary.
 *
 *  \param widget    The widget firing the event
 *  \param event     The GdkEventCross that caused the callback
 *  \param user_data The ui_statusbar_t object containing widget
 *
 *  \return TRUE if further event processing should be blocked.
 */
static gboolean ui_statusbar_cross_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    ui_statusbar_t *sb = (ui_statusbar_t *)user_data;
    if (event && event->type == GDK_ENTER_NOTIFY) {
        GdkDisplay *display;
        /* Sanity check arguments */
        if (sb == NULL) {
            /* Should be impossible */
            return FALSE;
        }
        /* If the "hand" pointer hasn't been created yet, create it */
        display = gtk_widget_get_display(widget);
        if (display != NULL && sb->hand_ptr == NULL) {
            sb->hand_ptr = gdk_cursor_new_from_name(display, "pointer");
            if (sb->hand_ptr != NULL) {
                g_object_ref_sink(G_OBJECT(sb->hand_ptr));
            } else {
                fprintf(stderr, "GTK3 CURSOR: Could not allocate custom pointer for status bar\n");
            }
        }
        /* If the "hand" pointer is OK, use it */
        if (sb->hand_ptr != NULL) {
            GdkWindow *window = gtk_widget_get_window(widget);
            if (window) {
                gdk_window_set_cursor(window, sb->hand_ptr);
            }
        }
    } else {
        /* We're leaving the target widget, so change the pointer back
         * to default */
        GdkWindow *window = gtk_widget_get_window(widget);
        if (window) {
            gdk_window_set_cursor(window, NULL);
        }
    }
    return FALSE;
}

/** \brief Lay out the disk drive widgets inside a status bar.
 *
 *  Depending on which drives are enabled, any given drive may appear
 *  on different columns or even rows. This routine handles that flow
 *  as the configuration changes.
 *
 *  \param bar_index Which status bar to lay out.
 */
static void layout_statusbar_drives(int bar_index)
{
    int i, j, state, tde = 0;
    int enabled_drive_index = 0;
    GtkWidget *bar = allocated_bars[bar_index].bar;
    if (!bar) {
        return;
    }
    /* Delete all the drives and dividers that may exist. WARNING:
     * This code assumes that the drive widgets are the rightmost
     * elements of the status bar. */
    for (i = 0; i < ((DRIVE_NUM + 1) / 2) * 2; ++i) {
        for (j = 0; j < 2; ++j) {
            GtkWidget *child = gtk_grid_get_child_at(GTK_GRID(bar), 6+i, j);
            if (child) {
                /* Fun GTK3 fact! If you destroy an event box, then
                 * even if the thing it contains still has references
                 * left, that child is destroyed _anyway_. To avoid
                 * this tragic eventuality, we detach the child files
                 * before removing the box. */
                /* TODO: This implies that we really should not be
                 * relying on g_object_ref to preserve objects, and
                 * instead keep track of widget indices the hard
                 * way. */
                if (GTK_IS_EVENT_BOX(child)) {
                    GtkWidget *grandchild = gtk_bin_get_child(GTK_BIN(child));
                    gtk_container_remove(GTK_CONTAINER(child), grandchild);
                }
                gtk_container_remove(GTK_CONTAINER(bar), child);
            }
        }
    }
    state = sb_state.drives_enabled;
    tde = sb_state.drives_tde_enabled;
    for (i = 0; i < DRIVE_NUM; ++i) {
        if (state & 1) {
            GtkWidget *drive = allocated_bars[bar_index].drives[i];
            GtkWidget *event_box = gtk_event_box_new();
            int row = enabled_drive_index % 2;
            int column = (enabled_drive_index / 2) * 2 + SB_COL_DRIVE;
            if (row == 0) {
                gtk_grid_attach(GTK_GRID(bar),
                        gtk_separator_new(GTK_ORIENTATION_VERTICAL),
                        column - 1, 0, 1, 2);
            }
            gtk_container_add(GTK_CONTAINER(event_box), drive);
            gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);
            g_signal_connect(event_box, "button-press-event", G_CALLBACK(ui_do_drive_popup), GINT_TO_POINTER(i));
            g_signal_connect(event_box, "enter-notify-event", G_CALLBACK(ui_statusbar_cross_cb), &allocated_bars[i]);
            g_signal_connect(event_box, "leave-notify-event", G_CALLBACK(ui_statusbar_cross_cb), &allocated_bars[i]);
            gtk_widget_show_all(event_box);
            if (tde & 1) {
                gtk_widget_show(gtk_grid_get_child_at(GTK_GRID(drive), 2, 0));
            } else {
                gtk_widget_hide(gtk_grid_get_child_at(GTK_GRID(drive), 2, 0));
            }
            gtk_grid_attach(GTK_GRID(bar), event_box, column, row, 1, 1);
            ++enabled_drive_index;
        }
        state >>= 1;
        tde >>= 1;
    }
    gtk_widget_show_all(bar);
}

/** Widget destruction callback for status bars.
 *
 * \param sb      The status bar being destroyed. This should be
 *                registered in some ui_statusbar_t structure as the
 *                bar field.
 * \param ignored User data pointer mandated by GTK. Unused.
 */
static void destroy_statusbar_cb(GtkWidget *sb, gpointer ignored)
{
    int i, j;

    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar == sb) {
            allocated_bars[i].bar = NULL;
#if 0
            if (allocated_bars[i].msg) {
                g_object_unref(G_OBJECT(allocated_bars[i].msg));
                allocated_bars[i].msg = NULL;
            }
#endif
            if (allocated_bars[i].speed != NULL) {
                g_object_unref(G_OBJECT(allocated_bars[i].speed));
                allocated_bars[i].speed = NULL;
            }

            if (allocated_bars[i].crt != NULL) {
                g_object_unref(G_OBJECT(allocated_bars[i].crt));
                allocated_bars[i].crt = NULL;
            }
            if (allocated_bars[i].mixer != NULL) {
                g_object_unref(G_OBJECT(allocated_bars[i].mixer));
                allocated_bars[i].mixer = NULL;
            }
            if (allocated_bars[i].tape) {
                g_object_unref(G_OBJECT(allocated_bars[i].tape));
                allocated_bars[i].tape = NULL;
            }
            if (allocated_bars[i].tape_menu) {
                g_object_unref(G_OBJECT(allocated_bars[i].tape_menu));
                allocated_bars[i].tape_menu = NULL;
            }
            if (allocated_bars[i].joysticks) {
                g_object_unref(G_OBJECT(allocated_bars[i].joysticks));
                allocated_bars[i].joysticks = NULL;
            }
            for (j = 0; j < DRIVE_NUM; ++j) {
                if (allocated_bars[i].drives[j]) {
                    g_object_unref(G_OBJECT(allocated_bars[i].drives[j]));
                    g_object_unref(G_OBJECT(allocated_bars[i].drive_popups[j]));
                    allocated_bars[i].drives[j] = NULL;
                    allocated_bars[i].drive_popups[j] = NULL;
                }
            }
            if (allocated_bars[i].hand_ptr) {
                g_object_unref(G_OBJECT(allocated_bars[i].hand_ptr));
                allocated_bars[i].hand_ptr = NULL;
            }
        }
    }
}


/** \brief  Handler for the 'toggled' event of the CRT controls checkbox
 *
 * Toggles the display state of the CRT controls
 *
 * \param[in]   widget  checkbox triggering the event
 * \param[in]   data    extra event data (unused
 */
static void on_crt_toggled(GtkWidget *widget, gpointer data)
{
    gboolean state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    ui_enable_crt_controls((gboolean)state);
}


/** \brief  Handler for the 'toggled' event of the mixer controls checkbox
 *
 * Toggles the display state of the mixer controls
 *
 * \param[in]   widget  checkbox triggering the event
 * \param[in]   data    extra event data (unused
 */
static void on_mixer_toggled(GtkWidget *widget, gpointer data)
{
    gboolean state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    ui_enable_mixer_controls((gboolean)state);
}



/** \brief Create a popup menu to attach to a disk drive widget.
 *
 *  \param unit The index of the drive, 0-3 for drives 8-11.
 *
 *  \return The GtkMenu for use as a popup, as a floating reference.
 */
static GtkWidget *ui_drive_menu_create(int unit)
{
    char buf[128];
    GtkWidget *drive_menu = gtk_menu_new();
    GtkWidget *drive_menu_item;

    snprintf(buf, 128, "Attach disk to drive #%d...", unit + 8);
    buf[127] = 0;

    drive_menu_item = gtk_menu_item_new_with_label(buf);
    g_signal_connect(drive_menu_item, "activate",
            G_CALLBACK(ui_disk_attach_callback), GINT_TO_POINTER(unit + 8));
    gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);
    snprintf(buf, 128, "Detach disk from drive #%d", unit + 8);
    buf[127] = 0;
    drive_menu_item = gtk_menu_item_new_with_label(buf);
    g_signal_connect(drive_menu_item, "activate",
            G_CALLBACK(ui_disk_detach_callback), GINT_TO_POINTER(unit + 8));
    gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);
    /* GTK2/GNOME UI put TDE and Read-only checkboxes here, but that
     * seems excessive or possibly too fine-grained, so skip that for
     * now */
    ui_populate_fliplist_menu(drive_menu, unit + 8, 0);


    gtk_container_add(GTK_CONTAINER(drive_menu),
            gtk_separator_menu_item_new());
    drive_menu_item = gtk_menu_item_new_with_label("Configure drives ...");
    g_signal_connect(drive_menu_item, "activate",
            G_CALLBACK(on_drive_configure_activate), NULL);
    gtk_container_add(GTK_CONTAINER(drive_menu), drive_menu_item);

    gtk_widget_show_all(drive_menu);
    return drive_menu;
}

/** \brief Create a new status bar.
 *
 *  This function should be called once as part of creating a new
 *  machine window.
 *
 *  \return A new status bar, as a floating reference, or NULL if all
 *          possible status bars have been allocated already.
 */
GtkWidget *ui_statusbar_create(void)
{
    GtkWidget *sb, *speed, *tape, *tape_events, *joysticks;
    GtkWidget *crt = NULL;
    GtkWidget *mixer = NULL;
    int i, j;

    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar == NULL) {
            break;
        }
    }

    if (i >= MAX_STATUS_BARS) {
        /* Fatal error? */
        return NULL;
    }

    /* While the status bar itself is returned floating, we sink all
     * of its information-bearing subwidgets. This is so that we can
     * remove or add them to the status bar as the configuration
     * demands, while ensuring they remain alive. They receive an
     * extra dereference in ui_statusbar_destroy() so nothing should
     * leak. */
    sb = vice_gtk3_grid_new_spaced(8, 0);

    /* First column: CPU/FPS */

    speed = statusbar_speed_widget_create();
    g_object_ref_sink(G_OBJECT(speed));
    g_object_set(speed, "margin-left", 8, NULL);

    /* don't add CRT or Mixer controls when VSID */
    if (machine_class != VICE_MACHINE_VSID) {
        crt = gtk_check_button_new_with_label("CRT controls");
        gtk_widget_set_can_focus(crt, FALSE);
        g_object_ref_sink(G_OBJECT(crt));
        gtk_widget_set_halign(crt, GTK_ALIGN_START);
        gtk_widget_show_all(crt);
        g_signal_connect(crt, "toggled", G_CALLBACK(on_crt_toggled), NULL);

        mixer = gtk_check_button_new_with_label("Mixer controls");
        gtk_widget_set_can_focus(mixer, FALSE);
        g_object_ref_sink(G_OBJECT(mixer));
        gtk_widget_set_halign(mixer, GTK_ALIGN_START);
        gtk_widget_show_all(mixer);
        g_signal_connect(mixer, "toggled", G_CALLBACK(on_mixer_toggled), NULL);
    }

    g_signal_connect(sb, "destroy", G_CALLBACK(destroy_statusbar_cb), NULL);
    allocated_bars[i].bar = sb;
    allocated_bars[i].speed = speed;
    gtk_grid_attach(GTK_GRID(sb), speed, SB_COL_SPEED, 0, 1, 2);

    /* Second column: separator */
    gtk_grid_attach(GTK_GRID(sb), gtk_separator_new(GTK_ORIENTATION_VERTICAL),
            SB_COL_SEP_MSG, 0, 1, 2);

    /* TODO: skip VSID and add another separator after the checkbox */
    if (machine_class != VICE_MACHINE_VSID) {
        allocated_bars[i].crt = crt;
        gtk_grid_attach(GTK_GRID(sb), crt, SB_COL_CRT, 0, 1, 1);
        allocated_bars[i].mixer = mixer;
        gtk_grid_attach(GTK_GRID(sb), mixer, SB_COL_CRT, 1, 1, 1);
        /* add separator */
        gtk_grid_attach(GTK_GRID(sb), gtk_separator_new(GTK_ORIENTATION_VERTICAL),
                SB_COL_SEP_CRT, 0, 1, 2);
    }

    if ((machine_class != VICE_MACHINE_C64DTV)
            && (machine_class != VICE_MACHINE_VSID)) {
        tape = ui_tape_widget_create();
        g_object_ref_sink(G_OBJECT(tape));
        /* Clicking the tape status is supposed to pop up a window. This
         * requires a way to make sure events are captured by random
         * internal widgets; the GtkEventBox manages that task for us. */
        tape_events = gtk_event_box_new();
        gtk_event_box_set_visible_window(GTK_EVENT_BOX(tape_events), FALSE);
        gtk_container_add(GTK_CONTAINER(tape_events), tape);
        gtk_grid_attach(GTK_GRID(sb), tape_events, SB_COL_TAPE, 0, 1, 1);
        allocated_bars[i].tape = tape;
        allocated_bars[i].tape_menu = ui_create_datasette_control_menu();
        g_object_ref_sink(G_OBJECT(allocated_bars[i].tape_menu));
        g_signal_connect(tape_events, "button-press-event",
                G_CALLBACK(ui_do_datasette_popup), GINT_TO_POINTER(i));
        g_signal_connect(tape_events, "enter-notify-event",
                G_CALLBACK(ui_statusbar_cross_cb), &allocated_bars[i]);
        g_signal_connect(tape_events, "leave-notify-event",
                G_CALLBACK(ui_statusbar_cross_cb), &allocated_bars[i]);
    }

    if (machine_class != VICE_MACHINE_VSID) {
        joysticks = ui_joystick_widget_create();
        g_object_ref(joysticks);
        gtk_widget_set_halign(joysticks, GTK_ALIGN_END);
        gtk_grid_attach(GTK_GRID(sb), joysticks, SB_COL_TAPE, 1, 1, 1);
        allocated_bars[i].joysticks = joysticks;
    }

    /* Third column on: Drives. */
    for (j = 0; j < DRIVE_NUM; ++j) {
        GtkWidget *drive = ui_drive_widget_create(j);
        GtkWidget *drive_menu = ui_drive_menu_create(j);
        g_object_ref_sink(G_OBJECT(drive));
        g_object_ref_sink(G_OBJECT(drive_menu));
        allocated_bars[i].drives[j] = drive;
        allocated_bars[i].drive_popups[j] = drive_menu;
    }
    /* WARNING: The current implementation of ui_enable_drive_status()
     * relies on the fact that the drives are the last elements of the
     * statusbar display. If more widgets are added past this point,
     * that function will need to change as well. */
    layout_statusbar_drives(i);

    /* Set an impossible number of joyports to enabled so that the status
     * is guarenteed to be updated. */
    sb_state.joyports_enabled = ~0;
    vice_gtk3_update_joyport_layout();
    return sb;
}

/** \brief Statusbar API function to register an elapsed time.
 *
 *  \param current The current time value.
 *  \param total   The maximum time value
 *
 *  \todo This function is not implemented and its API is not
 *        understood.
 *
 * \note    Since the statusbar message display widget has been removed, we
 *          have some space to implement a widget to display information
 *          regarding playback/recording.
 */
void ui_display_event_time(unsigned int current, unsigned int total)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

/** \brief Statusbar API function to display playback status.
 *
 *  \param playback_status Unknown.
 *  \param version         Unknown.
 *
 *  \todo This function is not implemented and its API is not
 *        understood.
 *
 * \note    Since the statusbar message display widget has been removed, we
 *          have some space to implement a widget to display information
 *          regarding playback/recording.
 */
void ui_display_playback(int playback_status, char *version)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

/** \brief Statusbar API function to display recording status.
 *
 *  \param recording_status Unknown.
 *
 *  \todo This function is not implemented and its API is not
 *        understood.
 *
 * \note    Since the statusbar message display widget has been removed, we
 *          have some space to implement a widget to display information
 *          regarding playback/recording.
 */
void ui_display_recording(int recording_status)
{
    NOT_IMPLEMENTED_WARN_ONLY();
}

#if 0
/** \brief Directly and unconditionally set the status bar message text.
 *
 *  \param text The text to display in the status bar.
 */
static void
display_statustext_internal(const char *text)
{
#if 0
    int i;
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].msg) {
            gtk_label_set_text(allocated_bars[i].msg, text);
        }
    }
#endif
}
#endif

#if 0
/** \brief Timeout callback for messages in the status bar.
 *
 *  If the message ID associated with this callback matches the
 *  currently displayed message, erases the current message.
 *
 *  \param data The message ID for this callback.
 *  \return TRUE if processing of the timeout callback should not
 *          propagate.
 *  \sa ui_sb_state_s::statustext_msgid
 */
static gboolean ui_statustext_fadeout(gpointer data)
{
    intptr_t my_id = GPOINTER_TO_INT(data);
    if (my_id == sb_state.statustext_msgid) {
        display_statustext_internal("");
    }
    return FALSE;
}
#endif

/** \brief Statusbar API function to display a message in the status bar.
 *
 *  \param text     The text to display.
 *  \param fade_out If nonzero, erase the text after five seconds
 *                  unless it has already been replaced.
 */
void ui_display_statustext(const char *text, int fade_out)
{
    /*
     * No longer used, but needs to remain here since it's called from
     * src/network.c
     */
#if 0
    ++sb_state.statustext_msgid;
    display_statustext_internal(text);
    if (fade_out) {
        g_timeout_add(5000, ui_statustext_fadeout,
                GINT_TO_POINTER(sb_state.statustext_msgid));
    }
#endif
}

/** \brief Statusbar API function to display current volume
 *
 * This function is a NOP since the volume can be checked and altered via the
 * Mixer Controls via the statusbar.
 *
 * \param[in]   vol     new volume level
 */
void ui_display_volume(int vol)
{
    /* NOP */
}


/** \brief Statusbar API function to display current joyport inputs.
 *  \param joyport An array of bytes of size at least
 *                 JOYPORT_MAX_PORTS+1, with data regarding each
 *                 active joyport.
 *  \warning The joyport array is, for all practical purposes,
 *           _1-indexed_. joyport[0] is unused.
 *  \sa ui_sb_state_s::current_joyports Describes the format of the
 *      data encoded in the joyport array. Note that current_joyports
 *      is 0-indexed as is typical for C arrays.
 */
void ui_display_joyport(uint8_t *joyport)
{
    int i;
    for (i = 0; i < JOYPORT_MAX_PORTS; ++i) {
        /* Compare the new value to the current one, set the new
         * value, and queue a redraw if and only if there was a
         * change. And yes, the input joystick ports are 1-indexed. I
         * don't know either. */
        if (sb_state.current_joyports[i] != joyport[i+1]) {
            int j;
            sb_state.current_joyports[i] = joyport[i+1];
            for (j = 0; j < MAX_STATUS_BARS; ++j) {
                if (allocated_bars[j].joysticks) {
                    GtkWidget *grid;
                    GtkWidget *widget;

                    grid = gtk_bin_get_child(GTK_BIN(allocated_bars[j].joysticks));
                    widget = gtk_grid_get_child_at(GTK_GRID(grid), i + 1, 0);
                    if (widget) {
                        gtk_widget_queue_draw(widget);
                    }
                }
            }
        }
    }
    /* Restrict visible joystick display to just the ones the
     * configuration supports */
    vice_gtk3_update_joyport_layout();
}

/** \brief Statusbar API function to report changes in tape control
 *         status.
 *
 *  \param control The new tape control. See the DATASETTE_CONTROL_*
 *                 constants in datasette.h for legal values of this
 *                 parameter.
 */
void ui_display_tape_control_status(int control)
{
    if (control != sb_state.tape_control) {
        int i;
        sb_state.tape_control = control;
        for (i = 0; i < MAX_STATUS_BARS; ++i) {
            if (allocated_bars[i].tape) {
                GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(allocated_bars[i].tape), 2, 0);
                if (widget) {
                    gtk_widget_queue_draw(widget);
                }
            }
        }
    }
}

/** \brief Statusbar API function to report changes in tape position.
 *
 *  \param counter The new value of the position counter. 
 *
 *  \note Only the last three digits of the counter will be displayed.
 */
void ui_display_tape_counter(int counter)
{
    if (counter != sb_state.tape_counter) {
        int i;
        char buf[8];
        snprintf(buf, 8, "%03d", counter%1000);
        buf[7] = 0;
        sb_state.tape_counter = counter;
        for (i = 0; i < MAX_STATUS_BARS; ++i) {
            if (allocated_bars[i].tape) {
                GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(allocated_bars[i].tape), 1, 0);
                if (widget) {
                    gtk_label_set_text(GTK_LABEL(widget), buf);
                }
            }
        }
    }
}

/** \brief Statusbar API function to report changes in the tape motor.
 *
 *  \param motor Nonzero if the tape motor is now on.
 */
void ui_display_tape_motor_status(int motor)
{
    if (motor != sb_state.tape_motor_status) {
        int i;
        sb_state.tape_motor_status = motor;
        for (i = 0; i < MAX_STATUS_BARS; ++i) {
            if (allocated_bars[i].tape) {
                GtkWidget *widget = gtk_grid_get_child_at(GTK_GRID(allocated_bars[i].tape), 2, 0);
                if (widget) {
                    gtk_widget_queue_draw(widget);
                }
            }
        }
    }
}

/** \brief Statusbar API function to report changes in tape status.
 *  \param tape_status The new tape status.
 *  \note This function does nothing and its API is not
 *        understood. Furthermore, no other extant UIs appear to react
 *        to this call.
 */
void ui_set_tape_status(int tape_status)
{
    /* printf("TAPE DRIVE STATUS: %d\n", tape_status); */
}

/** \brief Statusbar API function to report mounting or unmounting of
 *         a tape image.
 *
 *  \param image The filename of the tape image (if mounted), or the
 *               empty string or NULL (if unmounting).
 */
void ui_display_tape_current_image(const char *image)
{
#if 0
    char buf[256];
    if (image && *image) {
        snprintf(buf, 256, "Attached %s to tape unit", image);
    } else {
        snprintf(buf, 256, "Tape unit is empty");
    }

    buf[255] = 0;
    ui_display_statustext(buf, 1);
#endif
}

/** \brief Statusbar API function to report changes in drive LED
 *         intensity.
 *  \param drive_number The unit to update (0-3 for drives 8-11)
 *  \param pwm1         The intensity of the first LED (0=off,
 *                      1000=maximum intensity)
 *  \param led_pwm2     The intensity of the second LED (0=off,
 *                      1000=maximum intensity)
 *  \todo The statusbar API does not yet support dual-unit disk
 *        drives.
 */
void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int led_pwm2)
{
    int i;
    if (drive_number < 0 || drive_number > DRIVE_NUM-1) {
        /* TODO: Fatal error? */
        return;
    }
    sb_state.current_drive_leds[drive_number][0] = pwm1;
    sb_state.current_drive_leds[drive_number][1] = led_pwm2;
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar) {
            GtkWidget *drive, *led;
            drive = allocated_bars[i].drives[drive_number];
            led = gtk_grid_get_child_at(GTK_GRID(drive), 2, 0);
            if (led) {
                gtk_widget_queue_draw(led);
            }
        }
    }
}

/** \brief Statusbar API function to report changes in drive head
 *         location.
 *  \param drive_number      The unit to update (0-3 for drives 8-11)
 *  \param drive_base        Currently unused.
 *  \param half_track_number Twice the value of the head
 *                           location. 18.0 is 36, while 18.5 would be
 *                           37.
 *  \todo The statusbar API does not yet support dual-unit disk
 *        drives. The drive_base argument will likely come into play
 *        once it does.
 */
void ui_display_drive_track(unsigned int drive_number,
                            unsigned int drive_base,
                            unsigned int half_track_number)
{
    int i;
    if (drive_number > DRIVE_NUM-1) {
        /* TODO: Fatal error? */
        return;
    }
    for (i = 0; i < MAX_STATUS_BARS; ++i) {
        if (allocated_bars[i].bar) {
            GtkWidget *drive, *track;
            drive = allocated_bars[i].drives[drive_number];
            track = gtk_grid_get_child_at(GTK_GRID(drive), 1, 0);
            if (track) {
                char track_str[16];
                snprintf(track_str, 16, "%.1lf", half_track_number / 2.0);
                track_str[15] = 0;
                gtk_label_set_text(GTK_LABEL(track), track_str);
            }
        }
    }
}

/** \brief Update information about each drive.
 *
 *  \param state           A bitmask int, where bits 0-3 indicate
 *                         whether or not drives 8-11 respectively are
 *                         being emulated carefully enough to provide
 *                         LED information.
 *  \param drive_led_color An array of size at least DRIVE_NUM that
 *                         provides information about the LEDs on this
 *                         drive. An element of this array will only
 *                         be checked if the corresponding bit in
 *                         state is 1.
 *  \note Before calling this function, the drive configuration
 *        resources (Drive8Type, Drive9Type, etc) should all be set to
 *        the values you wish to display.
 *  \warning If a drive's LEDs are active when its LED values change,
 *           the UI will not reflect the LED type change until the
 *           next time the led's values are updated. This should not
 *           happen under normal circumstances.
 *  \sa compute_drives_enabled_mask() for how this function determines
 *      which drives are truly active
 *  \sa ui_sb_state_s::drive_led_types for the data in each element of
 *      drive_led_color
 */
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color)
{
    int i, enabled;

    /* Update the drive LEDs first, unconditionally. */
    enabled = state;
    for (i = 0; i < DRIVE_NUM; ++i) {
        if (enabled & 1) {
            sb_state.drive_led_types[i] = drive_led_color[i];
            sb_state.current_drive_leds[i][0] = 0;
            sb_state.current_drive_leds[i][1] = 0;
        }
        enabled >>= 1;
    }

    /* Now give enabled its "real" value based on the drive
     * definitions. */
    enabled = compute_drives_enabled_mask();

    /* Now, if necessary, update the status bar layouts. We won't need
     * to do this if the only change was the kind of drives hooked up,
     * instead of the number */
    if ((state != sb_state.drives_tde_enabled) || (enabled != sb_state.drives_enabled)) {
        sb_state.drives_enabled = enabled;
        sb_state.drives_tde_enabled = state;
        for (i = 0; i < MAX_STATUS_BARS; ++i) {
            layout_statusbar_drives(i);
        }
    }
}

/** \brief Statusbar API function to report mounting or unmounting of
 *         a disk image.
 *
 *  \param drive_number 0-3 to represent drives at device 8-11.
 *  \param image        The filename of the disk image (if mounted),
 *                      or the empty string or NULL (if unmounting).
 *  \todo This API is insufficient to describe drives with two disk units.
 */
void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
#if 0
    char buf[256];
    if (image && *image) {
        snprintf(buf, 256, "Attached %s to unit %d", image, drive_number + 8);
    } else {
        snprintf(buf, 256, "Unit %d is empty", drive_number + 8);
    }
    buf[255] = 0;
    ui_display_statustext(buf, 1);
#endif
}


/** \brief  Determine if the CRT controls widget is enabled in \a window
 *
 * \param[in]   window  GtkWindow instance
 *
 * \return  bool
 */
gboolean ui_statusbar_crt_controls_enabled(GtkWidget *window)
{
    GtkWidget *bin;
    GtkWidget *bar;
    GtkWidget *check;
    gboolean active;

    debug_gtk3("called.");
    bin = gtk_bin_get_child(GTK_BIN(window));
    if (bin != NULL) {
        bar = gtk_grid_get_child_at(GTK_GRID(bin), 0, 2);  /* FIX */
        if (bar != NULL) {
            check = gtk_grid_get_child_at(GTK_GRID(bar), SB_COL_CRT, 0);
            if (check != NULL) {
                active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
                debug_gtk3("CRT controls enabled = %s.",
                        active ? "TRUE" : "FALSE");
                return active;
            } else {
                debug_gtk3("Couldn't get Checkbox.");
            }
        } else {
            debug_gtk3("Couldn't get statusbar.");
        }
    } else {
        debug_gtk3("Couldn't get BIN.");
    }
    debug_gtk3("OOPS!");
    return FALSE;
}


/** \brief  Determine if the mixer controls widget is enabled in \a window
 *
 * \param[in]   window  GtkWindow instance
 *
 * \return  bool
 */
gboolean ui_statusbar_mixer_controls_enabled(GtkWidget *window)
{
    GtkWidget *bin;
    GtkWidget *bar;
    GtkWidget *check;
    gboolean active;

    debug_gtk3("called.");
    bin = gtk_bin_get_child(GTK_BIN(window));
    if (bin != NULL) {
        bar = gtk_grid_get_child_at(GTK_GRID(bin), 0, 2);  /* FIX */
        if (bar != NULL) {
            check = gtk_grid_get_child_at(GTK_GRID(bar), SB_COL_CRT, 1);
            if (check != NULL) {
                active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));
                debug_gtk3("Mixer controls enabled = %s.",
                        active ? "TRUE" : "FALSE");
                return active;
            }
        }
    }
    debug_gtk3("OOPS!");
    return FALSE;
}


void ui_display_speed(float percent, float framerate, int warp_flag)
{
    GtkWidget *speed;

    speed = allocated_bars[0].speed;
    if (speed != NULL) {
        statusbar_speed_widget_update(speed, percent, framerate, warp_flag);
    }
    speed = allocated_bars[1].speed;
    if (speed != NULL) {
        statusbar_speed_widget_update(speed, percent, framerate, warp_flag);
    }
}
