/*
 * uilib.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/asl.h>

#include "private.h"
#include "uilib.h"
#include "lib.h"
#include "imagecontents.h"
#include "diskcontents.h"
#include "tapecontents.h"
#include "mui/filereq.h"

struct uilib_fs_style_type_s {
    image_contents_t *(*content_read_function)(const char *);
    void *hook_proc;
    int TemplateID;
    char *initialdir_resource;
    char *file_resource;
};
typedef struct uilib_fs_style_type_s uilib_fs_style_type_t;

static image_contents_t *read_disk_image_contents(const char *name);
static image_contents_t *read_tape_image_contents(const char *name);
static image_contents_t *read_disk_or_tape_image_contents(const char *name);

static void uilib_select_tape_hook_proc(void){} /* FIXME */
static void uilib_select_hook_proc(void){} /* FIXME */

static uilib_fs_style_type_t styles[UILIB_SELECTOR_STYLES_NUM + 1] = {
    /* UILIB_SELECTOR_STYLE_DEFAULT */
    { NULL,
      NULL, 0, "InitialDefaultDir", NULL },
    /* UILIB_SELECTOR_STYLE_TAPE */
    { read_tape_image_contents,
      uilib_select_tape_hook_proc, IDD_OPENTAPE_TEMPLATE, "InitialTapeDir", NULL },
    /* UILIB_SELECTOR_STYLE_DISK */
    { read_disk_image_contents,
      uilib_select_hook_proc, IDD_OPEN_TEMPLATE, "InitialDiskDir", NULL },
    /* UILIB_SELECTOR_STYLE_DISK_AND_TAPE */
    { read_disk_or_tape_image_contents,
      uilib_select_hook_proc, IDD_OPEN_TEMPLATE, "InitialAutostartDir", NULL },
    /* UILIB_SELECTOR_STYLE_CART */
    { NULL,
      NULL, 0, "InitialCartDir", NULL },
    /* UILIB_SELECTOR_STYLE_SNAPSHOT */
    { NULL,
      NULL, 0, "InitialSnapshotDir", NULL },
    /* UILIB_SELECTOR_STYLE_EVENT_START */
    { NULL,
      NULL, 0, "EventSnapshotDir", "EventStartSnapshot" },
    /* UILIB_SELECTOR_STYLE_EVENT_END */
    { NULL,
      NULL, 0, "EventSnapshotDir", "EventEndSnapshot" },
    /* DUMMY entry Insert new styles before this */
    { NULL,
      NULL, 0, NULL, NULL }
};

static char *ui_file_selector_initialfile[UILIB_SELECTOR_STYLES_NUM];

static image_contents_t *read_disk_image_contents(const char *name)
{
    return diskcontents_filesystem_read(name);
}

static image_contents_t *read_tape_image_contents(const char *name)
{
    return tapecontents_read(name);
}

static image_contents_t *read_disk_or_tape_image_contents(const char *name)
{
    image_contents_t *contents;

    contents = diskcontents_filesystem_read(name);
    if (contents == NULL) {
        contents = tapecontents_read(name);
    }
    return contents;
}

char *uilib_select_file_autostart(const char *title,
                                   unsigned int filterlist, unsigned int type,
                                   int style, int *autostart,
                                   char *resource_readonly)
{
    char *name = NULL;
    char *initialdir = NULL;
    char *initialfile = NULL;
    char *ret = NULL;

    if (styles[style].initialdir_resource != NULL) {
        resources_get_value(styles[style].initialdir_resource, (void *)&initialdir);
    }

    initialfile = ui_file_selector_initialfile[style];
    if (styles[style].file_resource != NULL) {
        resources_get_value(styles[style].file_resource, (void *)&initialfile);
    }

    name = ui_filereq(title, styles[style].TemplateID, initialdir, initialfile, styles[style].content_read_function, autostart, resource_readonly);

    if (name != NULL) {
        char newdir[1024], *ptr = PathPart(name);

        memcpy(newdir, name, (int)(ptr - name));
        newdir[(int)(ptr - name)] = '\0';

        lib_free(ui_file_selector_initialfile[style]);
        if (styles[style].file_resource != NULL) {
            resources_set_value(styles[style].file_resource, FilePart(name));
        }
        ui_file_selector_initialfile[style] = lib_stralloc(FilePart(name));

        resources_set_value(styles[style].initialdir_resource, newdir);
        ret = lib_stralloc(name);
    }

    return ret;
}

char *uilib_select_file(const char *title, unsigned int filterlist,
                         unsigned int type, int style)
{
    return uilib_select_file_autostart(title, filterlist, type, style, NULL, NULL);
}
