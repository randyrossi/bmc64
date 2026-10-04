// sd_fs.h on a PC folder, for the host tests (updater and profiles). The
// "card" is the folder in SD_TEST_ROOT. Setting SD_TEST_CRASH_AFTER=N makes
// the Nth rename exit the process first, to simulate a power cut, and
// SD_TEST_FREE_KB fakes the free space. Behaves like FatFs where it matters
// (a rename never replaces an existing file; sd_mkdir accepts an existing
// folder).

#include "../../src/sdcard/sd_fs.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <utime.h>

#define REMOVE_TREE_DEPTH 24

struct sd_file {
  FILE *f;
};

static void full(const char *path, char *out, size_t size) {
  const char *root = getenv("SD_TEST_ROOT");
  snprintf(out, size, "%s%s", root ? root : ".", path);
}

static int from_errno(void) {
  switch (errno) {
  case ENOENT:
  case ENOTDIR:
    return SD_NOT_FOUND;
  case EEXIST:
    return SD_EXISTS;
  case ENOTEMPTY:
  case EACCES:
  case EPERM:
  case EBUSY:
  case EROFS:
    return SD_DENIED;
  case ENAMETOOLONG:
  case EINVAL:
    return SD_INVALID;
  default:
    return SD_ERROR;
  }
}

static void to_info(const char *name, const struct stat *st, sd_info *info) {
  snprintf(info->name, sizeof(info->name), "%s", name);
  info->size = (uint32_t)st->st_size;
  info->is_dir = S_ISDIR(st->st_mode);
  struct tm tm;
  memset(&info->mtime, 0, sizeof(info->mtime));
  if (localtime_r(&st->st_mtime, &tm) != NULL) {
    info->mtime.year = (uint16_t)(tm.tm_year + 1900);
    info->mtime.month = (uint8_t)(tm.tm_mon + 1);
    info->mtime.day = (uint8_t)tm.tm_mday;
    info->mtime.hour = (uint8_t)tm.tm_hour;
    info->mtime.minute = (uint8_t)tm.tm_min;
    info->mtime.second = (uint8_t)tm.tm_sec;
  }
}

sd_file *sd_open(const char *path, int mode) {
  char p[1024];
  full(path, p, sizeof(p));
  const char *how = mode == SD_WRITE ? "wb" : mode == SD_APPEND ? "ab" : "rb";
  FILE *f = fopen(p, how);
  if (f == NULL) {
    return NULL;
  }
  sd_file *u = malloc(sizeof(*u));
  u->f = f;
  return u;
}

int sd_read(sd_file *f, void *buf, unsigned len, unsigned *got) {
  *got = (unsigned)fread(buf, 1, len, f->f);
  return ferror(f->f) ? SD_ERROR : SD_OK;
}

int sd_write(sd_file *f, const void *buf, unsigned len) {
  return fwrite(buf, 1, len, f->f) == len ? SD_OK : SD_ERROR;
}

int sd_seek(sd_file *f, uint32_t pos) {
  return fseek(f->f, (long)pos, SEEK_SET) == 0 ? SD_OK : SD_ERROR;
}

uint32_t sd_size(sd_file *f) {
  long here = ftell(f->f);
  fseek(f->f, 0, SEEK_END);
  long size = ftell(f->f);
  fseek(f->f, here, SEEK_SET);
  return (uint32_t)size;
}

int sd_sync(sd_file *f) {
  return fflush(f->f) == 0 ? SD_OK : SD_ERROR;
}

int sd_close(sd_file *f) {
  int rc = fclose(f->f);
  free(f);
  return rc == 0 ? SD_OK : SD_ERROR;
}

int sd_stat_info(const char *path, sd_info *info) {
  char p[1024];
  struct stat st;
  full(path, p, sizeof(p));
  if (stat(p, &st) != 0) {
    return from_errno();
  }
  if (info != NULL) {
    const char *slash = strrchr(path, '/');
    to_info(slash ? slash + 1 : path, &st, info);
  }
  return SD_OK;
}

int sd_rename(const char *from, const char *to) {
  static int count;
  const char *crash = getenv("SD_TEST_CRASH_AFTER");
  if (crash && ++count > atoi(crash)) {
    fflush(NULL);
    _exit(3); // power cut
  }
  char a[1024], b[1024];
  struct stat st;
  full(from, a, sizeof(a));
  full(to, b, sizeof(b));
  if (stat(b, &st) == 0) {
    return SD_EXISTS; // like FatFs: the destination must not exist
  }
  return rename(a, b) == 0 ? SD_OK : from_errno();
}

