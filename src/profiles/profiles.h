#ifndef BMC64_PROFILES_H
#define BMC64_PROFILES_H

// Profiles: complete, saved setups of BMC64 that can be switched to from the
// menu. How they work for users, and the file formats, are in
// docs/PROFILES.md.
//
// This is the core logic only (profile files, start-up, machines). It doesn't depend on Circle or VICE, so it can be tested on the
// PC (tools/profiles_test). File access goes through src/sdcard/sd_fs.h. The
// on-screen menu is in third_party/common/menu_profiles.c.
//
// Nothing here runs per frame, and with no profiles nothing is read or
// written at boot beyond one attempt to open /profiles/active.txt. Nothing
// is written to the card during boot.

#ifdef __cplusplus
extern "C" {
#endif

#define PROFILES_MAIN_ID "main"
#define PROFILES_DIR "/profiles"

#define PROFILES_MAX_ID_LEN 32
#define PROFILES_MAX_NAME_LEN 32
#define PROFILES_MAX_CATEGORY_LEN 32
#define PROFILES_MAX_DESCRIPTION_LEN 64
#define PROFILES_MAX_MACHINE_LEN 64
#define PROFILES_MAX_PATH_LEN 256

// Startup disks are for drives 8 to 11.
#define PROFILES_FIRST_DRIVE 8
#define PROFILES_NUM_DRIVES 4

typedef enum {
   PROFILES_OK = 0,
   PROFILES_ERROR = -1,
   PROFILES_NOT_IMPLEMENTED = -2,
} ProfilesResult;

// Which choice the profile list offers first ("start" in profile.txt).
typedef enum {
   PROFILE_START_SWITCH = 0,
   PROFILE_START_ONCE,
} ProfileStart;

typedef struct {
   char id[PROFILES_MAX_ID_LEN + 1];
   char name[PROFILES_MAX_NAME_LEN + 1];
   char category[PROFILES_MAX_CATEGORY_LEN + 1];
   // A machines.txt section header, e.g. "C64/PAL/HDMI/VICE 720p@50Hz", or
   // just the machine, e.g. "C64". Empty for Main, which runs on whatever
   // machine booted.
   char machine[PROFILES_MAX_MACHINE_LEN + 1];
   ProfileStart start;
} ProfileInfo;

// ---- Start-up (profile_boot.c) ----

// Called once, before the emulator reads its settings. booted_machine is the
// machine this kernel emulates, as machines.txt spells it ("C64", "C128",
// "VIC20", "Plus4", "Plus4Emu", "Pet"). Decides which profile runs; any
// problem falls back to Main with a message (profiles_boot_message).
void profiles_boot_init(const char *booted_machine);

// Called once the emulator is running ("boot complete"). Does the work that
// mustn't slow boot down, such as removing a used Start-once entry.
void profiles_after_boot(void);

// A message for the user about how start-up went, or "".
const char *profiles_boot_message(void);

// Call before BMC64 restarts itself (e.g. a settings change that needs a
// restart), so a Start-once profile starts again.
void profiles_before_reboot(void);

// Safe mode: Main at the next power-on.
void profiles_reset_to_main(void);

// ---- The running profile (profile_boot.c) ----

const ProfileInfo *profiles_running(void);
int profiles_running_is_main(void);

// 1 if profiles are in use: /profiles/active.txt was there at start-up.
// Known from start-up, so it costs nothing to ask.
int profiles_in_use(void);

// The VICE settings file for the running profile, or NULL for Main (VICE's
// usual vice.ini).
const char *profiles_vice_config(void);

// The BMC64 settings file for the running profile. For Main it's main_file,
// the machine's usual file (e.g. "/settings-vic20.txt").
const char *profiles_settings_file(const char *main_file);

// The path of a file in a profile's folder, e.g. ("geos", "vice.ini").
void profiles_path(const char *id, const char *file, char *out, int out_size);

// ---- The profile list (profile_store.c) ----
// Only read when the menu asks for it, and freed again afterwards, so it
// costs nothing at boot. Main is always entry 0; the rest are sorted by
// name. Profiles for every machine are listed.

int profiles_list_open(void);
const ProfileInfo *profiles_list_at(int index);
void profiles_list_close(void);

// ---- Profile actions (profile_store.c) ----

// Switch to and Start once only update /profiles/active.txt; the caller then
// restarts BMC64. Profiles for another machine aren't supported yet.
int profiles_switch_to(const char *id);
int profiles_start_once(const char *id);

// Creates a profile for the running machine with only its profile.txt and
// returns its id. The caller then saves the settings into it.
int profiles_create(const char *name, char *id_out, int id_size);

int profiles_rename_running(const char *name);
int profiles_delete(const char *id);

// ---- Startup disks and autostart of the running profile (profile_boot.c) ----
// Saved to the profile straight away; nothing else is saved with them.
// For Main they go in /profiles/main/<machine>.txt.

// The autostart file or snapshot, or "" when there is none.
const char *profiles_autostart(void);
int profiles_set_autostart(const char *path);
int profiles_clear_autostart(void);

// paths[0] is drive 8; empty or NULL entries mean no disk.
int profiles_set_startup_disks(const char *const paths[PROFILES_NUM_DRIVES]);
int profiles_clear_startup_disks(void);

// ---- Machines (profile_machine.c) ----

// Short machine name for menus from a machine value,
// e.g. "VIC20/PAL/HDMI/..." gives "VIC-20". Empty for an empty value.
void profiles_machine_label(const char *machine, char *out, int out_size);

// 1 if a profile's machine value is for the booted machine
// ("C64/PAL/..." and "C64" are both for "C64"). Case doesn't matter.
int profiles_machine_matches(const char *machine, const char *booted_machine);

#ifdef __cplusplus
}
#endif

#endif
