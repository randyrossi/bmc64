#include "profiles_internal.h"

#include <stdio.h>
#include <string.h>

// Start-up: which profile runs, the machine to go back to, and the
// autostart (docs/PROFILES.md, Switch to, or Start once; Profiles and
// machines; Autostart).
//
// Not implemented yet: startup disks.

// What booted ("C64/PAL/HDMI") and its machine name ("C64").
static char booted_desc[64];
static char booted_machine[16];
static ProfileFile running;
static int running_is_main = 1;
// active.txt was there at start-up.
static int in_use;
// Started with Start once.
static int running_once;
// The Start-once entry still has to be removed from active.txt.
static int once_to_remove;
// Main's last machine, from active.txt.
static char main_machine[PROFILES_MAX_MACHINE_LEN + 1];
// The power-on profile's machine, after a Start-once profile on another one.
static char return_machine[PROFILES_MAX_MACHINE_LEN + 1];
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

// The machine a profile runs on; Main's is the one it last ran on.
static void machine_of(const char *id, const char *main_value, char *out,
                       int out_size) {
   out[0] = '\0';
   if (strcmp(id, PROFILES_MAIN_ID) == 0) {
      pkv_copy(out, out_size, main_value);
      return;
   }
   ProfileFile pf;
   if (profile_file_read(id, &pf) == PROFILES_OK) {
      pkv_copy(out, out_size, pf.info.machine);
   }
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
   if (!profiles_machine_matches(pf.info.machine, booted_desc)) {
      snprintf(boot_message, sizeof(boot_message),
               "Profile %s\nis for another machine.\nStarted Main instead.",
               pf.info.name);
      return;
   }
   running = pf;
   running_is_main = 0;
}

void profiles_boot_init(const char *booted) {
   snprintf(booted_desc, sizeof(booted_desc), "%s", booted ? booted : "");
   profiles_machine_name(booted_desc, booted_machine, sizeof(booted_machine));
   start_main();
   memset(&main_extras, 0, sizeof(main_extras));
   main_machine[0] = '\0';
   return_machine[0] = '\0';
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
   strcpy(main_machine, af.main_machine);
   start_from_active(&af);
   if (running_is_main) {
      main_file_read(booted_machine, &main_extras);
   }
   // Start once on another machine: the next power-on goes back to the
   // power-on profile's machine.
   if (running_once && !running_is_main) {
      machine_of(af.profile, main_machine, return_machine,
                 sizeof(return_machine));
      if (return_machine[0] == '\0' ||
          profiles_machine_matches(return_machine, booted_desc)) {
         return_machine[0] = '\0';
      }
   }
}

const char *profiles_return_machine(void) {
   return return_machine;
}

void profiles_after_boot(void) {
   // Main remembers the machine it last ran on.
   int remember = in_use && running_is_main &&
                  !profiles_machine_matches(main_machine, booted_desc);
   if (!once_to_remove && !remember) {
      return;
   }
   once_to_remove = 0;
   ActiveFile af;
   if (active_file_read(&af) == PROFILES_OK) {
      af.once[0] = '\0';
      if (remember) {
         strcpy(af.main_machine, booted_desc);
         strcpy(main_machine, booted_desc);
      }
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
   // Main's machine is forgotten too: safe mode applies its own.
   if (strcmp(af.profile, PROFILES_MAIN_ID) != 0 || af.once[0] ||
       af.main_machine[0]) {
      active_file_clear(&af);
      active_file_write(&af);
   }
}

const char *profiles_booted(void) {
   return booted_desc;
}

const char *profiles_booted_machine(void) {
   return booted_machine;
}

const char *profiles_machine_for(const char *id) {
   static char machine[PROFILES_MAX_MACHINE_LEN + 1];
   if (id == NULL) {
      return "";
   }
   machine_of(id, main_machine, machine, sizeof(machine));
   if (machine[0] == '\0' || profiles_machine_matches(machine, booted_desc)) {
      return "";
   }
   return machine;
}

void profiles_remember_main_machine(ActiveFile *af) {
   // Main may not have been remembered yet (no active.txt at boot).
   if (running_is_main &&
       !profiles_machine_matches(af->main_machine, booted_desc)) {
      strcpy(af->main_machine, booted_desc);
   }
}

void profiles_machine_switched(const char *entry) {
   ActiveFile af;
   if (active_file_read(&af) != PROFILES_OK) {
      return;
   }
   char now[16];
   profiles_machine_name(entry, now, sizeof(now));
   if (strcmp(af.profile, PROFILES_MAIN_ID) != 0) {
      ProfileFile pf;
      char have[16];
      int same = profile_file_read(af.profile, &pf) == PROFILES_OK;
      if (same) {
         profiles_machine_name(pf.info.machine, have, sizeof(have));
         same = profiles_machine_matches(have, now);
      }
      if (same) {
         // Same machine, another standard or output: the profile follows.
         profiles_machine_desc(entry, pf.info.machine,
                               sizeof(pf.info.machine));
         profile_file_write(af.profile, &pf);
      } else {
         // Its settings are for the old machine: Main from now on.
         strcpy(af.profile, PROFILES_MAIN_ID);
      }
   }
   if (strcmp(af.profile, PROFILES_MAIN_ID) == 0) {
      pkv_copy(af.main_machine, sizeof(af.main_machine), entry);
   }
   af.once[0] = '\0';
   active_file_write(&af);
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
