/** \file   dirmenupopup.c
 *  \brief  Create a menu to show a directory of a drive or tape deck
 *
 * FIXME: The current code depends way too much on internal/core code. The code
 *        to retrieve the current disk/tape image should be implemented
 *        somewhere in the core code, not here.
 *
 *
 *  \author Bas Wassink <b.wassink@ziggo.nl>
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
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "imagecontents/diskcontents.h"
#include "drive.h"
#include "drivetypes.h"
#include "diskimage.h"
#include "diskimage/fsimage.h"
#include "vdrive/vdrive.h"
#include "attach.h"
#include "autostart.h"

#include "tape.h"

#include "dirmenupopup.h"


/** \brief  Function to read the contents of an image
 *
 * FIXME:   Hiding the pointer-ness of a function is NOT a good idea
 */
static read_contents_func_type content_func;

/** \brief  Function to call when a file in the directory is selected
 */
static void (*response_func)(const char *, int);

/** \brief  Disk image being used
 *
 * FIXME:   Somehow pass this via the event handlers
 */
static const char *autostart_diskimage;

/** \brief  CSS style string to set the CBM font
 */
static const char *DIRENT_CSS = "label { font-family: \"CBM\"; }";

/** \brief  Reference to the CSS provider used for directory entries
 */
static GtkCssProvider *css_provider;


/* FIXME: stole this from arch/unix/x11/gnome/x11ui.c
 *
 * And I still get warnings from Pango
 */
static unsigned char *convert_utf8(unsigned char *s)
{
    unsigned char *d, *r;

    r = d = lib_malloc((size_t)(strlen((char *)s) * 2 + 1));
    while (*s) {
        if (*s < 0x80) {
            *d = *s;
        } else {
            /* special latin1 character handling */
            if (*s == 0xa0) {
                *d = 0x20;
            } else {
                if (*s == 0xad) {
                    *s = 0xed;
                }
                *d++ = 0xc0 | (*s >> 6);
                *d = (*s & ~0xc0) | 0x80;
            }
        }
        s++;
        d++;
    }
    *d = '\0';
    return r;
}


/** \brief  Handler for the "activate" event of a menu item
 *
 * \param[in]   item    menu item triggering the event
 * \param[in]   data    index in the directory (0 = header)
 */
static void on_item_activate(GtkWidget *item, gpointer data)
{
    int index = GPOINTER_TO_INT(data);

    debug_gtk3("Got index %d, triggering response function", index);
    response_func(autostart_diskimage, index);
}



/** \brief  Create CSS style provider for the directory entries
 *
 * This way we won't be (re)creating 144 or even 296 style provider
 *
 * \return  bool
 */
static gboolean create_css_provider(void)
{
    GError *err = NULL;

    /* instanciate CSS provider */
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, DIRENT_CSS, -1, &err);
    if (err != NULL) {
        log_error(LOG_ERR, "CSS error: %s", err->message);
        g_error_free(err);
        return FALSE;
    }
    return TRUE;
}


/** \brief  Apply CSS provider to \a widget to set the CBM font
 *
 * \param[in,out]   widget  label in a GtkMenuItem
 *
 * \return  bool
 */
static gboolean apply_css_provider(GtkWidget *widget)
{
    GtkStyleContext *css_context;

    css_context = gtk_widget_get_style_context(widget);
    if (css_context == NULL) {
        log_error(LOG_ERR, "Couldn't get style context of widget");
        return FALSE;
    }

    gtk_style_context_add_provider(css_context,
            GTK_STYLE_PROVIDER(css_provider),
            GTK_STYLE_PROVIDER_PRIORITY_USER);
    return TRUE;
}


/** \brief  Create a popup menu to select a file to autostart
 *
 * XXX: This is an UNHOLY MESS, and should be refactored
 *
 * \param[in]   dev         device index (0-3 for drives, < 0 for tape)
 * \param[in]   func        function to read image contents
 * \param[in]   response    function to call when an item has been selected
 *
 * \return  GtkMenu
 */
