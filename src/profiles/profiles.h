#ifndef BMC64_PROFILES_H
#define BMC64_PROFILES_H

// Profiles: complete, saved setups of BMC64 that can be switched to from the
// menu. User behaviour, file formats and the numbered rules ([P1], [S3], ...)
// are in docs/PROFILES.md.
//
// This is the core logic only (profile files, start-up, shared settings,
// machines). It doesn't depend on Circle or VICE, so it can be tested on the
// PC (tools/profiles_test). The on-screen menu is in
// third_party/common/menu_profiles.c.
//
// Nothing here runs per frame, and with no profiles nothing is read or
// written at boot beyond one attempt to open /profiles/active.txt
// (docs/PROFILES.md [F1]-[F6]).

#ifdef __cplusplus
extern "C" {
#endif

#define PROFILES_MAIN_ID "main"

#define PROFILES_MAX_ID_LEN 32
#define PROFILES_MAX_NAME_LEN 32
#define PROFILES_MAX_CATEGORY_LEN 32
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
   // machines.txt section header, e.g. "C64/PAL/HDMI/VICE 720p@50Hz".
   // Empty for Main, which runs on whatever machine booted.
   char machine[PROFILES_MAX_MACHINE_LEN + 1];
   ProfileStart start;
} ProfileInfo;

// ---- The running profile (profile_boot.c) ----

const ProfileInfo *profiles_running(void);
int profiles_running_is_main(void);

// ---- The profile list (profile_store.c) ----
// Only read when the menu asks for it, and freed again afterwards
// (docs/PROFILES.md [F3]). Main is always entry 0.

int profiles_list_open(void);
const ProfileInfo *profiles_list_at(int index);
void profiles_list_close(void);

// ---- Profile actions (profile_store.c) ----
// Switch to and Start once restart BMC64 when they succeed.

int profiles_switch_to(const char *id);
int profiles_start_once(const char *id);
int profiles_save_new(const char *name);
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

// ---- Shared settings (profile_system.c) ----

// 1 if a settings.txt key is a shared setting, kept in system.txt for all
// profiles rather than in each profile (docs/PROFILES.md, Shared settings).
int profiles_is_shared_setting(const char *key);

// ---- Machines (profile_machine.c) ----

// Short machine name for menus from a machines.txt section header,
// e.g. "VIC20/PAL/HDMI/..." gives "VIC-20". Empty for an empty header.
void profiles_machine_label(const char *machine, char *out, int out_size);

#ifdef __cplusplus
}
#endif

#endif
