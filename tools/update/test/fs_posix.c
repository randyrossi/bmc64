// update_fs.h on a PC folder, for the host tests. The "card" is the folder
// in UPDATE_TEST_ROOT. Setting UPDATE_TEST_CRASH_AFTER=N makes the Nth
// rename exit the process first, to simulate a power cut mid-update.

#include "update_fs.h"

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

struct uf_file {
  FILE *f;
  int write;
};

static void full(const char *path, char *out, size_t size) {
  const char *root = getenv("UPDATE_TEST_ROOT");
  snprintf(out, size, "%s%s", root ? root : ".", path);
}

uf_file *uf_open(const char *path, int write) {
  char p[1024];
  full(path, p, sizeof(p));
  FILE *f = fopen(p, write ? "wb" : "rb");
  if (f == NULL) {
    return NULL;
  }
  uf_file *u = malloc(sizeof(*u));
  u->f = f;
  u->write = write;
  return u;
}

int uf_read(uf_file *f, void *buf, unsigned len, unsigned *got) {
  *got = (unsigned)fread(buf, 1, len, f->f);
  return ferror(f->f) ? -1 : 0;
}

int uf_write(uf_file *f, const void *buf, unsigned len) {
  return fwrite(buf, 1, len, f->f) == len ? 0 : -1;
}

int uf_seek(uf_file *f, uint32_t pos) { return fseek(f->f, (long)pos, SEEK_SET); }

uint32_t uf_size(uf_file *f) {
  long here = ftell(f->f);
  fseek(f->f, 0, SEEK_END);
  long size = ftell(f->f);
  fseek(f->f, here, SEEK_SET);
  return (uint32_t)size;
}

int uf_close(uf_file *f) {
  int rc = fclose(f->f);
  free(f);
  return rc == 0 ? 0 : -1;
}

int uf_stat(const char *path, uint32_t *size, int *is_dir) {
  char p[1024];
  struct stat st;
  full(path, p, sizeof(p));
  if (stat(p, &st) != 0) {
    return -1;
  }
  if (size) *size = (uint32_t)st.st_size;
  if (is_dir) *is_dir = S_ISDIR(st.st_mode);
  return 0;
}

int uf_rename(const char *from, const char *to) {
  static int count;
  const char *crash = getenv("UPDATE_TEST_CRASH_AFTER");
  if (crash && ++count > atoi(crash)) {
    fflush(NULL);
    _exit(3); // power cut
  }
  char a[1024], b[1024];
  struct stat st;
  full(from, a, sizeof(a));
  full(to, b, sizeof(b));
  if (stat(b, &st) == 0) {
    return -1; // like FatFs: the destination must not exist
  }
  return rename(a, b) == 0 ? 0 : -1;
}

int uf_unlink(const char *path) {
  char p[1024];
  struct stat st;
  full(path, p, sizeof(p));
  if (stat(p, &st) != 0) {
    return -1;
  }
  return (S_ISDIR(st.st_mode) ? rmdir(p) : unlink(p)) == 0 ? 0 : -1;
}

int uf_mkdir(const char *path) {
  char p[1024];
  full(path, p, sizeof(p));
  return (mkdir(p, 0777) == 0 || errno == EEXIST) ? 0 : -1;
}

int uf_list(const char *path, uf_dir_cb cb, void *ctx) {
  char p[1024];
  full(path, p, sizeof(p));
  DIR *d = opendir(p);
  if (d == NULL) {
    return -1;
  }
  struct dirent *e;
  while ((e = readdir(d)) != NULL) {
    if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) {
      continue;
    }
    char child[1300];
    struct stat st;
    snprintf(child, sizeof(child), "%s/%s", p, e->d_name);
    int is_dir = stat(child, &st) == 0 && S_ISDIR(st.st_mode);
    if (cb(ctx, e->d_name, is_dir)) {
      break;
    }
  }
  closedir(d);
  return 0;
}

int uf_free_kb(uint32_t *kb) {
  const char *fake = getenv("UPDATE_TEST_FREE_KB");
  if (fake) {
    *kb = (uint32_t)atol(fake);
    return 0;
  }
  char p[1024];
  struct statvfs st;
  full("/", p, sizeof(p));
  if (statvfs(p, &st) != 0) {
    return -1;
  }
  unsigned long long kb64 = (unsigned long long)st.f_bavail * st.f_frsize / 1024;
  *kb = kb64 > 0xffffffffu ? 0xffffffffu : (uint32_t)kb64;
  return 0;
}
