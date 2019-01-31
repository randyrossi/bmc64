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


/** \brief  Tokens that are illegal in a path/filename
 *
 */
static const char *illegal_name_tokens = "/?*:";


static char *boot_path = NULL;
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
        exit(0);
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
        exit(0);
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

char *archdep_program_name(void)
{
    static char *program_name = NULL;

    if (program_name == NULL) {
        char *p, name[1024];

        GetProgramName(name, 1024);
        p = FilePart(name);

        if (p != NULL) {
            program_name = lib_stralloc(p);
        }
    }

    return program_name;
}

const char *archdep_boot_path(void)
{
    if (boot_path == NULL) {
        char cwd[1024];
        BPTR lock;

        lock = GetProgramDir();
        if (NameFromLock(lock, cwd, 1024)) {
            if (cwd[strlen(cwd) - 1] != ':') {
                strcat(cwd, "/");
            }
            boot_path = lib_stralloc(cwd);
        }
    }

    return boot_path;
}

char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

    if (default_path == NULL) {
        const char *boot_path;

        boot_path = archdep_boot_path();

        default_path = util_concat(emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   "PRINTER", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                                   boot_path, "PRINTER", NULL);
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
    return util_concat(fname, "~", NULL);
}

char *archdep_default_resource_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "vice-sdl.ini", NULL);
}


/** \brief  Get path to VICE session file
 *
 * The 'session file' is a file that is used to store settings between VICE
 * runs, storing things like the last used directory.
 *
 * \return  path to session file
 */
char *archdep_default_session_file_name(void)
{
    return util_concat(archdep_boot_path(), "/vice-sdl-session.ini", NULL);
}


char *archdep_default_fliplist_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "fliplist-", machine_get_name(), ".vfl", NULL);
}


char *archdep_default_rtc_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "vice-sdl.rtc", NULL);
}

char *archdep_default_autostart_disk_image_file_name(void)
{
    const char *home;

    home = archdep_boot_path();
    return util_concat(home, "autostart-", machine_get_name(), ".d64", NULL);
}

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

char *archdep_default_save_resource_file_name(void)
{
    return archdep_default_resource_file_name();
}

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

int archdep_default_logger(const char *level_string, const char *txt)
{
    return 0;
}

int archdep_path_is_relative(const char *path)
{
    if (path == NULL) {
        return 0;
    }

    return (strchr(path, ':') == NULL);
}

int archdep_spawn(const char *name, char **argv, char **stdout_redir, const char *stderr_redir)
{
    return -1;
}

/* return malloc'd version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    BPTR lock;

    lock = Lock(orig_name, ACCESS_READ);
    if (lock) {
        char name[1024];
        LONG rc;
        rc = NameFromLock(lock, name, 1024);
        UnLock(lock);
        if (rc) {
            *return_path = lib_stralloc(name);
            return 0;
        }
    }
    *return_path = lib_stralloc(orig_name);
    return 0;
}


/** \brief  Sanitize \a path by removing invalid characters for the current OS
 *
 * \param[in,out]   path    0-terminated string
 */
void archdep_sanitize_path(char *path)
{
    return; /* FIXME: stub */
}


void archdep_startup_log_error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    va_end(ap);
}

char *archdep_filename_parameter(const char *name)
{
    return lib_stralloc(name);
}

char *archdep_quote_parameter(const char *name)
{
    return lib_stralloc(name);
}

char *archdep_tmpnam(void)
{
    return lib_stralloc(tmpnam(NULL));
}

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


int archdep_mkdir(const char *pathname, int mode)
{
    return mkdir(pathname, (mode_t)mode);
}

int archdep_rmdir(const char *pathname)
{
    return rmdir(pathname);
}

int archdep_stat(const char *file_name, unsigned int *len, unsigned int *isdir)
{
    struct stat statbuf;

    if (stat(file_name, &statbuf) < 0) {
        return -1;
    }

    *len = statbuf.st_size;
    *isdir = S_ISDIR(statbuf.st_mode);

    return 0;
}

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name)
{
    SetProtection(file_name, 0);
    return 0;
}

int archdep_file_is_blockdev(const char *name)
{
    return 0;
}

int archdep_file_is_chardev(const char *name)
{
    return 0;
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

int archdep_rename(const char *oldpath, const char *newpath)
{
    return rename(oldpath, newpath);
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

#ifdef USE_SDLUI2
char *archdep_sdl2_default_renderers[] = {
    "opengl",
    "compositing",
    "software",
    NULL
};
#endif
