/*
 * menu_profiles.c
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "menu_profiles.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

// RASPI includes
#include "circle.h"
#include "demo.h"
#include "menu.h"
#include "menu_switch.h"
#include "ui.h"
#include "../../src/profiles/profiles.h"

extern void reboot(void);

// Folder whose name shows the running profile's autostart.
static struct menu_item *autostart_folder;
// Items that show the running profile's name (NULL for Main).
static struct menu_item *status_item;
static struct menu_item *save_item;

static void show_result(int result) {
  if (result == PROFILES_NOT_IMPLEMENTED) {
    ui_info("Not implemented yet");
  } else if (result != PROFILES_OK) {
    ui_error("Profile action failed");
  }
}

// For actions that save straight away: confirm what was saved.
static void show_saved(int result, const char *message) {
  if (result == PROFILES_OK) {
    ui_info("%s", message);
  } else {
    show_result(result);
  }
}

static const char *base_name(const char *path) {
  const char *slash = strrchr(path, '/');
  return slash ? slash + 1 : path;
}

static void update_autostart_label(void) {
  if (autostart_folder == NULL) {
    return;
  }
  const char *autostart = profiles_autostart();
  snprintf(autostart_folder->name, sizeof(autostart_folder->name),
           "Set autostart: %s", autostart[0] ? base_name(autostart) : "None");
}

// "Name (Machine)" for the profile lists.
static void profile_label(const ProfileInfo *info, char *out, int out_size) {
  char machine[16];
  profiles_machine_label(info->machine, machine, sizeof(machine));
  if (machine[0]) {
    snprintf(out, out_size, "%s (%s)", info->name, machine);
  } else {
    snprintf(out, out_size, "%s", info->name);
  }
}

// Pushes a menu with a heading. Call ready() once its items are added.
static struct menu_item *push_menu(int w_chars, int h_chars,
                                   const char *title) {
  struct menu_item *root = ui_push_menu(w_chars, h_chars);
  ui_menu_add_read_only_heading(root, title);
  ui_menu_add_divider(root);
  return root;
}

// The cursor must start on a selectable item, not on the heading.
static void ready(void) {
  ui_select_first_interactive_item();
}

static void list_popped(struct menu_item *old_root,
                        struct menu_item *new_root) {
  profiles_list_close();
}

// Adds one button per profile to a list, grouped into folders by category.
// Main and profiles without a category come first. The button's str_value
// holds the profile id. Returns the number of buttons added.
static int add_profile_buttons(struct menu_item *root, int id,
                               int skip_main_and_running,
                               void (*on_chosen)(struct menu_item *)) {
  const char *running = profiles_running()->id;
  int count = profiles_list_open();
  int added = 0;
  for (int pass = 0; pass < count; pass++) {
    // Pass 0 adds profiles without a category; each later pass adds the
    // category of profile 'pass' if that profile is the first one in it.
    struct menu_item *parent = root;
    const char *category = "";
    if (pass > 0) {
      category = profiles_list_at(pass)->category;
      if (!category[0]) {
        continue;
      }
      int seen = 0;
      for (int i = 1; i < pass && !seen; i++) {
        seen = strcmp(profiles_list_at(i)->category, category) == 0;
      }
      if (seen) {
        continue;
      }
      parent = ui_menu_add_folder(root, (char *)category);
    }
    for (int i = 0; i < count; i++) {
      const ProfileInfo *info = profiles_list_at(i);
      if (strcmp(info->category, category) != 0) {
        continue;
      }
      int is_running = strcmp(info->id, running) == 0;
      if (skip_main_and_running &&
          (is_running || strcmp(info->id, PROFILES_MAIN_ID) == 0)) {
        continue;
      }
      struct menu_item *item = ui_menu_add_button(id, parent, "");
      profile_label(info, item->name, sizeof(item->name));
      snprintf(item->str_value, sizeof(item->str_value), "%s", info->id);
      if (is_running) {
        strcpy(item->displayed_value, "(*)");
      }
      item->on_value_changed = on_chosen;
      added++;
    }
  }
  return added;
}

// ---- Machines ----

static int booted_ntsc;
static BMC64VideoOut booted_out;

// What booted, in machines.txt terms, e.g. "C64/PAL/HDMI".
static void describe_booted(const char *machine, char *out, int out_size) {
  int timing = circle_get_machine_timing();
  booted_ntsc = timing == MACHINE_TIMING_NTSC_HDMI ||
                timing == MACHINE_TIMING_NTSC_COMPOSITE ||
                timing == MACHINE_TIMING_NTSC_CUSTOM_HDMI ||
                timing == MACHINE_TIMING_NTSC_DPI ||
                timing == MACHINE_TIMING_NTSC_CUSTOM_DPI;
  if (timing == MACHINE_TIMING_NTSC_COMPOSITE ||
      timing == MACHINE_TIMING_PAL_COMPOSITE) {
    booted_out = BMC64_VIDEO_OUT_COMPOSITE;
  } else if (timing >= MACHINE_TIMING_NTSC_DPI &&
             timing <= MACHINE_TIMING_NTSC_CUSTOM_DPI) {
    booted_out = BMC64_VIDEO_OUT_DPI;
  } else {
    booted_out = BMC64_VIDEO_OUT_HDMI;
  }
  snprintf(out, out_size, "%s/%s/%s", machine, booted_ntsc ? "NTSC" : "PAL",
           booted_out == BMC64_VIDEO_OUT_COMPOSITE ? "Composite"
           : booted_out == BMC64_VIDEO_OUT_DPI     ? "DPI"
                                                   : "HDMI");
}

// Applies the first machines.txt entry with every part of machine,
// preferring the video standard and output that booted. Returns 0, or -1 if
// there's no such entry or it can't be applied.
static int apply_machine(const char *machine) {
  struct machine_entry *head;
  load_machines(&head);
  struct machine_entry *best = NULL;
  int best_score = -1;
  for (struct machine_entry *ptr = head; ptr; ptr = ptr->next) {
    if (ptr->class == BMC64_MACHINE_CLASS_PLUS4EMU && circle_get_model() < 3) {
      continue;  // not offered on this Pi
    }
    if (!profiles_machine_covers(machine, ptr->header)) {
      continue;
    }
    int ntsc = ptr->video_standard == BMC64_VIDEO_STANDARD_NTSC;
    int score = (ntsc == booted_ntsc ? 2 : 0) + (ptr->video_out == booted_out);
    if (score > best_score) {
      best = ptr;
      best_score = score;
    }
  }
  int result = -1;
  if (best != NULL && switch_apply_files(best) == 0) {
    result = 0;
  }
  free_machines(head);
  return result;
}

void menu_profiles_boot(const char *machine) {
  char booted[64];
  describe_booted(machine, booted, sizeof(booted));
  profiles_boot_init(booted);
}

void menu_profiles_before_reboot(void) {
  profiles_before_reboot();
  // A Start-once profile on another machine: the power-on profile's machine
  // was put back after boot, so this restart needs the profile's again.
  if (profiles_return_machine()[0] != '\0') {
    apply_machine(profiles_running()->machine);
  }
}

void menu_profiles_machine_switched(struct machine_entry *entry) {
  profiles_machine_switched(entry->header);
}

// ---- Select profile ----

// Why the last menu_profiles_start() couldn't start a profile, or "".
static char start_error[64];

void menu_profiles_start(const char *id, int once) {
  start_error[0] = '\0';
  if (strcmp(id, profiles_running()->id) == 0) {
    snprintf(start_error, sizeof(start_error), "Already running");
  } else {
    const char *needed = profiles_machine_for(id);
    if (needed[0] != '\0' && apply_machine(needed) != 0) {
      snprintf(start_error, sizeof(start_error),
               "No machine for this profile in machines.txt");
    } else if ((once ? profiles_start_once(id) : profiles_switch_to(id)) !=
               PROFILES_OK) {
      snprintf(start_error, sizeof(start_error), "Can't start this profile");
    } else {
      reboot();
    }
  }
  ui_error("%s", start_error);
}

const char *menu_profiles_start_error(void) {
  return start_error;
}

void menu_profiles_clear_start_error(void) {
  start_error[0] = '\0';
}

static void start_chosen(struct menu_item *item) {
  menu_profiles_start(item->str_value, item->id == MENU_PROFILES_START_ONCE);
}

static void profile_selected(struct menu_item *item) {
  if (strcmp(item->str_value, profiles_running()->id) == 0) {
    ui_info("Already running");
    return;
  }
  ProfileStart start = PROFILE_START_SWITCH;
  for (int i = 0; profiles_list_at(i) != NULL; i++) {
    if (strcmp(profiles_list_at(i)->id, item->str_value) == 0) {
      start = profiles_list_at(i)->start;
      break;
    }
  }

  // The profile's "start" choice is offered first.
  static const int switch_first[] = {MENU_PROFILES_SWITCH_TO,
                                     MENU_PROFILES_START_ONCE};
  static const int once_first[] = {MENU_PROFILES_START_ONCE,
                                   MENU_PROFILES_SWITCH_TO};
  const int *order = start == PROFILE_START_ONCE ? once_first : switch_first;

  struct menu_item *root = push_menu(32, 4, item->name);
  for (int i = 0; i < 2; i++) {
    struct menu_item *button = ui_menu_add_button(
        order[i], root,
        order[i] == MENU_PROFILES_SWITCH_TO ? "Switch to" : "Start once");
    strcpy(button->str_value, item->str_value);
    button->on_value_changed = start_chosen;
  }
  ready();
}

static void show_select_list(void) {
  struct menu_item *root = push_menu(-1, -1, "Select profile");
  root->on_popped_off = list_popped;
  add_profile_buttons(root, MENU_PROFILES_SELECT_ITEM, 0, profile_selected);
  ready();
}

// ---- New profile / Rename ----

static void update_name_labels(void) {
  if (status_item != NULL) {
    if (profiles_running_is_main()) {
      snprintf(status_item->name, sizeof(status_item->name),
               "Profile: Main (no profile)");
    } else {
      snprintf(status_item->name, sizeof(status_item->name), "Profile: %s",
               profiles_running()->name);
    }
  }
  if (save_item != NULL) {
    snprintf(save_item->name, sizeof(save_item->name), "Save settings (%s)",
             profiles_running()->name);
  }
}

// Saves the current settings as a new profile and restarts into it.
static void create_profile(const char *name) {
  char id[PROFILES_MAX_ID_LEN + 1];
  if (profiles_create(name, id, sizeof(id)) != PROFILES_OK) {
    ui_error("Can't create the profile");
    return;
  }
  if (menu_save_settings_to_profile(id) != 0) {
    profiles_delete(id);
    ui_error("Can't save the new profile");
    return;
  }
  if (profiles_switch_to(id) != PROFILES_OK) {
    ui_error("Profile saved, but can't\nswitch to it");
    return;
  }
  reboot();
}

static void name_entered(struct menu_item *item) {
  if (item->str_value[0] == '\0') {
    return;
  }
  if (item->id == MENU_PROFILES_NEW_NAME) {
    create_profile(item->str_value);
  } else if (profiles_rename_running(item->str_value) == PROFILES_OK) {
    update_name_labels();
    ui_pop_menu();
    ui_info("Profile renamed");
  } else {
    ui_error("Can't rename the profile");
  }
}

static void show_name_dialog(int id, const char *title, const char *name) {
  struct menu_item *root = push_menu(32, 3, title);
  struct menu_item *field = ui_menu_add_text_field_limit(
      id, root, "", (char *)name, PROFILES_MAX_NAME_LEN);
  field->on_value_changed = name_entered;
  ready();
}

// ---- Delete ----

static void delete_confirmed(struct menu_item *item) {
  if (profiles_delete(item->str_value) != PROFILES_OK) {
    ui_error("Can't delete the profile.\nIs anything else in its\nfolder?");
    return;
  }
  // Close the confirmation and the list (item is freed by this).
  ui_pop_menu();
  ui_pop_menu();
  ui_info("Profile deleted");
}

static void delete_selected(struct menu_item *item) {
  struct menu_item *root = push_menu(32, 3, item->name);
  struct menu_item *confirm = ui_menu_add_button(
      MENU_PROFILES_DELETE_CONFIRM, root, "Delete this profile");
  strcpy(confirm->str_value, item->str_value);
  confirm->on_value_changed = delete_confirmed;
  ready();
}

static void show_delete_list(void) {
  struct menu_item *root = push_menu(-1, -1, "Delete profile");
  root->on_popped_off = list_popped;
  if (add_profile_buttons(root, MENU_PROFILES_DELETE_ITEM, 1,
                          delete_selected) == 0) {
    ui_menu_add_button(MENU_TEXT, root, "No profiles to delete");
  }
  ready();
}

// ---- Menu items ----

static void profiles_item_chosen(struct menu_item *item) {
  switch (item->id) {
  case MENU_PROFILES_SELECT:
    show_select_list();
    break;
  case MENU_PROFILES_NEW:
    show_name_dialog(MENU_PROFILES_NEW_NAME, "New profile name", "");
    break;
  case MENU_PROFILES_RENAME:
    if (profiles_running_is_main()) {
      ui_info("Main can't be renamed");
    } else {
      show_name_dialog(MENU_PROFILES_RENAME_NAME, "Rename profile",
                       profiles_running()->name);
    }
    break;
  case MENU_PROFILES_DELETE:
    show_delete_list();
    break;
  case MENU_PROFILES_AUTOSTART_FILE:
    menu_show_autostart_files(MENU_PROFILES_AUTOSTART_PICK);
    break;
  case MENU_PROFILES_CLEAR_AUTOSTART:
    show_saved(profiles_clear_autostart(), "Nothing will autostart");
    update_autostart_label();
    break;
  case MENU_PROFILES_AUTO_ATTACH_DISKS: {
    const char *paths[PROFILES_NUM_DRIVES];
    char message[64] = "No disks attached, so none will be attached at boot";
    int length = 0;
    for (int i = 0; i < PROFILES_NUM_DRIVES; i++) {
      paths[i] = attached_disk_name[i];
      if (paths[i][0]) {
        length += snprintf(message + length, sizeof(message) - length,
                           length ? ", %d" : "Disks %d", PROFILES_FIRST_DRIVE + i);
      }
    }
    if (length) {
      snprintf(message + length, sizeof(message) - length,
               " will be attached at boot");
    }
    show_saved(profiles_set_startup_disks(paths), message);
    break;
  }
  case MENU_PROFILES_CLEAR_AUTO_ATTACH:
    show_saved(profiles_clear_startup_disks(),
               "No disks will be attached at boot");
    break;
  default:
    break;
  }
}

static struct menu_item *add_item(int id, struct menu_item *parent,
                                  const char *name) {
  struct menu_item *item = ui_menu_add_button(id, parent, (char *)name);
  item->on_value_changed = profiles_item_chosen;
  return item;
}

void menu_profiles_add_status_line(struct menu_item *root) {
  // Shown once profiles are in use, also for Main; choosing it opens the
  // profile list.
  if (profiles_running_is_main() && !profiles_in_use()) {
    return;
  }
  status_item = add_item(MENU_PROFILES_SELECT, root, "");
  update_name_labels();
}

void build_profiles_menu(struct menu_item *root) {
  struct menu_item *parent = ui_menu_add_folder(root, "Profiles");
  add_item(MENU_PROFILES_SELECT, parent, "Select profile...");
  add_item(MENU_PROFILES_NEW, parent, "New profile from current settings");

  struct menu_item *manage = ui_menu_add_folder(parent, "Manage profile");
  add_item(MENU_PROFILES_RENAME, manage, "Rename profile...");
  add_item(MENU_PROFILES_DELETE, manage, "Delete profile...");

  autostart_folder = ui_menu_add_folder(parent, "Set autostart");
  add_item(MENU_PROFILES_AUTOSTART_FILE, autostart_folder,
           "Autostart Prg/Disk...");
  update_autostart_label();

  add_item(MENU_PROFILES_CLEAR_AUTOSTART, parent, "Clear autostart");
}

void build_profiles_drive_items(struct menu_item *drives) {
  struct menu_item *parent = ui_menu_add_folder(drives, "Auto-attach options");
  add_item(MENU_PROFILES_AUTO_ATTACH_DISKS, parent,
           "Auto-attach current disks at boot");
  add_item(MENU_PROFILES_CLEAR_AUTO_ATTACH, parent,
           "Clear auto-attached disks");
}

void menu_profiles_label_save_item(struct menu_item *item) {
  if (profiles_running_is_main()) {
    return;
  }
  save_item = item;
  update_name_labels();
}

void menu_profiles_autostart_chosen(const char *path) {
  ui_pop_menu();  // the file list
  int result = profiles_set_autostart(path);
  update_autostart_label();
  if (result == PROFILES_OK) {
    ui_info("Autostart set: %s", base_name(path));
  } else {
    ui_error("Can't set the autostart");
  }
}

// How long the boot autostart waits for the Pi's clock to be set from the
// network before starting anyway.
#define DELAYED_AUTOSTART_TIMEOUT_SECS 30

// The boot autostart while it waits for time sync, or "".
static char delayed_autostart[PENDING_EMU_AUTOSTART_MAX];
static unsigned long delayed_autostart_start;

// Whether the network is up or may still come up, so time sync may yet come.
static int network_coming_up(void) {
  switch (circle_get_network_status()) {
  case CIRCLE_NETWORK_ETHERNET_INITIALIZING:
  case CIRCLE_NETWORK_ETHERNET_WAITING_FOR_DHCP:
  case CIRCLE_NETWORK_ETHERNET_CONNECTED:
  case CIRCLE_NETWORK_WIFI_DEVICE_INITIALIZING:
  case CIRCLE_NETWORK_WIFI_WPA_INITIALIZING:
  case CIRCLE_NETWORK_WIFI_CONNECTING:
  case CIRCLE_NETWORK_WIFI_CONNECTED:
    return 1;
  default:
    return 0;
  }
}

static void start_boot_autostart(const char *path) {
  // Before the autostart, so with Warp after boot on, autostart sees warp
  // already on and leaves it on when it finishes.
  menu_apply_warp_after_boot();
  // Queued for the main loop rather than started here: VICE's autostart puts
  // back the warp state it started with when it finishes, so it must start
  // after the warp state above is in place.
  emu_autostart_interrupt(path);
}

void menu_profiles_boot_complete(void) {
  // A Start-once profile on another machine: the next power-on is back on
  // the power-on profile's machine.
  const char *back = profiles_return_machine();
  if (back[0] != '\0' && apply_machine(back) != 0) {
    ui_error("No machine for %s\nin machines.txt", back);
  }
  profiles_after_boot();
  const char *message = profiles_boot_message();
  if (message[0] != '\0') {
    ui_error("%s", message);
  }

  // The profile's startup disks, as if attached from the Drives menu.
  for (int i = 0; i < PROFILES_NUM_DRIVES && !raspi_demo_mode; i++) {
    const char *disk = profiles_startup_disk(i);
    if (disk[0] == '\0') {
      continue;
    }
    int unit = PROFILES_FIRST_DRIVE + i;
    struct stat st;
    if (stat(disk, &st) != 0) {
      ui_error("Can't find the disk for\ndrive %d: %s", unit, base_name(disk));
    } else if (emux_attach_disk_image(unit, (char *)disk) != 0) {
      ui_error("Can't attach the disk for\ndrive %d: %s", unit, base_name(disk));
    } else {
      snprintf(attached_disk_name[i], MAX_STR_VAL_LEN, "%s", disk);
    }
  }

  // The profile's autostart, as if picked from Autostart Prg/Disk.
  const char *autostart = profiles_autostart();
  if (autostart[0] != '\0' && !raspi_demo_mode) {
    struct stat st;
    if (stat(autostart, &st) != 0) {
      ui_error("Can't find the autostart\n%s", base_name(autostart));
    } else if (menu_delay_autostart_for_time_sync() &&
               !circle_network_time_synced() && network_coming_up()) {
      // Held until the Pi's clock is set, so software that reads the time
      // as it loads (GEOS, C64 OS via a CMD HD or IDE64) gets it right.
      snprintf(delayed_autostart, sizeof(delayed_autostart), "%s", autostart);
      delayed_autostart_start = circle_get_ticks();
      printf("Autostart waiting for time sync\n");
      return;
    } else {
      start_boot_autostart(autostart);
      return;
    }
  }
  menu_apply_warp_after_boot();
}

void menu_profiles_check_delayed_autostart(void) {
  if (delayed_autostart[0] == '\0') {
    return;
  }
  int synced = circle_network_time_synced();
  if (!synced && network_coming_up() &&
      circle_get_ticks() - delayed_autostart_start <
          DELAYED_AUTOSTART_TIMEOUT_SECS * 1000000UL) {
    return;
  }
  printf("Autostart %s\n", synced ? "after time sync" : "without time sync");
  char path[sizeof(delayed_autostart)];
  strcpy(path, delayed_autostart);
  delayed_autostart[0] = '\0';
  start_boot_autostart(path);
}

void menu_profiles_cancel_delayed_autostart(void) {
  delayed_autostart[0] = '\0';
}
