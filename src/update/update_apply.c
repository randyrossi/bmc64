#include "update_apply.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "update_fs.h"
#include "update_hash.h"
#include "update_manifest.h"

#define JOURNAL_HEADER "bmc64-update-journal 1"
#define PATH_MAX_LEN 256

// ---- small file helpers ----

struct names {
  char **list;
  int *is_dir;
  int count;
  int cap;
};

static int collect(void *ctx, const char *name, int is_dir) {
  struct names *n = ctx;
  if (n->count == n->cap) {
    int cap = n->cap ? n->cap * 2 : 16;
    char **list = realloc(n->list, (size_t)cap * sizeof(char *));
    int *dirs = realloc(n->is_dir, (size_t)cap * sizeof(int));
    if (list) n->list = list;
    if (dirs) n->is_dir = dirs;
    if (list == NULL || dirs == NULL) {
      return 1;
    }
    n->cap = cap;
  }
  n->list[n->count] = strdup(name);
  n->is_dir[n->count] = is_dir;
  if (n->list[n->count] == NULL) {
    return 1;
  }
  n->count++;
  return 0;
}

static void free_names(struct names *n) {
  for (int i = 0; i < n->count; i++) {
    free(n->list[i]);
  }
  free(n->list);
  free(n->is_dir);
}

int ua_rmtree(const char *path) {
  int is_dir = 0;
  if (uf_stat(path, NULL, &is_dir) != 0) {
    return 0;
  }
  if (is_dir) {
    // Collect the names first; deleting while a folder is open for listing
    // is not safe.
    struct names n = {0};
    uf_list(path, collect, &n);
    for (int i = 0; i < n.count; i++) {
      char child[PATH_MAX_LEN];
      snprintf(child, sizeof(child), "%s/%s", path, n.list[i]);
      ua_rmtree(child);
    }
    free_names(&n);
  }
  return uf_unlink(path);
}

// Creates every folder along path (a folder path, not a file).
static int mkdirs(const char *path) {
  char buf[PATH_MAX_LEN];
  snprintf(buf, sizeof(buf), "%s", path);
  for (char *p = buf + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      if (uf_mkdir(buf) != 0) {
        return -1;
      }
      *p = '/';
    }
  }
  return uf_mkdir(buf);
}

// Creates the folders a file at path needs.
static int mkdirs_for_file(const char *path) {
  char buf[PATH_MAX_LEN];
  snprintf(buf, sizeof(buf), "%s", path);
  char *slash = strrchr(buf, '/');
  if (slash == NULL || slash == buf) {
    return 0;
  }
  *slash = '\0';
  return mkdirs(buf);
}

static int write_text(const char *path, const char *text) {
  uf_file *f = uf_open(path, 1);
  if (f == NULL) {
    return -1;
  }
  int rc = uf_write(f, text, (unsigned)strlen(text));
  if (uf_close(f) != 0) {
    rc = -1;
  }
  return rc;
}

// Reads a whole (small) file into a NUL-terminated buffer.
static char *read_text(const char *path) {
  uf_file *f = uf_open(path, 0);
  if (f == NULL) {
    return NULL;
  }
  uint32_t size = uf_size(f);
  char *text = malloc(size + 1);
  unsigned got = 0;
  if (text == NULL || uf_read(f, text, size, &got) != 0 || got != size) {
    free(text);
    uf_close(f);
    return NULL;
  }
  text[size] = '\0';
  uf_close(f);
  return text;
}

static int exists(const char *path) { return uf_stat(path, NULL, NULL) == 0; }

void ua_backup_dir(const char *running, char *out, unsigned size) {
  char version[32];
  const char *v = (running[0] == 'v' || running[0] == 'V') ? running + 1 : running;
  unsigned i = 0;
  for (; v[i] && i < sizeof(version) - 1; i++) {
    char c = v[i];
    int ok = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z') || c == '.' || c == '-' || c == '_';
    version[i] = ok ? c : '_';
  }
  version[i] = '\0';
  snprintf(out, size, "%s/v%s", UA_BACKUP, version);
}

// ---- growing text buffer ----

struct text {
  char *buf;
  size_t len;
  size_t cap;
  int failed;
};

static void text_add(struct text *t, const char *fmt, ...) {
  if (t->failed) {
    return;
  }
  va_list ap;
  for (;;) {
    size_t room = t->cap - t->len;
    va_start(ap, fmt);
    int n = t->buf ? vsnprintf(t->buf + t->len, room, fmt, ap) : -1;
    va_end(ap);
    if (n >= 0 && (size_t)n < room) {
      t->len += (size_t)n;
      return;
    }
    size_t cap = t->cap ? t->cap * 2 : 4096;
    if (n >= 0 && cap < t->len + (size_t)n + 1) {
      cap = t->len + (size_t)n + 1;
    }
    char *buf = realloc(t->buf, cap);
    if (buf == NULL) {
      t->failed = 1;
      return;
    }
    t->buf = buf;
    t->cap = cap;
  }
}

