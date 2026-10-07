#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// A profile's machine: the start of a machines.txt section header
// ("C64/PAL/HDMI", "C64"), where the missing parts match anything, or a
// whole header ("C64/PAL/HDMI/VICE 720p@50Hz") (docs/PROFILES.md, Profiles
// and machines).

// machine / video standard / video output / description
#define MACHINE_PARTS 4

typedef struct {
   int count;
   char part[MACHINE_PARTS][PROFILES_MAX_MACHINE_LEN + 1];
} MachineParts;

// Machine names as machines.txt spells them, and how menus show them.
static const struct {
   const char *section;
   const char *label;
} machine_labels[] = {
   {"VIC20", "VIC-20"},
   {"Plus4", "Plus/4"},
   {"Plus4Emu", "Plus/4"},
   {"Pet", "PET"},
};

static int lower(char c) {
   return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;
}

static int equal_ignoring_case(const char *a, const char *b) {
   while (*a && lower(*a) == lower(*b)) {
      a++;
      b++;
   }
   return *a == '\0' && *b == '\0';
}

// Splits a machine value at '/', trimming spaces. The description is the
// rest after the third '/', so it may hold '/' itself.
static void split(const char *value, MachineParts *parts) {
   memset(parts, 0, sizeof(*parts));
   if (value == NULL) return;
   const char *p = value;
   while (parts->count < MACHINE_PARTS) {
      const char *end = parts->count < MACHINE_PARTS - 1 ? strchr(p, '/') : NULL;
      size_t length = end ? (size_t)(end - p) : strlen(p);
      while (length > 0 && *p == ' ') {
         p++;
         length--;
      }
      while (length > 0 && p[length - 1] == ' ') length--;
      if (length > PROFILES_MAX_MACHINE_LEN) length = PROFILES_MAX_MACHINE_LEN;
      memcpy(parts->part[parts->count], p, length);
      parts->part[parts->count][length] = '\0';
      parts->count++;
      if (end == NULL) break;
      p = end + 1;
   }
   // "" is no value at all.
   if (parts->count == 1 && parts->part[0][0] == '\0') parts->count = 0;
}

void profiles_machine_label(const char *machine, char *out, int out_size) {
   if (out == 0 || out_size <= 0) return;
   out[0] = '\0';
   if (machine == 0) return;

   MachineParts parts;
   split(machine, &parts);
   const char *label = parts.count > 0 ? parts.part[0] : "";
   for (unsigned i = 0; i < sizeof(machine_labels) / sizeof(machine_labels[0]); i++) {
      if (equal_ignoring_case(label, machine_labels[i].section)) {
         label = machine_labels[i].label;
         break;
      }
   }
   snprintf(out, (size_t)out_size, "%s", label);
}

int profiles_machine_matches(const char *machine, const char *other) {
   MachineParts a;
   MachineParts b;
   split(machine, &a);
   split(other, &b);
   if (a.count == 0 || b.count == 0 || a.part[0][0] == '\0') {
      return 0;
   }
   int common = a.count < b.count ? a.count : b.count;
   for (int i = 0; i < common; i++) {
      if (!equal_ignoring_case(a.part[i], b.part[i])) return 0;
   }
   return 1;
}

int profiles_machine_covers(const char *machine, const char *header) {
   MachineParts want;
   MachineParts have;
   split(machine, &want);
   split(header, &have);
   if (want.count == 0 || want.part[0][0] == '\0' || want.count > have.count) {
      return 0;
   }
   for (int i = 0; i < want.count; i++) {
      if (!equal_ignoring_case(want.part[i], have.part[i])) return 0;
   }
   return 1;
}

void profiles_machine_desc(const char *machine, char *out, int out_size) {
   MachineParts parts;
   split(machine, &parts);
   out[0] = '\0';
   for (int i = 0; i < parts.count && i < 3; i++) {
      int len = (int)strlen(out);
      snprintf(out + len, (size_t)(out_size - len), "%s%s", i ? "/" : "",
               parts.part[i]);
   }
}

void profiles_machine_name(const char *machine, char *out, int out_size) {
   MachineParts parts;
   split(machine, &parts);
   snprintf(out, (size_t)out_size, "%s", parts.count > 0 ? parts.part[0] : "");
}
