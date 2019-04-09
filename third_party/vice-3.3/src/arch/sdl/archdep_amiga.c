/*
 * archdep_amiga.c
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

#ifndef __VBCC__
#define __USE_INLINE__
#endif

#include <proto/dos.h>
#include <proto/exec.h>

#ifndef AMIGA_OS4
#include <proto/socket.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include "archdep.h"
#include "findpath.h"
#include "ioutil.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "ui.h"
#include "util.h"

#if defined(AMIGA_OS4)
#include <exec/execbase.h>
#ifndef __USE_BASETYPE__
  extern struct Library * SysBase;
#else
  extern struct ExecBase * SysBase;
#endif /* __USE_BASETYPE__ */
#endif


static int run_from_wb = 0;

#ifndef AMIGA_OS4
struct Library *SocketBase;
#endif

#ifdef POWERSDL_AMIGA_INLINE
struct Library *PowerSDLBase = NULL;
#define SDLLIBBASE PowerSDLBase
#define SDLLIBNAME "powersdl.library"
#endif

#ifdef SDL_AMIGA_INLINE
struct Library *SDLBase = NULL;
#define SDLLIBBASE SDLBase
#define SDLLIBNAME "SDL.library"
#endif

#if defined(SDL_AMIGA_INLINE) || defined(POWERSDL_AMIGA_INLINE)
void SDL_Quit(void)
{
    SDL_RealQuit();
    CloseLibrary(SDLLIBBASE);
}
#endif

#ifdef SDL_AMIGA_INLINE
int SDL_Init(Uint32 flags)
{
    SDLLIBBASE = OpenLibrary(SDLLIBNAME, 0L);

    if (!SDLLIBBASE) {
        printf("Unable to open %s\n", SDLLIBNAME);
        archdep_vice_exit(0);
    }

    return SDL_RealInit(flags);
}
#endif

#ifdef POWERSDL_AMIGA_INLINE
int VICE_SDL_Init(Uint32 flags)
{
    SDLLIBBASE = OpenLibrary(SDLLIBNAME, 0L);

    if (!SDLLIBBASE) {
        printf("Unable to open %s\n", SDLLIBNAME);
        archdep_vice_exit(0);
    }
    return SDL_Init(flags);
}

#define SDL_REALINIT VICE_SDL_Init
#endif

#define __USE_INLINE__

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#ifdef AMIGA_OS4
struct Library *ExpansionBase = NULL;
struct ExpansionIFace *IExpansion = NULL;
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

typedef struct amiga_libs_s {
    char *lib_name;
    void **lib_base;
    int lib_version;
    void **interface_base;
    int action;
    int **var;
} amiga_libs_t;

static amiga_libs_t amiga_libs[] = {
#ifdef AMIGA_OS4
    { "expansion.library", &ExpansionBase, 50, &IExpansion, LIBS_ACTION_WARNING, &pci_lib_loaded },
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
        if (!amiga_libs[i].lib_base[0]) {
            if (amiga_libs[i].action == LIBS_ACTION_ERROR) {
                return -1;
            } else {
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

static int archdep_init_extra(int *argc, char **argv)
{
    if (*argc == 0) { /* run from WB */
        run_from_wb = 1;
    } else { /* run from CLI */
        run_from_wb = 0;
    }
    load_libs();

    return 0;
}

#if 0
char *archdep_default_autostart_disk_image_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "autostart-", machine_get_name(), ".d64", NULL);
}
#endif

char *archdep_default_hotkey_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "sdl-hotkey-", machine_get_name(), ".vkm", NULL);
}

char *archdep_default_joymap_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "sdl-joymap-", machine_get_name(), ".vjm", NULL);
}


#if 0
FILE *archdep_open_default_log_file(void)
{
    if (run_from_wb) {
        char *fname;
        FILE *f;

        fname = util_concat(archdep_boot_path(), "vice.log", NULL);
        f = fopen(fname, MODE_WRITE_TEXT);

        lib_free(fname);

        if (f == NULL) {
            return stdout;
        }

        return f;
    } else {
        return stdout;
    }
}
#endif

int archdep_default_logger(const char *level_string, const char *txt)
{
    return 0;
}


int archdep_spawn(const char *name, char **argv, char **stdout_redir, const char *stderr_redir)
{
    return -1;
}


/** \brief  Sanitize \a path by removing invalid characters for the current OS
 *
 * \param[in,out]   path    0-terminated string
 */
void archdep_sanitize_path(char *path)
{
    return; /* FIXME: stub */
}

#if 0
char *archdep_tmpnam(void)
{
    return lib_stralloc(tmpnam(NULL));
}
#endif

FILE *archdep_mkstemp_fd(char **filename, const char *mode)
{
    char *tmp;
    FILE *fd;

    tmp = lib_stralloc(tmpnam(NULL));

    fd = fopen(tmp, mode);

    if (fd == NULL) {
        return NULL;
    }

    *filename = tmp;

    return fd;
}


int archdep_require_vkbd(void)
{
    return 0;
}

static void archdep_shutdown_extra(void)
{
    lib_free(boot_path);
    close_libs();
}

#define LF (LDF_DEVICES | LDF_VOLUMES | LDF_ASSIGNS | LDF_READ)

static int CountEntries(void)
{
    int entries = 1;
    struct DosList *dl = LockDosList(LF);

    while (dl = NextDosEntry(dl, LF)) {
        entries++;
    }
    UnLockDosList(LF);

    return entries;
}

char **archdep_list_drives(void)
{
    int drive_count = CountEntries();
    char **result, **p;
    struct DosList *dl = LockDosList(LF);

    result = lib_malloc(sizeof(char*) * drive_count);
    p = result;

    while (dl = NextDosEntry(dl, LF)) {
        *p++ = lib_stralloc(BADDR(dl->dol_Name));
    }
    *p = NULL;

    UnLockDosList(LF);

    return result;
}

char *archdep_get_current_drive(void)
{
    char *p = ioutil_current_dir();
    char *p2 = strchr(p, ':');

    if (p2 == NULL) {
        return lib_stralloc("PROGDIR:");
    }

    p2[1] = '\0';

    return p;
}

void archdep_set_current_drive(const char *drive)
{
    BPTR lck = Lock(drive, ACCESS_READ);

    if (lck) {
        CurrentDir(lck);
        UnLock(lck);
    } else {
        ui_error("Failed to change to drive %s", drive);
    }
}


/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a black (default) config, so an educated guess works good
   enough most of the time :)

   FIXME: add more languages/actual detection
*/
int kbd_arch_get_host_mapping(void)
{
    return KBD_MAPPING_US;
}
