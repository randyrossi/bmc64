/** \file   uimenu.c
 * \brief   Native GTK3 menu handling
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 * \author  Marcus Sutton <loggedoubt@gmail.com>
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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "debug_gtk3.h"

#include "kbd.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "uiapi.h"
#include "uiabout.h"
#include "uistatusbar.h"
#include "util.h"

#include "uimenu.h"


/** \brief  Menu accelerator object
 */
typedef struct ui_accel_data_s {
    GtkWidget *widget;      /**< widget connected to the accelerator */
    ui_menu_item_t *item;   /**< menu item connected to the accelerator */
} ui_accel_data_t;


/** \brief  Reference to the accelerator group
 */
static GtkAccelGroup *accel_group = NULL;


/** \brief  Create an empty submenu and add it to a menu bar
 *
 * \param[in]       bar     the menu bar to add the submenu to
 * \param[in]       label   label of the submenu to create
 *
 * \return  a reference to the new submenu
*/
GtkWidget *ui_menu_submenu_create(GtkWidget *bar, const char *label)
{
    GtkWidget *submenu_item;
    GtkWidget *new_submenu;

    submenu_item = gtk_menu_item_new_with_label(label);
    new_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(submenu_item), new_submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(bar), submenu_item);

    return new_submenu;
}

/** \brief  Constructor for accelerator data */
static ui_accel_data_t *ui_accel_data_new(GtkWidget *widget, ui_menu_item_t *item)
{
    ui_accel_data_t *accel_data = lib_malloc(sizeof(ui_accel_data_t));
    accel_data->widget = widget;
    accel_data->item = item;
    return accel_data;
}

/** \brief  Destructor for accelerator data
 *
 * FIXME:   this doesn't get triggered
 */
static void ui_accel_data_delete(gpointer data, GClosure *closure)
{
    debug_gtk3("Freeing accelerator data\n");
    lib_free(data);
}

/** \brief  Callback that forwards accelerator codes.
 */
static void handle_accelerator(GtkAccelGroup *accel_grp,
                               GObject *acceleratable,
                               guint keyval,
                               GdkModifierType modifier,
                               gpointer user_data)
{
    ui_accel_data_t *accel_data = (ui_accel_data_t *)user_data;
    accel_data->item->callback(accel_data->widget, accel_data->item->data);
}

/** \brief  Add menu \a items to \a menu
 *
 * \param[in,out]   menu    Gtk menu
 * \param[in]       items   menu items to add to \a menu
 *
 * \return  \a menu
 */
GtkWidget *ui_menu_add(GtkWidget *menu, ui_menu_item_t *items)
{
    size_t i = 0;
    while (items[i].label != NULL || items[i].type >= 0) {
        GtkWidget *item = NULL;
        GtkWidget *submenu;

        switch (items[i].type) {
            case UI_MENU_TYPE_ITEM_ACTION:  /* fall through */
                /* normal callback item */
                /* debug_gtk3("adding menu item '%s'\n", items[i].label); */
                item = gtk_menu_item_new_with_mnemonic(items[i].label);
                if (items[i].callback != NULL) {
                    g_signal_connect(
                            item,
                            "activate",
                            G_CALLBACK(items[i].callback),
                            (gpointer)(items[i].data));
                } else {
                    /* no callback: 'grey-out'/'ghost' the item */
                    gtk_widget_set_sensitive(item, FALSE);
                }
                break;
            case UI_MENU_TYPE_ITEM_CHECK:
                /* check mark item */
                item = gtk_check_menu_item_new_with_mnemonic(items[i].label);
                if (items[i].callback != NULL) {
                   /* use `data` as the resource to determine the state of
                     * the checkmark
                     */
                    if (items[i].data != NULL) {
                        int state;
                        resources_get_int((const char *)items[i].data, & state);
                        gtk_check_menu_item_set_active(
                                GTK_CHECK_MENU_ITEM(item), (gboolean)state);
                    }
                    /* connect signal handler AFTER setting the state, otherwise
                     * the callback gets triggered, leading to odd results */
                    g_signal_connect(
                            item,
                            "activate",
                            G_CALLBACK(items[i].callback),
                            items[i].data);
                } else {
                    /* grey out */
                    gtk_widget_set_sensitive(item, FALSE);
                }
                break;

            case UI_MENU_TYPE_SEPARATOR:
                /* add a separator */
                item = gtk_separator_menu_item_new();
                break;

            case UI_MENU_TYPE_SUBMENU:
                /* add a submenu */
                submenu = gtk_menu_new();
                item = gtk_menu_item_new_with_mnemonic(items[i].label);
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
                ui_menu_add(submenu, (ui_menu_item_t *)items[i].data);
                break;

            default:
                item = NULL;
                break;
        }
        if (item != NULL) {

            if (items[i].keysym != 0 && items[i].callback != NULL) {
                GClosure *accel_closure;
#if 0
                debug_gtk3("adding accelerator %d to item %s'\n",
                        items[i].keysym, items[i].label);
#endif
                /* Normally you would use gtk_widget_add_accelerator
                 * here, but that will disable the accelerators if the
                 * menu is hidden, which can be configured to happen
                 * while in fullscreen. We instead create the closure
                 * by hand, add it to the GtkAccelGroup, and update
                 * the accelerator information. */
                accel_closure = g_cclosure_new(G_CALLBACK(handle_accelerator),
                                               ui_accel_data_new(item, &items[i]),
                                               ui_accel_data_delete);
                gtk_accel_group_connect(accel_group, items[i].keysym, items[i].modifier, GTK_ACCEL_MASK, accel_closure);
                gtk_accel_label_set_accel(GTK_ACCEL_LABEL(gtk_bin_get_child(GTK_BIN(item))), items[i].keysym, items[i].modifier);
            }

            gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        }
        i++;
    }
    return menu;
}


/** \brief  Create accelerator group and add it to \a window
 *
 * \param[in]       window  top level window
 */
void ui_menu_init_accelerators(GtkWidget *window)
{
    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
}
