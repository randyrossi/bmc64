#include "profiles_internal.h"
#include "../sdcard/sd_fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Profile files on the SD card: /profiles/active.txt, profile.txt, the
// profile list, and creating, renaming and deleting profiles
// (docs/PROFILES.md, Profile files).

// Most profiles listed; more are ignored.
#define PROFILES_MAX_LISTED 256

static const ProfileInfo main_info = {
   PROFILES_MAIN_ID, "Main", "", "", PROFILE_START_SWITCH
};

const ProfileInfo *profiles_main_info(void) {
   return &main_info;
}

void profiles_path(const char *id, const char *file, char *out, int out_size) {
   snprintf(out, (size_t)out_size, "%s/%s/%s", PROFILES_DIR, id, file);
}

// ---- Ids ----

int profiles_id_valid(const char *id) {
   if (id == NULL || id[0] == '\0' || strlen(id) > PROFILES_MAX_ID_LEN) {
      return 0;
   }
   for (const char *c = id; *c; c++) {
      if (!((*c >= 'a' && *c <= 'z') || (*c >= '0' && *c <= '9') ||
            *c == '-')) {
         return 0;
      }
   }
   return 1;
}

static int id_used(const char *id) {
   char path[PROFILES_MAX_PATH_LEN];
   snprintf(path, sizeof(path), "%s/%s", PROFILES_DIR, id);
   return strcmp(id, PROFILES_MAIN_ID) == 0 || sd_stat(path, NULL, NULL) == 0;
}

