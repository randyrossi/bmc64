#include "update_zip.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <zlib.h>

#define SIG_EOCD 0x06054b50u
#define SIG_CENTRAL 0x02014b50u
#define SIG_LOCAL 0x04034b50u
#define EOCD_SIZE 22
#define EOCD_SEARCH (EOCD_SIZE + 65535)

static uint16_t rd16(const uint8_t *p) { return (uint16_t)(p[0] | p[1] << 8); }

static uint32_t rd32(const uint8_t *p) {
  return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
         (uint32_t)p[3] << 24;
}

static int read_at(uf_file *f, uint32_t pos, void *buf, unsigned len) {
  unsigned got = 0;
  if (uf_seek(f, pos) != 0 || uf_read(f, buf, len, &got) != 0 || got != len) {
    return -1;
  }
  return 0;
}

static int fail(char *err, unsigned errlen, const char *msg) {
  snprintf(err, errlen, "%s", msg);
  return -1;
}

int uz_open(uz_zip *z, const char *path, char *err, unsigned errlen) {
  memset(z, 0, sizeof(*z));
  z->file = uf_open(path, 0);
  if (z->file == NULL) {
    return fail(err, errlen, "cannot open the zip");
  }
  z->size = uf_size(z->file);
  if (z->size < EOCD_SIZE) {
    uz_close(z);
    return fail(err, errlen, "not a zip file");
  }

  // Find the end-of-central-directory record, which may be followed by a
  // comment of up to 64 KB.
  uint32_t search = z->size < EOCD_SEARCH ? z->size : EOCD_SEARCH;
  uint8_t *tail = malloc(search);
  if (tail == NULL || read_at(z->file, z->size - search, tail, search) != 0) {
    free(tail);
    uz_close(z);
    return fail(err, errlen, "cannot read the zip");
  }
  int eocd = -1;
  for (int i = (int)search - EOCD_SIZE; i >= 0; i--) {
    if (rd32(tail + i) == SIG_EOCD) {
      eocd = i;
      break;
    }
  }
  if (eocd < 0) {
    free(tail);
    uz_close(z);
    return fail(err, errlen, "not a zip file");
  }
  const uint8_t *e = tail + eocd;
  uint16_t disk = rd16(e + 4), cd_disk = rd16(e + 6);
  uint16_t entries_here = rd16(e + 8), entries = rd16(e + 10);
  uint32_t cd_size = rd32(e + 12), cd_offset = rd32(e + 16);
  free(tail);
  if (disk != 0 || cd_disk != 0 || entries_here != entries) {
    uz_close(z);
    return fail(err, errlen, "multi-part zips are not supported");
  }
  if (entries == 0xffff || cd_size == 0xffffffffu || cd_offset == 0xffffffffu) {
    uz_close(z);
    return fail(err, errlen, "ZIP64 is not supported");
  }
  if (cd_offset > z->size || cd_size > z->size - cd_offset) {
    uz_close(z);
    return fail(err, errlen, "damaged zip (directory)");
  }

  uint8_t *cd = malloc(cd_size ? cd_size : 1);
  z->entries = calloc(entries ? entries : 1, sizeof(uz_entry));
  z->names = malloc(cd_size + entries + 1);
  if (cd == NULL || z->entries == NULL || z->names == NULL ||
      read_at(z->file, cd_offset, cd, cd_size) != 0) {
    free(cd);
    uz_close(z);
    return fail(err, errlen, "cannot read the zip directory");
  }

  uint32_t pos = 0;
  char *names = z->names;
  for (int i = 0; i < entries; i++) {
    if (pos + 46 > cd_size || rd32(cd + pos) != SIG_CENTRAL) {
      free(cd);
      uz_close(z);
      return fail(err, errlen, "damaged zip (entry)");
    }
    const uint8_t *c = cd + pos;
    uint16_t nlen = rd16(c + 28), xlen = rd16(c + 30), clen = rd16(c + 32);
    if (pos + 46 + nlen + xlen + clen > cd_size) {
      free(cd);
      uz_close(z);
      return fail(err, errlen, "damaged zip (entry)");
    }
    uz_entry *ent = &z->entries[i];
    ent->flags = rd16(c + 8);
    ent->method = rd16(c + 10);
    ent->crc = rd32(c + 16);
    ent->csize = rd32(c + 20);
    ent->usize = rd32(c + 24);
    ent->local_offset = rd32(c + 42);
    memcpy(names, c + 46, nlen);
    names[nlen] = '\0';
    ent->name = names;
    ent->is_dir = nlen > 0 && names[nlen - 1] == '/';
    names += nlen + 1;
    pos += 46 + nlen + xlen + clen;
  }
  free(cd);
  z->count = entries;
  return 0;
}

