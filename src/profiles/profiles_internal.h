#ifndef BMC64_PROFILES_INTERNAL_H
#define BMC64_PROFILES_INTERNAL_H

// Shared between the files in src/profiles and their tests only.

#include "profiles.h"

#define PROFILES_ACTIVE_FILE PROFILES_DIR "/active.txt"

// Profile files are small; this is the most that is read of one.
#define PROFILES_MAX_FILE_LEN 2048

// ---- key=value files (profile_kv.c) ----

typedef void (*pkv_cb)(void *ctx, const char *key, const char *value);

// Calls cb for each key=value line. Changes text.
void pkv_parse(char *text, pkv_cb cb, void *ctx);

// Appends "key=value\n" to buf (holding *len bytes). Nothing is written for
// an empty value. Returns -1 if it doesn't fit.
int pkv_append(char *buf, int size, int *len, const char *key,
               const char *value);

// Copies a value, cut to fit and kept on one line.
void pkv_copy(char *out, int out_size, const char *value);

// ---- profile.txt (profile_store.c) ----

typedef struct {
   ProfileInfo info;
   char description[PROFILES_MAX_DESCRIPTION_LEN + 1];
   char disks[PROFILES_NUM_DRIVES][PROFILES_MAX_PATH_LEN + 1];
   char autostart[PROFILES_MAX_PATH_LEN + 1];
} ProfileFile;

// Fills pf from profile.txt text (changes text). Returns 0 if the profile is
// valid: it has a name and a machine.
int profile_file_parse(char *text, ProfileFile *pf);

// Writes pf as profile.txt text. Returns the length, or -1 if it won't fit.
int profile_file_format(const ProfileFile *pf, char *buf, int size);

int profile_file_read(const char *id, ProfileFile *pf);
int profile_file_write(const char *id, const ProfileFile *pf);

// Main's own file, /profiles/main/<machine>.txt (e.g. c64.txt), with the
// same keys as profile.txt; only the autostart and startup disks are used.
// A missing file reads as empty.
int main_file_read(const char *booted_machine, ProfileFile *pf);
int main_file_write(const char *booted_machine, const ProfileFile *pf);

// ---- active.txt (profile_store.c) ----

typedef struct {
   char profile[PROFILES_MAX_ID_LEN + 1];
   char once[PROFILES_MAX_ID_LEN + 1];
   // The machine Main last ran on ("main_machine"): what booted, e.g.
   // "C64/PAL/HDMI", or the machines.txt entry chosen with Switch machine.
   char main_machine[PROFILES_MAX_MACHINE_LEN + 1];
   // Values that aren't profile ids (e.g. a typo), cut to fit; "" if none.
   char bad_profile[PROFILES_MAX_ID_LEN + 1];
   char bad_once[PROFILES_MAX_ID_LEN + 1];
} ActiveFile;

// Empties af: Main, nothing else set.
void active_file_clear(ActiveFile *af);

// Fills af from active.txt text (changes text). Missing or invalid ids
// become "main" (profile) and "" (once); invalid ones are also kept in
// bad_profile / bad_once so start-up can say so.
void active_file_parse(char *text, ActiveFile *af);

// Returns -1, with af set to Main, if there is no active.txt.
int active_file_read(ActiveFile *af);
int active_file_write(const ActiveFile *af);

// ---- Settings files (profile_store.c) ----

// A profile's BMC64 settings for a machine:
// /profiles/<id>/settings-<machine>.txt, e.g. settings-plus4.txt.
void profiles_settings_path(const char *id, const char *machine, char *out,
                            int out_size);

// ---- Ids (profile_store.c) ----

// 1 for a valid profile id: 1 to 32 lowercase letters, digits and '-'.
int profiles_id_valid(const char *id);

// Makes an unused id from a name (docs/PROFILES.md, Profile files).
int profiles_make_id(const char *name, char *out, int out_size);

// ---- Machines (profile_machine.c) ----

// The machine, standard and output of a machine value, e.g. "C64/PAL/HDMI"
// from "C64/PAL/HDMI/VICE 720p@50Hz".
void profiles_machine_desc(const char *machine, char *out, int out_size);

// The machine part, e.g. "C64" from "C64/PAL/HDMI".
void profiles_machine_name(const char *machine, char *out, int out_size);

// ---- State (profile_store.c / profile_boot.c) ----

const ProfileInfo *profiles_main_info(void);

// The booted machine's name, e.g. "C64" from "C64/PAL/HDMI".
const char *profiles_booted_machine(void);

// Before leaving Main for a profile: sets af's main_machine to what booted,
// if Main is running and hasn't remembered it.
void profiles_remember_main_machine(ActiveFile *af);

// Updates the running profile's name after a rename.
void profiles_running_renamed(const char *name);

#endif
