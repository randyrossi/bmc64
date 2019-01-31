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

#include <stdbool.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "debug.h"
#include "machine.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "psid.h"
#include "uicommands.h"
#include "ui.h"
#include "uisidattach.h"

#include "vsidcontrolwidget.h"


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


/** \brief  Temporary callback for the media buttons
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void fake_callback(GtkWidget *widget, gpointer data)
{
    debug_gtk3("got callback for '%s;\n", (const char *)data);
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
    if (tune_current >= tune_count) {
        tune_current = 1;
    } else {
        tune_current++;
    }
    psid_init_driver();
    machine_play_psid(tune_current);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
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
    psid_init_driver();
    machine_play_psid(tune_current);
    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
}


/** \brief  Callback for 'fast forward'
 *
 * Fast forward using warp mode.
 *
 * \param[in]   widget  widget
 * \param[in]   data    icon name
 */
static void ffwd_callback(GtkWidget *widget, gpointer data)
{
    resources_toggle("WarpMode", NULL);
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




/** \brief  List of media control buttons
 */
static const vsid_ctrl_button_t buttons[] = {
    { "media-skip-backward", prev_tune_callback,
        "Go to previous subtune" },
    { "media-playback-start", play_callback,
        "Play tune" },
    { "media-playback-pause", pause_callback,
        "Pause playback" },
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
