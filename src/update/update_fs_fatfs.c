// update_fs.h on FatFs. FatFs is built with FF_FS_REENTRANT, so these calls
// are safe from the emulator core while the web UI uses the card on core 0.
//
// FatFs is used directly rather than stdio: new_io.cpp reads a whole file
// into RAM on its first seek, which would load the ~29 MB update zip.

#include "update_fs.h"

#include <ff.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char *circle_get_disk_volume(void);

struct uf_file {
  FIL fil;
};

static int fat_path(const char *path, char *out, unsigned size) {
  int n = snprintf(out, size, "%s:%s", circle_get_disk_volume(), path);
  return (n > 0 && (unsigned)n < size) ? 0 : -1;
}

uf_file *uf_open(const char *path, int write) {
  char p[320];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return NULL;
  }
  uf_file *f = malloc(sizeof(*f));
  if (f == NULL) {
    return NULL;
  }
  BYTE mode = write ? (FA_WRITE | FA_CREATE_ALWAYS) : FA_READ;
  if (f_open(&f->fil, p, mode) != FR_OK) {
    free(f);
    return NULL;
  }
  return f;
}

int uf_read(uf_file *f, void *buf, unsigned len, unsigned *got) {
  UINT n = 0;
  FRESULT fr = f_read(&f->fil, buf, len, &n);
  *got = n;
  return fr == FR_OK ? 0 : -1;
}

int uf_write(uf_file *f, const void *buf, unsigned len) {
  UINT n = 0;
  return (f_write(&f->fil, buf, len, &n) == FR_OK && n == len) ? 0 : -1;
}

int uf_seek(uf_file *f, uint32_t pos) {
  return f_lseek(&f->fil, pos) == FR_OK ? 0 : -1;
}

uint32_t uf_size(uf_file *f) { return (uint32_t)f_size(&f->fil); }

int uf_close(uf_file *f) {
  FRESULT fr = f_close(&f->fil);
  free(f);
  return fr == FR_OK ? 0 : -1;
}

int uf_stat(const char *path, uint32_t *size, int *is_dir) {
  char p[320];
  FILINFO info;
  if (fat_path(path, p, sizeof(p)) != 0 || f_stat(p, &info) != FR_OK) {
    return -1;
  }
  if (size) {
    *size = (uint32_t)info.fsize;
  }
  if (is_dir) {
    *is_dir = (info.fattrib & AM_DIR) != 0;
  }
  return 0;
}

int uf_rename(const char *from, const char *to) {
  char a[320];
  char b[320];
  // f_rename moves between folders of the same volume (it ignores the
  // volume in the destination).
  if (fat_path(from, a, sizeof(a)) != 0 || fat_path(to, b, sizeof(b)) != 0) {
    return -1;
  }
  return f_rename(a, b) == FR_OK ? 0 : -1;
}

int uf_unlink(const char *path) {
  char p[320];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return -1;
  }
  return f_unlink(p) == FR_OK ? 0 : -1;
}

int uf_mkdir(const char *path) {
  char p[320];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return -1;
  }
  FRESULT fr = f_mkdir(p);
  return (fr == FR_OK || fr == FR_EXIST) ? 0 : -1;
}

int uf_list(const char *path, uf_dir_cb cb, void *ctx) {
  char p[320];
  DIR dir;
  FILINFO info;
  if (fat_path(path, p, sizeof(p)) != 0 || f_opendir(&dir, p) != FR_OK) {
    return -1;
  }
  int rc = 0;
  for (;;) {
    if (f_readdir(&dir, &info) != FR_OK) {
      rc = -1;
      break;
    }
    if (info.fname[0] == '\0') {
      break;
    }
    if (strcmp(info.fname, ".") == 0 || strcmp(info.fname, "..") == 0) {
      continue;
    }
    if (cb(ctx, info.fname, (info.fattrib & AM_DIR) != 0)) {
      break;
    }
  }
  f_closedir(&dir);
  return rc;
}

int uf_free_kb(uint32_t *kb) {
  char p[32];
  DWORD clusters = 0;
  FATFS *fs = NULL;
  if (fat_path("", p, sizeof(p)) != 0 || f_getfree(p, &clusters, &fs) != FR_OK) {
    return -1;
  }
#if FF_MAX_SS != FF_MIN_SS
  uint64_t sector = fs->ssize;
#else
  uint64_t sector = FF_MAX_SS;
#endif
  *kb = (uint32_t)(((uint64_t)clusters * fs->csize * sector) / 1024);
  return 0;
}