// ---- journal ----

// Runs every step of the journal that is not done yet. A step "move A B" is
// done when B exists and, for files coming out of the staging folder, A is
// gone. Returns the number of steps that could not be completed.
static int run_journal(char *text) {
  int errors = 0;
  char *line = text;
  while (line && *line) {
    char *end = strchr(line, '\n');
    if (end) {
      *end = '\0';
    }
    if (strncmp(line, "move ", 5) == 0) {
      char *from = line + 5;
      char *to = strchr(from, ' ');
      if (to) {
        *to++ = '\0';
        int staged = strncmp(from, UA_TMP "/", strlen(UA_TMP) + 1) == 0;
        int have_from = exists(from);
        int have_to = exists(to);
        if (have_to && (!staged || !have_from)) {
          // Already done.
        } else if (have_from && !have_to) {
          if (uf_rename(from, to) != 0) {
            printf("update: cannot move %s to %s\n", from, to);
            errors++;
          }
        } else {
          printf("update: cannot finish moving %s to %s\n", from, to);
          errors++;
        }
      }
    } else if (strncmp(line, "mkdir ", 6) == 0) {
      mkdirs(line + 6);
    }
    line = end ? end + 1 : NULL;
  }
  return errors;
}

static int journal_complete(const char *text) {
  size_t n = strlen(JOURNAL_HEADER);
  size_t len = strlen(text);
  return strncmp(text, JOURNAL_HEADER, n) == 0 && len >= 4 &&
         strcmp(text + len - 4, "end\n") == 0;
}

static void finish(void) {
  uf_unlink(UA_ZIP);
  uf_unlink(UA_JOURNAL);
  ua_rmtree(UA_TMP);
}

int ua_resume(void) {
  if (!exists(UA_JOURNAL)) {
    return 0;
  }
  char *text = read_text(UA_JOURNAL);
  if (text == NULL || !journal_complete(text)) {
    // The journal was never finished, so nothing was moved yet. The zip is
    // still there and the update view will offer it again.
    free(text);
    printf("update: discarding an unfinished update\n");
    ua_rmtree(UA_TMP);
    return 0;
  }
  printf("update: finishing an interrupted update\n");
  int errors = run_journal(text);
  free(text);
  finish();
  printf("update: finished, %d problem(s)\n", errors);
  return 1;
}

// ---- apply ----

struct extract_ctx {
  uf_file *out;
  uh_sha256 sha;
};

static int write_out(void *ctx, const uint8_t *buf, unsigned len) {
  struct extract_ctx *e = ctx;
  uh_sha256_update(&e->sha, buf, len);
  return uf_write(e->out, buf, len);
}

// Extracts one zip entry to path. If sha is given, the data must match it.
static int extract_to(uz_zip *z, int index, const char *path,
                      const uint8_t *sha) {
  if (mkdirs_for_file(path) != 0) {
    return -1;
  }
  struct extract_ctx e;
  e.out = uf_open(path, 1);
  if (e.out == NULL) {
    return -1;
  }
  uh_sha256_init(&e.sha);
  int rc = uz_extract(z, index, write_out, &e);
  if (uf_close(e.out) != 0) {
    rc = -1;
  }
  if (rc == 0 && sha) {
    uint8_t got[32];
    uh_sha256_final(&e.sha, got);
    if (memcmp(got, sha, 32) != 0) {
      rc = -1;
    }
  }
  return rc;
}

static int compare_order(const void *a, const void *b) {
  const up_item *x = *(const up_item *const *)a;
  const up_item *y = *(const up_item *const *)b;
  if (x->order != y->order) {
    return x->order - y->order;
  }
  return strcmp(x->rec->path, y->rec->path);
}

struct prune_ctx {
  const char *keep;   // backup folder name to keep, e.g. "v5.1.10"
  struct names found;
};

static int find_old_backups(void *ctx, const char *name, int is_dir) {
  struct prune_ctx *p = ctx;
  if (is_dir && name[0] == 'v' && strcmp(name, p->keep) != 0) {
    return collect(&p->found, name, is_dir);
  }
  return 0;
}

