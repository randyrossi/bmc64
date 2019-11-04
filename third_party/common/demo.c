/*
 * demo.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */
#include "demo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "circle.h"
#include "emux_api.h"
#include "joy.h"
#include "menu.h"

int raspi_demo_mode = 0;

static int demo_script_loaded = 0;
static struct demo_entry *head = NULL;
static struct demo_entry *tail = NULL;
static struct demo_entry *curr = NULL;

static unsigned long entry_delay = 0;
static unsigned long entry_start = 0;

static int cartridge_attached = 0;
static int disk_attached = 0;
static int demo_port_1_device;
static int demo_port_2_device;

#define TICKS_PER_SECOND 1000000L
#define TEXT_LINE_LEN 80

// TODO: This feature needs to be documented and more user friendly
// (i.e. display errors when things go wrong)

void emu_set_demo_mode(int is_demo) {
  raspi_demo_mode = is_demo;
}

// Trap to perform the operations
static void pause_trap(uint16_t addr, void *data) {
  // Move to next entry
  if (curr == NULL) {
    curr = head;
  } else {
    curr = curr->next;
    if (curr == NULL) {
      curr = head;
    }
  }

  if (!curr)
    return;

  entry_start = circle_get_ticks();
  entry_delay = curr->timeout * TICKS_PER_SECOND;

  // Perform operation
  if (curr->operation == OP_RESET) {
    printf("Demo Reset\n");
    emux_reset(0 /* hard */);
  } else if (curr->operation == OP_DISKSWAP) {
    printf("Demo Disk swap %s\n", curr->file);
    if (emux_attach_disk_image(8, curr->file) < 0) {
      printf("ERROR: can't load demo disk.\n");
      raspi_demo_mode = 0;
      return;
    }
  } else if (curr->operation == OP_SNAPSHOT) {
    printf("Demo Snapshot %s\n", curr->file);
    if (emux_load_state(curr->file) < 0) {
      printf("ERROR: can't load demo snapshot.\n");
      raspi_demo_mode = 0;
      return;
    }
  } else if (curr->operation == OP_CARTRIDGE) {
    printf("Demo Cartridge %s\n", curr->file);
    if (emux_attach_cart(0, curr->file) < 0) {
      printf("ERROR: can't load demo cartridge.\n");
      raspi_demo_mode = 0;
      return;
    }
  } else if (curr->operation == OP_DETACH_DISK) {
    printf("Demo Detach Disk \n");
    emux_detach_disk(8);
  } else if (curr->operation == OP_DETACH_CART) {
    printf("Demo Detach Cartridge\n");
    emux_detach_cart(0);
  } else {
    printf("ERROR: Unknown demo operation %d.\n", curr->operation);
    raspi_demo_mode = 0;
    return;
  }

  if (curr->joyswap) {
    // Reversed from when demo script was loaded.
    joydevs[0].device = demo_port_2_device;
    joydevs[1].device = demo_port_1_device;
  } else {
    // As it was when demo script was loaded.
    joydevs[0].device = demo_port_1_device;
    joydevs[1].device = demo_port_2_device;
  }
}

// Load the demo script.
// TODO: This is never attempted again and never
// un-loaded even if it failed.
static void demo_load_script() {
  printf("Loading demo script\n");
  FILE *fp = fopen("demo.txt", "r");
  char line[TEXT_LINE_LEN];
  if (fp != NULL) {
    while (fgets(line, TEXT_LINE_LEN - 1, fp)) {
      if (feof(fp))
        break;
      if (strlen(line) == 0)
        continue;
      if (line[0] == '#')
        continue;

      char *operation = strtok(line, ",");
      if (operation == NULL)
        continue;

      char *file = strtok(NULL, ",");
      if (file == NULL)
        continue;

      char *timeout = strtok(NULL, ",");
      if (timeout == NULL)
        continue;

      char *mode = strtok(NULL, ",");
      if (mode == NULL)
        continue;

      struct demo_entry *new_entry =
          (struct demo_entry *)malloc(sizeof(struct demo_entry));
      memset(new_entry, 0, sizeof(struct demo_entry));

      char *joyswap = strtok(NULL, ",");
      if (joyswap != NULL) {
        if (joyswap[strlen(joyswap) - 1] == '\n') {
          joyswap[strlen(joyswap) - 1] = '\0';
        }
        new_entry->joyswap = atoi(joyswap);
      }

      if (strcmp(operation, "r") == 0) {
        new_entry->operation = OP_RESET;
      } else if (strcmp(operation, "d") == 0) {
        new_entry->operation = OP_DISKSWAP;
      } else if (strcmp(operation, "s") == 0) {
        new_entry->operation = OP_SNAPSHOT;
      } else if (strcmp(operation, "c") == 0) {
        new_entry->operation = OP_CARTRIDGE;
      } else if (strcmp(operation, "dc") == 0) {
        new_entry->operation = OP_DETACH_CART;
      } else if (strcmp(operation, "dd") == 0) {
        new_entry->operation = OP_DETACH_DISK;
      } else {
        printf("ERROR: Unknown demo operation %s.\n", operation);
        raspi_demo_mode = 0;
      }
      strcpy(new_entry->file, file);
      new_entry->timeout = atoi(timeout);
      if (strcmp(mode, "i") == 0) {
        new_entry->mode = MODE_INTERRUPTABLE;
      } else {
        new_entry->mode = MODE_UNINTERRUPTABLE;
      }

      if (head == NULL) {
        head = new_entry;
        tail = new_entry;
      } else {
        tail->next = new_entry;
        tail = new_entry;
      }
    }
    fclose(fp);
    printf("Demo script loaded\n");

    // Make note of which device is assigned to which port
    // so we know what is 'normal' and what is 'swapped'
    demo_port_1_device = joydevs[0].device;
    demo_port_2_device = joydevs[1].device;

    curr = NULL;
  }
  demo_script_loaded = 1;
}

// Is it time to do the next thing?
void demo_check(void) {
  if (!demo_script_loaded) {
    demo_load_script();
  }

  // Rollover safe way of checking duration
  if (circle_get_ticks() - entry_start >= entry_delay) {
    // Time to do the next thing
    printf("Next demo entry\n");
    emux_trap_main_loop(pause_trap, 0);
  }
}

// Used to reset the next demo timeout on an
// interruptable demo entry
void demo_reset_timeout(void) {
  if (raspi_demo_mode && curr && curr->mode == MODE_INTERRUPTABLE) {
    entry_start = circle_get_ticks();
  }
}

// Start over from the beginning of the script
void demo_reset(void) {
  if (!raspi_demo_mode) {
    return;
  }

  if (!demo_script_loaded) {
    demo_load_script();
  }

  curr = NULL;
  entry_start = circle_get_ticks();
  entry_delay = 1000;
}