void uz_close(uz_zip *z) {
  if (z->file) {
    uf_close(z->file);
  }
  free(z->entries);
  free(z->names);
  memset(z, 0, sizeof(*z));
}

int uz_find(const uz_zip *z, const char *name) {
  for (int i = 0; i < z->count; i++) {
    if (strcasecmp(z->entries[i].name, name) == 0) {
      return i;
    }
  }
  return -1;
}

#define IN_CHUNK 16384
#define OUT_CHUNK 32768

struct reader {
  uf_file *file;
  uint32_t left;
};

// Reads up to max bytes of the entry's compressed data; 0 at its end.
static int read_input(struct reader *r, uint8_t *buf, unsigned max) {
  if (r->left == 0) {
    return 0;
  }
  unsigned want = max < r->left ? max : r->left;
  unsigned got = 0;
  if (uf_read(r->file, buf, want, &got) != 0 || got == 0) {
    return -1;
  }
  r->left -= got;
  return (int)got;
}

struct checker {
  uz_out_fn out;
  void *ctx;
  uLong crc;
  uint32_t size;
};

static int check_output(struct checker *c, const uint8_t *buf, unsigned len) {
  c->crc = crc32(c->crc, buf, len);
  c->size += len;
  return c->out(c->ctx, buf, len);
}

// zlib is built with Z_SOLO, so it needs these.
static voidpf z_alloc(voidpf opaque, uInt items, uInt size) {
  (void)opaque;
  return calloc(items, size);
}

static void z_free(voidpf opaque, voidpf address) {
  (void)opaque;
  free(address);
}

static int copy_stored(struct reader *r, struct checker *c, uint8_t *in) {
  for (;;) {
    int n = read_input(r, in, IN_CHUNK);
    if (n <= 0) {
      return n;
    }
    if (check_output(c, in, (unsigned)n) != 0) {
      return -1;
    }
  }
}

// Raw DEFLATE, as zip stores it (no zlib header: negative window bits).
static int inflate_entry(struct reader *r, struct checker *c, uint8_t *in,
                         uint8_t *out) {
  z_stream zs;
  memset(&zs, 0, sizeof(zs));
  zs.zalloc = z_alloc;
  zs.zfree = z_free;
  if (inflateInit2(&zs, -MAX_WBITS) != Z_OK) {
    return -1;
  }
  int ret = Z_OK;
  while (ret != Z_STREAM_END) {
    if (zs.avail_in == 0) {
      int n = read_input(r, in, IN_CHUNK);
      if (n <= 0) {
        break; // read error, or the data ended before the stream did
      }
      zs.next_in = in;
      zs.avail_in = (uInt)n;
    }
    zs.next_out = out;
    zs.avail_out = OUT_CHUNK;
    ret = inflate(&zs, Z_NO_FLUSH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      break;
    }
    unsigned have = OUT_CHUNK - zs.avail_out;
    if (have && check_output(c, out, have) != 0) {
      ret = Z_ERRNO;
      break;
    }
  }
  inflateEnd(&zs);
  return ret == Z_STREAM_END ? 0 : -1;
}

int uz_extract(uz_zip *z, int index, uz_out_fn out, void *ctx) {
  if (index < 0 || index >= z->count) {
    return -1;
  }
  const uz_entry *ent = &z->entries[index];
  if (ent->flags & 1) {
    return -1; // encrypted
  }
  if (ent->method != 0 && ent->method != 8) {
    return -1;
  }
  uint8_t local[30];
  if (read_at(z->file, ent->local_offset, local, sizeof(local)) != 0 ||
      rd32(local) != SIG_LOCAL) {
    return -1;
  }
  uint32_t data = ent->local_offset + 30 + rd16(local + 26) + rd16(local + 28);
  if (data > z->size || ent->csize > z->size - data ||
      uf_seek(z->file, data) != 0) {
    return -1;
  }

  uint8_t *in = malloc(IN_CHUNK);
  uint8_t *outbuf = ent->method == 8 ? malloc(OUT_CHUNK) : NULL;
  if (in == NULL || (ent->method == 8 && outbuf == NULL)) {
    free(in);
    free(outbuf);
    return -1;
  }
  struct reader r = {z->file, ent->csize};
  struct checker c = {out, ctx, crc32(0L, Z_NULL, 0), 0};
  int rc = ent->method == 0 ? copy_stored(&r, &c, in)
                            : inflate_entry(&r, &c, in, outbuf);
  free(in);
  free(outbuf);
  if (rc != 0 || c.size != ent->usize || c.crc != ent->crc) {
    return -1;
  }
  return 0;
}
