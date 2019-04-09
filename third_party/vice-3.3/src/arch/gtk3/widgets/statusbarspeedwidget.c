/** \file   statusbarspeedwidget.c
 * \brief   CPU speed, FPS display widget for the statusbar
 *
 * Widget for the status bar that displays CPU speed, FPS and warp/pause state.
 * When left-clicking on the widget a menu will pop up allowing the user to
 * control refresh rate, emulation speed, warp and pause.
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
 */


#include "vice.h"
#include <gtk/gtk.h>
#include "vice_gtk3.h"
#include "basedialogs.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"

#include "statusbarspeedwidget.h"


/** \brief  Predefined emulation speeds (taken from vice.texi)
 */
static int emu_speeds[] = { 200, 100, 50, 20, 10, -1 };


/** \brief  Handler for the "activate" event of the "Advance frame" menu item
 *
 * \param[in]   widget  menu item (unused)
 * \param[in]   data    extra event data (unused
 */
static void on_advance_frame_activate(GtkWidget *widget, gpointer data)
{
    ui_advance_frame();
}


/** \brief  Add separator to \a menu
 *
 * Little helper function to add a separator item to a menu.
 *
 * \param[in,out]   menu    GtkMenu instance
 */
static void add_separator(GtkWidget *menu)
{
    GtkWidget *item = gtk_separator_menu_item_new();
    gtk_container_add(GTK_CONTAINER(menu), item);
}


/** \brief  Handler for the "toggled" event of the Enable warp menu item
 *
 * \param[in]   widget  menu item (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_warp_toggled(GtkWidget *widget, gpointer data)
{
    int warp;

    if (resources_get_int("WarpMode", &warp) == 0) {
        if (resources_set_int("WarpMode", !warp) < 0) {
            debug_gtk3("failed to toggle warp mode.");
        }
    }
}


/** \brief  Handler for the "toggled" event of the Pause menu item
 *
 * \param[in]   widget  menu item (unused)
 * \param[in]   data    extra even data (unused)
 */
static void on_pause_toggled(GtkWidget *widget, gpointer data)
{
    if (ui_emulation_is_paused()) {
        ui_pause_emulation(0);
    } else {
        ui_pause_emulation(1);
    }
}


/** \brief  Handler for the toggled event of a refresh rate submenu item
 *
 * \param[in]   widget  refresh rate submenu item
 * \param[in]   data    new fresh rate
 */
static void on_refreshrate_toggled(GtkWidget *widget, gpointer data)
{
    int refresh = GPOINTER_TO_INT(data);

    resources_set_int("RefreshRate", refresh);
}


/** \brief  Handler for the toggled event of a emulation speed submenu item
 *
 * \param[in]   widget  emulation speed submenu item
 * \param[in]   data    new emulation speed
 */
static void on_emulation_speed_toggled(GtkWidget *widget, gpointer data)
{
    int speed = GPOINTER_TO_INT(data);

    resources_set_int("Speed", speed);
}


/** \brief  Handler for the "toggled" event of the "custom refresh" menu item
 *
 * Pops up a dialog to set a custom refresh rate.
 *
 * \param[in]   widget  menu item
 * \param[in]   data    extra event data (unused)
 */
static void on_refresh_custom_toggled(GtkWidget *widget, gpointer data)
{
    int old_val;
    int new_val;

    resources_get_int("RefreshRate", &old_val);

    if (vice_gtk3_integer_input_box(
                "Set refresh rate",
                "Enter a new custom refresh rate",
                old_val, &new_val,
                1, 100)) {
        /* OK: */
        resources_set_int("RefreshRate", new_val);
    }
}


/** \brief  Handler for the "toggled" event of the "custom speed" menu item
 *
 * Pops up a dialog to set a custom emulation speed.
 *
 * \param[in]   widget  menu item
 * \param[in]   data    extra event data (unused)
 */
static void on_speed_custom_toggled(GtkWidget *widget, gpointer data)
{
    int old_val;
    int new_val;

    resources_get_int("Speed", &old_val);

    if (vice_gtk3_integer_input_box(
                "Set new emulation speed",
                "Enter a new custom emulation speed",
                old_val, &new_val,
                1, 1000)) {
        /* OK: */
        resources_set_int("Speed", new_val);
    }
}


/** \brief  Create emulation speed submenu
 *
 * \return  GtkMenu
 */
