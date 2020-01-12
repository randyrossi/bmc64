/*
 * archdep.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>

#include "archdep.h"
#include "circle.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "keyboard.h"
#include "util.h"
#include "vicemaxpath.h"

static const char *illegal_name_tokens = "/";
static char *default_path;
static char *program_name = NULL;

int archdep_init(int *argc, char **argv) { return 0; }

const char *archdep_program_name(void) {
  if (program_name == NULL) {
    program_name = lib_malloc(5);
    strcpy(program_name, "vice");
  }

  return program_name;
}

const char *archdep_boot_path(void) { return ""; }

char *archdep_default_sysfile_pathlist(const char *emu_id) {

  if (default_path == NULL) {
    const char *boot_path = archdep_boot_path();
    default_path =
        util_concat(boot_path, "/", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                    boot_path, "/", "DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                    NULL);
                    // boot_path, "/", "PRINTER", NULL);
  }

  return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname) {
  char *tmp;

  tmp = util_concat(fname, NULL);
  tmp[strlen(tmp) - 1] = '~';
  return tmp;
}

char *archdep_default_fliplist_file_name(void) {
  static char *fname;

  lib_free(fname);
  fname = util_concat(archdep_boot_path(), "/fliplist-", machine_get_name(),
                      ".vfl", NULL);
  return fname;
}

char *archdep_default_rtc_file_name(void) {
  static char *fname;

  lib_free(fname);
  fname = util_concat(archdep_boot_path(), "/vice.rtc", NULL);
  return fname;
}

char *archdep_default_autostart_disk_image_file_name(void) {
  const char *home;

  home = archdep_boot_path();
  return util_concat(home, "/autostart-", machine_get_name(), ".d64", NULL);
}

FILE *archdep_open_default_log_file(void) {
  // Don't ever need this for BMC64.
  return 0;
}

int archdep_default_logger(const char *level_string, const char *txt) {
  if (fputs(level_string, stdout) == EOF || fprintf(stdout, txt) < 0 ||
      fputc('\n', stdout) == EOF) {
    return -1;
  }
  return 0;
}

int archdep_path_is_relative(const char *path) {
  if (path == NULL) {
    return 0;
  }

  return *path != '/';
}

int archdep_spawn(const char *name, char **argv, char **pstdout_redir,
                  const char *stderr_redir) {
  log_error(LOG_DEFAULT, "archdep_spawn unimpl: %s", name);
  return -1;
}

/* return malloced version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name) {
  *return_path = lib_stralloc(orig_name);
  return 0;
}

/** \brief  Sanitize \a name by removing invalid characters for the current OS
 *
 * \param[in,out]   name    0-terminated string
 */
void archdep_sanitize_filename(char *name) {
  while (*name != '\0') {
    int i = 0;
    while (illegal_name_tokens[i] != '\0') {
      if (illegal_name_tokens[i] == *name) {
        *name = '_';
        break;
      }
      i++;
    }
    name++;
  }
}

void archdep_startup_log_error(const char *format, ...) {
  char *tmp;
  va_list args;

  va_start(args, format);
  tmp = lib_mvsprintf(format, args);
  va_end(args);
  printf(tmp);

  lib_free(tmp);
}

char *archdep_quote_parameter(const char *name) { return lib_stralloc(name); }

char *archdep_filename_parameter(const char *name) {
  char *exp;
  char *a;

  archdep_expand_path(&exp, name);
  a = archdep_quote_parameter(exp);
  lib_free(exp);
  return a;
}

char *archdep_tmpnam(void) { return lib_stralloc(tmpnam(NULL)); }

FILE *archdep_mkstemp_fd(char **filename, const char *mode) {
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

int archdep_mkdir(const char *pathname, int mode) {
  log_error(LOG_DEFAULT, "archdep_mkdir unimpl: %s", pathname);
  return 0;
}

int archdep_rmdir(const char *pathname) {
  log_error(LOG_DEFAULT, "archdep_rmdir unimpl: %s", pathname);
  return 0;
}

int archdep_stat(const char *file_name, unsigned int *len,
                 unsigned int *isdir) {
  struct stat st;
  int fn_len;

  // The consequence of having to yield to let some fake kernel threads
  // run is troublesome.  When the emulated machine calls into us for
  // real file operations (i.e. iecdevice), we take a really long time
  // and effectively halt the yields we have in the video sync hooks. This
  // has a consequence of making the sound system think there's more data
  // in our buffers than there really is and when things go back to normal
  // it freaks out and thinks we've been running too slow.  That causes an
  // error and emulation stops.  To guard against this, yield here too.
  circle_yield();

  if (file_name == NULL) {
    *len = 0;
    *isdir = 0;
    return -1;
  }

  // We can ignore these system files
  if (strcasecmp("./kernel7.img", file_name) == 0 ||
      strcasecmp("./kernel~1.img", file_name) == 0 ||
      strcasecmp("./kernel8-32.img", file_name) == 0 ||
      strcasecmp("./config.txt", file_name) == 0 ||
      strcasecmp("./cmdline.txt", file_name) == 0 ||
      strcasecmp("./bootstat.txt", file_name) == 0 ||
      strcasecmp("./bootcode.bin", file_name) == 0 ||
      strcasecmp("./start.elf", file_name) == 0) {
    return -1;
  }

  if (stat(file_name, &st) == 0) {
    *len = st.st_size;
    *isdir = st.st_mode & S_IFDIR;
    return 0;
  }
  return -1;
}

/* set permissions of given file to rw, respecting current umask */
int archdep_fix_permissions(const char *file_name) { return 0; }

int archdep_file_is_blockdev(const char *name) { return 0; }

int archdep_file_is_chardev(const char *name) { return 0; }

int archdep_rename(const char *oldpath, const char *newpath) {
  return rename(oldpath, newpath);
}

void archdep_shutdown(void) {
  if (default_path != NULL) {
    lib_free(default_path);
  }
}

char *archdep_extra_title_text(void) { return NULL; }

void archdep_vice_exit(int excode) {}

int archdep_vice_atexit(void (*function)(void)) { return 0; }

int kbd_arch_get_host_mapping(void) { return KBD_MAPPING_US; }
