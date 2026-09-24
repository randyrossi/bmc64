#include "update_plan.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "update_fs.h"
#include "update_hash.h"

#define HASH_CHUNK 16384

int up_hash_file(const char *path, uint8_t sha[32]) {
  uf_file *f = uf_open(path, 0);
  if (f == NULL) {
    return -1;
  }
  uint8_t *buf = malloc(HASH_CHUNK);
  if (buf == NULL) {
    uf_close(f);
    return -1;
  }
  uh_sha256 s;
  uh_sha256_init(&s);
  int rc = 0;
  for (;;) {
    unsigned got = 0;
    if (uf_read(f, buf, HASH_CHUNK, &got) != 0) {
      rc = -1;
      break;
    }
    if (got == 0) {
      break;
    }
    uh_sha256_update(&s, buf, got);
  }
  free(buf);
  uf_close(f);
  if (rc == 0) {
    uh_sha256_final(&s, sha);
  }
  return rc;
}

int up_path_ok(const char *path) {
  if (path[0] == '\0' || path[0] == '/') {
    return 0;
  }
  const char *seg = path;
  for (const char *p = path;; p++) {
    char c = *p;
    if (c == '/' || c == '\0') {
      size_t len = (size_t)(p - seg);
      if (len == 0 && c == '/') {
        return 0; // empty segment
      }
      if ((len == 1 && seg[0] == '.') ||
          (len == 2 && seg[0] == '.' && seg[1] == '.')) {
        return 0;
      }
      if (c == '\0') {
        break;
      }
      seg = p + 1;
      continue;
    }
    if (c < 0x21 || c > 0x7e || strchr("\\:*?\"<>|", c)) {
      return 0;
    }
  }
  return strlen(path) < 200;
}

static int is_kernel(const char *path) {
  return strchr(path, '/') == NULL && strncasecmp(path, "kernel", 6) == 0 &&
         strstr(path, ".img") != NULL;
}

static int is_board_kernel(const char *path, const char *board) {
  size_t n = strlen(board);
  return strncasecmp(path, board, n) == 0 &&
         (path[n] == '\0' || path[n] == '.');
}

static int is_pi_firmware(const char *path) {
  return strcasecmp(path, "bootcode.bin") == 0 ||
         strcasecmp(path, "start.elf") == 0 ||
         strcasecmp(path, "fixup.dat") == 0;
}

static int is_wifi_firmware(const char *path) {
  return strncasecmp(path, "firmware/", 9) == 0;
}

static int is_boot_config(const char *path) {
  return strcasecmp(path, "config.txt") == 0 ||
         strcasecmp(path, "cmdline.txt") == 0;
}

// Order of the moves when committing: least critical first, what the Pi
// needs to boot last.
static int commit_order(const up_item *it) {
  const char *path = it->rec->path;
  if (it->group == UP_GROUP_KERNEL) {
    return it->this_board ? 3 : 2;
  }
  if (is_wifi_firmware(path)) {
    return 1;
  }
  if (strcasecmp(path, "bootcode.bin") == 0) {
    return 5;
  }
  if (is_pi_firmware(path)) {
    return 4;
  }
  if (is_boot_config(path)) {
    return 6;
  }
  return 0;
}

static int compare_items(const void *a, const void *b) {
  const up_item *x = a, *y = b;
  if (x->group != y->group) {
    return x->group - y->group;
  }
  return strcasecmp(x->rec->path, y->rec->path);
}

struct grow {
  char *buf;
  unsigned len;
  unsigned cap;
};

static int grow_out(void *ctx, const uint8_t *data, unsigned len) {
  struct grow *g = ctx;
  if (g->len + len + 1 > g->cap) {
    unsigned cap = (g->len + len + 1) * 2;
    char *buf = realloc(g->buf, cap);
    if (buf == NULL) {
      return -1;
    }
    g->buf = buf;
    g->cap = cap;
  }
  memcpy(g->buf + g->len, data, len);
  g->len += len;
  g->buf[g->len] = '\0';
  return 0;
}

