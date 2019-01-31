/*
 * tuifs.c - A simple text-based file selector.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "types.h"

#include <conio.h>
#include <ctype.h>
#include <dirent.h>
#include <dos.h>
#include <errno.h>
#include <fnmatch.h>
#include <keys.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef HAVE_ALLEGRO_H
#include <allegro.h>
#endif

#include "imagecontents.h"
#include "ioutil.h"
#include "lib.h"
#include "util.h"
#include "tui.h"
#include "tui_backend.h"
#include "tuiimagebrowser.h"
#include "tuifs.h"
#include "tuiview.h"

/* ------------------------------------------------------------------------- */

enum file_type { FT_NORMAL, FT_DIR };

struct file_item {
    char name[0x100];
    enum file_type type;
};

struct file_list {
    int num_items;
    int num_used_items;
    struct file_item *items;
};

static struct file_list *file_list_create(void)
{
    struct file_list *new_list;

    new_list = lib_malloc(sizeof(struct file_list));
    new_list->num_items = new_list->num_used_items = 0;
    new_list->items = NULL;

    return new_list;
}

static void file_list_clear(struct file_list *fl)
{
    lib_free(fl->items);
    fl->items = NULL;
    fl->num_used_items = fl->num_items = 0;
}

static void file_list_free(struct file_list *fl)
{
    if (fl != NULL) {
        file_list_clear(fl);
        lib_free(fl);
    }
}

static void file_list_add_item(struct file_list *fl, const char *name, enum file_type type)
{
    if (fl->num_items == fl->num_used_items) {
        fl->num_items += 100;
        if (fl->items != NULL) {
            fl->items = lib_realloc(fl->items, fl->num_items * sizeof(*fl->items));
        } else {
            fl->items = lib_malloc(fl->num_items * sizeof(*fl->items));
        }
    }

    strcpy(fl->items[fl->num_used_items].name, name);
    fl->items[fl->num_used_items].type = type;
    fl->num_used_items++;
}

static int file_list_sort_func(const void *e1, const void *e2)
{
    struct file_item *f1 = (struct file_item *)e1;
    struct file_item *f2 = (struct file_item *)e2;

    /* Directories always come first. */
    if (f1->type != f2->type) {
        if (f1->type == FT_DIR) {
            return -1;
        }
        if (f2->type == FT_DIR) {
            return +1;
        }
    }
    return strcasecmp(f1->name, f2->name);
}

static void file_list_sort(struct file_list *fl)
{
    qsort(fl->items, fl->num_used_items, sizeof(struct file_item), file_list_sort_func);
}

/* XXX: Assumes `path' ends with a slash.  */
static struct file_list *file_list_read_lfn(const char *path, const char *pattern)
{
    struct dirent *d;
    struct file_list *fl;
    DIR *ds;
    int pathlen = strlen(path);

    if (path == NULL || *path == '\0') {
        ds = opendir(".");
    } else {
        ds = opendir(path);
    }

    if (ds == NULL) {
        return NULL;
    }

    fl = file_list_create();

    /* Skip ".".  */
    readdir(ds);

    {
        unsigned short old_djstat = _djstat_flags;

        /* This makes `stat()' faster.  FIXME: but it's still too slow
           imo...  */
        _djstat_flags = (_STAT_INODE | _STAT_EXEC_EXT | _STAT_EXEC_MAGIC | _STAT_DIRSIZE | _STAT_ROOT_TIME | _STAT_WRITEBIT);

        while ((d = readdir(ds)) != NULL) {
            struct stat s;
            int type;
            /* Warning: Assumes `path' has a trailing '/'.  */
            char *name = alloca(d->d_namlen + pathlen + 1);

            memcpy(name, path, pathlen);
            strcpy(name + pathlen, d->d_name);

            if (stat(name, &s) != -1) {
                type = S_ISDIR(s.st_mode) ? FT_DIR : FT_NORMAL;
                if (pattern == NULL || type == FT_DIR) {
                    file_list_add_item(fl, d->d_name, type);
                    continue;
                }
                {
                    char *p = lib_stralloc(pattern);
                    char *element;

                    element = strtok(p, ";");
                    do {
                        if (fnmatch(element, d->d_name, FNM_NOCASE) == 0) {
                            file_list_add_item(fl, d->d_name, type);
                        }
                        element = strtok(NULL, ";");
                    } while (element != NULL);
                    lib_free(p);
                }
            }
        }

        _djstat_flags = old_djstat;
    }

