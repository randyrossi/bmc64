// The parts of sd_fs.h built only on its other functions, so they work the
// same on the Pi and in the PC tests.

#include "sd_fs.h"

#include <stddef.h>

int sd_stat(const char *path, uint32_t *size, int *is_dir) {
  sd_info info;
  int rc = sd_stat_info(path, &info);
  if (rc == SD_OK) {
    if (size) *size = info.size;
    if (is_dir) *is_dir = info.is_dir;
  }
  return rc;
}

typedef struct {
  sd_dir_cb cb;
  void *ctx;
} list_names;

static int list_name(void *ctx, const sd_info *info) {
  list_names *names = (list_names *)ctx;
  return names->cb(names->ctx, info->name, info->is_dir);
}

int sd_list(const char *path, sd_dir_cb cb, void *ctx) {
  list_names names = {cb, ctx};
  return sd_list_info(path, list_name, &names);
}

int sd_free_kb(uint32_t *kb) {
  return sd_space_kb(NULL, kb);
}

int sd_read_file(const char *path, char *buf, int size) {
  if (size <= 0) {
    return SD_INVALID;
  }
  sd_file *f = sd_open(path, SD_READ);
  if (f == NULL) {
    return SD_NOT_FOUND;
  }
  int result = SD_ERROR;
  unsigned got = 0;
  if (sd_size(f) < (uint32_t)size &&
      sd_read(f, buf, (unsigned)size - 1, &got) == SD_OK) {
    buf[got] = '\0';
    result = (int)got;
  }
  sd_close(f);
  return result;
}

int sd_write_file(const char *path, const char *data, int len) {
  if (len < 0) {
    return SD_INVALID;
  }
  sd_file *f = sd_open(path, SD_WRITE);
  if (f == NULL) {
    return SD_ERROR;
  }
  int write_rc = sd_write(f, data, (unsigned)len);
  int close_rc = sd_close(f);
  return write_rc != SD_OK ? write_rc : close_rc;
}
