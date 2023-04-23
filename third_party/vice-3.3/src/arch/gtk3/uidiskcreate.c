/** \file   uidiskcreate.c
 * \brief   Gtk3 dialog to create and attach a new disk image
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
#include <string.h>

#include "basewidgets.h"
#include "basedialogs.h"
#include "debug_gtk3.h"
#include "widgethelpers.h"
#include "driveunitwidget.h"
#include "diskimage.h"
#include "filechooserhelpers.h"
#include "util.h"
#include "lib.h"
#include "charset.h"
#include "attach.h"
#include "vdrive/vdrive-internal.h"
#include "imagecontents.h"
#include "resources.h"
#include "ui.h"

#include "uidiskcreate.h"


/** \brief  Struct holding image type names and IDs
 */
typedef struct disk_image_type_s {
    const char *name;   /**< name (ext) */
    int id;             /**< image type ID */
} disk_image_type_t;


/* forward declaration */
static gboolean create_disk_image(const char *filename);


/** \brief  List of supported disk image types
 *
 * XXX: perhaps some function in diskimage.c or so producing a list of
 *      currently supported images types like this one would be better, that
 *      would avoid having to update UI's when a new image type is added or
 *      removed.
 */
static disk_image_type_t disk_image_types[] = {
    { "d64", DISK_IMAGE_TYPE_D64 },
    { "d67", DISK_IMAGE_TYPE_D67 },
    { "d71", DISK_IMAGE_TYPE_D71 },
    { "d80", DISK_IMAGE_TYPE_D80 },
    { "d81", DISK_IMAGE_TYPE_D81 },
    { "d82", DISK_IMAGE_TYPE_D82 },
    { "d1m", DISK_IMAGE_TYPE_D1M },
    { "d2m", DISK_IMAGE_TYPE_D2M },
    { "d4m", DISK_IMAGE_TYPE_D4M },
    { "dhd", DISK_IMAGE_TYPE_DHD },
    { "g64", DISK_IMAGE_TYPE_G64 },
    { "g71", DISK_IMAGE_TYPE_G71 },
    { "p64", DISK_IMAGE_TYPE_P64 },
    { "x64", DISK_IMAGE_TYPE_X64 },
    { NULL, -1 }
};


/** \brief  Drive unit to attach image to */
static int unit_number = 8;
/** \brief  Disk image type to create */
static int image_type = 1541;

/** \brief  GtkEntry containing the disk name */
static GtkWidget *disk_name;
/** \brief  GtkEntry containing the disk ID */
static GtkWidget *disk_id;
/** \brief  Set drive type when attaching */
static GtkWidget *set_drive_type;


/** \brief  Handler for 'response' event of the dialog
 *
 * This handler is called when the user clicks a button in the dialog.
 *
 * \param[in]   widget      the dialog
 * \param[in]   response_id response ID
 * \param[in]   data        extra data (unused)
 */
static void on_response(GtkWidget *widget, gint response_id, gpointer data)
{
    gchar *filename;
    int status = TRUE;

    switch (response_id) {

        case GTK_RESPONSE_ACCEPT:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            if (filename != NULL) {
                status = create_disk_image(filename);
            }
            g_free(filename);
            if (status) {
                /* image creation and attaching was succesful, exit dialog */
                gtk_widget_destroy(widget);
            }
            break;

        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            debug_gtk3("warning: unhandled response ID %d\n", response_id);
            break;
    }
}


/** \brief  Handler for the 'changed' event of the image type combo box
 *
 * \param[in]   combo   combo box
 * \param[in]   data    extra event data (unused)
 *
 */
static void on_disk_image_type_changed(GtkComboBox *combo, gpointer data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_combo_box_get_active(combo) >= 0) {
        model = gtk_combo_box_get_model(combo);
        if (gtk_combo_box_get_active_iter(combo, &iter)) {
            gtk_tree_model_get(model, &iter, 1, &image_type, -1);
            debug_gtk3("got disk image type %d\n", image_type);
        }
    }
}


static gboolean attempt_to_set_drive_type(void)
{
    if (resources_set_int_sprintf("Drive%dType", image_type, unit_number) < 0) {
        debug_gtk3("failed to set Drive%dType to %d\n", unit_number, image_type);
        return FALSE;
    }
    return TRUE;
}


/** \brief  Get the extension for image \a type
 *
 * \param[in]   type    image type
 *
 * \return  extension or `NULL` when not found
 */
static const char *get_ext_by_image_type(int type)
{
    int i = 0;

    for (i = 0; disk_image_types[i].name != NULL; i++) {
        if (disk_image_types[i].id == type) {
            return disk_image_types[i].name;
        }
    }
    return NULL;
}


/** \brief  Actually create the disk image and attach it
 *
 * \param[in]   filename    filename of the new image
 *
 * \return  bool
 */
