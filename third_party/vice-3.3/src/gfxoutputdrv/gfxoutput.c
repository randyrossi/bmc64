/*
 * gfxoutput.c - Graphics output driver.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "bmpdrv.h"
#include "gfxoutput.h"
#include "gifdrv.h"
#include "lib.h"
#include "log.h"
#include "iffdrv.h"
#include "nativedrv.h"
#include "pcxdrv.h"
#include "ppmdrv.h"
#include "godotdrv.h"

#ifdef HAVE_PNG
#include "pngdrv.h"
#endif

#ifdef HAVE_JPEG
#include "jpegdrv.h"
#endif

#ifdef HAVE_FFMPEG
#include "ffmpegdrv.h"
#endif

#ifdef HAVE_QUICKTIME
#include "quicktimedrv.h"
#endif

struct gfxoutputdrv_list_s {
    struct gfxoutputdrv_s *drv;
    struct gfxoutputdrv_list_s *next;
};
typedef struct gfxoutputdrv_list_s gfxoutputdrv_list_t;

static gfxoutputdrv_list_t *gfxoutputdrv_list = NULL;
static int gfxoutputdrv_list_count = 0;
static log_t gfxoutput_log = LOG_ERR;
static gfxoutputdrv_list_t *gfxoutputdrv_list_iter = NULL;


int gfxoutput_num_drivers(void)
{
    return gfxoutputdrv_list_count;
}

gfxoutputdrv_t *gfxoutput_drivers_iter_init(void)
{
    gfxoutputdrv_list_iter = gfxoutputdrv_list;
    return gfxoutputdrv_list_iter->drv;
}

gfxoutputdrv_t *gfxoutput_drivers_iter_next(void)
{
    if (gfxoutputdrv_list_iter) {
        gfxoutputdrv_list_iter = gfxoutputdrv_list_iter->next;
    }

    if (gfxoutputdrv_list_iter) {
        return gfxoutputdrv_list_iter->drv;
    }

    return NULL;
}

int gfxoutput_early_init(int help)
{
    /* Initialize graphics output driver list.  */
    gfxoutputdrv_list = lib_malloc(sizeof(gfxoutputdrv_list_t));
    gfxoutputdrv_list->drv = NULL;
    gfxoutputdrv_list->next = NULL;

    /* on early init for "-help" commandline, some initialization is skipped
       by the individual drivers */
    gfxoutput_init_bmp(help);
    gfxoutput_init_doodle(help);
    gfxoutput_init_koala(help);
#ifdef HAVE_GIF
    gfxoutput_init_gif(help);
#endif
    gfxoutput_init_iff(help);
#ifdef HAVE_JPEG
    gfxoutput_init_jpeg(help);
#endif
    gfxoutput_init_pcx(help);
#ifdef HAVE_PNG
    gfxoutput_init_png(help);
#endif
    gfxoutput_init_ppm(help);
#ifdef HAVE_FFMPEG
    gfxoutput_init_ffmpeg(help);
#endif
#ifdef HAVE_QUICKTIME
    gfxoutput_init_quicktime(help);
#endif
    gfxoutput_init_godot(help);
    return 0;
}

int gfxoutput_init(void)
{
    gfxoutput_log = log_open("Graphics Output");

    return 0;
}

void gfxoutput_shutdown(void)
{
    gfxoutputdrv_list_t *list, *next;

    list = gfxoutputdrv_list;

    while (list != NULL) {
        /* call shutdown function of driver */
        gfxoutputdrv_t *driver = list->drv;
        if (driver != NULL) {
            if (driver->shutdown != NULL) {
                driver->shutdown();
            }
        }

        next = list->next;
        lib_free(list);
        list = next;
    }
}

/*-----------------------------------------------------------------------*/

int gfxoutput_register(gfxoutputdrv_t *drv)
{
    gfxoutputdrv_list_t *current;

    current = gfxoutputdrv_list;

    /* Warp to end of list.  */
    while (current->next != NULL) {
        current = current->next;
    }

    /* Fill in entry.  */
    current->drv = drv;
    current->next = lib_malloc(sizeof(gfxoutputdrv_list_t));
    current->next->drv = NULL;
    current->next->next = NULL;

    gfxoutputdrv_list_count++;

    return 0;
}

gfxoutputdrv_t *gfxoutput_get_driver(const char *drvname)
{
    gfxoutputdrv_list_t *current = gfxoutputdrv_list;

    while (current->next != NULL) {
        if (strcmp(drvname, current->drv->name) == 0
            || strcmp(drvname, current->drv->displayname) == 0) {
            break;
        }
        current = current->next;
    }

    /* Requested graphics output driver is not registered.  */
    if (current->next == NULL) {
        log_error(gfxoutput_log,
                  "Requested graphics output driver %s not found.",
                  drvname);
        return NULL;
    }
    return current->drv;
}

int gfxoutput_resources_init(void)
{
    gfxoutputdrv_list_t *current = gfxoutputdrv_list;

    while (current->next != NULL) {
        gfxoutputdrv_t *driver = current->drv;
        if (driver && (driver->resources_init != NULL)) {
            int result = driver->resources_init();
            if (result != 0) {
                return result;
            }
        }
        current = current->next;
    }

    return 0;
}

int gfxoutput_cmdline_options_init(void)
{
    gfxoutputdrv_list_t *current = gfxoutputdrv_list;

    while (current->next != NULL) {
        gfxoutputdrv_t *driver = current->drv;
        if (driver && (driver->cmdline_options_init != NULL)) {
            int result = driver->cmdline_options_init();
            if (result != 0) {
                return result;
            }
        }
        current = current->next;
    }

    return 0;
}
