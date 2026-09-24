#include "update_manifest.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "update_hash.h"

static int add_record(um_manifest *m, const um_record *r) {
  if (m->count == m->cap) {
    int cap = m->cap ? m->cap * 2 : 256;
    um_record *recs = realloc(m->recs, (size_t)cap * sizeof(*recs));
    if (recs == NULL) {
      return -1;
    }
    m->recs = recs;
    m->cap = cap;
  }
  m->recs[m->count++] = *r;
  return 0;
}

// Splits off the next space-separated word; returns NULL if there is none.
static char *next_word(char **p) {
  char *s = *p;
  while (*s == ' ') s++;
  if (*s == '\0') {
    return NULL;
  }
  char *word = s;
  while (*s != '\0' && *s != ' ') s++;
  if (*s == ' ') {
    *s++ = '\0';
  }
  *p = s;
  return word;
}

int um_parse(um_manifest *m, char *text, int primary, char *err,
             unsigned errlen) {
  if (m->ntexts == 2) {
    free(text);
    snprintf(err, errlen, "too many manifests");
    return -1;
  }
  m->texts[m->ntexts++] = text;

  int format = 0;
  char target[32] = "";
  int line_no = 0;
  char *line = text;
  while (line != NULL && *line != '\0') {
    char *end = strchr(line, '\n');
    if (end) {
      *end = '\0';
    }
    size_t len = strlen(line);
    if (len && line[len - 1] == '\r') {
      line[len - 1] = '\0';
    }
    line_no++;

    char *p = line;
    char *first = next_word(&p);
    if (first != NULL && first[0] != '#') {
      if (strcmp(first, "format") == 0) {
        char *v = next_word(&p);
        format = v ? atoi(v) : 0;
      } else if (strcmp(first, "target") == 0) {
        char *v = next_word(&p);
        if (v == NULL || strlen(v) >= sizeof(target)) {
          snprintf(err, errlen, "bad target line");
          return -1;
        }
        strcpy(target, v);
      } else {
        um_record r;
        memset(&r, 0, sizeof(r));
        char *hex = next_word(&p);
        char *size = next_word(&p);
        char *path = next_word(&p);
        if (hex == NULL || size == NULL || path == NULL ||
            uh_hex_to_digest(hex, r.sha) != 0) {
          snprintf(err, errlen, "bad manifest line %d", line_no);
          return -1;
        }
        r.version = first;
        r.path = path;
        r.size = (uint32_t)strtoul(size, NULL, 10);
        if (add_record(m, &r) != 0) {
          snprintf(err, errlen, "out of memory");
          return -1;
        }
      }
    }
    line = end ? end + 1 : NULL;
  }

  if (format != UM_FORMAT) {
    snprintf(err, errlen, "unsupported manifest format %d", format);
    return -1;
  }
  if (primary) {
    if (target[0] == '\0') {
      snprintf(err, errlen, "manifest has no target");
      return -1;
    }
    strcpy(m->target, target);
    int targets = 0;
    for (int i = 0; i < m->count; i++) {
      if (strcmp(m->recs[i].version, target) == 0) {
        m->recs[i].is_target = 1;
        targets++;
      }
    }
    if (targets == 0) {
      snprintf(err, errlen, "manifest lists no files for %s", target);
      return -1;
    }
  }
  return 0;
}

void um_free(um_manifest *m) {
  free(m->recs);
  for (int i = 0; i < m->ntexts; i++) {
    free(m->texts[i]);
  }
  memset(m, 0, sizeof(*m));
}

int um_known(const um_manifest *m, const char *path, const uint8_t sha[32]) {
  for (int i = 0; i < m->count; i++) {
    if (memcmp(m->recs[i].sha, sha, 32) == 0 &&
        strcasecmp(m->recs[i].path, path) == 0) {
      return 1;
    }
  }
  return 0;
}

int um_version_cmp(const char *a, const char *b) {
  if (*a == 'v' || *a == 'V') a++;
  if (*b == 'v' || *b == 'V') b++;
  while (*a || *b) {
    long x = strtol(a, (char **)&a, 10);
    long y = strtol(b, (char **)&b, 10);
    if (x != y) {
      return x < y ? -1 : 1;
    }
    // Skip to the next numeric part.
    while (*a && *a != '.') a++;
    while (*b && *b != '.') b++;
    if (*a == '.') a++;
    if (*b == '.') b++;
  }
  return 0;
}
