#include "profiles_internal.h"

// Profile files on the SD card: /profiles/active.txt, the profile list,
// profile.txt, and creating, renaming and deleting profiles.
//
// Not implemented yet: Main is the only profile and the actions do nothing.

static const ProfileInfo main_info = {
   PROFILES_MAIN_ID, "Main", "", "", PROFILE_START_SWITCH
};

const ProfileInfo *profiles_main_info(void) {
   return &main_info;
}

int profiles_list_open(void) {
   return 1;
}

const ProfileInfo *profiles_list_at(int index) {
   return index == 0 ? &main_info : 0;
}

void profiles_list_close(void) {
}

int profiles_switch_to(const char *id) {
   (void)id;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_start_once(const char *id) {
   (void)id;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_save_new(const char *name) {
   (void)name;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_rename_running(const char *name) {
   (void)name;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_delete(const char *id) {
   (void)id;
   return PROFILES_NOT_IMPLEMENTED;
}
