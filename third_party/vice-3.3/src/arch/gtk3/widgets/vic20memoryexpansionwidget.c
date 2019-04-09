/** \file   vic20memoryexpansionwidget.c
 * \brief   VIC20 memory expansion widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Allow enabling/disabling VIC-20 RAM expansions, and also provides a list of
 * common memory expansion configurations.
 */

/*
 * $VICERES RamBlock0   xvic
 * $VICERES RamBlock1   xvic
 * $VICERES RamBlock2   xvic
 * $VICERES RamBlock3   xvic
 * $VICERES RamBlock5   xvic
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

#include "vic20memoryexpansionwidget.h"


/** \brief  Struct containing information on the "common configurations"
 */
typedef struct common_config_s {
    const char *text;   /**< description */
    int blocks[5];      /**< enabled/disables states for blocks 0/1/2/3/5 */
} common_config_t;


/** \brief  List of available RAM expansions
 */
static const vice_gtk3_radiogroup_entry_t ram_blocks[] = {
    { "Block 0 (3KB at $0400-$0FFF)", 0 },
    { "Block 1 (8KB at $2000-$3FFF)", 1 },
    { "Block 2 (8KB at $4000-$5FFF)", 2 },
    { "Block 3 (8KB at $6000-$7FFF)", 3 },
    { "Block 5 (8KB at $A000-$BFFF)", 5 },
    { NULL, -1 }
};


/** \brief  List of common memory expansion configurations
 *
 * Taken from the Gtk2 UI
 */
static const common_config_t common_configs[] = {
    { "No expansion memory",    { 0, 0, 0, 0, 0 } },
    { "3KB (block 0)",          { 1, 0, 0, 0, 0 } },
    { "8KB (block 1)",          { 0, 1, 0, 0, 0 } },
    { "12KB (block 1/2)",       { 0, 1, 1, 0, 0 } },
    { "16KB (block 1/2/3)",     { 0, 1, 1, 1, 0 } },
    { "All (block 0/1/2/3/5)",  { 1 ,1 ,1 ,1 ,1 } },
    { NULL,                     { 0, 0, 0, 0, 0 } }
};


/** \brief  Reference to the RAM blocks widget
 *
 * Used by the combo box to set the proper check buttons
 */
static GtkWidget *blocks_widget = NULL;


/* currently unused */
#if 0
/** \brief  Get index of RAM \a block in array
 *
 * \param[in]   block   VIC20 RAM block
 *
 * \return  index in array or -1 when not found
 */
static int get_ram_block_index(int block)
{
    int i;

    for (i = 0; ram_blocks[i].text != NULL; i++) {
        if (ram_blocks[i].value == block) {
            return i;
        }
    }
    return -1;
}
#endif


/** \brief  Handler for the "toggled" event of the check buttons
 *
 * \param[in]   widget      check button triggering the event
 * \param[in]   user_data   RAM block ID (`int`)
 *
 * XXX: perhaps update the "common configurations" combo box depending on the
 *      currently active RAM expansions? (Gtk2 doesn't appear to do it, but it
 *      would be nice addition
 */
static void on_ram_block_toggled(GtkWidget *widget, gpointer user_data)
{
    int old_state;
    int new_state;
    int block = GPOINTER_TO_INT(user_data);

    resources_get_int_sprintf("RamBlock%d", &old_state, block);
    new_state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (new_state != old_state) {
        debug_gtk3("setting RamBlock%d to %s.",
                block, new_state ? "ON" : "OFF");
        resources_set_int_sprintf("RamBlock%d", new_state, block);
    }
}


/** \brief  Handler for the "changed" event of the "commong config" combobox
 *
 * If a valid item is selected, this sets the check buttons determining which
 * RAM expansions are enabled. The event handlers of the check buttons will set
 * the proper resources.
 *
 * \param[in]   widget      "common config" combo box
 * \param[in]   user_data   data for the event (unused)
 */
static void on_common_config_changed(GtkWidget *widget, gpointer user_data)
{
    int index = gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    int count = (int)(sizeof common_configs / sizeof common_configs[0]) - 1;
    const int *config;
    int i;

    if (index < 0 || index >= count) {
        return;
    }

    config = common_configs[index].blocks;
    for (i = 0; i < 5; i++) {
        GtkWidget *check = gtk_grid_get_child_at(GTK_GRID(blocks_widget), 0, i +1);
        debug_gtk3("setting RAM block %d to %d.", i, config[i]);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), config[i]);
    }

}


/** \brief  Create VIC20 common RAM configurations widget
 *
 * \return  GtkGrid
 */
static GtkWidget * vic20_common_config_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *combo;
    int i;

    grid = uihelpers_create_grid_with_label("Common configurations", 1);

    combo = gtk_combo_box_text_new();
    g_object_set(combo, "margin-left", 16, NULL);
    for (i = 0; common_configs[i].text != NULL; i++) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
               NULL, common_configs[i].text);
    }
    g_signal_connect(combo, "changed", G_CALLBACK(on_common_config_changed),
            NULL);

    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create VIC20 RAM blocks widget
 *
 * \return  GtkGrid
 */
static GtkWidget * vic20_ram_blocks_widget_create(void)
{
    GtkWidget *grid;
    int i;

    grid = uihelpers_create_grid_with_label("RAM blocks", 1);
    for (i = 0; ram_blocks[i].name != NULL; i++) {

        GtkWidget *check = gtk_check_button_new_with_label(ram_blocks[i].name);
        int active;

        resources_get_int_sprintf("RamBlock%d", &active, ram_blocks[i].id);
        g_object_set(check, "margin-left", 16, NULL);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check), active);
        g_signal_connect(check, "toggled", G_CALLBACK(on_ram_block_toggled),
                GINT_TO_POINTER(ram_blocks[i].id));
        gtk_grid_attach(GTK_GRID(grid), check, 0, i + 1, 1, 1);
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create VIC20 memory expansion widget
 *
 * Contains two sub-widgets, one "common configurations" combo box, and one
 * group of check buttons to enable/disable each RAM expansion
 *
 * \return  GtkGrid
 */
GtkWidget *vic20_memory_expansion_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *common;

    grid = uihelpers_create_grid_with_label("Memory expansions", 1);

    common = vic20_common_config_widget_create();
    g_object_set(common, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), common, 0, 1, 1, 1);

    blocks_widget = vic20_ram_blocks_widget_create();
    g_object_set(blocks_widget, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), blocks_widget, 0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



