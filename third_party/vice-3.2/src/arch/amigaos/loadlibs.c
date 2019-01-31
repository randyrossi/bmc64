/*
 * loadlibs.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "videoarch.h"
#include "palette.h"
#include "video.h"
#include "viewport.h"
#include "kbd.h"
#include "keyboard.h"
#include "loadlibs.h"
#include "lib.h"
#include "log.h"
#include "fullscreenarch.h"
#include "pointer.h"

#define __USE_INLINE__
#define MUIPROG_H

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

#include "private.h"
#include "statusbar.h"
#include "mui/mui.h"

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
#include <cybergraphx/cgxvideo.h>
#include <proto/cgxvideo.h>
#include "video/renderyuv.h"
#endif

#ifdef AMIGA_OS4
struct Library *GadToolsBase = NULL;
struct GadToolsIFace *IGadTools = NULL;
/*struct Library *SocketBase  = NULL;*/
/*struct SocketIFace *ISocket = NULL;*/
struct MUIMasterIFace *IMUIMaster = NULL;
struct Library *ExpansionBase = NULL;
struct ExpansionIFace *IExpansion = NULL;
struct Library *AIN_Base = NULL;
struct AIN_IFace *IAIN = NULL;
int amigainput_lib_loaded = 1;
#endif

#if !defined(AMIGA_OS4) && defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
struct Library *CGXVideoBase = NULL;
int xvideo_lib_loaded = 1;
#endif

#if !defined(AMIGA_OS4) && !defined(HAVE_PROTO_CYBERGRAPHICS_H)
struct Library *P96Base = NULL;
#endif

#if !defined(AMIGA_OS4) && defined(HAVE_PROTO_CYBERGRAPHICS_H)
struct Library *CyberGfxBase = NULL;
#endif

#if defined(AMIGA_AROS) && !defined(WORKING_AROS_AUTO)
struct IntuitionBase *IntuitionBase = NULL;
struct Library *AslBase = NULL;
struct Library *DiskfontBase = NULL;
struct Library *GadToolsBase = NULL;
struct GfxBase *GfxBase = NULL;
struct Library *LocaleBase = NULL;
#endif

struct Library *MUIMasterBase = NULL;

#ifndef AMIGA_OS4
struct Library *LowLevelBase = NULL;
int lowlevel_lib_loaded = 1;
#endif

#ifdef HAVE_PROTO_OPENPCI_H
struct Library *OpenPciBase = NULL;
#endif

#if defined(HAVE_PROTO_OPENPCI_H) || defined(AMIGA_OS4)
int pci_lib_loaded = 1;
#endif

/* ----------------------------------------------------------------------- */

#define LIBS_ACTION_ERROR     0
#define LIBS_ACTION_WARNING   1

#ifndef MUIMASTER_NAME
#define MUIMASTER_NAME "muimaster.library"
#endif

#ifndef MUIMASTER_VMIN
#define MUIMASTER_VMIN 11
#endif

typedef struct amiga_libs_s {
    char *lib_name;
    void **lib_base;
    int lib_version;
    void **interface_base;
    int action;
    int **var;
} amiga_libs_t;

static amiga_libs_t amiga_libs[] = {
#if defined(AMIGA_AROS) && !defined(WORKING_AROS_AUTO)
    { "intuition.library", &IntuitionBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
    { "asl.library", &AslBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
    { "diskfont.library", &DiskfontBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
    { "gadtools.library", &GadToolsBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
    { "graphics.library", &GfxBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
    { "locale.library", &LocaleBase, 39L, NULL, LIBS_ACTION_ERROR, NULL },
#endif
#ifdef AMIGA_OS4
    /*{ "bsdsocket.library", &SocketBase, 4, &ISocket, LIBS_ACTION_ERROR, NULL },*/
    { "gadtools.library", &GadToolsBase, 39, &IGadTools, LIBS_ACTION_ERROR, NULL },
    { "expansion.library", &ExpansionBase, 50, &IExpansion, LIBS_ACTION_WARNING, &pci_lib_loaded },
    /*{ "AmigaInput.library", &AIN_Base, 51, &IAIN, LIBS_ACTION_WARNING, &amigainput_lib_loaded },*/
    { MUIMASTER_NAME, &MUIMasterBase, MUIMASTER_VMIN, &IMUIMaster, LIBS_ACTION_ERROR, NULL },
#endif
#if defined(AMIGA_M68K) && !defined(HAVE_PROTO_CYBERGRAPHICS_H)
    { "Picasso96API.library", &P96Base, 2, NULL, LIBS_ACTION_ERROR, NULL },
#endif
#if (defined(AMIGA_M68K) && defined(HAVE_PROTO_CYBERGRAPHICS_H)) || defined(AMIGA_MORPHOS) || defined(AMIGA_AROS)
    { CYBERGFXNAME, &CyberGfxBase, 41, NULL, LIBS_ACTION_ERROR, NULL },
#ifdef HAVE_XVIDEO
    { "cgxvideo.library", &CGXVideoBase, 41, NULL, LIBS_ACTION_WARNING, &xvideo_lib_loaded },
#endif
#endif
#ifndef AMIGA_OS4
    { MUIMASTER_NAME, &MUIMasterBase, MUIMASTER_VMIN, NULL, LIBS_ACTION_ERROR, NULL },
    { "lowlevel.library", &LowLevelBase, 37, NULL, LIBS_ACTION_WARNING, &lowlevel_lib_loaded },
#endif
#ifdef HAVE_PROTO_OPENPCI_H
    { "openpci.library", &OpenPciBase, 0, NULL, LIBS_ACTION_WARNING, &pci_lib_loaded },
#endif
    { NULL, NULL, 0, NULL, 0, NULL }
};

int load_libs(void)
{
    int i = 0;

    while (amiga_libs[i].lib_name) {
        amiga_libs[i].lib_base[0] = OpenLibrary(amiga_libs[i].lib_name, amiga_libs[i].lib_version);
#ifdef AMIGA_OS4
        if (amiga_libs[i].lib_base[0]) {
            amiga_libs[i].interface_base[0] = GetInterface(amiga_libs[i].lib_base[0], "main", 1, NULL);
            if (amiga_libs[i].interface_base[0] == NULL) {
                CloseLibrary(amiga_libs[i].lib_base[0]);
                amiga_libs[i].lib_base[0] = NULL;
            }
        }
#endif
        if (amiga_libs[i].lib_base[0]) {
            log_message(LOG_DEFAULT, "Loaded %s (%d).", amiga_libs[i].lib_name, amiga_libs[i].lib_version);
        } else {
            if (amiga_libs[i].action == LIBS_ACTION_ERROR) {
                log_message(LOG_DEFAULT, "Could not load %s (%d), this is a critical library, emulator will be closed.", amiga_libs[i].lib_name, amiga_libs[i].lib_version); 
                return -1;
            } else {
                log_message(LOG_DEFAULT, "Could not load %s (%d), this is an optional library, features that need this library will be disabled.", amiga_libs[i].lib_name, amiga_libs[i].lib_version);
                amiga_libs[i].var[0] = 0;
            }
        }
        i++;
    }
    return 0;
}

void close_libs(void)
{
    int i = 0;

    while (amiga_libs[i].lib_name) {
#ifdef AMIGA_OS4
        if (amiga_libs[i].interface_base) {
            DropInterface((struct Interface *)amiga_libs[i].interface_base[0]);
        }
#endif
        if (amiga_libs[i].lib_base) {
            CloseLibrary(amiga_libs[i].lib_base[0]);
        }
        i++;
    }
}
