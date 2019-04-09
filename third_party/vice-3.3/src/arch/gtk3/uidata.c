/** \file   uidata.c
 * \brief   GTK3 binary resources handling
 *
 * Mostly a wrapper around the GResource API, which appears to be the way to
 * handle binary resources in Gtk3 applications.
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * \note    WORK IN PROGRESS, leave this alone for now -- compyx
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
#include <stdio.h>
#include <stdlib.h>

#include "archdep.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "util.h"

#include "uidata.h"



/** \brief  Reference to the global GResource data
 */
static GResource *gresource = NULL;


/** \brief  Intialize the GResource binary blob handling
 *
 * First tries to load from src/arch/gtk3/data and then from VICEDIR/data.
 * Loading from VICEDIR/data will fail, the vice.gresource file doesn't get
 * copied there yet with a make install, nor does the VICEDIR/data dir exist.
 *
 * \return  bool
 */
int uidata_init(void)
{
    GError *err = NULL;
#ifdef HAVE_DEBUG_GTK3UI
    char **files;
    int i;
#endif
    char *path;
    char *dir;

    /* try directory with VICE's data files */
    dir = archdep_get_vice_datadir();
    debug_gtk3("trying archdep_get_vice_datadir() (%s).", dir);
    path = archdep_join_paths(dir, "vice.gresource", NULL);
    lib_free(dir);

    gresource = g_resource_load(path, &err);
    if (gresource == NULL && err != NULL) {
        debug_gtk3("failed to load resource data '%s': %s.",
                path, err->message);
        g_clear_error(&err);
        lib_free(path);
        return 0;
    }
    lib_free(path);
    g_resources_register(gresource);

    /* debugging: show files in the resource blob */
#ifdef HAVE_DEBUG_GTK3UI
    files = g_resource_enumerate_children(
            gresource,
            UIDATA_ROOT_PATH,
            G_RESOURCE_LOOKUP_FLAGS_NONE,
            &err);
    if (files == NULL && err != NULL) {
        debug_gtk3("couldn't enumerate children: %s.", err->message);
        g_clear_error(&err);
        return 0;
    }
    debug_gtk3("Listing files in the GResource file:");
    for (i = 0; files[i] != NULL; i++) {
        debug_gtk3("%d: %s.", i, files[i]);
    }
#endif
    return 1;
}


/** \brief  Clean up GResource data
 */
void uidata_shutdown(void)
{
    debug_gtk3("freeing GResource data.");
    if (gresource != NULL) {
        g_free(gresource);
        gresource = NULL;
    }
}


/** \brief  Get a pixbuf from the GResource blob
 *
 * \param   name    virtual path to the file
 *
 * \return  pixbuf or `NULL` on error
 */
GdkPixbuf * uidata_get_pixbuf(const char *name)
{
    GdkPixbuf *buf;
    GError *err = NULL;
    char *path;

    path = util_concat(UIDATA_ROOT_PATH, "/", name, NULL);
    debug_gtk3("attempting to load resource '%s'.", path);
    buf = gdk_pixbuf_new_from_resource(path, &err);
    lib_free(path);
    if (buf == NULL) {
        debug_gtk3("failed: %s.", err->message);
        /* TODO: log error */
        g_clear_error(&err);
    }
    return buf;
}
