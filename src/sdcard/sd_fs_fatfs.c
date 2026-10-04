// sd_fs.h on FatFs. FatFs is built with FF_FS_REENTRANT, so these calls
// are safe from the emulator core while the web UI uses the card on core 0.
//
// FatFs is used directly rather than stdio: new_io.cpp reads a whole file
// into RAM on its first seek, which would load the ~29 MB update zip, and
// it can't create or delete folders.

#include "sd_fs.h"

#include <ff.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern const char *circle_get_disk_volume(void);

// Room for the volume, a path and a little more (".part" etc.).
#define FAT_PATH_LEN 640

// Deepest folder sd_remove_tree goes into.
#define REMOVE_TREE_DEPTH 24

struct sd_file {
  FIL fil;
};

static int fat_path(const char *path, char *out, unsigned size) {
  int n = snprintf(out, size, "%s:%s", circle_get_disk_volume(), path);
  return (n > 0 && (unsigned)n < size) ? 0 : -1;
}

static int result(FRESULT fr) {
  switch (fr) {
  case FR_OK:
    return SD_OK;
  case FR_NO_FILE:
  case FR_NO_PATH:
    return SD_NOT_FOUND;
  case FR_EXIST:
    return SD_EXISTS;
  case FR_DENIED:
  case FR_WRITE_PROTECTED:
  case FR_LOCKED:
    return SD_DENIED;
  case FR_INVALID_NAME:
    return SD_INVALID;
  default:
    return SD_ERROR;
  }
}

static void to_info(const FILINFO *fi, sd_info *info) {
  snprintf(info->name, sizeof(info->name), "%s", fi->fname);
  info->size = (uint32_t)fi->fsize;
  info->is_dir = (fi->fattrib & AM_DIR) != 0;
  memset(&info->mtime, 0, sizeof(info->mtime));
  if (fi->fdate != 0) {
    info->mtime.year = (uint16_t)(((fi->fdate >> 9) & 0x7F) + 1980);
    info->mtime.month = (uint8_t)((fi->fdate >> 5) & 0x0F);
    info->mtime.day = (uint8_t)(fi->fdate & 0x1F);
    info->mtime.hour = (uint8_t)((fi->ftime >> 11) & 0x1F);
    info->mtime.minute = (uint8_t)((fi->ftime >> 5) & 0x3F);
    info->mtime.second = (uint8_t)((fi->ftime & 0x1F) * 2);
  }
}

sd_file *sd_open(const char *path, int mode) {
  char p[FAT_PATH_LEN];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return NULL;
  }
  sd_file *f = malloc(sizeof(*f));
  if (f == NULL) {
    return NULL;
  }
  BYTE fa = FA_READ;
  if (mode == SD_WRITE) {
    fa = FA_WRITE | FA_CREATE_ALWAYS;
  } else if (mode == SD_APPEND) {
    fa = FA_WRITE | FA_OPEN_APPEND;
  }
  if (f_open(&f->fil, p, fa) != FR_OK) {
    free(f);
    return NULL;
  }
  return f;
}

int sd_read(sd_file *f, void *buf, unsigned len, unsigned *got) {
  UINT n = 0;
  FRESULT fr = f_read(&f->fil, buf, len, &n);
  *got = n;
  return result(fr);
}

int sd_write(sd_file *f, const void *buf, unsigned len) {
  UINT n = 0;
  FRESULT fr = f_write(&f->fil, buf, len, &n);
  if (fr != FR_OK) {
    return result(fr);
  }
  return n == len ? SD_OK : SD_ERROR;  // a short write: the card is full
}

int sd_seek(sd_file *f, uint32_t pos) {
  return result(f_lseek(&f->fil, pos));
}

uint32_t sd_size(sd_file *f) { return (uint32_t)f_size(&f->fil); }

int sd_sync(sd_file *f) { return result(f_sync(&f->fil)); }

int sd_close(sd_file *f) {
  FRESULT fr = f_close(&f->fil);
  free(f);
  return result(fr);
}

