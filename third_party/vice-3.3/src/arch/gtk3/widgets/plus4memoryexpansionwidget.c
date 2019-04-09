/** \file   plus4memoryexpansionwidget.c
 * \brief   Plus4 memory expansion widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RamSize     xplus4
 * $VICERES MemoryHack  xplus4
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "plus4memhacks.h"

#include "plus4memoryexpansionwidget.h"


/** \brief  Helper constants
 */
enum {
    RAM_16KB,           /**< normal 16KB machine */
    RAM_32KB,           /**< normal 32KB machine */
    RAM_64KB,           /**< normal 64KB machine */
    RAM_256KB_CSORY,    /**< 256KB machine with memory expansion hack */
    RAM_256KB_HANNES,   /**< 256KB machine with memory expansion hack */
    RAM_1MB_HANNES,     /**< 1024KB machine with memory expansion hack */
    RAM_4MB_HANNES      /**< 4096KB machine with memory expansion hack */
};


/** \brief  List of memory sizes, including memory expansion hacks
 *
 * This list is used to build the radio button group
 */
static const vice_gtk3_radiogroup_entry_t expansions[] = {
    { "16KB",           RAM_16KB },
    { "32KB",           RAM_32KB},
    { "64KB",           RAM_64KB },
    { "256KB (CSORY)",  RAM_256KB_CSORY },
    { "256KB (HANNES)", RAM_256KB_HANNES },
    { "1MB (HANNES)",   RAM_1MB_HANNES },
    { "4MB (HANNES)",   RAM_4MB_HANNES },
    { NULL, -1 }
};


/** \brief  User-defined callback
 *
 * Takes two arguments: ram-size in KB and memory hack type
 */
static void (*extra_callback)(int, int) = NULL;


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * Sets memory size and enabled/disables memory expansion hacks, depending on
 * installed memory size
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   enum to determine what memory config to use
 */
static void on_radio_toggled(GtkWidget *widget, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        int ram = RAM_64KB;                     /* RAM size in KB */
        int hack = MEMORY_HACK_NONE;            /* RAM expansion 'hacks' */
        int index = GPOINTER_TO_INT(user_data); /* index in radio group */

        switch (index) {
            case RAM_16KB:
                ram = 16;
                break;
            case RAM_32KB:
                ram = 32;
                break;
            case RAM_64KB:
                ram = 64;
                break;
            case RAM_256KB_CSORY:
                ram = 256;
                hack = MEMORY_HACK_C256K;
                break;
            case RAM_256KB_HANNES:
                ram = 256;
                hack = MEMORY_HACK_H256K;
                break;
            case RAM_1MB_HANNES:
                ram = 1024;
                hack = MEMORY_HACK_H1024K;
                break;
            case RAM_4MB_HANNES:
                ram = 4096;
                hack = MEMORY_HACK_H4096K;
                break;
            default:
                /* should never get here */
                fprintf(stderr, "%s:%d:%s(): invalid memory size index %d\n",
                        __FILE__, __LINE__, __func__, index);
        }

        debug_gtk3("setting RamSize to %d, MemoryHack to %d.", ram, hack);

        resources_set_int("RamSize", ram);
        resources_set_int("MemoryHack", hack);

        if (extra_callback != NULL) {
            extra_callback(ram, hack);
        }
    }

}


/** \brief  Create Plus/4 memory expansion widget
 *
 * \return  GtkGrid
 */
GtkWidget *plus4_memory_expansion_widget_create(void)
{
    GtkWidget *grid;

    grid = uihelpers_radiogroup_create("RAM settings",
            expansions, on_radio_toggled, 0);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set user-defined callback
 *
 * This callback will be called with RAM-size and hack-type arguments
 *
 * \param[in]   callback    user-defined callback
 */
void plus4_memory_expansion_widget_set_callback(void (*callback)(int, int))
{
    extra_callback = callback;
}
