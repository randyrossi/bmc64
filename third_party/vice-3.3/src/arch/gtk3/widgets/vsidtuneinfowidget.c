/** \file   vsidtuneinfowidget.c
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
#include "debug_gtk3.h"
#include "machine.h"
#include "lib.h"
#include "util.h"


#include "hvsc.h"
#include "vsidcontrolwidget.h"

#include "vsidtuneinfowidget.h"


/** \brief  Rows in the driver info grid
 */
enum {
    DRV_INFO_SID_IMAGE = 0,
    DRV_INFO_DRIVER_ADDR,
    DRV_INFO_LOAD_ADDR,
    DRV_INFO_INIT_ADDR,
    DRV_INFO_PLAY_ADDR,
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

#if 0
/* temporary for testing: */
static GtkWidget *sldb_widget;
#endif


/** \brief  List of song lenghts
 */
static long *song_lengths;


/** \brief  Number of songs
 */
static int song_lengths_count;


/** \brief  Handler for the 'destroy' event of the widget
 *
 * Clean up memory used.
 *
 * \param[in]   widget  tune info widget (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer data)
{
    if (song_lengths != NULL) {
        free(song_lengths);
        song_lengths = NULL;
        song_lengths_count = 0;
    }
}


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
gchar *convert_to_utf8(const char *s)
{
    GError *err = NULL;
    gchar *utf8;

    utf8 = g_convert(s, -1, "UTF-8", "ISO-8859-1", NULL, NULL, &err);
    if (err != NULL) {
        debug_gtk3("GError: %d: %s.", err->code, err->message);
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

    if (song_lengths != NULL) {

        unsigned long total = song_lengths[tune_current - 1];
        unsigned int ts = (unsigned int)(total % 60);
        unsigned int tm = (unsigned int)(total / 60);
        unsigned int th = (unsigned int)(total / 60 / 60);


        g_snprintf(buffer, 256, "%u:%02u:%02u / %u:%02u:%02u",
                h, m, s, th, tm, ts);
    } else {
        g_snprintf(buffer, 256, "%u:%02u:%02u", h, m, s);
    }
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


/** \brief  Set a label in the driver info grid at \a row to a \addr
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


#if 0
/** \brief  Create temp songlength widget
 *
 * \return  GtkLabel
 */
static GtkWidget *create_sldb_widget(void)
{
    GtkWidget *label;

    label = gtk_label_new("N/A");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_label_set_line_wrap(GTK_LABEL(label), TRUE);
    gtk_widget_show_all(label);
    return label;
}
#endif


/** \brief  Create widget to show tune information
 *
 * \return  GtkGrid
 */
GtkWidget *vsid_tune_info_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    int row = 0;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* widget title */
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>SID file info:</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(G_OBJECT(label), "margin-bottom", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 2, 1);
    row++;

    /* title */
    label = create_left_aligned_label("Name:");
    name_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_widget, 1, row, 1, 1);
    row++;

    /* author */
    label = create_left_aligned_label("Author:");
    author_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), author_widget, 1, row, 1, 1);
    row++;

    /* copyright */
    label = create_left_aligned_label("(C):");
    copyright_widget = create_readonly_entry();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), copyright_widget, 1, row, 1, 1);
    row++;

    /* tune number (x of x) */
    label = create_left_aligned_label("Tune:");
    tune_num_widget = create_tune_num_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tune_num_widget, 1, row, 1, 1);
    row++;

    /* model widget */
    label = create_left_aligned_label("Model:");
    model_widget = create_model_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), model_widget, 1, row, 1, 1);
    row++;

    /* IRQ widget */
    label = create_left_aligned_label("IRQ:");
    irq_widget = create_irq_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), irq_widget, 1, row, 1, 1);
    row++;

    /* sync widget */
    label = create_left_aligned_label("Sync:");
    sync_widget = create_sync_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sync_widget, 1, row, 1, 1);
    row++;

    /* runtime widget */
    label = create_left_aligned_label("Run time:");
    runtime_widget = create_runtime_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), runtime_widget, 1, row, 1, 1);
    row ++;

    /* driver info */
    label = create_left_aligned_label("Driver:");
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    driver_info_widget = create_driver_info_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), driver_info_widget, 1, row, 1, 1);
    row++;

#if 0
    /* song length info */
    label = create_left_aligned_label("Song lengths:");
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    sldb_widget = create_sldb_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), sldb_widget, 1, 9, 1, 1);
#endif
    g_signal_connect(grid, "destroy", G_CALLBACK(on_destroy), NULL);

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
 * Also sets progress in current tune and handles skipping to the next tune
 * if HVSC SLDB is found. So it probably does too much.
 *
 * \param[in]   sec run time in seconds
 */
void vsid_tune_info_widget_set_time(unsigned int sec)
{
    long total;
    gdouble fraction;

    update_runtime_widget(sec);

    /* HVSC support? */
    if (song_lengths != NULL) {
        /* get song length in seconds */
        total = song_lengths[tune_current - 1];
        /* determine progress bar value */
        fraction = 1.0 - ((gdouble)(total - sec) / (gdouble)total);
        if (fraction < 0.0) {
            fraction = 1.0;
            /* skip to next tune, if repeat is off */
            if (!vsid_control_widget_get_repeat()) {
                vsid_control_widget_next_tune();
                fraction = 0.0;
            }
        }
        vsid_control_widget_set_progress(fraction);
    } else {
        /* non-HVSC fallback: fill progress bar */
        vsid_control_widget_set_progress(1.0);
    }
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

/** \brief  Set song lengths for each sub-tune
 *
 * For now this is more of a debugging/test function, the idea is to allow
 * tunes to automatically skip to the next song when their time is up.
 *
 * \param[in]   SID file
 *
 * \return  bool
 */
int vsid_tune_info_widget_set_song_lengths(const char *psid)
{
    int num;
#if 0
    int i;
    char **lstr;
    char *display;
#endif
    debug_gtk3("trying to get song lengths for '%s'.", psid);

    num = hvsc_sldb_get_lengths(psid, &song_lengths);
    if (num < 0) {
        debug_gtk3("failed to get song lengths.");
#if 0
        gtk_label_set_text(GTK_LABEL(sldb_widget), "Failed to get SLDB info");
#endif
        return 0;
    }
    song_lengths_count = num;
    return 1;
#if 0
    /* alloc memory for strings */
    lstr = lib_malloc((size_t)(num + 1) * sizeof *lstr);
    /* convert each timestamp to string */
    for (i = 0; i < num; i++) {
        lstr[i] = lib_msprintf("#%d: %ld:%02ld",
                i + 1,
                song_lengths[i] / 60, song_lengths[i] % 60);
    }
    lstr[i] = NULL; /* terminate list */

    /* join strings */

    /* Here be dragons: the cast should not be required: */
    display = util_strjoin((const char **)lstr, ", ");
    if (sldb_widget != NULL) {
        gtk_label_set_text(GTK_LABEL(sldb_widget), display);
    }

    lib_free(display);
    for (i = 0; i < num; i++) {
        lib_free(lstr[i]);
    }
    lib_free(lstr);
    return 1;
#endif
}


int vsid_tune_info_widget_get_song_lengths(long **dest)
{
    *dest = song_lengths;
    return song_lengths_count;
}
