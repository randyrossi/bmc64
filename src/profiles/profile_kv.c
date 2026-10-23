//
// profile_kv.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// The key=value format shared by all profile files (docs/PROFILES.md,
// Profile files): one entry per line, '#' starts a comment line, spaces
// around keys and values are ignored, and Windows line endings are accepted.

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
