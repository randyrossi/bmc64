/** \file   vsidmainwidget.c
 * \brief   GTK3 tune info widget for VSID
 *
 * Displays (sub)tune information of a PSID file
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
#include "machine.h"
#include "lib.h"
#include "util.h"

#include "vsidtuneinfowidget.h"


/** \brief  Rows in the driver info grid
 */
enum {
    DRV_INFO_SID_IMAGE = 0,
    DRV_INFO_DRIVER_ADDR,
    DRV_INFO_LOAD_ADDR,
    DRV_INFO_INIT_ADDR,
    DRV_INFO_PLAY_ADDR
};


/** \brief  Labels for the driver info grid
 */
static const char *driver_info_labels[] = {
    "SID image:",
    "Driver address:",
    "Load address:",
    "Init address:",
    "Play address:"
};


/*
 * SID address/size variables
 *
 * These need to be kept track of, so the SID image calculation works
 */
static uint16_t load_addr;
static uint16_t data_size;

/*
 * sub)tune variables
 *
 * These need to be kept track of, so the "X of Y (default: Z)" tune info
 * widget gets rendered properly
 */
static int tune_count;
static int tune_current;
static int tune_default;

/* widget references */
static GtkWidget *tune_info_grid;
static GtkWidget *name_widget;
static GtkWidget *author_widget;
static GtkWidget *copyright_widget;
static GtkWidget *tune_num_widget;
static GtkWidget *model_widget;
static GtkWidget *irq_widget;
static GtkWidget *sync_widget;
static GtkWidget *runtime_widget;
static GtkWidget *driver_info_widget;


/** \brief  Create left aligned label, \a text can use HTML markup
 *
 * \param[in]   text    label text
 *
 * \return  GtkLabel
 */
