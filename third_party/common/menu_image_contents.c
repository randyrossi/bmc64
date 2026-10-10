/*
 * menu_image_contents.c
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 */

#include "menu_image_contents.h"

#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "ui.h"

// A D81 holds up to 296 files; tapes are rarely longer. Lines past this
// are not shown.
#define MAX_IMAGE_LINES 320

// The image the open listing came from.
static char image_path[MAX_STR_VAL_LEN];

static void program_chosen(struct menu_item *item) {
  ui_info("Starting...");
  if (emux_autostart_image_file(image_path, item->value) < 0) {
    ui_pop_menu();
    ui_error("Failed to autostart file");
    return;
  }
  ui_pop_all_and_toggle();
}

// Left closes the listing, back to the file list.
static void close_listing(struct menu_item *item) {
  (void)item;
  ui_pop_menu();
}

static void add_line(struct menu_item *root,
                     const struct emux_image_line *line) {
  struct menu_item *item;
  if (line->program > 0) {
    item = ui_menu_add_button(MENU_IMAGE_CONTENTS_FILE, root, "");
    item->value = line->program;
    item->on_value_changed = program_chosen;
    item->on_back = close_listing;
  } else {
    // The header, blocks free and files that can't be started: shown,
    // but the cursor skips them.
    item = ui_menu_add_read_only_heading(root, "");
    item->disabled = 1;
  }
  int length = line->length < MAX_MENU_STR ? line->length : MAX_MENU_STR;
  memcpy(item->name, line->text, length);
  item->raw_text_len = length;
}

void menu_image_contents_show(const char *path) {
  struct emux_image_line *lines =
      malloc(MAX_IMAGE_LINES * sizeof(struct emux_image_line));
  if (lines == NULL) {
    ui_error("Out of memory");
    return;
  }

  // A long .tap takes a moment to scan.
  ui_info("Reading...");
  int count = emux_read_image_contents(path, lines, MAX_IMAGE_LINES);
  ui_pop_menu();
  if (count <= 0) {
    free(lines);
    ui_error("Can't read this image");
    return;
  }

  strncpy(image_path, path, sizeof(image_path) - 1);
  image_path[sizeof(image_path) - 1] = '\0';

  struct menu_item *root = ui_push_menu(-1, -1);
  int programs = 0;
  for (int i = 0; i < count; i++) {
    add_line(root, &lines[i]);
    if (lines[i].program > 0) {
      programs++;
    }
  }
  free(lines);

  if (programs == 0) {
    ui_menu_add_divider(root);
    ui_menu_add_read_only_heading(root, "No programs to start")->disabled = 1;
  }
  ui_select_first_interactive_item();
}
