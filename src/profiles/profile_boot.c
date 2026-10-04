#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// Start-up: which profile runs, and its autostart
// (docs/PROFILES.md, Switch to, or Start once; Autostart).
//
// Not implemented yet: startup disks.

static char booted_machine[16];
static ProfileFile running;
static int running_is_main = 1;
// active.txt was there at start-up.
static int in_use;
// Started with Start once.
static int running_once;
// The Start-once entry still has to be removed from active.txt.
static int once_to_remove;
// Main's startup actions (/profiles/main/<machine>.txt), when Main runs.
static ProfileFile main_extras;
static char boot_message[128];
// Kept for as long as the emulator runs: VICE holds on to the vice.ini path.
static char vice_config_path[PROFILES_MAX_PATH_LEN];
static char settings_path[PROFILES_MAX_PATH_LEN];

static void start_main(void) {
   running_is_main = 1;
   memset(&running, 0, sizeof(running));
   running.info = *profiles_main_info();
}

// Picks the profile active.txt asks for, or Main with a message.
static void start_from_active(const ActiveFile *a) {
   const char *id = a->profile;
   if (a->once[0]) {
      id = a->once;
      running_once = 1;
      once_to_remove = 1;
   } else if (a->bad_once[0]) {
      // A typo in once=: ignored, the usual profile starts.
      snprintf(boot_message, sizeof(boot_message),
               "once=%s in active.txt\nisn't a profile id.\nIgnored it.",
               a->bad_once);
   }
   if (!a->once[0] && a->bad_profile[0]) {
      // A typo in profile=: Main starts.
      snprintf(boot_message, sizeof(boot_message),
               "profile=%s in active.txt\nisn't a profile id.\n"
               "Started Main instead.", a->bad_profile);
   }
   if (strcmp(id, PROFILES_MAIN_ID) == 0) {
      return;
   }

   ProfileFile pf;
   if (profile_file_read(id, &pf) != PROFILES_OK) {
      snprintf(boot_message, sizeof(boot_message),
               "Can't read profile\n%s\nStarted Main instead.", id);
      return;
   }
   if (!profiles_machine_matches(pf.info.machine, booted_machine)) {
      snprintf(boot_message, sizeof(boot_message),
               "Profile %s\nis for another machine.\nStarted Main instead.",
               pf.info.name);
      return;
   }
   running = pf;
   running_is_main = 0;
}

void profiles_boot_init(const char *booted) {
   snprintf(booted_machine, sizeof(booted_machine), "%s", booted ? booted : "");
   start_main();
   memset(&main_extras, 0, sizeof(main_extras));
   running_once = 0;
   once_to_remove = 0;
   in_use = 0;
   boot_message[0] = '\0';

   // With no profiles this is the only file access.
   ActiveFile af;
   if (active_file_read(&af) != PROFILES_OK) {
      return;
   }
   in_use = 1;
   start_from_active(&af);
   if (running_is_main) {
      main_file_read(booted_machine, &main_extras);
   }
}

void profiles_after_boot(void) {
   if (!once_to_remove) {
      return;
   }
   once_to_remove = 0;
   ActiveFile af;
   if (active_file_read(&af) == PROFILES_OK && af.once[0]) {
      af.once[0] = '\0';
      active_file_write(&af);
   }
}

const char *profiles_boot_message(void) {
   return boot_message;
}

void profiles_before_reboot(void) {
   if (!running_once || running_is_main) {
      return;
   }
   ActiveFile af;
   active_file_read(&af);
   strcpy(af.once, running.info.id);
   active_file_write(&af);
}

void profiles_reset_to_main(void) {
   ActiveFile af;
   // Nothing to do without profiles; don't create /profiles.
   if (active_file_read(&af) != PROFILES_OK) {
      return;
   }
   if (strcmp(af.profile, PROFILES_MAIN_ID) != 0 || af.once[0]) {
      strcpy(af.profile, PROFILES_MAIN_ID);
      af.once[0] = '\0';
      active_file_write(&af);
   }
}

const char *profiles_booted_machine(void) {
   return booted_machine;
}

const ProfileInfo *profiles_running(void) {
   return &running.info;
}

int profiles_running_is_main(void) {
   return running_is_main;
}

int profiles_in_use(void) {
   return in_use;
}

void profiles_running_renamed(const char *name) {
   pkv_copy(running.info.name, sizeof(running.info.name), name);
}

const char *profiles_vice_config(void) {
   if (running_is_main) {
      return NULL;
   }
   profiles_path(running.info.id, "vice.ini", vice_config_path,
                 sizeof(vice_config_path));
   return vice_config_path;
}

const char *profiles_settings_file(const char *main_file) {
   if (running_is_main) {
      return main_file;
   }
   profiles_path(running.info.id, "settings.txt", settings_path,
                 sizeof(settings_path));
   return settings_path;
}

const char *profiles_autostart(void) {
   return running_is_main ? main_extras.autostart : running.autostart;
}

// Saves path ("" for none) as the running profile's autostart, straight away.
static int save_autostart(const char *path) {
   if (path == NULL || strlen(path) >= sizeof(running.autostart)) {
      return PROFILES_ERROR;
   }
   if (running_is_main) {
      ProfileFile extras = main_extras;
      pkv_copy(extras.autostart, sizeof(extras.autostart), path);
      if (main_file_write(booted_machine, &extras) != PROFILES_OK) {
         return PROFILES_ERROR;
      }
      main_extras = extras;
      // Start-up only reads Main's file when active.txt exists.
      ActiveFile af;
      if (active_file_read(&af) != PROFILES_OK &&
          active_file_write(&af) != PROFILES_OK) {
         return PROFILES_ERROR;
      }
      return PROFILES_OK;
   }
   // Re-read the file so changes made since start-up (a rename) are kept.
   ProfileFile pf;
   if (profile_file_read(running.info.id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   pkv_copy(pf.autostart, sizeof(pf.autostart), path);
   if (profile_file_write(running.info.id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   pkv_copy(running.autostart, sizeof(running.autostart), path);
   return PROFILES_OK;
}

int profiles_set_autostart(const char *path) {
   if (path == NULL || path[0] == '\0') {
      return PROFILES_ERROR;
   }
   return save_autostart(path);
}

int profiles_clear_autostart(void) {
   return save_autostart("");
}

int profiles_set_startup_disks(const char *const paths[PROFILES_NUM_DRIVES]) {
   (void)paths;
   return PROFILES_NOT_IMPLEMENTED;
}

int profiles_clear_startup_disks(void) {
   return PROFILES_NOT_IMPLEMENTED;
}
