// Whole-file helpers for sd_fs.h, built only on its other functions so they
// work the same on the Pi and in the PC tests.

#include "sd_fs.h"

#include <stddef.h>

int sd_read_file(const char *path, char *buf, int size) {
  if (size <= 0) {
    return -1;
  }
  sd_file *f = sd_open(path, 0);
  if (f == NULL) {
    return -1;
  }
  int result = -1;
  unsigned got = 0;
  if (sd_size(f) < (uint32_t)size &&
      sd_read(f, buf, (unsigned)size - 1, &got) == 0) {
    buf[got] = '\0';
    result = (int)got;
  }
  sd_close(f);
  return result;
}

int sd_write_file(const char *path, const char *data, int len) {
  if (len < 0) {
    return -1;
  }
  sd_file *f = sd_open(path, 1);
  if (f == NULL) {
    return -1;
  }
  int write_rc = sd_write(f, data, (unsigned)len);
  int close_rc = sd_close(f);
  return (write_rc == 0 && close_rc == 0) ? 0 : -1;
}
