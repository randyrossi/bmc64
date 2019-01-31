/*
 * noinlineargs.c
 *
 * Written by
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

#ifdef NO_INLINE_STDARG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "videoarch.h"
#include "palette.h"
#include "video.h"
#include "viewport.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "fullscreenarch.h"
#include "pointer.h"

#define __USE_INLINE__

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include <proto/muimaster.h>
#ifdef AMIGA_M68K
#include <libraries/mui.h>
#endif
#include <proto/graphics.h>

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>
#ifdef AMIGA_MORPHOS
#include <exec/execbase.h>
#endif
#if !defined(AMIGA_MORPHOS) && !defined(AMIGA_AROS)
#include <inline/cybergraphics.h>
#endif
#else
#ifdef HAVE_PROTO_PICASSO96_H
#include <proto/Picasso96.h>
#else
#include <proto/Picasso96API.h>
#endif
#endif

#include <proto/gadtools.h>
#include <proto/asl.h>

#include "private.h"
#include "statusbar.h"
#include "mui/mui.h"

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
#include <cybergraphx/cgxvideo.h>
#include <proto/cgxvideo.h>
#include "video/renderyuv.h"
#endif

#define TAGAMOUNT(start_var, type) va_start(ap, start_var);     \
                                   temp = start_var;            \
                                   while (temp!=TAG_DONE) {     \
                                       temp = va_arg(ap, type); \
                                       temp = va_arg(ap, type); \
                                       amount++;                \
                                   }                            \
                                   va_end(ap)

#define MAKETAGS(start_var, type) tags[0].ti_Tag=start_var;    \
                                  va_start(ap, start_var);     \
                                  while ( i < amount) {        \
                                      temp = va_arg(ap, type); \
                                      tags[i].ti_Data = temp;  \
                                      i++;                     \
                                      temp = va_arg(ap, type); \
                                      tags[i].ti_Tag = temp;   \
                                  }                            \
                                  tags[i].ti_Data = TAG_DONE;  \
                                  va_end(ap)

ULONG BestCModeIDTags(Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    ULONG ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = BestCModeIDTagList(tags);
    lib_free(tags);
    return ret;
}

APTR LockBitMapTags(APTR bitmap, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    APTR ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = LockBitMapTagList(bitmap, tags);
    lib_free(tags);
    return ret;
}

struct Screen *OpenScreenTags(CONST struct NewScreen *screen, ULONG tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    struct Screen *ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = OpenScreenTagList(screen, tags);
    lib_free(tags);
    return ret;
}

struct Window *OpenWindowTags(CONST struct NewWindow *window, ULONG tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    struct Window *ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = OpenWindowTagList(window, tags);
    lib_free(tags);
    return ret;
}

LONG ObtainBestPen(struct ColorMap *cm, ULONG r, ULONG g, ULONG b, ULONG tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    LONG ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = ObtainBestPenA(cm, r, g, b, tags);
    lib_free(tags);
    return ret;
}

APTR GetVisualInfo(struct Screen *screen, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    APTR ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = GetVisualInfoA(screen, tags);
    lib_free(tags);
    return ret;
}

struct Menu *CreateMenus(CONST struct NewMenu *newmenu, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    struct Menu *ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = CreateMenusA(newmenu, tags);
    lib_free(tags);
    return ret;
}

BOOL LayoutMenus(struct Menu *menu, APTR vi, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    BOOL ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = LayoutMenusA(menu, vi, tags);
    lib_free(tags);
    return ret;
}

BOOL AslRequestTags(APTR requester, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    BOOL ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = AslRequest(requester, tags);
    lib_free(tags);
    return ret;
}

APTR AllocAslRequestTags(ULONG reqType, Tag tag, ...)
{
    va_list ap;
    int amount = 0;
    int i = 0;
    ULONG temp;
    APTR ret;
    struct TagItem *tags;

    TAGAMOUNT(tag, ULONG);

    tags = lib_malloc(sizeof(struct TagItem)*(amount + 1));

    MAKETAGS(tag, ULONG);

    ret = AllocAslRequest(reqType, tags);
    lib_free(tags);
    return ret;
}

#if 0 /* this one I have to figure out how to handle */
Object *MUI_MakeObject(LONG type, ...)
{
    return(MUI_MakeObjectA(type, (ULONG *)(((ULONG)&type)+4)));
}
#endif

#endif
