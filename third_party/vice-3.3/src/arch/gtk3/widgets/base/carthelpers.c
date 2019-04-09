/** \file   carthelpers.c
 * \brief   Cartridge helper functions
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
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
#include <gtk/gtk.h>

#include "machine.h"
#include "resources.h"
#include "lib.h"
#include "debug_gtk3.h"
#include "cartridge.h"

#include "carthelpers.h"


int (*carthelpers_save_func)(int type, const char *filename);
int (*carthelpers_flush_func)(int type);
int (*carthelpers_is_enabled_func)(int type);
int (*carthelpers_enable_func)(int type);
int (*carthelpers_disable_func)(int type);


/** \brief  Placeholder function for functions accepting (int)
 *
 * Makes sure calling for example, carthelpers_flush_func() doesn't dereference
 * a NULL pointer when that was passed into carthelpers_set_functions()
 *
 * \return  -1
 */
static int null_handler(int type)
{
    debug_gtk3("warning: not implemented (NULL).");
    return -1;
}


/** \brief  Placeholder function for functions accepting (int, const char *)
 *
 * Makes sure calling for example, carthelpers_save_func() doesn't dereference
 * a NULL pointer when that was passed into carthelpers_set_functions()
 *
 * \return  -1
 */
static int null_handler_save(int type, const char *filename)
{
    debug_gtk3("warning: not implemented (NULL).");
    return -1;
}


/** \brief  Set cartridge helper functions
 *
 * This function helps to avoid the problems with VSID wrt cartridge code:
 * VSID doesn't link against any cartridge code and since the various widgets
 * in src/arch/gtk3 are linked into a single .a object which VSID also links to
 * we need a way to use cartridge functions without VSID borking during linking.
 * Passing in pointers to the cart functions in ${emu}ui.c (except vsidui.c)
 * 'solves' this problem.
 *
 * Normally \a save_func should be cartridge_save_image(), \a fush_func should
 * be cartridge_flush_image() and \a enabled_func should be
 * \a cartridge_type_enabled.
 * These are the functions used by many/all(?) cartridge widgets
 *
 * \param[in]   save_func       cartridge image save-as function
 * \param[in]   flush_func      cartridge image flush/save function
 * \param[in]   is_enabled_func cartridge enabled state function
 * \param[in]   enable_func     cartridge enable function
 * \param[in]   disable_func    cartridge disable function
 */
void carthelpers_set_functions(
        int (*save_func)(int, const char *),
        int (*flush_func)(int),
        int (*is_enabled_func)(int),
        int (*enable_func)(int),
        int (*disable_func)(int))
{
    carthelpers_save_func = save_func ? save_func : null_handler_save;
    carthelpers_flush_func = flush_func ? flush_func : null_handler;
    carthelpers_is_enabled_func = is_enabled_func ? is_enabled_func : null_handler;
    carthelpers_enable_func = enable_func ? enable_func : null_handler;
    carthelpers_disable_func = disable_func ? disable_func : null_handler;
}


/** \brief  Handler for the "destroy" event of a cart enable check button
 *
 * Frees the cartridge name stored as a property in the check button.
 *
 * \param[in]   check   check button
 * \param[in]   data    unused
 */
static void on_cart_enable_check_button_destroy(GtkCheckButton *check,
                                                gpointer data)
{
    char *name = g_object_get_data(G_OBJECT(check), "CartridgeName");
    if (name != NULL) {
        lib_free(name);
    }
}


/** \brief  Handler for the "toggled" event of the cart enable check button
 *
 * When this function fails to set a resource, it'll revert to the old state,
 * unfortunately this also triggers a new event (calling this very function).
 * I still have to figure out how to temporarily block signals (it's not like
 * Qt)
 *
 * \param[in,out]   check   check button
 * \param[in]       data    unused
 */
static void on_cart_enable_check_button_toggled(GtkCheckButton *check,
                                                gpointer data)
{
    int id;
    int state;
#ifdef HAVE_DEBUG_GTK3UI
    const char *name;
    name = (const char *)g_object_get_data(G_OBJECT(check), "CartridgeName");
#endif

    id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(check), "CartridgeId"));
    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check));

    debug_gtk3("setting to %s '%s' (%d).", state ? "enable" : "disable",
            name, id);

    if (state) {
        if (carthelpers_enable_func(id) < 0) {
            debug_gtk3("failed to enable %s cartridge.", name);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), FALSE);
        }
    } else {
        if (carthelpers_disable_func(id) < 0) {
            debug_gtk3("failed to disable %s cartridge.", name);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), TRUE);
        }
    }
}

/** \brief  Create a check button to enable/disable a cartridge
 *
 * Creates a check button that enables/disables a cartridge. The \a cart_name
 * argument is copied to allow for debug/error messages to mention the cart
 * by name, rather than by ID. The name is freed when the check button is
 * destroyed.
 *
 * What the widget basically does is call cartridge_enable(\a cart_id) or
 * cartridge_disable(\a cart_id), using cartridge_type_enabled(\a cart_id) to
 * set the initial state of the widget. But since all Gtk3 widgets are
 * currently linked into a big lib and vsid doesn't like that, we use some
 * function pointer magic is used.
 *
 * \param[in]   cart_name   cartridge name (see cartridge.h)
 * \param[in]   cart_id     cartridge ID (see cartridge.h)
 *
 * \return  GtkCheckButton
 */
GtkWidget *carthelpers_create_enable_check_button(const char *cart_name,
                                                  int cart_id)
{
    GtkWidget *check;
    char *title;
    gchar *name;

    title = lib_msprintf("Enable %s cartridge", cart_name);
    check = gtk_check_button_new_with_label(title);
    lib_free(title);    /* Gtk3 makes a copy of the title */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
            carthelpers_is_enabled_func(cart_id));

    name = lib_stralloc(cart_name);
    g_object_set_data(G_OBJECT(check), "CartridgeName", (gpointer)name);
    g_object_set_data(G_OBJECT(check), "CartridgeId", GINT_TO_POINTER(cart_id));

    g_signal_connect(check, "destroy",
            G_CALLBACK(on_cart_enable_check_button_destroy), NULL);
    g_signal_connect(check, "toggled",
            G_CALLBACK(on_cart_enable_check_button_toggled), NULL);

    return check;
}