static GtkWidget *emulation_speed_submenu_create(void)
{
    GtkWidget *menu;
    GtkWidget *item;
    char buffer[256];
    int curr_speed;
    int i;
    gboolean found = FALSE;

    if (resources_get_int("Speed", &curr_speed) < 0) {
        curr_speed = 100;
    }

    menu = gtk_menu_new();

    /* fixed values */
    for (i = 0; emu_speeds[i] >= 0; i++) {
        g_snprintf(buffer, 256, "%d%%", emu_speeds[i]);
        item = gtk_check_menu_item_new_with_label(buffer);
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
        if (curr_speed == emu_speeds[i]) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
            found = TRUE;
        }
        gtk_container_add(GTK_CONTAINER(menu), item);

        g_signal_connect(item, "toggled",
                G_CALLBACK(on_emulation_speed_toggled),
                GINT_TO_POINTER(emu_speeds[i]));
    }

    /* no limit */
    item = gtk_check_menu_item_new_with_label("Unlimited");
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
    if (curr_speed == 0) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
        found = TRUE;
    }
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled",
            G_CALLBACK(on_emulation_speed_toggled), GINT_TO_POINTER(0));

    add_separator(menu);

    /* custom speed */
    if (!found) {
        g_snprintf(buffer, 256, "Custom (%d%%) ...", curr_speed);
        item = gtk_check_menu_item_new_with_label(buffer);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    } else {
        item = gtk_check_menu_item_new_with_label("Custom ...");
    }
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled",
            G_CALLBACK(on_speed_custom_toggled), GINT_TO_POINTER(curr_speed));


    gtk_widget_show_all(menu);
    return menu;
}


/** \brief  Create refresh rate submenu
 *
 * \return  GtkMenu
 */
static GtkWidget *refreshrate_submenu_create(void)
{
    GtkWidget *menu;
    GtkWidget *item;
    int i;
    char buffer[256];
    int refresh;
    gboolean found = FALSE;

    if (resources_get_int("RefreshRate", &refresh) < 0) {
        refresh = 0;
    }

    menu = gtk_menu_new();

    /* Auto */
    item = gtk_check_menu_item_new_with_label("Auto");
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
    if (refresh == 0) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
        found = TRUE;
    }
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled", G_CALLBACK(on_refreshrate_toggled),
            GINT_TO_POINTER(0));

    add_separator(menu);

    /* 1/1 through 1/10 */
    for (i = 1; i <= 10; i++) {
        g_snprintf(buffer, 256, "1/%d", i);
        item = gtk_check_menu_item_new_with_label(buffer);
        gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
        if (refresh == i) {
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
            found = TRUE;
        }
        gtk_container_add(GTK_CONTAINER(menu), item);
        g_signal_connect(item, "toggled", G_CALLBACK(on_refreshrate_toggled),
                GINT_TO_POINTER(i));
    }

    add_separator(menu);

    /* custom refresh rate */
    if (!found) {
        /* refresh rate not found yet, so it has to be a custom value */
        g_snprintf(buffer, 256, "Custom (1/%d) ...", refresh);
        item = gtk_check_menu_item_new_with_label(buffer);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    } else {
        /* standard refresh rate */
        item = gtk_check_menu_item_new_with_label("Custom ...");
    }
    gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM(item), TRUE);
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled", G_CALLBACK(on_refresh_custom_toggled),
            NULL);
    gtk_widget_show_all(menu);
    return menu;
}


/** \brief  Create popup menu for the statusbar speed widget
 *
 * \return  GtkMenu
 */
GtkWidget *speed_menu_popup_create(void)
{
    GtkWidget *menu;
    GtkWidget *submenu;
    GtkWidget *item;
    GtkWidget *child;
    int warp;

    menu = gtk_menu_new();

    /* Refresh rate submenu */
    item = gtk_menu_item_new_with_label("Refresh rate");
    gtk_container_add(GTK_CONTAINER(menu), item);
    submenu = refreshrate_submenu_create();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

    /* Emulation speed submenu */
    item = gtk_menu_item_new_with_label("Maximum speed");
    gtk_container_add(GTK_CONTAINER(menu), item);
    submenu = emulation_speed_submenu_create();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);

    add_separator(menu);

    /* pause */
    item = gtk_check_menu_item_new_with_label("Pause emulation (Alt+P)");
    child = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_markup(GTK_LABEL(child),
            "Pause emulation (" VICE_MOD_MASK_HTML "+P)");

    if (ui_emulation_is_paused()) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), TRUE);
    }
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled", G_CALLBACK(on_pause_toggled), NULL);

    /* advance frame */
    item = gtk_menu_item_new_with_label("Advance frame (Alt+Shift+P)");
    child = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_markup(GTK_LABEL(child),
            "Advance frame (" VICE_MOD_MASK_HTML "+Shift+P)");
    if (!ui_emulation_is_paused()) {
        gtk_widget_set_sensitive(item, FALSE);
    }
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(on_advance_frame_activate),
            NULL);

    /* enable warp mode */
    item = gtk_check_menu_item_new_with_label("Enable warp mode (Alt+W)");
    child = gtk_bin_get_child(GTK_BIN(item));
    gtk_label_set_markup(GTK_LABEL(child),
            "Enable warp mode (" VICE_MOD_MASK_HTML "+W)");
    if (resources_get_int("WarpMode", &warp) < 0) {
        warp = 0;
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), (gboolean)warp);
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "toggled", G_CALLBACK(on_warp_toggled), NULL);

    gtk_widget_show_all(menu);
    return menu;
}


