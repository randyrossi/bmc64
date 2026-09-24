// Minimal zip reader for the updater: reads the central directory and streams
// one entry at a time (stored, or deflate through zlib), checking its CRC-32
// and size. ZIP64, encryption and multi-disk archives are refused.

#ifndef BMC64_UPDATE_ZIP_H
#define BMC64_UPDATE_ZIP_H

#include <stdint.h>

#include "update_fs.h"

// Receives extracted data. Returns 0 on success.
typedef int (*uz_out_fn)(void *ctx, const uint8_t *buf, unsigned len);

typedef struct {
  char *name;            // as stored, '/' separated; folders end with '/'
  uint16_t method;       // 0 stored, 8 deflate
  uint16_t flags;
  uint32_t crc;
  uint32_t csize;
  uint32_t usize;
  uint32_t local_offset;
  int is_dir;
} uz_entry;

typedef struct {
  uf_file *file;
  uint32_t size;
  int count;
  uz_entry *entries;
  char *names;           // storage for all entry names
} uz_zip;

// Returns 0 on success; on failure err holds a short reason.
int uz_open(uz_zip *z, const char *path, char *err, unsigned errlen);
void uz_close(uz_zip *z);

// Index of the entry with this name (case-insensitive), or -1.
int uz_find(const uz_zip *z, const char *name);

// Streams the entry's uncompressed bytes to out. Returns 0 only if the data
// decompressed cleanly and matched the entry's size and CRC-32.
int uz_extract(uz_zip *z, int index, uz_out_fn out, void *ctx);

#endif
