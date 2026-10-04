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

// Adds a "Profile: <name>" line under the machine line, unless Main is running.
void menu_profiles_add_status_line(struct menu_item *root);

// Adds the "Profiles" folder.
void build_profiles_menu(struct menu_item *root);

// Adds the startup disk entries to the Drives menu.
void build_profiles_drive_items(struct menu_item *drives);

// Labels "Save settings" with the running profile, unless Main is running.
void menu_profiles_label_save_item(struct menu_item *item);

#ifdef __cplusplus
extern "C" {
#endif

// Called once the emulator is running: finishes start-up and shows any
// message about it.
void menu_profiles_boot_complete(void);

#ifdef __cplusplus
}
#endif

#endif
