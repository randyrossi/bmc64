/** \file   crtpreviewwidget.c
 * \brief   Widget to show CRT cart image data before attaching
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "cartridge.h"
#include "c64/cart/crt.h"
#include "machine.h"

#include "crtpreviewwidget.h"


/** \brief  CHIP packet type descriptions
 *
 * Used for column 0
 */
static const gchar *packet_type[4] = {
    "ROM", "RAM", "Flash", "<invalid>"
};


/** \brief  Game/Exrom line state descriptions
 */
static const gchar *romstate[2] = {
    "active (lo)", "inactive (hi)"
};


static FILE *(*open_func)(const char *, crt_header_t *header) = NULL;
static int (*chip_func)(crt_chip_header_t *, FILE *) = NULL;


static GtkWidget *crtid_label = NULL;
static GtkWidget *crtname_label = NULL;
static GtkWidget *exrom_label = NULL;
static GtkWidget *game_label = NULL;
static GtkWidget *chip_tree = NULL;


/** \brief  Renderer handler to print CHIP packet load address as hex
 *
 * \param[in]       column      tree view column
 * \param[in,out]   renderer    cell renderer
 * \param[in]       model       tree model
 * \param[in]       iter        tree iterator
 * \param[in]       user_data   extra event data (unused)
 */
static void load_to_hex(GtkTreeViewColumn *column,
                        GtkCellRenderer *renderer,
                        GtkTreeModel *model,
                        GtkTreeIter *iter,
                        gpointer userdata)
{
    gchar buffer[0x10];
    guint value;

    gtk_tree_model_get(model, iter, 1, &value, -1);
    g_snprintf(buffer, sizeof(buffer), "$%04X", value);
    g_object_set(renderer, "text", buffer, NULL);
}


/** \brief  Renderer handler to print CHIP packet size as hex
 *
 * \param[in]       column      tree view column
 * \param[in,out]   renderer    cell renderer
 * \param[in]       model       tree model
 * \param[in]       iter        tree iterator
 * \param[in]       user_data   extra event data (unused)
 */
static void size_to_hex(GtkTreeViewColumn *column,
                        GtkCellRenderer *renderer,
                        GtkTreeModel *model,
                        GtkTreeIter *iter,
                        gpointer userdata)
{
    gchar buffer[0x10];
    guint value;

    gtk_tree_model_get(model, iter, 2, &value, -1);
    g_snprintf(buffer, sizeof(buffer), "$%04X", value);
    g_object_set(renderer, "text", buffer, NULL);
}


/** \brief  Create left-indented label
 *
 * \param[in]   s   text for label
 *
 * \return  GtkLabel
 */
static GtkWidget *create_label(const char *s)
{
    GtkWidget *label = gtk_label_new(s);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Create CHIP packets view and model
 *
 * \return  GtkTreeView
 */
static GtkWidget *create_tree_view(void)
{
    GtkListStore *model;
    GtkWidget *view;
    GtkTreeViewColumn *col_type;
    GtkTreeViewColumn *col_load;
    GtkTreeViewColumn *col_size;
    GtkTreeViewColumn *col_bank;

    GtkCellRenderer *renderer;

    /* set up model */
    model = gtk_list_store_new(4,
            G_TYPE_STRING,  /* packet type ('RAM', 'ROM', 'Flash') */
            G_TYPE_UINT,   /* load address */
            G_TYPE_UINT,    /* size */
            G_TYPE_UINT     /* bank */
            );


    /* set up view */
    view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), TRUE);

    /* create standard renderer (also handles int/float despite its name */
    renderer = gtk_cell_renderer_text_new();

    /* type */
    col_type = gtk_tree_view_column_new_with_attributes(
            "type", renderer, "text", 0, NULL);

    /* load */
    col_load = gtk_tree_view_column_new_with_attributes(
            "load", renderer, "text", 1, NULL);
    /* set HEX format handler */
    gtk_tree_view_column_set_cell_data_func(col_load, renderer, load_to_hex,
            NULL, NULL);


    /* size */
    col_size = gtk_tree_view_column_new_with_attributes(
            "size", renderer, "text", 2, NULL);
    /* set HEX format handler */
    gtk_tree_view_column_set_cell_data_func(col_size, renderer, size_to_hex,
            NULL, NULL);

    /* bank */
    col_bank = gtk_tree_view_column_new_with_attributes(
            "bank", renderer, "text", 3, NULL);

    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col_type);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col_load);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col_size);
    gtk_tree_view_append_column(GTK_TREE_VIEW(view), col_bank);

    gtk_widget_show(view);
    return view;
}


/** \brief  Delete all rows from the model
 */
static void chip_packets_clear(void)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(chip_tree));
    gtk_list_store_clear(GTK_LIST_STORE(model));
}


/** \brief  Add a row to the CHIP packet table
 *
 * \param[in]   type    packet type
 * \param[in]   load    load address
 * \param[in]   size    packet size
 * \param[in]   bank    packet bank index
 */