int sd_unlink(const char *path) {
  char p[1024];
  struct stat st;
  full(path, p, sizeof(p));
  if (stat(p, &st) != 0) {
    return from_errno();
  }
  return (S_ISDIR(st.st_mode) ? rmdir(p) : unlink(p)) == 0 ? SD_OK
                                                           : from_errno();
}

int sd_mkdir(const char *path) {
  char p[1024];
  full(path, p, sizeof(p));
  return (mkdir(p, 0777) == 0 || errno == EEXIST) ? SD_OK : from_errno();
}

int sd_set_mtime(const char *path, const sd_time *mtime) {
  char p[1024];
  full(path, p, sizeof(p));
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_year = mtime->year - 1900;
  tm.tm_mon = mtime->month - 1;
  tm.tm_mday = mtime->day;
  tm.tm_hour = mtime->hour;
  tm.tm_min = mtime->minute;
  tm.tm_sec = mtime->second;
  tm.tm_isdst = -1;
  struct utimbuf times;
  times.actime = times.modtime = mktime(&tm);
  return utime(p, &times) == 0 ? SD_OK : from_errno();
}

int sd_list_info(const char *path, sd_info_cb cb, void *ctx) {
  char p[1024];
  full(path, p, sizeof(p));
  DIR *d = opendir(p);
  if (d == NULL) {
    return from_errno();
  }
  struct dirent *e;
  sd_info info;
  while ((e = readdir(d)) != NULL) {
    if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
      continue;
    }
    char child[1300];
    struct stat st;
    snprintf(child, sizeof(child), "%s/%s", p, e->d_name);
    if (stat(child, &st) != 0) {
      continue;
    }
    to_info(e->d_name, &st, &info);
    if (cb(ctx, &info)) {
      break;
    }
  }
  closedir(d);
  return SD_OK;
}

typedef struct {
  unsigned removed;
  void (*progress)(void *ctx);
  void *ctx;
} remove_state;

static int remove_tree(const char *p, unsigned depth, remove_state *state) {
  struct stat st;
  if (depth > REMOVE_TREE_DEPTH) return SD_INVALID;
  if (stat(p, &st) != 0) return from_errno();
  if (S_ISDIR(st.st_mode)) {
    DIR *d = opendir(p);
    if (d == NULL) return from_errno();
    struct dirent *e;
    int rc = SD_OK;
    while (rc == SD_OK && (e = readdir(d)) != NULL) {
      if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
        continue;
      }
      char child[1300];
      snprintf(child, sizeof(child), "%s/%s", p, e->d_name);
      rc = remove_tree(child, depth + 1, state);
      if (state->progress != NULL && (state->removed & 15) == 0) {
        state->progress(state->ctx);
      }
    }
    closedir(d);
    if (rc != SD_OK) return rc;
    if (rmdir(p) != 0) return from_errno();
  } else if (unlink(p) != 0) {
    return from_errno();
  }
  state->removed++;
  return SD_OK;
}

int sd_remove_tree(const char *path, unsigned *removed,
                   void (*progress)(void *ctx), void *ctx) {
  char p[1024];
  full(path, p, sizeof(p));
  remove_state state = {0, progress, ctx};
  int rc = remove_tree(p, 0, &state);
  if (removed != NULL) {
    *removed = state.removed;
  }
  return rc;
}

int sd_space_kb(uint32_t *total_kb, uint32_t *free_kb) {
  char p[1024];
  struct statvfs st;
  full("/", p, sizeof(p));
  if (statvfs(p, &st) != 0) {
    return SD_ERROR;
  }
  unsigned long long total = (unsigned long long)st.f_blocks * st.f_frsize / 1024;
  unsigned long long avail = (unsigned long long)st.f_bavail * st.f_frsize / 1024;
  const char *fake = getenv("SD_TEST_FREE_KB");
  if (fake) {
    avail = (unsigned long long)atol(fake);
  }
  if (total_kb) *total_kb = total > 0xffffffffu ? 0xffffffffu : (uint32_t)total;
  if (free_kb) *free_kb = avail > 0xffffffffu ? 0xffffffffu : (uint32_t)avail;
  return SD_OK;
}