static gboolean create_disk_image(const char *filename)
{
    char *fname_copy;
    char name_vice[IMAGE_CONTENTS_NAME_LEN + 1];
    char id_vice[IMAGE_CONTENTS_ID_LEN + 1];
    const char *name_gtk3;
    const char *id_gtk3;
    char *vdr_text;
    int status = TRUE;

    memset(name_vice, 0, IMAGE_CONTENTS_NAME_LEN + 1);
    memset(id_vice, 0, IMAGE_CONTENTS_ID_LEN + 1);
    name_gtk3 = gtk_entry_get_text(GTK_ENTRY(disk_name));
    id_gtk3 = gtk_entry_get_text(GTK_ENTRY(disk_id));

    /* fix extension of filename */
    fname_copy = util_add_extension_const(filename,
                                          get_ext_by_image_type(image_type));

    /* convert name & ID to PETSCII */
    if (name_gtk3 != NULL && *name_gtk3 != '\0') {
        strncpy(name_vice, name_gtk3, IMAGE_CONTENTS_NAME_LEN);
        charset_petconvstring((unsigned char *)name_vice, 0);
    }
    if (id_gtk3 != NULL && *id_gtk3 != '\0') {
        strncpy(id_vice, id_gtk3, IMAGE_CONTENTS_ID_LEN);
        charset_petconvstring((unsigned char *)id_vice, 0);
    } else {
        strcpy(id_vice, "00");
    }

    vdr_text = util_concat(name_vice, ",", id_vice, NULL);
#if 0
    vice_gtk3_message_info("Creating disk image",
            "Attaching '%s' at unit #%d, type %d, name '%s', ID '%s'\n"
            "Passing \"%s\" to vdrive",
            filename, unit_number, image_type, name_gtk3, id_gtk3,
            vdr_text);
#endif

    /* create image */
    if (vdrive_internal_create_format_disk_image(fname_copy, vdr_text,
                image_type) < 0) {
        vice_gtk3_message_error("Fail", "Could not create image '%s'",
                fname_copy);
        status = FALSE;
    } else {
        /* do we need to attempt to set the proper drive type? */
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(set_drive_type))) {
            /* try to set the proper drive type, but keep going if it fails */
            if (!attempt_to_set_drive_type()) {
                vice_gtk3_message_error("Core error",
                        "Failed to set drive type to %d\nContinuing.",
                        image_type);
            }
        }

        /* finally attach the disk image */
        if (file_system_attach_disk(unit_number, fname_copy) < 0) {
            vice_gtk3_message_error("fail", "Could not attach image '%s'",
                    fname_copy);
            status = FALSE;
        }
    }

    lib_free(fname_copy);
    lib_free(vdr_text);
    return status;
}




/** \brief  Create model for the image type combo box
 *
 * \return  model
 */
static GtkListStore *create_disk_image_type_model(void)
{
    GtkListStore *model;
    GtkTreeIter iter;
    int i;

    model = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    for (i = 0; disk_image_types[i].name != NULL; i++) {
        gtk_list_store_append(model, &iter);
        gtk_list_store_set(model, &iter,
                0, disk_image_types[i].name,
                1, disk_image_types[i].id,
                -1);
    }
    return model;
}


/** \brief  Create combo box with image types
 *
 * \return  GtkComboBox
 */
static GtkWidget *create_disk_image_type_widget(void)
{
    GtkWidget *combo;
    GtkListStore *model;
    GtkCellRenderer *renderer;

    model = create_disk_image_type_model();
    combo = gtk_combo_box_new_with_model(GTK_TREE_MODEL(model));
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer,
            "text", 0, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

    g_signal_connect(combo, "changed", G_CALLBACK(on_disk_image_type_changed),
            NULL);
    return combo;
}


/** \brief  Create the 'extra' widget for the dialog
 *
 * \param[in]   parent  parent widget (dialog, unused at the moment)
 * \param[in]   unit    default unit number (unused)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_extra_widget(GtkWidget *parent, int unit)
{
    GtkWidget *grid;
    GtkWidget *unit_widget;
    GtkWidget *type_widget;
    GtkWidget *label;

    /* create a grid with some spacing and margins */
    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    g_object_set(grid, "margin-left", 16, "margin-right", 16, NULL);

    /* add unit selection widget */
    unit_widget = drive_unit_widget_create(unit, &unit_number, NULL);
    gtk_widget_set_valign(unit_widget, GTK_ALIGN_CENTER);
    gtk_grid_attach(GTK_GRID(grid), unit_widget, 0, 0, 1, 1);

    /* disk name */
    label = gtk_label_new("Name:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    disk_name = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(disk_name), IMAGE_CONTENTS_NAME_LEN);
    gtk_entry_set_max_length(GTK_ENTRY(disk_name), IMAGE_CONTENTS_NAME_LEN);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), disk_name, 2, 0, 1, 1);

    /* disk ID */
    label = gtk_label_new("ID:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    disk_id = gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(disk_id), IMAGE_CONTENTS_ID_LEN);
    gtk_entry_set_max_length(GTK_ENTRY(disk_id), IMAGE_CONTENTS_ID_LEN);
    gtk_grid_attach(GTK_GRID(grid), label, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), disk_id, 4, 0, 1, 1);

    /* add image type selection widget */
    label = gtk_label_new("Type:");
    type_widget = create_disk_image_type_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 5, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), type_widget, 6, 0, 1, 1);

    /* add 'set drive type for attached image' checkbox */
    set_drive_type = gtk_check_button_new_with_label(
            "Set proper drive type when attaching image");
    /* disable by default */
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(set_drive_type), FALSE);
    gtk_grid_attach(GTK_GRID(grid), set_drive_type, 0, 1, 4, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create and show 'attach new disk image' dialog
 *
 */
void uidiskcreate_dialog_show(GtkWidget *parent, gpointer data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;
    int unit;

    unit = GPOINTER_TO_INT(data);
    /* TODO: stuff some UNIT_MIN/UNIT_MAX defines in some file */
    if (unit < 8 || unit > 11) {
        unit = 8;
    }
    unit_number = unit;

    dialog = gtk_file_chooser_dialog_new(
            "Create and attach a new disk image",
            ui_get_active_window(),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            /* buttons */
            "Save", GTK_RESPONSE_ACCEPT,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
            create_extra_widget(dialog, unit));

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
            TRUE);

    filter = create_file_chooser_filter(file_chooser_filter_disk, FALSE);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    gtk_widget_show(dialog);
}
