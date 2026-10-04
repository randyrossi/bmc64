#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// Start-up: which profile runs, its startup disks and autostart
// (docs/PROFILES.md, Switch to or Start once, Auto-attached disks, Autostart).
//
// Not implemented yet: startup disks and autostart.

static char booted_machine[16];
static ProfileFile running;
static int running_is_main = 1;
// Started with Start once.
static int running_once;
// The Start-once entry still has to be removed from active.txt.
static int once_to_remove;
static char boot_message[128];
// Kept for as long as the emulator runs: VICE holds on to the vice.ini path.
static char vice_config_path[PROFILES_MAX_PATH_LEN];
static char settings_path[PROFILES_MAX_PATH_LEN];

static void start_main(void) {
   running_is_main = 1;
   memset(&running, 0, sizeof(running));
   running.info = *profiles_main_info();
}

void profiles_boot_init(const char *booted) {
   snprintf(booted_machine, sizeof(booted_machine), "%s", booted ? booted : "");
   start_main();
   running_once = 0;
   once_to_remove = 0;
   boot_message[0] = '\0';

   // With no profiles this is the only file access ([F1]).
   ActiveFile af;
   if (active_file_read(&af) != PROFILES_OK) {
      return;
   }
   const char *id = af.profile;
   if (af.once[0]) {
      id = af.once;
      running_once = 1;
      once_to_remove = 1;
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