static void chip_packet_add(uint16_t type,
                            uint16_t load,
                            uint16_t size,
                            uint16_t bank)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(chip_tree));

    debug_gtk3("adding row: %u, %u, %u, %u.", type, load, size, bank);

    gtk_list_store_insert_with_values(GTK_LIST_STORE(model), NULL, -1,
        0, packet_type[type & 0x03], 1, load, 2, size, 3, bank, -1);
}


/** \brief  Create cartridge preview widget
 *
 * \return  GtkGrid
 */
GtkWidget *crt_preview_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *scroll;
    int row;

    grid = uihelpers_create_grid_with_label("CRT Header:", 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    row = 1;

    label = create_label("ID:");
    crtid_label = create_label("<unknown>");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), crtid_label, 1, row, 1, 1);
    row++;

    label = create_label("Name:");
    crtname_label = create_label("<unknown>");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), crtname_label, 1, row, 1, 1);
    row++;

    label = create_label("EXROM:");
    exrom_label = create_label("<unknown>");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), exrom_label, 1, row, 1, 1);
    row++;

    label = create_label("GAME:");
    game_label = create_label("<unknown>");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), game_label, 1, row, 1, 1);
    row++;

    label = gtk_label_new(NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_label_set_markup(GTK_LABEL(label), "<b>CHIP packets:</b>");
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 2, 1);
    row++;

    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_vexpand(scroll, TRUE);
    chip_tree = create_tree_view();
    gtk_widget_set_vexpand(chip_tree, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), chip_tree);
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, row, 2, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set function to open a CRT file and read its header
 *
 * Required to avoid linking errors with VSID
 *
 * \param[in]   func    crt_open() reference
 */
void crt_preview_widget_set_open_func(FILE *(*func)(const char *, crt_header_t *))
{
    open_func = func;
}


/** \brief  Set function to read a CRT CHIP packet
 *
 * Required to avoid linking errors with VSID
 *
 * \param[in]   func    crt_read_chip_header() reference
 */
void crt_preview_widget_set_chip_func(int (*func)(crt_chip_header_t *, FILE *))
{
    chip_func = func;
}


/** \brief  Update widget with data from CTR image \a path
 *
 * \param[in]   path    path to CRT file
 */
void crt_preview_widget_update(const gchar *path)
{
    FILE *fd;
    crt_header_t header;
    crt_chip_header_t chip;
    gchar buffer[1024];
#ifdef HAVE_DEBUG_GTK3UI
    int packets = 0;
#endif

    /*
     * Guard against non C64/C128 carts
     * Once we implement CRT headers for VIC20 and others, this needs to be
     * removed.
     */
    debug_gtk3("Got path '%s'.", path);
    if (machine_class != VICE_MACHINE_C64
            && machine_class != VICE_MACHINE_C64SC
            && machine_class != VICE_MACHINE_C128)
    {
        debug_gtk3("Machine class != c64/c128, skipping.");
        return;
    }

    fd = open_func(path, &header);
    if (fd == NULL) {
        debug_gtk3("failed to open crt image");
        gtk_label_set_text(GTK_LABEL(crtid_label), "<unknown>");
        gtk_label_set_text(GTK_LABEL(crtname_label), "<unknown>");
        gtk_label_set_text(GTK_LABEL(exrom_label), "<unknown>");
        gtk_label_set_text(GTK_LABEL(game_label), "<unknown>");
        return;
    }

    /* type */
    g_snprintf(buffer, sizeof(buffer), "%d", (int)header.type);
    gtk_label_set_text(GTK_LABEL(crtid_label), buffer);

    /* name */
    gtk_label_set_text(GTK_LABEL(crtname_label), header.name);

    /* exrom */
    gtk_label_set_text(GTK_LABEL(exrom_label),
            romstate[header.exrom ? 1 : 0]);

    /* game */
    gtk_label_set_text(GTK_LABEL(game_label),
            romstate[header.game ? 1 : 0]);

    /* clear CHIP packet table */
    chip_packets_clear();


    while (!feof(fd)) {
        long int pos;
        uint32_t skip;

        debug_gtk3("reading packet #%d.", packets++);
        if (chip_func(&chip, fd) != 0) {
            debug_gtk3("couldn't read further CHIP packets, exiting.");
            break;
        }
        skip = chip.size;

        debug_gtk3("chip packet contents:");
        debug_gtk3("    skip = %lu", (long unsigned)chip.skip);
        debug_gtk3("    load = %u", chip.start);
        debug_gtk3("    size = %u", chip.size);
        debug_gtk3("    bank = %u", chip.bank);

        chip_packet_add(chip.type, chip.start, chip.size, chip.bank);

        pos = ftell(fd) + skip;
        debug_gtk3("next chip packet offset = %lx", (unsigned long)pos);
        if (fseek(fd, pos, SEEK_SET) != 0) {
            debug_gtk3("OEPS");
            break;
        }
    }

    debug_gtk3("read %d CHIP packets.", packets);
    fclose(fd);
}