static void write_report(const up_plan *p, const char *bdir, const char *kdir,
                         struct text *t) {
  static const char *const titles[] = {"Replaced", "Added", "Kept (not replaced)"};
  text_add(t, "BMC64 update report\n\nFrom: v%s\nTo:   %s\n\n",
           p->running[0] == 'v' ? p->running + 1 : p->running, p->target);
  text_add(t, "Original files are saved in %s (kernels in %s).\n", bdir, kdir);
  if (p->legacy) {
    text_add(t, "%s was released before the updater, so the updater can't be "
                "used again. To update, copy a newer release to the card by "
                "hand.\n", p->target);
  }
  for (int section = 0; section < 3; section++) {
    int any = 0;
    for (int i = 0; i < p->count; i++) {
      const up_item *it = &p->items[i];
      int s = !it->replace ? 2 : it->status == UP_NEW ? 1 : 0;
      if (s != section) {
        continue;
      }
      if (!any) {
        text_add(t, "\n%s:\n", titles[section]);
        any = 1;
      }
      text_add(t, "  %s%s\n", it->rec->path,
               it->status == UP_CHANGED ? "  (changed by you)" : "");
    }
  }
}

static void write_restore(const up_plan *p, const char *bdir, const char *kdir,
                          struct text *t) {
  const char *old = p->running[0] == 'v' ? p->running + 1 : p->running;
  text_add(t,
           "How to go back to BMC64 v%s by hand\n"
           "\n"
           "If %s does not work for you, put the SD card in a computer and:\n"
           "\n"
           "1. Copy everything in %s to the top of the SD card, keeping its\n"
           "   folders (C64/, firmware/, ...) and replacing the files there.\n"
           "   Skip RESTORE.txt, update-report.txt and " UA_MARKER ".\n"
           "2. Copy everything in %s to the top of the SD card.\n"
           "\n"
           "Files that were new in %s stay on the card; they do no harm.\n",
           old, p->target, bdir + 1, kdir + 1, p->target);
}

