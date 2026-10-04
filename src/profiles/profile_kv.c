#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// The key=value format shared by all profile files (docs/PROFILES.md, File
// reference): one entry per line, '#' starts a comment line, spaces around
// keys and values are ignored, and Windows line endings are accepted.

static int is_space(char c) {
   return c == ' ' || c == '\t' || c == '\r';
}

static char *trim(char *s) {
   while (is_space(*s)) s++;
   char *end = s + strlen(s);
   while (end > s && is_space(end[-1])) end--;
   *end = '\0';
   return s;
}

void pkv_parse(char *text, pkv_cb cb, void *ctx) {
   char *line = text;
   while (line != NULL && *line != '\0') {
      char *next = strchr(line, '\n');
      if (next != NULL) {
         *next++ = '\0';
      }
      line = trim(line);
      char *equals = strchr(line, '=');
      if (line[0] != '#' && equals != NULL) {
         *equals = '\0';
         char *key = trim(line);
         char *value = trim(equals + 1);
         if (key[0] != '\0') {
            cb(ctx, key, value);
         }
      }
      line = next;
   }
}

int pkv_append(char *buf, int size, int *len, const char *key,
               const char *value) {
   if (value == NULL || value[0] == '\0') {
      return 0;
   }
   int n = (int)(strlen(key) + 1 + strlen(value) + 1);
   if (n >= size - *len) {
      return -1;
   }
   char *out = buf + *len;
   out += sprintf(out, "%s=", key);
   for (const char *c = value; *c; c++) {
      // A value can't span lines.
      *out++ = (*c == '\n' || *c == '\r') ? ' ' : *c;
   }
   *out++ = '\n';
   *out = '\0';
   *len += n;
   return 0;
}

void pkv_copy(char *out, int out_size, const char *value) {
   snprintf(out, (size_t)out_size, "%s", value);
   // Values come from one line, but names typed in the menu could hold
   // anything; keep stored values on one line.
   for (char *c = out; *c; c++) {
      if (*c == '\n' || *c == '\r') *c = ' ';
   }
}