static GtkWidget *create_left_aligned_label(const char *text)
{
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Convert a string from a SID header field to UTF-8
 *
 * \param[in]   s   string from SID header, nul-terminated
 *
 * \return  UTF-8 encoded string, or the original when conversion failed
 *
 * \note    the string returned needs to be freed with g_free()
 */
static gchar *convert_to_utf8(const char *s)
{
    GError *err = NULL;
    gchar *utf8;

    utf8 = g_convert(s, -1, "UTF-8", "ISO-8859-1", NULL, NULL, &err);
    if (err != NULL) {
        debug_gtk3("GError: %d: %s\n", err->code, err->message);
        g_free(utf8);
        utf8 = g_strdup(s);
    }
    return utf8;
}


/** \brief  Create a label to display text and allow users to copy that text
 *
 * \return  GtkLabel
 */
static GtkWidget *create_readonly_entry(void)
{
    GtkWidget *label = gtk_label_new(NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    /* allow users to copy the label text */
    gtk_label_set_selectable(GTK_LABEL(label), TRUE);
    /* avoid the label getting focus */
    gtk_widget_set_can_focus(label, FALSE);
    return label;
}


/** \brief  Create widget to display tune number information
 *
 * Creates label with text 'tune X of Y (Default: Z)'
 *
 * \return  GtkLabel
 */
static GtkWidget *create_tune_num_widget(void)
{
    GtkWidget *label;
    char *text;

    text = lib_msprintf("%d of %d (Default: %d)",
            tune_current, tune_count, tune_default);
    label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    lib_free(text);
    return label;
}


/** \brief  Update tune number widget
 *
 * Updates the 'X of Y (default: Z)' widget
 */
static void update_tune_num_widget(void)
{
    char *text;

    text = lib_msprintf("%d of %d (Default: %d)",
            tune_current, tune_count, tune_default);
    gtk_label_set_text(GTK_LABEL(tune_num_widget), text);
    lib_free(text);
}


/** \brief  Create IRQ widget
 */
static GtkWidget *create_irq_widget(void)
{
    GtkWidget *label;

    label = gtk_label_new("-");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Update IRQ widget
 *
 * \param[in]   irq irq source
 */
static void update_irq_widget(const char *irq)
{
    gtk_label_set_text(GTK_LABEL(irq_widget), irq);
}


/** \brief  Create SID model widget
 */
static GtkWidget *create_model_widget(void)
{
    GtkWidget *label;

    label = gtk_label_new("-");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Update SID model widget
 *
 * \param[in]   model   SID chip model
 */
static void update_model_widget(int model)
{
    if (model == 0) {
        gtk_label_set_text(GTK_LABEL(model_widget), "MOS 6581");
    } else {
        gtk_label_set_text(GTK_LABEL(model_widget), "MOS 8580");
    }
}


/** \brief  Create run time widget
 *
 * Creates a widget which displays hours, minutes and seconds
 */
static GtkWidget *create_runtime_widget(void)
{
    GtkWidget *label = gtk_label_new("0:00:00");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Update the run time widget
 *
 * Displays the run time of the current (sub)tune in hours, minutes and seconds
 *
 * \param[in]   sec current run time in seconds
 */
static void update_runtime_widget(unsigned int sec)
{
    char buffer[256];
    unsigned int s;
    unsigned int m;
    unsigned int h;

    s = sec % 60;
    m = sec / 60;
    h = sec / 60 / 60;

    /* don't use lib_msprintf() here, this function gets called a lot and
     * malloc() isn't fast */
    g_snprintf(buffer, 256, "%u:%02u:%02u", h, m, s);
    gtk_label_set_text(GTK_LABEL(runtime_widget), buffer);
}


/** \brief  Create sync widget
 */
static GtkWidget *create_sync_widget(void)
{
    GtkWidget *label;

    label = gtk_label_new("-");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    return label;
}


/** \brief  Update sync widget
 *
 * \param[in]   sync    sync factor ID
 */
static void update_sync_widget(int sync)
{
    if (sync == 1) {
        gtk_label_set_text(GTK_LABEL(sync_widget), "PAL (50Hz)");
    } else {
        gtk_label_set_text(GTK_LABEL(sync_widget), "NTSC (60Hz)");
    }
}


/** \brief  Create driver information widget
 */
static GtkWidget *create_driver_info_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    size_t i = 0;

    grid = vice_gtk3_grid_new_spaced(16, 0);

    while (i < sizeof driver_info_labels / sizeof driver_info_labels[1]) {
        /* parameter label */
        label = gtk_label_new(driver_info_labels[i]);
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), label, 0, (int)i, 1, 1);
        /* parameter value */
        label = gtk_label_new("-");
        gtk_widget_set_halign(label, GTK_ALIGN_START);
        gtk_grid_attach(GTK_GRID(grid), label, 1, (int)i, 1, 1);
        i++;
    }
    return grid;
}


/** \brief  Set a label in the driver info grid at \a row to \addr
 *
 * \param[in]   row     row in the grid
 * \param[in]   addr    16-bit address
 */
static void driver_info_set_addr(int row, uint16_t addr)
{
    GtkWidget *label;
    char text[6];   /* "$1234" + '\0' */

    label = gtk_grid_get_child_at(GTK_GRID(driver_info_widget), 1, row);
    if (label != NULL && GTK_IS_LABEL(label)) {
        g_snprintf(text, 6, "$%04X", addr);
        gtk_label_set_text(GTK_LABEL(label), text);
    }
}


/** \brief  Set memory range of the SID image
 */
static void driver_info_set_image(void)
{
    GtkWidget *label;
    char text[12];  /* "$1234-$5678" + '\0' */

    label = gtk_grid_get_child_at(GTK_GRID(driver_info_widget), 1,
            DRV_INFO_SID_IMAGE);
    if (label != NULL && GTK_IS_LABEL(label)) {
        g_snprintf(text, 12, "$%04X-$%04X",
                load_addr, load_addr + data_size - 1U);
        gtk_label_set_text(GTK_LABEL(label), text);
    }
}


/** \brief  Create widget to show tune information
 *
 * \return  GtkGrid
 */
GtkWidget *vsid_tune_info_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* title */
    label = create_left_aligned_label("Name:");
    name_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_widget, 1, 0, 1, 1);

    /* author */
    label = create_left_aligned_label("Author:");
    author_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), author_widget, 1, 1, 1, 1);

    /* copyright */
    label = create_left_aligned_label("Copyright:");
    copyright_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), copyright_widget, 1, 2, 1, 1);

    /* tune number (x of x) */
    label = create_left_aligned_label("Tune:");
    tune_num_widget = create_tune_num_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tune_num_widget, 1, 3, 1, 1);

    /* model widget */
    label = create_left_aligned_label("Model:");
    model_widget = create_model_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), model_widget, 1, 4, 1, 1);

    /* IRQ widget */
    label = create_left_aligned_label("IRQ:");
    irq_widget = create_irq_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), irq_widget, 1, 5, 1, 1);

    /* sync widget */
    label = create_left_aligned_label("Synchronization:");
    sync_widget = create_sync_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sync_widget, 1, 6, 1, 1);

    /* runtime widget */
    label = create_left_aligned_label("Run time:");
    runtime_widget = create_runtime_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), runtime_widget, 1, 7, 1, 1);

    /* driver info */
    label = create_left_aligned_label("Driver info:");
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    driver_info_widget = create_driver_info_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), driver_info_widget, 1,8, 1, 1);

    gtk_widget_show_all(grid);
    tune_info_grid = grid;
    return grid;
}


