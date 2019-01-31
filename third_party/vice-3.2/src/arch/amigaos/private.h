/*
 * private.h
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

#ifndef VICE_PRIVATE_H_
#define VICE_PRIVATE_H_

/* This is just a temporary place for some stuff */

#ifndef __USE_INLINE__
#define __USE_INLINE__
#endif

#include <stdio.h>
#include <stdlib.h>

#include "attach.h"
#include "autostart.h"
#include "drive.h"
#include "vsync.h"
#include "machine.h"
#include "ui.h"
#include "resources.h"
#include "videoarch.h"

#undef BYTE
#undef WORD

#include "timer.h"

#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/intuition.h>
#include <proto/asl.h>
#include <proto/graphics.h>
#include <proto/dos.h>
#include <proto/iffparse.h>

struct os_s {
    struct Screen *screen;
    struct Window *window;
    int visible_width;
    int visible_height;
    struct BitMap *window_bitmap;
    unsigned long pixfmt;
    unsigned int bpr, bpp;
    char *window_name;
    APTR VisualInfo;
    struct Menu *menu;
    char window_title[1024];
    int has_statusbar;
    int pens[16];
    struct TextFont *font;
    int disk_width, tape_width;
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    APTR vlayer_handle;
    ULONG vlayer_colorkey;
#endif
};

extern video_canvas_t *canvaslist;

#ifdef AMIGA_OS4
extern timer_t *vice_timer;
#else
extern void *vice_timer;
#endif

typedef struct ui_menu_toggle_s {
    /* Name of resource.  */
    const char *name;
    /* IDM of the corresponding menu item.  */
    int idm;
} ui_menu_toggle_t;

typedef struct ui_res_possible_values_s {
    int value;
    int idm; /* The last idm has to be zero.  */
} ui_res_possible_values_t;

typedef struct ui_res_value_list_s {
    const char *name;
    const ui_res_possible_values_t *vals;
    int default_idm;
} ui_res_value_list_t;

typedef int (*ui_machine_specific_t)(video_canvas_t *canvas, int idm);

extern void ui_register_menu_layout(struct NewMenu *menu);
extern void ui_register_machine_specific(ui_machine_specific_t func);
extern void ui_register_menu_toggles(const ui_menu_toggle_t *toggles);
extern void ui_register_res_values(const ui_res_value_list_t *valuelist);

extern int ui_menu_create(video_canvas_t *canvas);
extern int ui_menu_update(video_canvas_t *canvas);
extern int ui_menu_handle(video_canvas_t *canvas, int idm);
extern int ui_menu_destroy(video_canvas_t *canvas);

#endif /* VICE_PRIVATE_H_ */