GtkWidget *dir_menu_popup_create(
        int dev,
        read_contents_func_type func,
        void (*response)(const char *, int))
{
    GtkWidget *menu;
    GtkWidget *item;
    char buffer[1024];
    image_contents_t *contents = NULL;
    image_contents_file_list_t *entry;
    char *utf8;
    char *tmp;
    int index;
    GtkWidget *label;

    debug_gtk3("DEVICE = %d.", dev);

    /* create style provider */
    if (!create_css_provider()) {
        debug_gtk3("failed to create CSS provider, borking");
        return NULL;
    }

    /* set callback functions */
    content_func = func;
    response_func = response;

    /* create new menu */
    menu = gtk_menu_new();

    /* create menu header */
    if (dev >= 0) {
        g_snprintf(buffer, 1024, "Directory of unit %d:", dev + 8);
    } else {
        g_snprintf(buffer, 1024, "Directory of attached tape:");
    }
    item = gtk_menu_item_new_with_label(buffer);
    gtk_container_add(GTK_CONTAINER(menu), item);

    if (dev >= 0) {
        /*
         * The following is complete horseshit, this needs to be implemented in
         * a function in drive/vdrive somehow. This much dereferencing in UI
         * code is not normal method.
         */

        debug_gtk3("DEV = %d.", dev);

        vdrive_t *vdrive = NULL;
        struct disk_image_s *diskimg = NULL;
        autostart_diskimage = NULL;

        debug_gtk3("Getting vdrive reference for unit #%d.", dev + 8);
        vdrive = file_system_get_vdrive(dev + 8);
        if (vdrive == NULL) {
            debug_gtk3("failed: got NULL.");
        } else {
            debug_gtk3("OK, Getting disk image from vdrive instance.");
            diskimg = vdrive->image;
            if (diskimg == NULL) {
                debug_gtk3("failed: got NULL.");
            } else {
                debug_gtk3("OK, Getting fsimage from disk image.");
                autostart_diskimage = diskimg->media.fsimage->name;
                if (autostart_diskimage == NULL) {
                    debug_gtk3("failed: got NULL.");
                } else {
                    debug_gtk3("Got '%s'.", autostart_diskimage);
                }
            }
        }

       debug_gtk3("fsimage is %s.", autostart_diskimage);
    } else {
        debug_gtk3("Trying tape for some reason.");
        /* tape image */
        if (tape_image_dev1 == NULL) {
            item = gtk_menu_item_new_with_label("<<NO IMAGE ATTACHED>>");
            gtk_container_add(GTK_CONTAINER(menu), item);
            return menu;
        }
        autostart_diskimage = tape_image_dev1->name;
    }

    debug_gtk3("Did we get some image?");
    if (autostart_diskimage != NULL) {
        /* read dir and add them as menu items */
        contents = content_func(autostart_diskimage);
        if (contents == NULL) {
            debug_gtk3("content reading function failed!");
            item = gtk_menu_item_new_with_label(
                    "Failed to read directory");
            gtk_container_add(GTK_CONTAINER(menu), item);
        } else {
            debug_gtk3("Getting disk name & ID:");
            /* DISK name & ID */
            tmp = image_contents_to_string(contents, 0);
            utf8 = (char *)convert_utf8((unsigned char *)tmp);
            item = gtk_menu_item_new_with_label(utf8);
            label = gtk_bin_get_child(GTK_BIN(item));
            apply_css_provider(label);

            gtk_container_add(GTK_CONTAINER(menu), item);
            lib_free(tmp);
            lib_free(utf8);

            /* add separator */
            item = gtk_separator_menu_item_new();
            gtk_container_add(GTK_CONTAINER(menu), item);

            /* add files */
            index = 0;
            for (entry = contents->file_list; entry != NULL;
                    entry = entry->next) {

                tmp = image_contents_file_to_string(entry, 0);
                utf8 = (char *)convert_utf8((unsigned char *)tmp);
                item = gtk_menu_item_new_with_label(utf8);
                label = gtk_bin_get_child(GTK_BIN(item));
                apply_css_provider(label);

                gtk_container_add(GTK_CONTAINER(menu), item);

                g_signal_connect(item, "activate",
                        G_CALLBACK(on_item_activate), GINT_TO_POINTER(index));
                index++;
                lib_free(tmp);
                lib_free(utf8);
            }
        }
        if (contents != NULL) {
            image_contents_destroy(contents);
        }
    } else {
        debug_gtk3("autostart_diskimage is NULL");
        item = gtk_menu_item_new_with_label("<<NO IMAGE ATTACHED>>");
        gtk_container_add(GTK_CONTAINER(menu), item);
    }
    gtk_widget_show_all(GTK_WIDGET(menu));


    return menu;
}