    file_list_sort(fl);
    closedir(ds);

    return fl;
}

static struct file_list *file_list_read_nolfn(const char *path, const char *pattern)
{
    char *cwd = ioutil_current_dir();
    struct file_list *fl = NULL;
    struct find_t f;

    if (cwd == NULL) {
        return NULL;
    }

    if (ioutil_chdir(path) < 0) {
        goto end;
    }

    if (_dos_findfirst("*.*", (_A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR | _A_ARCH), &f)) {
        goto end;
    }

    fl = file_list_create();

    /* (We skip `.' here.) */

    while (!_dos_findnext(&f)) {
        strlwr(f.name);
        if (pattern == NULL || (f.attrib & _A_SUBDIR)) {
            file_list_add_item(fl, f.name, (f.attrib & _A_SUBDIR) ? FT_DIR : FT_NORMAL);
            continue;
        }
        {
            char *p = lib_stralloc(pattern);
            char *element;

            element = strtok(p, ";");
            do {
                if (fnmatch(element, f.name, FNM_NOCASE) == 0) {
                    file_list_add_item(fl, f.name, (f.attrib & _A_SUBDIR) ? FT_DIR : FT_NORMAL);
                }
                element = strtok(NULL, ";");
            } while (element != NULL);
            lib_free(p);
        }
    }

    file_list_sort(fl);

end:
    ioutil_chdir(cwd);
    return fl;
}

static struct file_list *file_list_read(const char *path, const char *pattern)
{
    /* XXX: This check is only half-OK.  We actually need Allegro to be up
       and running for this to work properly.  */
#ifdef HAVE_ALLEGRO_H
    if ((os_type == OSTYPE_WIN95)
#ifdef OSTYPE_WIN98
        || (os_type == OSTYPE_WIN98)
#endif
#ifdef OSTYPE_WINME
        || (os_type == OSTYPE_WINME)
#endif
#ifdef OSTYPE_WIN2000
        || (os_type == OSTYPE_WIN2000)
#endif
#ifdef OSTYPE_WINXP
        || (os_type == OSTYPE_WINXP)
#endif
#ifdef OSTYPE_WIN2003
        || (os_type == OSTYPE_WIN2003)
#endif
#ifdef OSTYPE_WINVISTA
        || (os_type == OSTYPE_WINVISTA)
#endif
        || (os_type == OSTYPE_WINNT)) {
        return file_list_read_lfn(path, pattern);
    } else {
        return file_list_read_nolfn(path, pattern);
    }
#else
    return file_list_read_lfn(path, pattern);
#endif
}