// Loads the zip's manifest, then the card's copy of the installed one (which
// lets a downgrade recognise the newer release's files).
// Reads the card's /bmc64-manifest.txt into m. Returns 0 on success.
static int load_card_manifest(um_manifest *m, int primary, char *err,
                              unsigned errlen) {
  uf_file *f = uf_open("/" UM_NAME, 0);
  if (f == NULL) {
    snprintf(err, errlen, "The card has no " UM_NAME ".");
    return -1;
  }
  uint32_t size = uf_size(f);
  char *text = malloc(size + 1);
  unsigned got = 0;
  if (text == NULL || uf_read(f, text, size, &got) != 0 || got != size) {
    free(text);
    uf_close(f);
    snprintf(err, errlen, "Can't read the card's " UM_NAME ".");
    return -1;
  }
  uf_close(f);
  text[size] = '\0';
  return um_parse(m, text, primary, err, errlen);
}

struct hash_ctx {
  uh_sha256 sha;
};

static int hash_out(void *ctx, const uint8_t *buf, unsigned len) {
  uh_sha256_update(&((struct hash_ctx *)ctx)->sha, buf, len);
  return 0;
}

// 1 if the zip holds exactly the files the manifest lists for version, at the
// listed sizes.
static int zip_matches_version(const uz_zip *z, const um_manifest *m,
                               const char *version) {
  int files = 0;
  for (int i = 0; i < m->count; i++) {
    const um_record *r = &m->recs[i];
    if (strcmp(r->version, version) != 0) {
      continue;
    }
    int zi = uz_find(z, r->path);
    if (zi < 0 || z->entries[zi].is_dir || z->entries[zi].usize != r->size) {
      return 0;
    }
    files++;
  }
  int in_zip = 0;
  for (int i = 0; i < z->count; i++) {
    in_zip += !z->entries[i].is_dir;
  }
  return files > 0 && files == in_zip;
}

// A zip without a manifest: find the release it is in the card's history. The
// sizes pick the candidates; a kernel's SHA-256 confirms one. (Every file
// that is installed is checked against its SHA-256 again when unpacked.)
static int identify_release(uz_zip *z, um_manifest *m, char *err,
                            unsigned errlen) {
  const char *found = NULL;
  for (int i = 0; i < m->count && found == NULL; i++) {
    const char *version = m->recs[i].version;
    int seen = 0;
    for (int j = 0; j < i && !seen; j++) {
      seen = strcmp(m->recs[j].version, version) == 0;
    }
    if (seen || !zip_matches_version(z, m, version)) {
      continue;
    }
    // Confirm with the contents of one file, a kernel if there is one:
    // kernels differ between every release.
    const um_record *check = NULL;
    for (int j = 0; j < m->count; j++) {
      const um_record *r = &m->recs[j];
      if (strcmp(r->version, version) != 0) {
        continue;
      }
      if (check == NULL) {
        check = r;
      }
      if (strncasecmp(r->path, "kernel", 6) == 0) {
        check = r;
        break;
      }
    }
    struct hash_ctx h;
    uh_sha256_init(&h.sha);
    uint8_t sha[32];
    if (check && uz_extract(z, uz_find(z, check->path), hash_out, &h) == 0) {
      uh_sha256_final(&h.sha, sha);
      if (memcmp(sha, check->sha, 32) == 0) {
        found = version;
      }
    }
  }
  if (found == NULL) {
    snprintf(err, errlen,
             "It has no " UM_NAME " and doesn't match any BMC64 release this "
             "card knows.");
    return -1;
  }
  snprintf(m->target, sizeof(m->target), "%s", found);
  for (int i = 0; i < m->count; i++) {
    m->recs[i].is_target = strcmp(m->recs[i].version, found) == 0;
  }
  m->legacy = 1;
  return 0;
}

int up_load_manifests(uz_zip *z, um_manifest *m, char *err, unsigned errlen) {
  int index = uz_find(z, UM_NAME);
  if (index < 0) {
    // A release from before the updater: the card's manifest lists it.
    char why[80];
    if (load_card_manifest(m, 0, why, sizeof(why)) != 0) {
      snprintf(err, errlen,
               "It has no " UM_NAME ", so it can only be recognised from the "
               "card's copy, and that failed: %s", why);
      return -1;
    }
    return identify_release(z, m, err, errlen);
  }
  struct grow g = {0};
  if (uz_extract(z, index, grow_out, &g) != 0 || g.buf == NULL) {
    free(g.buf);
    snprintf(err, errlen, "Its " UM_NAME " is damaged.");
    return -1;
  }
  if (um_parse(m, g.buf, 1, err, errlen) != 0) {
    return -1;
  }
  // The card's copy only adds history; a missing or damaged one just means
  // less of it.
  char ignore[80];
  load_card_manifest(m, 0, ignore, sizeof(ignore));
  return 0;
}

