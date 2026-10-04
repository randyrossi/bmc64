#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// A profile's machine: the machines.txt section it runs on
// (docs/PROFILES.md, Profiles and machines).
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

void profiles_machine_label(const char *machine, char *out, int out_size) {
   if (out == 0 || out_size <= 0) return;
   out[0] = '\0';
   if (machine == 0) return;

   // The machine is the section header up to the first '/'.
   char first[PROFILES_MAX_MACHINE_LEN + 1];
   size_t length = strcspn(machine, "/");
   if (length > PROFILES_MAX_MACHINE_LEN) length = PROFILES_MAX_MACHINE_LEN;
   memcpy(first, machine, length);
   first[length] = '\0';

   const char *label = first;
   for (unsigned i = 0; i < sizeof(machine_labels) / sizeof(machine_labels[0]); i++) {
      if (strcmp(first, machine_labels[i].section) == 0) {
         label = machine_labels[i].label;
         break;
      }
   }
   snprintf(out, (size_t)out_size, "%s", label);
}
