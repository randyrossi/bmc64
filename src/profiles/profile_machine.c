#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// A profile's machine: the machines.txt section it runs on, or just the
// machine (docs/PROFILES.md, Profiles and machines).
//
// Not implemented yet: switching machines for a profile.

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

// The machine part of a machine value: up to the first '/'.
static void machine_part(const char *machine, char *out, int out_size) {
   size_t length = strcspn(machine, "/");
   if (length > (size_t)out_size - 1) length = (size_t)out_size - 1;
   memcpy(out, machine, length);
   out[length] = '\0';
}

void profiles_machine_label(const char *machine, char *out, int out_size) {
   if (out == 0 || out_size <= 0) return;
   out[0] = '\0';
   if (machine == 0) return;

   char first[PROFILES_MAX_MACHINE_LEN + 1];
   machine_part(machine, first, sizeof(first));

   const char *label = first;
   for (unsigned i = 0; i < sizeof(machine_labels) / sizeof(machine_labels[0]); i++) {
      if (equal_ignoring_case(first, machine_labels[i].section)) {
         label = machine_labels[i].label;
         break;
      }
   }
   snprintf(out, (size_t)out_size, "%s", label);
}

int profiles_machine_matches(const char *machine, const char *booted_machine) {
   if (machine == 0 || booted_machine == 0 || booted_machine[0] == '\0') {
      return 0;
   }
   char first[PROFILES_MAX_MACHINE_LEN + 1];
   machine_part(machine, first, sizeof(first));
   return first[0] != '\0' && equal_ignoring_case(first, booted_machine);
}