int sd_stat_info(const char *path, sd_info *info) {
  char p[FAT_PATH_LEN];
  FILINFO fi;
  if (fat_path(path, p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  FRESULT fr = f_stat(p, &fi);
  if (fr == FR_OK && info != NULL) {
    to_info(&fi, info);
  }
  return result(fr);
}

int sd_rename(const char *from, const char *to) {
  char a[FAT_PATH_LEN];
  char b[FAT_PATH_LEN];
  // f_rename moves between folders of the same volume (it ignores the
  // volume in the destination).
  if (fat_path(from, a, sizeof(a)) != 0 || fat_path(to, b, sizeof(b)) != 0) {
    return SD_INVALID;
  }
  return result(f_rename(a, b));
}

int sd_unlink(const char *path) {
  char p[FAT_PATH_LEN];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  return result(f_unlink(p));
}

int sd_mkdir(const char *path) {
  char p[FAT_PATH_LEN];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  FRESULT fr = f_mkdir(p);
  return fr == FR_EXIST ? SD_OK : result(fr);
}

int sd_set_mtime(const char *path, const sd_time *mtime) {
  char p[FAT_PATH_LEN];
  if (fat_path(path, p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  // FAT holds 1980 to 2107, to the even second.
  if (mtime->year < 1980 || mtime->year > 2107 || mtime->month < 1 ||
      mtime->month > 12 || mtime->day < 1 || mtime->day > 31 ||
      mtime->hour > 23 || mtime->minute > 59 || mtime->second > 59) {
    return SD_INVALID;
  }
  FILINFO fi;
  fi.fdate = (WORD)(((mtime->year - 1980) << 9) | (mtime->month << 5) |
                    mtime->day);
  fi.ftime = (WORD)((mtime->hour << 11) | (mtime->minute << 5) |
                    (mtime->second / 2));
  return result(f_utime(p, &fi));
}

int sd_list_info(const char *path, sd_info_cb cb, void *ctx) {
  char p[FAT_PATH_LEN];
  DIR dir;
  FILINFO fi;
  if (fat_path(path, p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  FRESULT fr = f_opendir(&dir, p);
  if (fr != FR_OK) {
    return result(fr);
  }
  int rc = SD_OK;
  sd_info info;
  for (;;) {
    fr = f_readdir(&dir, &fi);
    if (fr != FR_OK) {
      rc = result(fr);
      break;
    }
    if (fi.fname[0] == '\0') {
      break;
    }
    if (strcmp(fi.fname, ".") == 0 || strcmp(fi.fname, "..") == 0) {
      continue;
    }
    to_info(&fi, &info);
    if (cb(ctx, &info)) {
      break;
    }
  }
  f_closedir(&dir);
  return rc;
}

// ---- sd_remove_tree ----
//
// FatFs has no recursive delete (f_unlink refuses a folder that isn't empty),
// so this is the usual walk: unlink each entry while reading the folder, then
// the folder itself. It edits one path in place and shares one FILINFO, so it
// needs little stack; the depth limit bounds the recursion.

typedef struct {
  FILINFO fi;
  unsigned removed;
  void (*progress)(void *ctx);
  void *ctx;
} remove_state;

static FRESULT remove_folder(char *path, unsigned path_size, unsigned depth,
                             remove_state *state) {
  if (depth > REMOVE_TREE_DEPTH) return FR_INVALID_NAME;
  DIR dir;
  FRESULT fr = f_opendir(&dir, path);
  if (fr != FR_OK) return fr;

  while (fr == FR_OK && f_readdir(&dir, &state->fi) == FR_OK &&
         state->fi.fname[0] != '\0') {
    // f_readdir doesn't return these, but following ".." would delete the
    // parent folder.
    if (strcmp(state->fi.fname, ".") == 0 || strcmp(state->fi.fname, "..") == 0) {
      continue;
    }
    unsigned length = (unsigned)strlen(path);
    unsigned name_length = (unsigned)strlen(state->fi.fname);
    if (length + 1 + name_length + 1 > path_size) {
      fr = FR_INVALID_NAME;
      break;
    }
    path[length] = '/';
    memcpy(path + length + 1, state->fi.fname, name_length + 1);
    if (state->fi.fattrib & AM_DIR) {
      fr = remove_folder(path, path_size, depth + 1, state);
    } else {
      fr = f_unlink(path);
      if (fr == FR_OK) state->removed++;
    }
    path[length] = '\0';
    if (state->progress != NULL && (state->removed & 15) == 0) {
      state->progress(state->ctx);
    }
  }
  f_closedir(&dir);
  if (fr != FR_OK) return fr;

  fr = f_unlink(path);  // the folder itself, now empty
  if (fr == FR_OK) state->removed++;
  return fr;
}

int sd_remove_tree(const char *path, unsigned *removed,
                   void (*progress)(void *ctx), void *ctx) {
  char p[FAT_PATH_LEN];
  remove_state state;
  state.removed = 0;
  state.progress = progress;
  state.ctx = ctx;
  FRESULT fr;
  if (fat_path(path, p, sizeof(p)) != 0) {
    fr = FR_INVALID_NAME;
  } else if ((fr = f_stat(p, &state.fi)) == FR_OK) {
    if (state.fi.fattrib & AM_DIR) {
      fr = remove_folder(p, sizeof(p), 0, &state);
    } else {
      fr = f_unlink(p);
      if (fr == FR_OK) state.removed++;
    }
  }
  if (removed != NULL) {
    *removed = state.removed;
  }
  return result(fr);
}

int sd_space_kb(uint32_t *total_kb, uint32_t *free_kb) {
  char p[32];
  DWORD clusters = 0;
  FATFS *fs = NULL;
  if (fat_path("", p, sizeof(p)) != 0) {
    return SD_INVALID;
  }
  FRESULT fr = f_getfree(p, &clusters, &fs);
  if (fr != FR_OK) {
    return result(fr);
  }
#if FF_MAX_SS != FF_MIN_SS
  uint64_t sector = fs->ssize;
#else
  uint64_t sector = FF_MAX_SS;
#endif
  uint64_t cluster_bytes = (uint64_t)fs->csize * sector;
  uint64_t total_clusters = fs->n_fatent > 2 ? fs->n_fatent - 2 : 0;
  if (total_kb != NULL) {
    *total_kb = (uint32_t)((total_clusters * cluster_bytes) / 1024);
  }
  if (free_kb != NULL) {
    *free_kb = (uint32_t)(((uint64_t)clusters * cluster_bytes) / 1024);
  }
  return SD_OK;
}