int ua_apply(const up_plan *p, uz_zip *z, ua_progress_fn progress, void *ctx,
             char *err, unsigned errlen) {
  char bdir[64];
  char kdir[80];
  char path[PATH_MAX_LEN];
  char dest[PATH_MAX_LEN];
  ua_backup_dir(p->running, bdir, sizeof(bdir));
  snprintf(kdir, sizeof(kdir), "%s/kernel%s", UA_BACKUP, bdir + strlen(UA_BACKUP));

  // A release from before the updater has no manifest; the card keeps its
  // own copy (the updater is not used again after such a release).
  int manifest_index = uz_find(z, UM_NAME);
  if (manifest_index < 0 && !p->legacy) {
    snprintf(err, errlen, "no " UM_NAME " in the zip");
    return -1;
  }

  // The chosen files, in the order they will be moved into place.
  const up_item **chosen = calloc((size_t)(p->count ? p->count : 1), sizeof(*chosen));
  if (chosen == NULL) {
    snprintf(err, errlen, "out of memory");
    return -1;
  }
  int nchosen = 0;
  uint64_t bytes = manifest_index < 0 ? 0 : z->entries[manifest_index].usize;
  for (int i = 0; i < p->count; i++) {
    if (p->items[i].replace) {
      chosen[nchosen++] = &p->items[i];
      bytes += p->items[i].rec->size;
    }
  }
  qsort(chosen, (size_t)nchosen, sizeof(*chosen), compare_order);

  // 1. Room for the new files. Backups are moves and need no space.
  uint32_t free_kb = 0;
  if (uf_free_kb(&free_kb) == 0 && (uint64_t)free_kb * 1024 < bytes + 1024 * 1024) {
    snprintf(err, errlen, "not enough free space: need %lu KB",
             (unsigned long)(bytes / 1024 + 1024));
    free(chosen);
    return -1;
  }

  // 2. Extract and verify everything into the staging folder. Nothing on the
  //    card has changed until the journal is written.
  ua_rmtree(UA_TMP);
  if (uf_mkdir(UA_TMP) != 0) {
    snprintf(err, errlen, "cannot create " UA_TMP);
    free(chosen);
    return -1;
  }
  for (int i = 0; i < nchosen; i++) {
    const up_item *it = chosen[i];
    if (progress) {
      progress(ctx, "Extracting", it->rec->path, i, nchosen);
    }
    snprintf(path, sizeof(path), "%s/%s", UA_TMP, it->rec->path);
    if (extract_to(z, it->zip_index, path, it->rec->sha) != 0) {
      snprintf(err, errlen, "%.40s is damaged in the zip", it->rec->path);
      ua_rmtree(UA_TMP);
      free(chosen);
      return -1;
    }
  }
  if (manifest_index >= 0 &&
      extract_to(z, manifest_index, UA_TMP "/" UM_NAME, NULL) != 0) {
    snprintf(err, errlen, UM_NAME " is damaged in the zip");
    ua_rmtree(UA_TMP);
    free(chosen);
    return -1;
  }

  struct text report = {0};
  struct text restore = {0};
  write_report(p, bdir, kdir, &report);
  write_restore(p, bdir, kdir, &restore);
  int wrote = !report.failed && !restore.failed &&
              write_text(UA_TMP "/update-report.txt", report.buf) == 0 &&
              write_text(UA_TMP "/RESTORE.txt", restore.buf) == 0 &&
              write_text(UA_TMP "/" UA_MARKER, "") == 0;
  free(report.buf);
  free(restore.buf);
  if (!wrote) {
    snprintf(err, errlen, "cannot write to the SD card");
    ua_rmtree(UA_TMP);
    free(chosen);
    return -1;
  }

  // 3. Make room for the backup. Only one non-kernel backup is kept; only
  //    folders the updater made (they hold the marker) are ever deleted.
  if (progress) {
    progress(ctx, "Preparing", "", 0, 1);
  }
  struct prune_ctx prune = {bdir + strlen(UA_BACKUP) + 1, {0}};
  uf_list(UA_BACKUP, find_old_backups, &prune);
  for (int i = 0; i < prune.found.count; i++) {
    char marker[PATH_MAX_LEN + 16];
    snprintf(path, sizeof(path), "%s/%s", UA_BACKUP, prune.found.list[i]);
    snprintf(marker, sizeof(marker), "%s/" UA_MARKER, path);
    if (exists(marker)) {
      ua_rmtree(path);
    }
  }
  free_names(&prune.found);
  if (exists(bdir)) {
    char marker[PATH_MAX_LEN];
    snprintf(marker, sizeof(marker), "%s/" UA_MARKER, bdir);
    if (!exists(marker)) {
      snprintf(err, errlen, "%s exists and was not made by BMC64", bdir);
      ua_rmtree(UA_TMP);
      free(chosen);
      return -1;
    }
    ua_rmtree(bdir); // same version again (a repair): replace its backup
  }
  if (mkdirs(bdir) != 0 || mkdirs(kdir) != 0) {
    snprintf(err, errlen, "cannot create %s", bdir);
    ua_rmtree(UA_TMP);
    free(chosen);
    return -1;
  }

  // 4. Write the journal: every move, in order.
  struct text j = {0};
  text_add(&j, JOURNAL_HEADER "\n");
  for (int i = 0; i < nchosen; i++) {
    const up_item *it = chosen[i];
    snprintf(path, sizeof(path), "/%s", it->rec->path);
    if (exists(path)) {
      if (it->group == UP_GROUP_KERNEL) {
        snprintf(dest, sizeof(dest), "%s/%s", kdir, it->rec->path);
        uf_unlink(dest); // an older backup of this same version's kernel
      } else {
        snprintf(dest, sizeof(dest), "%s/%s", bdir, it->rec->path);
        mkdirs_for_file(dest);
      }
      text_add(&j, "move %s %s\n", path, dest);
    } else {
      mkdirs_for_file(path);
    }
    text_add(&j, "move %s/%s %s\n", UA_TMP, it->rec->path, path);
  }
  if (manifest_index >= 0) {
    if (exists("/" UM_NAME)) {
      text_add(&j, "move /" UM_NAME " %s/" UM_NAME "\n", bdir);
    }
    text_add(&j, "move " UA_TMP "/" UM_NAME " /" UM_NAME "\n");
  }
  text_add(&j, "move " UA_TMP "/update-report.txt %s/update-report.txt\n", bdir);
  text_add(&j, "move " UA_TMP "/RESTORE.txt %s/RESTORE.txt\n", bdir);
  text_add(&j, "move " UA_TMP "/" UA_MARKER " %s/" UA_MARKER "\n", bdir);
  for (int i = 0; i < p->ndirs; i++) {
    text_add(&j, "mkdir /%s\n", p->dirs[i]);
  }
  text_add(&j, "end\n");
  free(chosen);
  if (j.failed || write_text(UA_JOURNAL, j.buf) != 0) {
    free(j.buf);
    snprintf(err, errlen, "cannot write the journal");
    ua_rmtree(UA_TMP);
    return -1;
  }

  // 5. Commit. From here an interruption is finished by ua_resume() on the
  //    next boot. The zip is closed first: it is deleted at the end.
  uz_close(z);
  if (progress) {
    progress(ctx, "Installing", "", 0, 1);
  }
  int errors = run_journal(j.buf);
  free(j.buf);
  finish();
  if (errors) {
    snprintf(err, errlen, "%d file(s) could not be moved; see bmc64.log", errors);
  }
  return errors;
}
