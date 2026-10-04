#include "profiles_internal.h"

// Start-up: which profile runs, its startup disks and autostart
// (docs/PROFILES.md, Switch to or Start once, Auto-attached disks, Autostart).
//
// Not implemented yet: Main always runs and nothing is read at boot.

const ProfileInfo *profiles_running(void) {
   return profiles_main_info();
}

int profiles_running_is_main(void) {
   return 1;
}

const char *profiles_autostart(void) {
   return "";
}

int profiles_set_autostart(const char *path) {
   (void)path;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_clear_autostart(void) {
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_set_startup_disks(const char *const paths[PROFILES_NUM_DRIVES]) {
   (void)paths;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_clear_startup_disks(void) {
   return PROFILES_NOT_IMPLEMENTED;
}