/** \brief  Event handler for the mouse clicks on the speed widget
 *
 * \param[in]   widget  event box around the speed label
 * \param[in]   event   event
 * \param[in]   data    extra event data (unused)
 *
 * \return  TRUE when event was handled, FALSE otherwise
 */
static gboolean on_widget_clicked(GtkWidget *widget,
                                  GdkEvent *event,
                                  gpointer data)
{
    int mouse;

    if (resources_get_int("Mouse", &mouse) < 0) {
        mouse = 0;
    }

    if (((GdkEventButton *)event)->button == GDK_BUTTON_SECONDARY) {
        GtkWidget *menu = speed_menu_popup_create();
        gtk_menu_popup_at_widget(GTK_MENU(menu), widget,
                GDK_GRAVITY_NORTH_WEST, GDK_GRAVITY_SOUTH_WEST,
                event);
        return TRUE;
    }
    return FALSE;
}


/** \brief  Reference to the alternate "hand" mouse pointer
 *
 * FIXME:   Do I need to clean this up somehow? Gdk docs aren't clear at all
 */
static GdkCursor *mouse_ptr;


/** \brief  Handler for the "enter/leave" events of the event box
 *
 * This changes the mouse cursor into a little "hand" when hovering over the
 * widget, to indicate to the user they can click on the widget.
 *
 * \param[in]   widget  widget triggering the event
 * \param[in]   event   event triggered
 * \param[in]   data    extra event data (unused)
 *
 * \TODO:   refactor, code can be simplified
 */
static gboolean on_widget_hover(GtkWidget *widget,
                                GdkEvent *event,
                                gpointer data)
{
    if (event != NULL) {

        GdkDisplay *display;
        int mouse;

        if (resources_get_int("Mouse", &mouse) < 0) {
            mouse = 0;
        }

        if (event->type == GDK_ENTER_NOTIFY) {
            display = gtk_widget_get_display(widget);
            if (display != NULL && mouse_ptr == NULL) {
                mouse_ptr = gdk_cursor_new_from_name(display, "pointer");
            }
            if (mouse_ptr != NULL) {
                GdkWindow *window = gtk_widget_get_window(widget);
                if (window != NULL) {
                    gdk_window_set_cursor(window, mouse_ptr);
                }
            }
        } else {
            GdkWindow *window = gtk_widget_get_window(widget);
            if (window != NULL) {
                gdk_window_set_cursor(window, NULL);
            }
        }
        return TRUE;
    }
    return FALSE;
}


/** \brief  Create widget to display CPU/FPS/pause
 *
 * \return  GtkLabel
 */
GtkWidget *statusbar_speed_widget_create(void)
{
    GtkWidget *label;
    GtkWidget *event_box;

    label = gtk_label_new("CPU: 100%, FPS: 50.125");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(label, TRUE);
    /* gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END); */

    event_box = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);
    gtk_container_add(GTK_CONTAINER(event_box), label);
    gtk_widget_show(label);

    if (machine_class != VICE_MACHINE_VSID) {
        g_signal_connect(event_box, "button-press-event",
                G_CALLBACK(on_widget_clicked), NULL);
        g_signal_connect(event_box, "enter-notify-event",
                G_CALLBACK(on_widget_hover), NULL);
        g_signal_connect(event_box, "leave-notify-event",
                G_CALLBACK(on_widget_hover), NULL);
    }
    return event_box;
}


/** \brief  Update the speed widget's display state
 *
 * \param[in]   percent     CPU speed in percentage
 * \param[in]   framerate   number of frames per second
 * \param[in]   warp_flag   warp is enabled flag
 *
 * \todo    Also handle/display pause state
 */
void statusbar_speed_widget_update(
        GtkWidget *widget,
        float percent,
        float framerate,
        int warp_flag)
{
    GtkWidget *label;
    char buffer[1024];
    int cpu = (int)(percent + 0.5);
    int fps = (int)(framerate + 0.5);

    g_snprintf(buffer, 1024, "%d%% cpu, %d fps %s%s",
            cpu,
            fps,
            warp_flag ? " (warp)" : "",
            ui_emulation_is_paused() ? " (paused)" : "");

    label = gtk_bin_get_child(GTK_BIN(widget));

    gtk_label_set_text(GTK_LABEL(label), buffer);
}