int up_build(up_plan *p, uz_zip *z, const um_manifest *m, const char *running,
             const char *board_kernel, up_progress_fn progress, void *ctx,
             char *err, unsigned errlen) {
  memset(p, 0, sizeof(*p));
  snprintf(p->running, sizeof(p->running), "%s", running);
  snprintf(p->target, sizeof(p->target), "%s", m->target);
  p->legacy = m->legacy;
  p->direction = um_version_cmp(running, m->target) < 0 ? -1
                 : um_version_cmp(running, m->target) > 0 ? 1 : 0;

  int targets = 0;
  for (int i = 0; i < m->count; i++) {
    targets += m->recs[i].is_target;
  }
  p->items = calloc((size_t)targets, sizeof(up_item));
  p->dirs = calloc((size_t)(z->count ? z->count : 1), sizeof(char *));
  if (p->items == NULL || p->dirs == NULL) {
    up_free(p);
    snprintf(err, errlen, "out of memory");
    return -1;
  }

  // Folders in the zip (disks/C64, ...) are created if missing.
  for (int i = 0; i < z->count; i++) {
    if (z->entries[i].is_dir) {
      char *name = z->entries[i].name;
      name[strlen(name) - 1] = '\0'; // drop the trailing '/'
      if (!up_path_ok(name)) {
        up_free(p);
        snprintf(err, errlen, "unsafe folder name in zip: %.40s", name);
        return -1;
      }
      p->dirs[p->ndirs++] = name;
    }
  }

  int done = 0;
  for (int i = 0; i < m->count; i++) {
    const um_record *rec = &m->recs[i];
    if (!rec->is_target) {
      continue;
    }
    if (!up_path_ok(rec->path)) {
      up_free(p);
      snprintf(err, errlen, "unsafe file name: %.40s", rec->path);
      return -1;
    }
    // The zip must hold every file the manifest lists, at the listed size.
    int zi = uz_find(z, rec->path);
    if (zi < 0 || z->entries[zi].is_dir ||
        z->entries[zi].usize != rec->size) {
      up_free(p);
      snprintf(err, errlen, "package incomplete: %.40s", rec->path);
      return -1;
    }

    if (progress) {
      progress(ctx, rec->path, done, targets);
    }
    done++;

    up_item it;
    memset(&it, 0, sizeof(it));
    it.rec = rec;
    it.zip_index = zi;
    it.group = is_kernel(rec->path) ? UP_GROUP_KERNEL
               : (is_pi_firmware(rec->path) || is_wifi_firmware(rec->path))
                   ? UP_GROUP_FIRMWARE
                   : UP_GROUP_CONFIG;
    it.this_board =
        it.group == UP_GROUP_KERNEL && is_board_kernel(rec->path, board_kernel);

    char card[256];
    snprintf(card, sizeof(card), "/%s", rec->path);
    uint32_t size = 0;
    int is_dir = 0;
    if (uf_stat(card, &size, &is_dir) != 0) {
      it.status = UP_NEW;
    } else if (is_dir) {
      up_free(p);
      snprintf(err, errlen, "a folder is in the way: %.40s", rec->path);
      return -1;
    } else if (it.group == UP_GROUP_KERNEL && size != rec->size) {
      // Kernels are never edited by hand: a different size is enough.
      it.status = UP_UPDATE;
    } else {
      uint8_t sha[32];
      if (up_hash_file(card, sha) != 0) {
        up_free(p);
        snprintf(err, errlen, "cannot read %.40s", rec->path);
        return -1;
      }
      if (memcmp(sha, rec->sha, 32) == 0) {
        p->up_to_date++;
        continue;
      }
      it.status = (it.group == UP_GROUP_KERNEL || um_known(m, rec->path, sha))
                      ? UP_UPDATE
                      : UP_CHANGED;
    }
    it.replace = it.status != UP_CHANGED;
    it.order = commit_order(&it);
    p->items[p->count++] = it;
  }

  qsort(p->items, (size_t)p->count, sizeof(up_item), compare_items);
  return 0;
}

void up_free(up_plan *p) {
  free(p->items);
  free(p->dirs);
  memset(p, 0, sizeof(*p));
}