static int file_list_find(const struct file_list *fl, const char *str, int len)
{
    int i;

    for (i = 0; i < fl->num_used_items; i++) {
        if (strncasecmp(fl->items[i].name, str, len) == 0) {
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------------- */

static void file_selector_display_path(const char *path, int x, int y, int width)
{
    int i, xx;

    tui_set_attr(MENU_BORDER, MENU_BACK, 0);
    tui_hline(x, y, 0xcd, width);

    tui_set_attr(MENU_FORE, MENU_BACK, 0);

    for (i = strlen(path) - 1, xx = MIN(x + width - 1, x + i); i >= 0 && xx >= x; i--, xx--) {
        char c;

        /* Display ellipsis on the left if longer than the line.  */
        if (xx <= x + 1 && i > 1) {
            c = '.';
        } else {
            c = path[i] == '/' ? '\\' : path[i];
        }
        tui_put_char(xx, y, c);
    }
}

static void file_selector_display_item(struct file_list *fl, int num, int first_item_num, int x, int y, int width, int height, int num_cols)
{
    y += (num - first_item_num) % height;
    x += ((num - first_item_num) / height) * width;

    if (num >= fl->num_used_items) {
        tui_hline(x, y, ' ', width);
    } else {
        int len = strlen(fl->items[num].name);

        /* XXX: Assumes `width' is > 5!  */
        if (len > width - 2) {
            char *name = alloca(width - 2 + 1);

            if (fl->items[num].type == FT_DIR) {
                memcpy(name, fl->items[num].name, width - 3);
                name[width - 4] = name[width - 5] = '.';
                name[width - 3] = '/';
            } else {
                memcpy(name, fl->items[num].name, width - 2);
                name[width - 3] = name[width - 4] = '.';
            }
            name[width - 2] = '\0';
            tui_display(x, y, width, " %s ", name);
        } else {
            if (fl->items[num].type == FT_DIR) {
                tui_display(x, y, width, " %s/ ", fl->items[num].name);
            } else {
              tui_display(x, y, width, " %s ", fl->items[num].name);
            }
        }
    }
}

static void file_selector_update(struct file_list *fl, int first_item_num, int x, int y, int width, int height, int num_cols)
{
    int i;

    for (i = 0; i < num_cols * height; i++) {
        file_selector_display_item(fl, first_item_num + i, first_item_num, x, y, width, height, num_cols);
    }
}

/* ------------------------------------------------------------------------- */

/* Convert an "alt + letter" key into a drive number, from 0 to 25.  Return
   0 if this is not an "alt + letter" keycode. */
static int alt_key_to_drive_num(int keycode)
{
    static int key_table[0x100] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 23, 5, 18, 20,
        25, 21, 9, 15, 16, 0, 0, 0, 0, 1, 19, 4, 6, 7, 8, 10, 11, 12, 0, 0,
        0, 0, 0, 26, 24, 3, 22, 2, 14, 13, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    if (keycode < 0x100 || keycode >= 0x200) {
        return 0;
    } else {
        return key_table[keycode - 0x100];
    }
}

/* Make sure there is a trailing '/' in `*path'.  */
static void slashize_path(char **path)
{
    int len = strlen(*path);

    if ((*path)[len - 1] != '/') {
        *path = lib_realloc(*path, len + 2);
        (*path)[len] = '/';
        (*path)[len + 1] = '\0';
    }
}

static char *change_path(struct file_list *fl, char *return_path, int curr_item)
{
    char *new_path;

    if (strcmp(fl->items[curr_item].name, "..") == 0) {
        char *p = return_path + strlen(return_path) - 1;

        if (*p == '/') {
            p--;
        }
        for (; *p != '/' && p > return_path; p--) {
        }

        if (p == return_path) {
            new_path = lib_stralloc(return_path);
        } else {
            new_path = lib_malloc(p - return_path + 2);
            memcpy(new_path, return_path, p - return_path + 1);
            new_path[p - return_path + 1] = '\0';
        }
    } else {
        new_path = util_concat(return_path, fl->items[curr_item].name, "/", NULL);
    }

    return new_path;
}

/* FIXME: documentation.  */
char *tui_file_selector(const char *title, const char *directory,
                        const char *pattern, const char *default_item,
                        read_contents_func_type contents_func,
                        char **browse_file_return,
                        unsigned int *browse_file_number_return)
{
    static char *return_path = NULL;
    struct file_list *fl;
    int curr_item, first_item, need_update;
    int x, y, width, height, num_cols, num_lines, field_width;
    int num_files;
    char str[0x100];
    int str_len = 0;
    tui_area_t backing_store = NULL;

    if (contents_func != NULL) {
        *browse_file_return = NULL;
    }

    if (browse_file_number_return != NULL) {
        *browse_file_number_return = 0;
    }

    if (directory != NULL) {
        return_path = lib_stralloc(directory);
    } else {
        return_path = ioutil_current_dir();
    }

    slashize_path(&return_path);

    fl = file_list_read(return_path, pattern);
    if (fl == NULL) {
        return NULL;
    }

    first_item = curr_item = 0;
    num_cols = 4;
    field_width = 18;
    num_lines = 17;
    height = num_lines + 2;
    width = field_width * num_cols + 4;
    num_files = num_cols * num_lines;

    if (default_item != NULL && *default_item) {
        int i;

        for (i = 0; i < fl->num_items; i++) {
            if (!strcasecmp(default_item, fl->items[i].name)) {
                curr_item = i;
                while (curr_item - first_item >= num_files) {
                    first_item += num_lines;
                }
                break;
            }
        }
    }

    x = CENTER_X(width);
    y = CENTER_Y(height);

    need_update = 1;

    tui_area_get(&backing_store, x, y, width + 2, height + 1);

    tui_display_window(x, y, width, height, MENU_BORDER, MENU_BACK, title, NULL);

    while (1) {
        int key;

        tui_set_attr(MENU_FORE, MENU_BACK, 0);
        if (need_update) {
            file_selector_display_path(return_path, x + 1, y + height - 1, width - 2);
            file_selector_update(fl, first_item, x + 2, y + 1, field_width, num_lines, num_cols);
            tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);
            tui_display(0, tui_num_lines() - 1, tui_num_cols(), "\030\031\033\032: Move  <Enter>: Select  %s<Alt>-<letter>: Change drive", contents_func != NULL ? "<Space>: Preview  " : "");
            need_update = 0;
        }
        tui_set_attr(MENU_FORE, MENU_HIGHLIGHT, 0);
        file_selector_display_item(fl, curr_item, first_item, x + 2, y + 1, field_width, num_lines, num_cols);
        key = getkey();
        tui_set_attr(MENU_FORE, MENU_BACK, 0);
        file_selector_display_item(fl, curr_item, first_item, x + 2, y + 1, field_width, num_lines, num_cols);

        switch (key) {
            case K_Escape:
                tui_area_put(backing_store, x, y);
                tui_area_free(backing_store);
                return NULL;
            case K_Left:
                str_len = 0;
                if (curr_item - num_lines >= 0) {
                    curr_item -= num_lines;
                    if (curr_item < first_item) {
                        if (first_item >= num_lines) {
                            first_item -= num_lines;
                            need_update = 1;
                        } else {
                            curr_item += num_lines;
                        }
                    }
                }
                break;
            case K_Up:
                str_len = 0;
                if (curr_item > 0) {
                    curr_item--;
                    if (curr_item < first_item) {
                        first_item = curr_item;
                        need_update = 1;
                    }
                }
                break;
            case K_Right:
                str_len = 0;
                if (curr_item + num_lines < fl->num_used_items) {
                    curr_item += num_lines;
                    if (curr_item - first_item >= num_files) {
                        first_item += num_lines;
                        need_update = 1;
                    }
                }
                break;
            case K_Down:
                str_len = 0;
                if (curr_item < fl->num_used_items - 1) {
                    curr_item++;
                    if (curr_item == first_item + num_files) {
                        first_item++;
                        need_update = 1;
                    }
                }
                break;
            case K_PageDown:
                str_len = 0;
                if (curr_item + num_files < fl->num_used_items) {
                    curr_item += num_files;
                    first_item += num_files;
                }
                need_update = 1;
                break;
            case K_PageUp:
                str_len = 0;
                if (curr_item - num_files >= 0) {
                    curr_item -= num_files;
                    first_item -= num_files;
                    if (first_item < 0) {
                        first_item = 0;
                    }
                    need_update = 1;
                }
                break;
            case K_Home:
                str_len = 0;
                curr_item = 0;
                if (first_item != 0) {
                    first_item = 0;
                    need_update = 1;
                }
                break;
            case K_End:
                str_len = 0;
                curr_item = fl->num_used_items - 1;
                first_item = curr_item - num_files + 1;
                if (first_item < 0) {
                    first_item = 0;
                }
                need_update = 1;
                break;
            case K_Return:
                str_len = 0;
                if (fl->items[curr_item].type == FT_DIR) {
                    struct file_list *new_fl;
                    char *new_path;

                    new_path = change_path(fl, return_path, curr_item);

                    new_fl = file_list_read(new_path, pattern);

                    if (new_fl != NULL) {
                        file_list_free(fl);
                        fl = new_fl;
                        first_item = curr_item = 0;
                        lib_free(return_path);
                        return_path = new_path;
                        need_update = 1;
                        ioutil_chdir(return_path);
                    } else {
                        lib_free(new_path);
                    }
                } else {
                    char *p = util_concat(return_path, fl->items[curr_item].name, NULL);

                    lib_free(return_path);
                    return_path = p;
                    tui_area_put(backing_store, x, y);
                    tui_area_free(backing_store);
                    return return_path;
                }
                break;
            case K_BackSpace:
                if (str_len > 1) {
                    int n;
                    str_len--;
                    n = file_list_find(fl, str, str_len);
                    if (n >= 0) {
                        curr_item = n;
                        if (curr_item < first_item) {
                            first_item = curr_item;
                            need_update = 1;
                        } else if (first_item + num_files <= curr_item) {
                            first_item = curr_item - num_files + 1;
                            need_update = 1;
                        }
                    }
                } else {
                    str_len = 0;
                    curr_item = 0;
                    if (first_item != 0) {
                        first_item = 0;
                        need_update = 1;
                    }
                }
                break;
            case ' ':
                if (contents_func != NULL && fl->items[curr_item].type != FT_DIR && browse_file_return != NULL) {
                    tui_display(0, tui_num_lines() - 1, tui_num_cols(), "");
                    *browse_file_return = tui_image_browser(fl->items[curr_item].name, contents_func, browse_file_number_return);
                    if (*browse_file_return != NULL) {
                        char *p = util_concat(return_path, fl->items[curr_item].name, NULL);

                        lib_free(return_path);
                        return_path = p;
                        tui_area_put(backing_store, x, y);
                        tui_area_free(backing_store);
                        return return_path;
                    }
                    need_update = 1;
                    break;
                } else {
                    tui_beep();
                }
            default:
                {
                    int drive_num;

                    drive_num = alt_key_to_drive_num(key);

                    if (drive_num > 0) {
                        /* `A-a' ... `A-z' change the current drive.  */
                        int num_available_drives;
                        int current_drive;

                        _dos_getdrive(&current_drive);
                        _dos_setdrive(current_drive, &num_available_drives);
                        if (drive_num <= num_available_drives) {
                            char *new_path;

                            /* FIXME: This is a hack...  Maybe there is a cleaner
                               way to do it, but for now I just don't know.  */
                            _dos_setdrive(drive_num, &num_available_drives);
                            new_path = ioutil_current_dir();
                            if (new_path != NULL) {
                                slashize_path(&new_path);
                                _dos_setdrive(current_drive, &num_available_drives);
                                if (new_path != NULL) {
                                    struct file_list *new_fl;

                                    new_fl = file_list_read(new_path, pattern);
                                    if (new_fl != NULL) {
                                        file_list_free(fl);
                                        fl = new_fl;
                                        first_item = curr_item = 0;
                                        lib_free(return_path);
                                        return_path = new_path;
                                        need_update = 1;
                                        ioutil_chdir(return_path);
                                    } else {
                                        lib_free(new_path);
                                    }
                                }
                            } else {
                                _dos_setdrive(current_drive, &num_available_drives);
                                tui_beep();
                            }
                        } else {
                            tui_beep();
                        }
                    } else if (isprint(key) && str_len < 0x100) {
                        int n;

                        str[str_len] = key;
                        n = file_list_find(fl, str, str_len + 1);
                        if (n < 0) {
                            tui_beep();
                        } else {
                            str_len++;
                            curr_item = n;
                            if (curr_item < first_item) {
                                first_item = curr_item;
                                need_update = 1;
                            } else if (first_item + num_files <= curr_item) {
                                first_item = curr_item - num_files + 1;
                                need_update = 1;
                            }
                        }
                    }
                }
                break;
        }
    }
}
