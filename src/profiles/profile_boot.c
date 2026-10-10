#include "profiles_internal.h"

#include "../sdcard/sd_fs.h"

#include <stdio.h>
#include <string.h>

// Start-up: which profile runs, the machine to go back to, and the startup
// disks and autostart (docs/PROFILES.md, Switch to, or Start once; Profiles
// and machines; Auto-attach disks; Autostart).

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
// The running profile's settings are still in an older profile's
// settings.txt; it's renamed to settings-<machine>.txt after boot.
static int settings_legacy;
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
   settings_legacy = 0;
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
   } else {
      // An older profile keeps its settings in settings.txt, which is always
      // for the machine it runs on (machine switches rename it first).
      char own[PROFILES_MAX_PATH_LEN];
      char legacy[PROFILES_MAX_PATH_LEN];
      profiles_settings_path(running.info.id, booted_machine, own, sizeof(own));
      profiles_path(running.info.id, "settings.txt", legacy, sizeof(legacy));
      settings_legacy = sd_stat(own, NULL, NULL) != SD_OK &&
                        sd_stat(legacy, NULL, NULL) == SD_OK;
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

// Renames an older profile's settings.txt to settings-<machine>.txt.
static void rename_legacy_settings(const char *id, const char *machine) {
   char own[PROFILES_MAX_PATH_LEN];
   char legacy[PROFILES_MAX_PATH_LEN];
   profiles_settings_path(id, machine, own, sizeof(own));
   profiles_path(id, "settings.txt", legacy, sizeof(legacy));
   if (sd_stat(own, NULL, NULL) != SD_OK) {
      sd_rename(legacy, own);
   }
}

void profiles_after_boot(void) {
   if (settings_legacy) {
      rename_legacy_settings(running.info.id, booted_machine);
      settings_legacy = 0;
   }
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
   if (strcmp(af.profile, PROFILES_MAIN_ID) != 0) {
      ProfileFile pf;
      if (profile_file_read(af.profile, &pf) == PROFILES_OK) {
         // The profile follows, also to another machine: each machine has
         // its own settings file in it, so the old machine's are kept. An
         // older profile's settings.txt is for its old machine.
         rename_legacy_settings(af.profile, pf.info.machine);
         profiles_machine_desc(entry, pf.info.machine,
                               sizeof(pf.info.machine));
         profile_file_write(af.profile, &pf);
      } else {
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
   if (settings_legacy) {
      profiles_path(running.info.id, "settings.txt", settings_path,
                    sizeof(settings_path));
   } else {
      profiles_settings_path(running.info.id, booted_machine, settings_path,
                             sizeof(settings_path));
   }
   return settings_path;
}

void profiles_new_settings_file(const char *id, char *out, int out_size) {
   profiles_settings_path(id, booted_machine, out, out_size);
}

const char *profiles_autostart(void) {
   return running_is_main ? main_extras.autostart : running.autostart;
}

// The running profile's startup actions as they'll be saved.
typedef struct {
   const char *autostart;  // NULL: unchanged
   const char *disks[PROFILES_NUM_DRIVES];
   int set_disks;
} StartupChange;

static int change_fits(const StartupChange *c) {
   if (c->autostart && strlen(c->autostart) >= sizeof(running.autostart)) {
      return 0;
   }
   for (int i = 0; c->set_disks && i < PROFILES_NUM_DRIVES; i++) {
      if (c->disks[i] && strlen(c->disks[i]) >= sizeof(running.disks[i])) {
         return 0;
      }
   }
   return 1;
}

static void apply_change(ProfileFile *pf, const StartupChange *c) {
   if (c->autostart) {
      pkv_copy(pf->autostart, sizeof(pf->autostart), c->autostart);
   }
   for (int i = 0; c->set_disks && i < PROFILES_NUM_DRIVES; i++) {
      pkv_copy(pf->disks[i], sizeof(pf->disks[i]),
               c->disks[i] ? c->disks[i] : "");
   }
}

// Saves the running profile's autostart and/or startup disks straight away.
static int save_startup(const StartupChange *c) {
   if (!change_fits(c)) {
      return PROFILES_ERROR;
   }
   if (running_is_main) {
      ProfileFile extras = main_extras;
      apply_change(&extras, c);
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
   apply_change(&pf, c);
   if (profile_file_write(running.info.id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   apply_change(&running, c);
   return PROFILES_OK;
}

int profiles_set_autostart(const char *path) {
   if (path == NULL || path[0] == '\0') {
      return PROFILES_ERROR;
   }
   StartupChange c = {path, {NULL}, 0};
   return save_startup(&c);
}

int profiles_clear_autostart(void) {
   StartupChange c = {"", {NULL}, 0};
   return save_startup(&c);
}

const char *profiles_startup_disk(int drive) {
   if (drive < 0 || drive >= PROFILES_NUM_DRIVES) {
      return "";
   }
   return running_is_main ? main_extras.disks[drive] : running.disks[drive];
}

int profiles_set_startup_disks(const char *const paths[PROFILES_NUM_DRIVES]) {
   StartupChange c = {NULL, {NULL}, 1};
   for (int i = 0; paths && i < PROFILES_NUM_DRIVES; i++) {
      c.disks[i] = paths[i];
   }
   return save_startup(&c);
}

int profiles_clear_startup_disks(void) {
   StartupChange c = {NULL, {NULL}, 1};
   return save_startup(&c);
}