/** \brief  Set tune \a name
 *
 * \param[in]   name    tune name
 */
void vsid_tune_info_widget_set_name(const char *name)
{
    char *utf8;

    utf8 = convert_to_utf8(name);
    gtk_label_set_text(GTK_LABEL(name_widget), utf8);
    g_free(utf8);
}


/** \brief  Set author
 *
 * \param[in]   name    author name
 */
void vsid_tune_info_widget_set_author(const char *name)
{
    char *utf8;

    utf8 = convert_to_utf8(name);
    gtk_label_set_text(GTK_LABEL(author_widget), utf8);
    g_free(utf8);
}


/** \brief  Set copyright info string
 *
 * \param[in]   name    copyright string
 */
void vsid_tune_info_widget_set_copyright(const char *name)
{
    char *utf8;

    utf8 = convert_to_utf8(name);
    gtk_label_set_text(GTK_LABEL(copyright_widget), utf8);
    g_free(utf8);
}


/** \brief  Set number of tunes
 *
 * \param[in]   num tune count
 */
void vsid_tune_info_widget_set_tune_count(int num)
{
    tune_count = num;
    update_tune_num_widget();
}


/** \brief  Set default tune
 *
 * \param[in]   num tune number
 */
void vsid_tune_info_widget_set_tune_default(int num)
{
    tune_default = num;
    update_tune_num_widget();
}


/** \brief  Set current tune
 *
 * \param[in]   num tune number
 */
void vsid_tune_info_widget_set_tune_current(int num)
{
    tune_current = num;
    update_tune_num_widget();
}


/** \brief  Set SID model
 *
 * \param[in]   model   model ID (0 = 6581, 1 = 8580)
 */
void vsid_tune_info_widget_set_model(int model)
{
    update_model_widget(model);
}


/** \brief  Set sync factor
 *
 * \param[in]   sync    sync factor (0 = 60Hz/NTSC, 1 = 50Hz/PAL)
 */
void vsid_tune_info_widget_set_sync(int sync)
{
    update_sync_widget(sync);
}


/** \brief  Set IRQ source
 *
 * \param[in]   irq irq source string
 */
void vsid_tune_info_widget_set_irq(const char *irq)
{
    update_irq_widget(irq);
}


/** \brief  Set current run time
 *
 * \param[in]   sec run time in seconds
 */
void vsid_tune_info_widget_set_time(unsigned int sec)
{
    update_runtime_widget(sec);
}


/** \brief  Set driver information
 *
 * \param[in]   text    driver information
 */
void vsid_tune_info_widget_set_driver(const char *text)
{
    /* NOP: replaced with separate driver parameter funcions */
}


/** \brief  Set driver address
 *
 * \param[in]   addr    driver address
 */
void vsid_tune_info_widget_set_driver_addr(uint16_t addr)
{
    driver_info_set_addr(DRV_INFO_DRIVER_ADDR, addr);
}


/** \brief  Set load address
 *
 * \param[in]   addr    load address
 */
void vsid_tune_info_widget_set_load_addr(uint16_t addr)
{
    load_addr = addr;   /* keep for calculating SID memory range */
    driver_info_set_addr(DRV_INFO_LOAD_ADDR, addr);

    /*
     * This is not strictly required, the size of the SID is set *after* the
     * load address is set, so calling this function only in the
     * vsid_tune_info_widget_set_data_size() call should suffice. But if
     * someone ever changes the call order, we'll get weird results.
     */
    driver_info_set_image();
}


/** \brief  Set init routine address
 *
 * \param[in]   addr    init routine address
 */
void vsid_tune_info_widget_set_init_addr(uint16_t addr)
{
    driver_info_set_addr(DRV_INFO_INIT_ADDR, addr);
}


/** \brief  Set play routine address
 *
 * \param[in]   addr    play routine address
 */
void vsid_tune_info_widget_set_play_addr(uint16_t addr)
{
    driver_info_set_addr(DRV_INFO_PLAY_ADDR, addr);
}


/** \brief  Set size of SID on actual machine
 *
 * \param[in]   size    size of SID
 */
void vsid_tune_info_widget_set_data_size(uint16_t size)
{
    data_size = size;   /* keep for calculating SID memory range */
    driver_info_set_image();
}

