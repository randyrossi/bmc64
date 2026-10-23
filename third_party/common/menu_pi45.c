//
// menu_pi45.c
//
// BMC64 - A bare metal Commodore emulator for Raspberry Pi
// Copyright (C) 2026 aminch <https://github.com/aminch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// See menu_pi45.h.
//
// Sources:
//  - BMC64, third_party/common/menu.c and ui.c: how menu items are made,
//    saved and loaded, and the CRT shader items; menu_switch.c and .h: the
//    machines.txt entries.
//  - CTA-861: 1280x720 is VIC 4 at 60 Hz and 19 at 50 Hz, 1920x1080 is
//    VIC 16 at 60 Hz and 31 at 50 Hz.
//

#include "menu_pi45.h"

#if RASPPI >= 4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "menu_switch.h"
#include "ui.h"

static struct machine_option *Option(struct machine_entry *entry, const char *key) {
  for (struct machine_option *opt = entry->options; opt != NULL; opt = opt->next) {
    if (strcmp(opt->key, key) == 0) {
      return opt;
    }
  }
  return NULL;
}

static int OptionInt(struct machine_entry *entry, const char *key) {
  struct machine_option *opt = Option(entry, key);
  return opt != NULL ? atoi(opt->value) : -1;
}

static void FreeEntry(struct machine_entry *entry) {
  struct machine_option *opt = entry->options;
  while (opt != NULL) {
    struct machine_option *next = opt->next;
    free(opt);
    opt = next;
  }
  free(entry);
}

static int Offered(struct machine_entry *entry) {
  if (entry->video_out != BMC64_VIDEO_OUT_HDMI) {
    return 0;
  }
#if RASPPI >= 5
  // The Pi 5's firmware doesn't set custom modes (hdmi_timings).
  if (OptionInt(entry, "hdmi_group") != 1) {
    return 0;
  }
#endif
  return 1;
}

// The 1080p CEA mode of a 720p one, at the same frame rate, or 0.
static int Mode1080(int mode) {
  switch (mode) {
  case 4: return 16;
  case 19: return 31;
  default: return 0;
  }
}

static int HasEntry(struct machine_entry *head, struct machine_entry *like, int mode) {
  for (struct machine_entry *e = head; e != NULL; e = e->next) {
    if (e->class == like->class && e->video_standard == like->video_standard &&
        e->video_out == BMC64_VIDEO_OUT_HDMI && OptionInt(e, "hdmi_group") == 1 &&
        OptionInt(e, "hdmi_mode") == mode) {
      return 1;
    }
  }
  return 0;
}

// Replaces the first FROM in TEXT (of SIZE bytes) with TO; 0 if there is
// none, or no room.
static int Replace(char *text, size_t size, const char *from, const char *to) {
  char *at = strstr(text, from);
  if (at == NULL || strlen(text) - strlen(from) + strlen(to) >= size) {
    return 0;
  }
  char rest[HEADER_LEN];
  snprintf(rest, sizeof rest, "%s", at + strlen(from));
  snprintf(at, size - (size_t)(at - text), "%s%s", to, rest);
  return 1;
}

// Scaling that isn't a whole-number scale (the C128's VDC) is a fit to the
// screen's size, so it is scaled with it; whole-number scales are kept, and
// made as large as fits the screen (src/pi45/board.cpp).
static void ScaleParams(struct machine_option *opt, int from_lines, int to_lines) {
  int fbw, fbh, sx, sy;
  if (opt == NULL || sscanf(opt->value, "%d,%d,%d,%d", &fbw, &fbh, &sx, &sy) != 4 ||
      fbw <= 0 || fbh <= 0 || sx <= 0 || sy <= 0 || (sx % fbw == 0 && sy % fbh == 0)) {
    return;
  }
  sx = (sx * to_lines / from_lines) & ~1;
  sy = (sy * to_lines / from_lines) & ~1;
  snprintf(opt->value, VALUE_LEN, "%d,%d,%d,%d", fbw, fbh, sx, sy);
}

// ENTRY (720p) at 1080p, CEA mode MODE, or NULL.
static struct machine_entry *Copy(struct machine_entry *entry, int mode) {
  const int from = 720, to = 1080;
  const char *from_name = "720p", *to_name = "1080p";
  struct machine_entry *copy = (struct machine_entry *)malloc(sizeof *copy);
  if (copy == NULL) {
    return NULL;
  }
  *copy = *entry;
  copy->options = NULL;
  copy->next = NULL;
  if (!Replace(copy->desc, sizeof copy->desc, from_name, to_name) ||
      !Replace(copy->header, sizeof copy->header, from_name, to_name)) {
    free(copy);
    return NULL;
  }
  struct machine_option **tail = &copy->options;
  for (struct machine_option *opt = entry->options; opt != NULL; opt = opt->next) {
    struct machine_option *o = (struct machine_option *)malloc(sizeof *o);
    if (o == NULL) {
      FreeEntry(copy);
      return NULL;
    }
    *o = *opt;
    o->next = NULL;
    if (strcmp(o->key, "hdmi_mode") == 0) {
      snprintf(o->value, VALUE_LEN, "%d", mode);
    } else if (strcmp(o->key, "scaling_params") == 0 ||
               strcmp(o->key, "scaling_params2") == 0) {
      ScaleParams(o, from, to);
    }
    *tail = o;
    tail = &o->next;
  }
  return copy;
}

void menu_pi45_machines(struct machine_entry **head) {
  struct machine_entry **link = head;
  while (*link != NULL) {
    struct machine_entry *entry = *link;
    if (!Offered(entry)) {
      *link = entry->next;
      FreeEntry(entry);
      continue;
    }
    int mode = OptionInt(entry, "hdmi_mode");
    int mode1080 = OptionInt(entry, "hdmi_group") == 1 ? Mode1080(mode) : 0;
    struct machine_entry *copy = NULL;
    if (mode1080 != 0 && !HasEntry(*head, entry, mode1080)) {
      copy = Copy(entry, mode1080);
    }
    if (copy == NULL) {
      link = &entry->next;
    } else {
      // 720p, then 1080p.
      copy->next = entry->next;
      entry->next = copy;
      link = &copy->next;
    }
  }
  int id = 0;
  for (struct machine_entry *e = *head; e != NULL; e = e->next) {
    e->id = id++;
  }
}

void menu_pi45_trim_shader(struct menu_item *shader) {
  // crt-pi with its default look: only Enable, Curvature and Reset.
  struct menu_item **link = &shader->first_child;
  while (*link != NULL) {
    int id = (*link)->id;
    if (id >= MENU_SHADER_CURVATURE_X && id <= MENU_SHADER_SHARPER) {
      *link = (*link)->next;
    } else {
      link = &(*link)->next;
    }
  }
}

int menu_pi45_load(const char *name, int value) {
  (void)value;
  return strcmp(name, "pi45_resolution") == 0;
}

#endif
