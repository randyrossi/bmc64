/** \file   superpetwidget.c
 * \brief   Widget to control various SuperPET related resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SuperPET        xpet
 * $VICERES Ram9            xpet
 * $VICERES RamA            xpet
 * $VICERES CPUswitch       xpet
 * $VICERES H6809RomAName   xpet
 * $VICERES H6809RomBName   xpet
 * $VICERES H6809RomCName   xpet
 * $VICERES H6809RomDName   xpet
 * $VICERES H6809RomEName   xpet
 * $VICERES H6809RomFName   xpet
 *
 * See the widgets/aciawidget.c file for additional resources.
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

#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "openfiledialog.h"
#include "aciawidget.h"

#include "superpetwidget.h"


static GtkWidget *superpet_enable_widget = NULL;
static GtkWidget *acia1_widget = NULL;
static GtkWidget *cpu_widget = NULL;
static GtkWidget *rom_widget = NULL;
static GtkWidget *ram_9xxx_widget = NULL;
static GtkWidget *ram_axxx_widget = NULL;


/** \brief  List of baud rates for the ACIA widget
 */
static int baud_rates[] = { 300, 1200, 2400, 9600, 19200, -1 };


/** \brief  List of CPU types
 */
static const vice_gtk3_radiogroup_entry_t cpu_types[] = {
    { "MOS 6502", 0 },
    { "Motorola 6809", 1 },
    { "Programmable", 2 },
    { NULL, -1 },
};


/** \brief  Handler for the "changed" event of the ROM text boxes
 *
 * \param[in]   widget      text entry
 * \param[in]   user_data   ROM index ('A'-'F') (`int`)
 */
static void on_superpet_rom_changed(GtkWidget *widget, gpointer user_data)
{
    int rom = GPOINTER_TO_INT(user_data);
    const char *path = gtk_entry_get_text(GTK_ENTRY(widget));

    debug_gtk3("setting H6809Rom%cName to '%s'.", rom, path);
    resources_set_string_sprintf("H6809Rom%cName", path, rom);
}


/** \brief  Handler for the "clicked" event of the ROM browse buttons
 *
 * \param[in]   widget      button
 * \param[in]   user_data   ROM index ('A'-'F') (`int`)
 */
static void on_superpet_rom_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    int rom = GPOINTER_TO_INT(user_data);
    gchar *filename;
    char title[256];

    g_snprintf(title, 256, "Select $%cXXX ROM", rom);

    filename = vice_gtk3_open_file_dialog(title, NULL, NULL, NULL);
    if (filename != NULL) {
        GtkWidget *grid;
        GtkWidget *entry;
        int row;

        /* determine location of related text entry */
        row = rom - 'A' + 1;
        grid = gtk_widget_get_parent(widget);
        entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, row);

        /* update text entry, forcing update of the related resource */
        gtk_entry_set_text(GTK_ENTRY(entry), filename);

        g_free(filename);
    }
}


/** \brief  Create check button for the SuperPET resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_superpet_enable_widget(void)
{
    return vice_gtk3_resource_check_button_new("SuperPET",
            "I/O Enable (disables x96)");
}


/** \brief  Create SuperPET CPU selection widget
 *
 * Select between 'MOS 6502, 'Motorola 6809' or 'programmable'
 *
 * \return  GtkGrid
 */
static GtkWidget *create_superpet_cpu_widget(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>CPU type</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    group = vice_gtk3_resource_radiogroup_new(
            "CPUswitch",
            cpu_types,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(G_OBJECT(group), "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to select SuperCPU ROMs at $A000-$FFFF
 *
 * \return  GtkGrid
 */
static GtkWidget *create_superpet_rom_widget(void)
{
    GtkWidget *grid;
    int bank;

    grid = uihelpers_create_grid_with_label("6809 ROMs", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    for (bank = 0; bank < 6; bank++) {

        GtkWidget *label;
        GtkWidget *entry;
        GtkWidget *browse;
        gchar buffer[64];
        const char *path;

        /* assumes ASCII, should be safe, except for old IBM main frames */
        g_snprintf(buffer, 64, "$%cxxx", bank + 'A');
        label = gtk_label_new(buffer);
        g_object_set(label, "margin-left", 16, NULL);

        entry = gtk_entry_new();
        gtk_widget_set_hexpand(entry, TRUE);
        resources_get_string_sprintf("H6809rom%cName", &path, bank + 'A');
        gtk_entry_set_text(GTK_ENTRY(entry), path);

        browse = gtk_button_new_with_label("Browse ...");

        gtk_grid_attach(GTK_GRID(grid), label, 0, bank + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), entry, 1, bank + 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), browse, 2, bank + 1, 1, 1);

        /* hook up event handlers */
        g_signal_connect(entry, "changed", G_CALLBACK(on_superpet_rom_changed),
                GINT_TO_POINTER(bank + 'A'));
        g_signal_connect(browse, "clicked",
                G_CALLBACK(on_superpet_rom_browse_clicked),
                GINT_TO_POINTER(bank + 'A'));

    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create check button for the Ram9 resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_superpet_9xxx_ram_widget(void)
{
    return vice_gtk3_resource_check_button_new("Ram9", "$9XXX as RAM");
}


/** \brief  Create check button for the RamA resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_superpet_axxx_ram_widget(void)
{
    return vice_gtk3_resource_check_button_new("RamA", "$AXXX as RAM");
}


/** \brief  Create a SuperPET-specific widget to be used in the PET model widget
 *
 * \return  GtkGrid
 */
GtkWidget *superpet_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>SuperPET settings</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 3, 1);

    superpet_enable_widget = create_superpet_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), superpet_enable_widget, 0, 1, 3, 1);

    acia1_widget = acia_widget_create(baud_rates);
    gtk_grid_attach(GTK_GRID(grid), acia1_widget, 0, 2, 3, 1);

    cpu_widget = create_superpet_cpu_widget();
    gtk_grid_attach(GTK_GRID(grid), cpu_widget, 0, 3, 1, 3);

    rom_widget = create_superpet_rom_widget();
    gtk_grid_attach(GTK_GRID(grid), rom_widget, 1 ,3 , 2, 3);

    ram_9xxx_widget = create_superpet_9xxx_ram_widget();
    g_object_set(ram_9xxx_widget, "margin-left", 8, NULL);
    gtk_widget_set_vexpand(ram_9xxx_widget, TRUE);
    gtk_widget_set_valign(ram_9xxx_widget, GTK_ALIGN_END);
    ram_axxx_widget = create_superpet_axxx_ram_widget();
    g_object_set(ram_axxx_widget, "margin-left", 8, NULL);
    gtk_widget_set_valign(ram_axxx_widget, GTK_ALIGN_END);

    gtk_grid_attach(GTK_GRID(grid), ram_9xxx_widget, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), ram_axxx_widget, 0, 5, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