int profiles_make_id(const char *name, char *out, int out_size) {
   char base[PROFILES_MAX_ID_LEN + 1];
   int length = 0;
   for (const char *c = name; *c && length < PROFILES_MAX_ID_LEN; c++) {
      char ch = *c;
      if (ch >= 'A' && ch <= 'Z') ch = ch - 'A' + 'a';
      if ((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9')) {
         base[length++] = ch;
      } else if (length > 0 && base[length - 1] != '-') {
         base[length++] = '-';
      }
   }
   while (length > 0 && base[length - 1] == '-') length--;
   base[length] = '\0';
   if (length == 0) {
      strcpy(base, "profile");
   }

   for (int number = 1; number < 1000; number++) {
      char candidate[PROFILES_MAX_ID_LEN + 1];
      if (number == 1) {
         snprintf(candidate, sizeof(candidate), "%s", base);
      } else {
         char suffix[8];
         int suffix_length = snprintf(suffix, sizeof(suffix), "-%d", number);
         int keep = (int)strlen(base);
         if (keep > PROFILES_MAX_ID_LEN - suffix_length) {
            keep = PROFILES_MAX_ID_LEN - suffix_length;
         }
         while (keep > 0 && base[keep - 1] == '-') keep--;
         snprintf(candidate, sizeof(candidate), "%.*s%s", keep, base, suffix);
      }
      if (!id_used(candidate)) {
         if ((int)strlen(candidate) >= out_size) return PROFILES_ERROR;
         strcpy(out, candidate);
         return PROFILES_OK;
      }
   }
   return PROFILES_ERROR;
}

// ---- profile.txt ----

static void profile_key(void *ctx, const char *key, const char *value) {
   ProfileFile *pf = (ProfileFile *)ctx;
   if (strcmp(key, "name") == 0) {
      pkv_copy(pf->info.name, sizeof(pf->info.name), value);
   } else if (strcmp(key, "machine") == 0) {
      pkv_copy(pf->info.machine, sizeof(pf->info.machine), value);
   } else if (strcmp(key, "category") == 0) {
      pkv_copy(pf->info.category, sizeof(pf->info.category), value);
   } else if (strcmp(key, "description") == 0) {
      pkv_copy(pf->description, sizeof(pf->description), value);
   } else if (strcmp(key, "start") == 0) {
      pf->info.start =
          strcmp(value, "once") == 0 ? PROFILE_START_ONCE : PROFILE_START_SWITCH;
   } else if (strcmp(key, "autostart") == 0) {
      pkv_copy(pf->autostart, sizeof(pf->autostart), value);
   } else if (strncmp(key, "disk_", 5) == 0) {
      int drive = atoi(key + 5);
      char expected[16];
      snprintf(expected, sizeof(expected), "disk_%d", drive);
      if (strcmp(key, expected) == 0 && drive >= PROFILES_FIRST_DRIVE &&
          drive < PROFILES_FIRST_DRIVE + PROFILES_NUM_DRIVES) {
         pkv_copy(pf->disks[drive - PROFILES_FIRST_DRIVE],
                  sizeof(pf->disks[0]), value);
      }
   }
   // Other keys are ignored.
}

int profile_file_parse(char *text, ProfileFile *pf) {
   memset(pf, 0, sizeof(*pf));
   pf->info.start = PROFILE_START_SWITCH;
   pkv_parse(text, profile_key, pf);
   return (pf->info.name[0] && pf->info.machine[0]) ? PROFILES_OK
                                                    : PROFILES_ERROR;
}

int profile_file_format(const ProfileFile *pf, char *buf, int size) {
   int len = 0;
   buf[0] = '\0';
   int rc = pkv_append(buf, size, &len, "name", pf->info.name);
   rc |= pkv_append(buf, size, &len, "description", pf->description);
   rc |= pkv_append(buf, size, &len, "category", pf->info.category);
   rc |= pkv_append(buf, size, &len, "machine", pf->info.machine);
   if (pf->info.start == PROFILE_START_ONCE) {
      rc |= pkv_append(buf, size, &len, "start", "once");
   }
   for (int i = 0; i < PROFILES_NUM_DRIVES; i++) {
      char key[16];
      snprintf(key, sizeof(key), "disk_%d", PROFILES_FIRST_DRIVE + i);
      rc |= pkv_append(buf, size, &len, key, pf->disks[i]);
   }
   rc |= pkv_append(buf, size, &len, "autostart", pf->autostart);
   return rc == 0 ? len : PROFILES_ERROR;
}

int profile_file_read(const char *id, ProfileFile *pf) {
   char path[PROFILES_MAX_PATH_LEN];
   char text[PROFILES_MAX_FILE_LEN];
   if (!profiles_id_valid(id) || strcmp(id, PROFILES_MAIN_ID) == 0) {
      return PROFILES_ERROR;
   }
   profiles_path(id, "profile.txt", path, sizeof(path));
   if (sd_read_file(path, text, sizeof(text)) < 0 ||
       profile_file_parse(text, pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   snprintf(pf->info.id, sizeof(pf->info.id), "%s", id);
   return PROFILES_OK;
}

int profile_file_write(const char *id, const ProfileFile *pf) {
   char path[PROFILES_MAX_PATH_LEN];
   char text[PROFILES_MAX_FILE_LEN];
   int len = profile_file_format(pf, text, sizeof(text));
   if (len < 0) {
      return PROFILES_ERROR;
   }
   profiles_path(id, "profile.txt", path, sizeof(path));
   return sd_write_file(path, text, len) == 0 ? PROFILES_OK : PROFILES_ERROR;
}

// ---- active.txt ----

static void active_key(void *ctx, const char *key, const char *value) {
   ActiveFile *af = (ActiveFile *)ctx;
   if (!profiles_id_valid(value)) {
      return;
   }
   if (strcmp(key, "profile") == 0) {
      strcpy(af->profile, value);
   } else if (strcmp(key, "once") == 0) {
      strcpy(af->once, value);
   }
}

void active_file_parse(char *text, ActiveFile *af) {
   strcpy(af->profile, PROFILES_MAIN_ID);
   af->once[0] = '\0';
   pkv_parse(text, active_key, af);
}

int active_file_read(ActiveFile *af) {
   char text[PROFILES_MAX_FILE_LEN];
   if (sd_read_file(PROFILES_ACTIVE_FILE, text, sizeof(text)) < 0) {
      strcpy(af->profile, PROFILES_MAIN_ID);
      af->once[0] = '\0';
      return PROFILES_ERROR;
   }
   active_file_parse(text, af);
   return PROFILES_OK;
}

int active_file_write(const ActiveFile *af) {
   char text[128];
   int len = 0;
   text[0] = '\0';
   if (pkv_append(text, sizeof(text), &len, "profile", af->profile) != 0 ||
       pkv_append(text, sizeof(text), &len, "once", af->once) != 0 ||
       sd_mkdir(PROFILES_DIR) != 0) {
      return PROFILES_ERROR;
   }
   // Write a new file, then swap it in, so a power cut never leaves a
   // half-written active.txt (a missing one just means Main).
   const char *temp = PROFILES_DIR "/active.new";
   if (sd_write_file(temp, text, len) != 0) {
      return PROFILES_ERROR;
   }
   sd_unlink(PROFILES_ACTIVE_FILE);
   return sd_rename(temp, PROFILES_ACTIVE_FILE) == 0 ? PROFILES_OK
                                                     : PROFILES_ERROR;
}

// ---- The profile list ----

static ProfileInfo *list;
static int list_count;

static int compare_names(const void *a, const void *b) {
   const char *x = ((const ProfileInfo *)a)->name;
   const char *y = ((const ProfileInfo *)b)->name;
   for (;; x++, y++) {
      int cx = (*x >= 'A' && *x <= 'Z') ? *x - 'A' + 'a' : *x;
      int cy = (*y >= 'A' && *y <= 'Z') ? *y - 'A' + 'a' : *y;
      if (cx != cy || cx == 0) return cx - cy;
   }
}

static int add_listed(void *ctx, const char *name, int is_dir) {
   int *capacity = (int *)ctx;
   ProfileFile pf;
   if (!is_dir || strcmp(name, PROFILES_MAIN_ID) == 0 ||
       profile_file_read(name, &pf) != PROFILES_OK) {
      return 0;
   }
   if (list_count == *capacity) {
      if (*capacity >= PROFILES_MAX_LISTED) return 1;
      int bigger = *capacity * 2;
      ProfileInfo *grown = realloc(list, sizeof(ProfileInfo) * (size_t)bigger);
      if (grown == NULL) return 1;
      list = grown;
      *capacity = bigger;
   }
   list[list_count++] = pf.info;
   return 0;
}

int profiles_list_open(void) {
   profiles_list_close();
   int capacity = 16;
   list = malloc(sizeof(ProfileInfo) * (size_t)capacity);
   if (list == NULL) {
      // Main is always there, even without memory for the list.
      return 1;
   }
   list[0] = main_info;
   list_count = 1;
   sd_list(PROFILES_DIR, add_listed, &capacity);
   qsort(list + 1, (size_t)(list_count - 1), sizeof(ProfileInfo),
         compare_names);
   return list_count;
}

const ProfileInfo *profiles_list_at(int index) {
   if (list == NULL) {
      return index == 0 ? &main_info : NULL;
   }
   return index >= 0 && index < list_count ? &list[index] : NULL;
}

void profiles_list_close(void) {
   free(list);
   list = NULL;
   list_count = 0;
}

// ---- Actions ----

// A profile that can be started: Main, or a valid profile for this machine.
static int check_startable(const char *id) {
   if (id != NULL && strcmp(id, PROFILES_MAIN_ID) == 0) {
      return PROFILES_OK;
   }
   ProfileFile pf;
   if (profile_file_read(id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   if (!profiles_machine_matches(pf.info.machine, profiles_booted_machine())) {
      return PROFILES_NOT_IMPLEMENTED;
   }
   return PROFILES_OK;
}

int profiles_switch_to(const char *id) {
   int rc = check_startable(id);
   if (rc != PROFILES_OK) {
      return rc;
   }
   ActiveFile af;
   strcpy(af.profile, id);
   af.once[0] = '\0';
   return active_file_write(&af);
}

int profiles_start_once(const char *id) {
   int rc = check_startable(id);
   if (rc != PROFILES_OK) {
      return rc;
   }
   ActiveFile af;
   active_file_read(&af);
   strcpy(af.once, id);
   return active_file_write(&af);
}

static void clean_name(const char *name, char *out, int out_size) {
   while (*name == ' ') name++;
   pkv_copy(out, out_size, name);
   int length = (int)strlen(out);
   while (length > 0 && out[length - 1] == ' ') out[--length] = '\0';
}

int profiles_create(const char *name, char *id_out, int id_size) {
   ProfileFile pf;
   memset(&pf, 0, sizeof(pf));
   clean_name(name, pf.info.name, sizeof(pf.info.name));
   if (pf.info.name[0] == '\0') {
      return PROFILES_ERROR;
   }
   // Same machine as the running profile; Main's is the booted machine.
   const ProfileInfo *running = profiles_running();
   pkv_copy(pf.info.machine, sizeof(pf.info.machine),
            running->machine[0] ? running->machine : profiles_booted_machine());
   if (pf.info.machine[0] == '\0') {
      return PROFILES_ERROR;
   }

   char id[PROFILES_MAX_ID_LEN + 1];
   char folder[PROFILES_MAX_PATH_LEN];
   if (profiles_make_id(pf.info.name, id, sizeof(id)) != PROFILES_OK ||
       (int)strlen(id) >= id_size || sd_mkdir(PROFILES_DIR) != 0) {
      return PROFILES_ERROR;
   }
   snprintf(folder, sizeof(folder), "%s/%s", PROFILES_DIR, id);
   if (sd_mkdir(folder) != 0) {
      return PROFILES_ERROR;
   }
   if (profile_file_write(id, &pf) != PROFILES_OK) {
      sd_unlink(folder);
      return PROFILES_ERROR;
   }
   strcpy(id_out, id);
   return PROFILES_OK;
}

int profiles_rename_running(const char *name) {
   const ProfileInfo *running = profiles_running();
   ProfileFile pf;
   char clean[PROFILES_MAX_NAME_LEN + 1];
   clean_name(name, clean, sizeof(clean));
   if (profiles_running_is_main() || clean[0] == '\0' ||
       profile_file_read(running->id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   strcpy(pf.info.name, clean);
   if (profile_file_write(running->id, &pf) != PROFILES_OK) {
      return PROFILES_ERROR;
   }
   profiles_running_renamed(clean);
   return PROFILES_OK;
}

// The files a profile's folder can hold. VICE leaves vice.in~ behind if a
// save of vice.ini is interrupted; the web UI's editor keeps the previous
// version of a file it saves as <name>.bak.
static const char *const profile_files[] = {
   "profile.txt", "vice.ini", "vice.in~", "settings.txt",
   "profile.txt.bak", "vice.ini.bak", "settings.txt.bak",
};

int profiles_delete(const char *id) {
   if (!profiles_id_valid(id) || strcmp(id, PROFILES_MAIN_ID) == 0 ||
       strcmp(id, profiles_running()->id) == 0) {
      return PROFILES_ERROR;
   }
   char path[PROFILES_MAX_PATH_LEN];
   for (unsigned i = 0; i < sizeof(profile_files) / sizeof(profile_files[0]);
        i++) {
      profiles_path(id, profile_files[i], path, sizeof(path));
      sd_unlink(path);
   }
   // Only removes the folder if nothing else is in it: a user's own files
   // in there are never deleted.
   snprintf(path, sizeof(path), "%s/%s", PROFILES_DIR, id);
   return sd_unlink(path) == 0 ? PROFILES_OK : PROFILES_ERROR;
}
