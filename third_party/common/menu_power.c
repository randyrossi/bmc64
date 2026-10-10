/*
 * menu_power.c
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#include "menu_power.h"

// RASPI includes
#include "circle.h"
#include "emux_api.h"
#include "menu.h"
#include "menu_profiles.h"
#include "ui.h"

// How long the shutdown message stays up before the display goes off.
#define POWER_OFF_MESSAGE_US (3 * 1000 * 1000)

// Detaching makes the emulator close each image file, which writes out
// anything it still has buffered; a plain restart can lose the last writes
// to a disk image.
static void write_out_images(void) {
  for (int unit = 8; unit <= 11; unit++) {
    emux_detach_disk(unit);
  }
  emux_detach_tape();
}

void menu_power_reboot(void) {
  write_out_images();
  menu_profiles_before_reboot();
  circle_reboot();
}

void menu_power_off(void) {
  write_out_images();
  // No menu_profiles_before_reboot(): the next start is a power-on, so a
  // Start-once profile should not start again.
  ui_info("Shutting down.\nIt is safe to remove power\nonce the screen goes blank.");
  circle_sleep(POWER_OFF_MESSAGE_US);
  circle_power_off();
}

static void reboot_chosen(struct menu_item *item) {
  (void)item;
  menu_power_reboot();
}

static void power_off_chosen(struct menu_item *item) {
  (void)item;
  menu_power_off();
}

// Cancel uses the menu's usual MENU_CONFIRM_CANCEL handling (pop).
static void confirm(const char *title, int id, const char *action,
                    void (*chosen)(struct menu_item *)) {
  struct menu_item *root = ui_push_menu(28, 4);
  ui_menu_add_read_only_heading(root, title);
  ui_menu_add_divider(root);
  ui_menu_add_button(id, root, action)->on_value_changed = chosen;
  ui_menu_add_button(MENU_CONFIRM_CANCEL, root, "Cancel");
  ui_select_first_interactive_item();
}

static void reboot_selected(struct menu_item *item) {
  (void)item;
  confirm("Reboot BMC64?", MENU_POWER_REBOOT, "Reboot now", reboot_chosen);
}

static void power_off_selected(struct menu_item *item) {
  (void)item;
  confirm("Shut down BMC64?", MENU_POWER_OFF, "Shut down now",
          power_off_chosen);
}

void build_power_menu(struct menu_item *root) {
  struct menu_item *parent = ui_menu_add_folder(root, "Power");
  ui_menu_add_button(MENU_SOFT_RESET, parent, "Soft Reset");
  ui_menu_add_button(MENU_HARD_RESET, parent, "Hard Reset");
  ui_menu_add_divider(parent);
  ui_menu_add_button(MENU_POWER_REBOOT, parent, "Reboot BMC64")
      ->on_value_changed = reboot_selected;
  ui_menu_add_button(MENU_POWER_OFF, parent, "Shut down")
      ->on_value_changed = power_off_selected;
  ui_menu_add_divider(parent);
}
