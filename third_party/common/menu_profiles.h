/*
 * menu_profiles.h
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#ifndef RASPI_MENU_PROFILES_H
#define RASPI_MENU_PROFILES_H

#include "ui.h"

// The on-screen menu for profiles (docs/PROFILES.md). The profile logic is
// in src/profiles; this file only builds and handles the menu items.

// Adds a "Profile: <name>" line under the machine line once profiles are in
// use. Choosing it opens Select profile.
void menu_profiles_add_status_line(struct menu_item *root);

// Adds the "Profiles" folder.
void build_profiles_menu(struct menu_item *root);

// Adds the startup disk entries to the Drives menu.
void build_profiles_drive_items(struct menu_item *drives);

// Labels "Save settings" with the running profile, unless Main is running.
void menu_profiles_label_save_item(struct menu_item *item);

// A file was picked for the profile's autostart (path includes the volume).
void menu_profiles_autostart_chosen(const char *path);

struct machine_entry;

// Switch machine applied entry; the profiles follow it.
void menu_profiles_machine_switched(struct machine_entry *entry);

// Call before BMC64 restarts itself (not for a profile or machine choice).
void menu_profiles_before_reboot(void);

#ifdef __cplusplus
extern "C" {
#endif

// Called once, before the emulator reads its settings, with the machine
// that booted ("C64", "VIC20", "Plus4Emu", ...). Decides which profile runs.
void menu_profiles_boot(const char *machine);

// Called once the emulator is running: finishes start-up and shows any
// message about it.
void menu_profiles_boot_complete(void);

#ifdef __cplusplus
}
#endif

#endif
