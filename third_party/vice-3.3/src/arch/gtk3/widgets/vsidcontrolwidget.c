/** \file   vsidcontrolwidget.c
 * \brief   GTK3 control widget for VSID
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "debug_gtk3.h"
#include "debug.h"
#include "machine.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "psid.h"
#include "uicommands.h"
#include "ui.h"
#include "uisidattach.h"
#include "vsidtuneinfowidget.h"

#include "vsidcontrolwidget.h"

/** \brief  Emulation speed during fast forward
 */
#define FFWD_SPEED  500





/** \brief  Object containing icon and callback
 */
typedef struct vsid_ctrl_button_s {
    const char *icon_name;                      /**< icon name */
    void (*callback)(GtkWidget *, gpointer);    /**< callback */
    const char *tooltip;                        /**< tool tip */
} vsid_ctrl_button_t;


/** \brief  Number of subtunes in the SID */
static int tune_count;

/** \brief  Current subtune number */
static int tune_current;

/** \brief  Default subtune number */
static int tune_default;


static GtkWidget *progress = NULL;

static GtkWidget *repeat = NULL;


/** \brief  Temporary callback for the media buttons
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void fake_callback(GtkWidget *widget, gpointer data)
{
    debug_gtk3("got callback for '%s'.", (const char *)data);
}


/** \brief  Callback for 'next subtune'
 *
 * Select next subtune, or wrap around to the first subtune
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void next_tune_callback(GtkWidget *widget, gpointer data)
{
    if (tune_current >= tune_count || tune_current <= 0 ) {
        tune_current = 1;
    } else {
        tune_current++;
    }

    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    psid_init_driver();
    machine_play_psid(tune_current);
}


/** \brief  Callback for 'previous subtune'
 *
 * Select previous subtune, or wrap aroun to the last subtune
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void prev_tune_callback(GtkWidget *widget, gpointer data)
{
    if (tune_current == 1) {
        tune_current = tune_count;
    } else {
        tune_current--;
    }

    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    psid_init_driver();
    machine_play_psid(tune_current);
}


/** \brief  Callback for 'fast forward'
 *
 * Fast forward using Speed resource (toggled)
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void ffwd_callback(GtkWidget *widget, gpointer data)
{
    int speed;

    if (resources_get_int("Speed", &speed) < 0) {
        /* error, shouldn't happen */
        return;
    }

    if (speed == 100) {
        resources_set_int("Speed", FFWD_SPEED);
    } else {
        resources_set_int("Speed", 100);
    }
}


/** \brief  Callback for 'play'
 *
 * Continue playback by using the emulator's pause feature.
 *
 * \note    ui_pause_emulation() appears to toggle pause mode when passed 1 and
 *          disable pause mode when passed 0.
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void play_callback(GtkWidget *widget, gpointer data)
{
    ui_pause_emulation(0);

    if (tune_current <= 0) {
        debug_gtk3("restarting with tune #%d.", tune_default);
        tune_current = tune_default;
        vsid_tune_info_widget_set_time(0);
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
        psid_init_driver();
        /* psid_init_tune(1); */
        machine_play_psid(tune_current);
    } else {
        /* return emulation speed back to 100% */
        resources_set_int("Speed", 100);
    }
}


/** \brief  Callback for 'pause'
 *
 * Pause playback by using the emulator's pause feature.
 *
 * \note    ui_pause_emulation() appears to toggle pause mode when passed 1 and
 *          disable pause mode when passed 0.
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void pause_callback(GtkWidget *widget, gpointer data)
{
    ui_pause_emulation(1);
}

/* XXX: this doesn't work and even segfaults when pushing play after a tune
 *      has played for some time.
 */
#if 0
static void stop_callback(GtkWidget *widget, gpointer data)
{
    debug_gtk3("called.");

#if 0
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
#endif
    machine_reset();
#if 0
    psid_init_driver();
    psid_init_tune(1);
#endif
    tune_current = -1;
}
#endif


/** \brief  List of media control buttons
 */
static const vsid_ctrl_button_t buttons[] = {
    { "media-skip-backward", prev_tune_callback,
        "Go to previous subtune" },
    { "media-playback-start", play_callback,
        "Play tune" },
    { "media-playback-pause", pause_callback,
        "Pause playback" },
#if 0
    { "media-playback-stop", stop_callback,
        "Stop playback (slightly fucked at the moment, so it doesn't work)"},
#endif
    { "media-seek-forward", ffwd_callback,
        "Fast forward" },
    { "media-skip-forward", next_tune_callback,
        "Go to next subtune" },   /* select next tune */
    { "media-eject", uisidattach_show_dialog,
        "Load PSID file" },   /* active file-open dialog */
    { "media-record", fake_callback,
        "Record media" },  /* start recording with current settings*/
    { NULL, NULL, NULL }
};



/** \brief  Create widget with media buttons to control VSID playback
 *
 * \return  GtkGrid
 */
GtkWidget *vsid_control_widget_create(void)
{
    GtkWidget *grid;
    int i;

    grid = vice_gtk3_grid_new_spaced(0, VICE_GTK3_DEFAULT);

    for (i = 0; buttons[i].icon_name != NULL; i++) {
        GtkWidget *button;

        button = gtk_button_new_from_icon_name(buttons[i].icon_name,
                GTK_ICON_SIZE_LARGE_TOOLBAR);
        /* always show the image, the button would useless without an image */
        gtk_button_set_always_show_image(GTK_BUTTON(button), TRUE);
        /* don't initialy focus on a button */
        gtk_widget_set_can_focus(button, FALSE);
#if 0
        /* don't grab focus when clicked */
        gtk_widget_set_focus_on_click(button, FALSE);
#endif
        gtk_grid_attach(GTK_GRID(grid), button, i, 0, 1,1);
        if (buttons[i].callback != NULL) {
            g_signal_connect(button, "clicked",
                    G_CALLBACK(buttons[i].callback),
                    (gpointer)(buttons[i].icon_name));
        }
        if (buttons[i].tooltip != NULL) {
            gtk_widget_set_tooltip_text(button, buttons[i].tooltip);
        }
    }

    /* add progress bar */
    progress = gtk_progress_bar_new();
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 1, i, 1);

    /* Add loop check button
     *
     * I'm pretty sure there's a loop icon, so perhaps add that to the control
     * buttons in stead of using this check button.
     */
    repeat = gtk_check_button_new_with_label("Loop current song");
    gtk_grid_attach(GTK_GRID(grid), repeat, 0, 2, i, 1);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat), TRUE);
    gtk_widget_set_can_focus(repeat, FALSE);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set number of tunes
 *
 * \param[in]   n   tune count
 */
void vsid_control_widget_set_tune_count(int n)
{
    tune_count = n;
}


/** \brief  Set current tune
 *
 * \param[in]   n   tune number
 */
void vsid_control_widget_set_tune_current(int n)
{
    tune_current = n;
}


/** \brief  Set default tune
 *
 * \param[in]   n   tune number
 */
void vsid_control_widget_set_tune_default(int n)
{
    tune_default = n;
}


/** \brief  Set tune progress bar value
 *
 * \param[in]   fraction    amount to fill (0.0 - 1.0)
 */
void vsid_control_widget_set_progress(gdouble fraction)
{
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), fraction);
}


/** \brief  Play next tune
 */
void vsid_control_widget_next_tune(void)
{
    next_tune_callback(NULL, NULL);
}


/** \brief  Get repeat/loop widget state
 *
 * \return  loop state
 */
gboolean vsid_control_widget_get_repeat(void)
{
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(repeat));
}
